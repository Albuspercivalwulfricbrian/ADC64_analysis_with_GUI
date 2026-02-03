#include "DischargeFitter.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

DischargeFitter::DischargeFitter(int gate_beg, int gate_end)
    : fGateBeg(gate_beg), fGateEnd(gate_end),
      fTauCMin(0.5f), fTauCMax(50.0f),
      fTauEMin(5.0f), fTauEMax(500.0f),
      fAmplitude(0.0f), fTauC(5.0f), fTauE(50.0f)
{
}

void DischargeFitter::SetWaveform(const std::vector<float> &wfm, float zero_level)
{
    fWfm = wfm;
    fZeroLevel = zero_level;
    fFitWfm.resize(wfm.size(), 0.0f);
}

void DischargeFitter::SetTauBounds(float tau_c_min, float tau_c_max,
                                   float tau_e_min, float tau_e_max)
{
    fTauCMin = tau_c_min;
    fTauCMax = tau_c_max;
    fTauEMin = tau_e_min;
    fTauEMax = tau_e_max;
}

// Raw Discharge function with A as prefactor
float DischargeFitter::DischargeValueRaw(float t, float A_prefactor, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;
    float exp_tc = exp(-t / tau_c);
    float exp_te = exp(-t / tau_e);
    return A_prefactor * (1.0f - (1.0f + t / tau_c) * exp_tc) * exp_te;
}

// Find peak value of raw function (prefactor A=1)
float DischargeFitter::FindPeakValue(float tau_c, float tau_e)
{
    auto key = std::make_pair(tau_c, tau_e);

    // Check cache first
    if (fPeakCache.find(key) != fPeakCache.end())
    {
        return fPeakCache[key];
    }

    // Numerically find peak
    float max_val = 0.0f;
    float max_t = 0.0f;

    // Search up to 5*tau_c (covers most of the pulse)
    for (float t = 0; t <= 5.0f * tau_c; t += 0.1f)
    {
        float val = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
        if (val > max_val)
        {
            max_val = val;
            max_t = t;
        }
    }

    // Refine around max_t
    for (int refine = 0; refine < 3; refine++)
    {
        float step = 0.02f * tau_c / (1 << refine);
        for (float t = max_t - step; t <= max_t + step; t += step / 10.0f)
        {
            if (t < 0)
                continue;
            float val = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
            if (val > max_val)
            {
                max_val = val;
                max_t = t;
            }
        }
    }

    fPeakCache[key] = max_val;
    return max_val;
}

// Discharge function with A as PEAK amplitude (what you want)
float DischargeFitter::DischargeValue(float t, float A_peak, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;

    // Get raw value with prefactor=1
    float raw_value = DischargeValueRaw(t, 1.0f, tau_c, tau_e);

    // Get peak of raw function
    float peak_raw = FindPeakValue(tau_c, tau_e);

    // Scale to desired peak amplitude
    if (peak_raw > 1e-6f)
    {
        return A_peak * (raw_value / peak_raw);
    }

    return 0.0f;
}

void DischargeFitter::CalculateInitialGuesses()
{
    if (fWfm.empty() || fSignalBegin >= fGateEnd)
        return;

    // Find data peak
    float data_peak = 0.0f;
    int peak_idx = fSignalBegin;

    for (int i = fSignalBegin; i <= fGateEnd && i < (int)fWfm.size(); i++)
    {
        if (fWfm[i] > data_peak)
        {
            data_peak = fWfm[i];
            peak_idx = i;
        }
    }

    fAmplitude = data_peak; // Peak amplitude

    // Estimate τ_e from decay after peak
    float decay_target = data_peak * 0.368f; // 1/e
    fTauE = fTauEMax;                        // Default to max

    for (int i = peak_idx + 1; i <= fGateEnd && i < (int)fWfm.size(); i++)
    {
        if (fWfm[i] <= decay_target)
        {
            fTauE = (i - peak_idx) * 1.0f;
            break;
        }
    }
    fTauE = std::clamp(fTauE, fTauEMin, fTauEMax);

    // Estimate τ_c from rise time (20%-80% to avoid noise)
    float t20 = -1.0f, t80 = -1.0f;
    for (int i = fSignalBegin; i < peak_idx; i++)
    {
        if (fWfm[i] >= 0.2f * data_peak && t20 < 0)
            t20 = i - fSignalBegin;
        if (fWfm[i] >= 0.8f * data_peak && t80 < 0)
            t80 = i - fSignalBegin;
    }

    if (t20 > 0 && t80 > t20)
    {
        fTauC = (t80 - t20) * 0.8f; // Empirical relation
    }
    else
    {
        fTauC = 5.0f; // Default
    }
    fTauC = std::clamp(fTauC, fTauCMin, fTauCMax);
}

void DischargeFitter::Fit()
{
    if (fWfm.empty() || fSignalBegin >= fGateEnd)
        return;

    CalculateInitialGuesses();

    float A = fAmplitude;
    float tau_c = fTauC;
    float tau_e = fTauE;

    float best_chi2 = std::numeric_limits<float>::max();
    float best_A = A, best_tau_c = tau_c, best_tau_e = tau_e;

    int num_fit_points = 0;
    for (int i = fSignalBegin; i <= fGateEnd; i += 2)
    {
        num_fit_points++;
    }

    // Degrees of freedom = points - parameters (3)
    float dof = std::max(1.0f, (float)num_fit_points - 3.0f);

    // Simple gradient descent with limited iterations
    for (int iter = 0; iter < 25; iter++)
    {
        float chi2 = 0.0f;
        float grad_A = 0.0f, grad_tau_c = 0.0f, grad_tau_e = 0.0f;

        // Use every other point for speed
        for (int i = fSignalBegin; i <= fGateEnd; i += 2)
        {
            float t = i - fSignalBegin;
            float model = DischargeValue(t, A, tau_c, tau_e);
            float error = model - fWfm[i];

            chi2 += error * error;

            // Numerical gradients
            float eps = 1e-3f;
            float dA = (DischargeValue(t, A + eps, tau_c, tau_e) - model) / eps;
            float dtau_c = (DischargeValue(t, A, tau_c + eps, tau_e) - model) / eps;
            float dtau_e = (DischargeValue(t, A, tau_c, tau_e + eps) - model) / eps;

            grad_A += 2.0f * error * dA;
            grad_tau_c += 2.0f * error * dtau_c;
            grad_tau_e += 2.0f * error * dtau_e;
        }

        // Normalize chi2 by degrees of freedom
        chi2 /= dof;

        // Keep best fit
        if (chi2 < best_chi2)
        {
            best_chi2 = chi2;
            best_A = A;
            best_tau_c = tau_c;
            best_tau_e = tau_e;
        }

        // Adaptive step size
        float step = 0.2f / (1.0f + iter * 0.1f);

        A -= step * grad_A;
        tau_c -= step * grad_tau_c;
        tau_e -= step * grad_tau_e;

        // Apply bounds
        A = std::max(0.1f, std::min(A, 10000.0f));
        tau_c = std::clamp(tau_c, fTauCMin, fTauCMax);
        tau_e = std::clamp(tau_e, fTauEMin, fTauEMax);

        // Early convergence check
        if (iter > 8 && fabs(chi2 - best_chi2) < 0.01f * best_chi2)
        {
            break;
        }
    }

    fAmplitude = best_A;
    fTauC = best_tau_c;
    fTauE = best_tau_e;

    // Generate fitted waveform
    for (int i = 0; i < (int)fFitWfm.size(); i++)
    {
        if (i >= fSignalBegin)
        {
            float t = i - fSignalBegin;
            fFitWfm[i] = DischargeValue(t, fAmplitude, fTauC, fTauE);
        }
        else
        {
            fFitWfm[i] = 0.0f;
        }
    }

    // Calculate final normalized chi2
    float ss_res = 0.0f, ss_tot = 0.0f;
    float mean = 0.0f;
    int count = 0;

    // Count ALL points in signal region for statistics
    for (int i = fSignalBegin; i <= fGateEnd; i++)
    {
        mean += fWfm[i];
        count++;
    }
    if (count > 0)
        mean /= count;

    // Degrees of freedom for final calculation
    float final_dof = std::max(1.0f, (float)count - 3.0f);

    for (int i = fSignalBegin; i <= fGateEnd; i++)
    {
        float error = fFitWfm[i] - fWfm[i];
        ss_res += error * error;
        ss_tot += (fWfm[i] - mean) * (fWfm[i] - mean);
    }

    // NORMALIZED chi2 (per degree of freedom)
    if (count > 0 && final_dof > 0)
    {
        fChi2 = ss_res / final_dof; // Normalized!
    }

    if (ss_tot > 1e-10f)
    {
        fR2 = 1.0f - ss_res / ss_tot;
    }
}

float DischargeFitter::GetFitValue(int sample)
{
    if (sample >= 0 && sample < (int)fFitWfm.size())
    {
        return fFitWfm[sample];
    }
    return 0.0f;
}

float DischargeFitter::GetFitValue(float x)
{
    int sample = (int)x;
    if (sample >= 0 && sample < (int)fFitWfm.size())
    {
        if (sample >= fSignalBegin)
        {
            float t = x - fSignalBegin;
            return DischargeValue(t, fAmplitude, fTauC, fTauE);
        }
    }
    return 0.0f;
}

float DischargeFitter::GetIntegral(int gate_beg, int gate_end)
{
    float integral = 0.0f;
    gate_beg = std::max(gate_beg, 0);
    gate_end = std::min(gate_end, (int)fFitWfm.size() - 1);

    for (int i = gate_beg; i <= gate_end; i++)
    {
        integral += fFitWfm[i];
    }
    return integral;
}