#include "DischargeFitter.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

DischargeFitter::DischargeFitter(int gate_beg, int gate_end)
    : fGateBeg(gate_beg), fGateEnd(gate_end), fSignalBegin(0),
      fTauCMin(0.5f), fTauCMax(50.0f),
      fTauEMin(5.0f), fTauEMax(500.0f),
      fAmplitude(0.0f), fTauC(5.0f), fTauE(50.0f),
      fChi2(999.0f), fR2(0.0f),
      fZeroLevel(0.0f), fCacheIndex(0)
{
    // Initialize cache with invalid values
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        fTauCCache[i] = -1.0f;
        fTauECache[i] = -1.0f;
        fPeakCache[i] = -1.0f;
    }
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

float DischargeFitter::DischargeValueRaw(float t, float A_prefactor, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;
    return A_prefactor * (1.0f - (1.0f + t / tau_c) * exp(-t / tau_c)) * exp(-t / tau_e);
}

float DischargeFitter::FindPeakValue(float tau_c, float tau_e)
{
    // FAST SIMPLE CACHE - linear search in small array
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (fabs(fTauCCache[i] - tau_c) < 0.001f &&
            fabs(fTauECache[i] - tau_e) < 0.001f)
        {
            return fPeakCache[i];
        }
    }

    // Calculate peak value (SIMPLIFIED - much faster)
    // For the discharge function, the peak value is approximately:
    // peak ≈ (tau_c / (tau_c + tau_e)) * exp(-tau_c / (tau_c + tau_e))
    // This is an analytical approximation that's VERY fast

    float ratio = tau_c / (tau_c + tau_e + 1e-6f);
    float peak_approx = ratio * exp(-ratio);

    // Quick refinement with just 5 points
    float t_peak = tau_c * (1.0f + tau_c / (tau_e + 1e-6f));
    float max_val = peak_approx;

    // Just check 5 points around the approximation
    for (int k = 0; k < 5; k++)
    {
        float t = t_peak * (0.8f + 0.1f * k);
        if (t < 0)
            continue;
        float exp_tc = exp(-t / tau_c);
        float exp_te = exp(-t / tau_e);
        float val = (1.0f - (1.0f + t / tau_c) * exp_tc) * exp_te;
        if (val > max_val)
            max_val = val;
    }

    // Store in cache (rounding helps cache hits)
    float rounded_tau_c = roundf(tau_c * 100.0f) / 100.0f;
    float rounded_tau_e = roundf(tau_e * 100.0f) / 100.0f;

    fTauCCache[fCacheIndex] = rounded_tau_c;
    fTauECache[fCacheIndex] = rounded_tau_e;
    fPeakCache[fCacheIndex] = max_val;

    fCacheIndex = (fCacheIndex + 1) % CACHE_SIZE;

    return max_val;
}

float DischargeFitter::DischargeValue(float t, float A_peak, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;
    float raw_value = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
    float peak_raw = FindPeakValue(tau_c, tau_e);
    return (peak_raw > 1e-6f) ? A_peak * (raw_value / peak_raw) : 0.0f;
}

void DischargeFitter::CalculateInitialGuesses()
{
    if (fWfm.empty() || fSignalBegin >= fGateEnd)
        return;

    float data_peak = 0.0f;
    int peak_idx = fSignalBegin;
    for (int i = fSignalBegin; i <= fGateEnd && i < (int)fWfm.size(); i++)
        if (fWfm[i] > data_peak)
        {
            data_peak = fWfm[i];
            peak_idx = i;
        }

    fAmplitude = data_peak;

    float decay_target = data_peak * 0.368f;
    fTauE = fTauEMax;
    for (int i = peak_idx + 1; i <= fGateEnd && i < (int)fWfm.size(); i++)
        if (fWfm[i] <= decay_target)
        {
            fTauE = (float)(i - 1 - peak_idx) + (decay_target - fWfm[i - 1]) / (fWfm[i] - fWfm[i - 1]);
            break;
        }
    fTauE = std::clamp(fTauE, fTauEMin, fTauEMax);

    float t20 = -1.0f, t80 = -1.0f;
    for (int i = fSignalBegin; i < peak_idx; i++)
    {
        if (fWfm[i] >= 0.2f * data_peak && t20 < 0)
            t20 = i - fSignalBegin;
        if (fWfm[i] >= 0.8f * data_peak && t80 < 0)
            t80 = i - fSignalBegin;
    }
    fTauC = (t20 > 0 && t80 > t20) ? (t80 - t20) * 0.8f : 5.0f;
    fTauC = std::clamp(fTauC, fTauCMin, fTauCMax);
}

float DischargeFitter::DischargeValueDerivativeA(float t, float A, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;
    float raw_value = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
    float peak_raw = FindPeakValue(tau_c, tau_e);
    return (peak_raw > 1e-6f) ? (raw_value / peak_raw) : 0.0f;
}

float DischargeFitter::DischargeValueDerivativeTauC(float t, float A, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;

    // SIMPLIFIED DERIVATIVE - skip expensive calculations
    // Most of the time, this derivative is small anyway
    static float last_t = -1.0f, last_tau_c = -1.0f, last_tau_e = -1.0f;
    static float cached_value = 0.0f;

    if (t == last_t && tau_c == last_tau_c && tau_e == last_tau_e)
    {
        return cached_value;
    }

    float peak_raw = FindPeakValue(tau_c, tau_e);
    if (peak_raw <= 1e-6f)
    {
        cached_value = 0.0f;
        return 0.0f;
    }

    // Simplified derivative - approximate
    float raw_val = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
    float d_raw_d_tau_c = A * raw_val * (t / (tau_c * tau_c * (tau_c + tau_e + 1e-6f)));

    cached_value = d_raw_d_tau_c / peak_raw;
    last_t = t;
    last_tau_c = tau_c;
    last_tau_e = tau_e;

    return cached_value;
}

float DischargeFitter::DischargeValueDerivativeTauE(float t, float A, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;

    // SIMPLIFIED DERIVATIVE
    static float last_t = -1.0f, last_tau_c = -1.0f, last_tau_e = -1.0f;
    static float cached_value = 0.0f;

    if (t == last_t && tau_c == last_tau_c && tau_e == last_tau_e)
    {
        return cached_value;
    }

    float peak_raw = FindPeakValue(tau_c, tau_e);
    if (peak_raw <= 1e-6f)
    {
        cached_value = 0.0f;
        return 0.0f;
    }

    // Simplified derivative
    float raw_val = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
    float d_raw_d_tau_e = -A * raw_val * (t / (tau_e * tau_e));

    cached_value = d_raw_d_tau_e / peak_raw;
    last_t = t;
    last_tau_c = tau_c;
    last_tau_e = tau_e;

    return cached_value;
}

void DischargeFitter::Fit(int max_iterations)
{
    if (fWfm.empty() || fSignalBegin >= fGateEnd)
        return;

    CalculateInitialGuesses();

    float A = fAmplitude, tau_c = fTauC, tau_e = fTauE;
    float best_chi2 = std::numeric_limits<float>::max();
    float best_A = A, best_tau_c = tau_c, best_tau_e = tau_e;

    int num_fit_points = 0;
    for (int i = fSignalBegin; i <= fGateEnd; i += 2)
        num_fit_points++;
    float dof = std::max(1.0f, (float)num_fit_points - 3.0f);

    for (int iter = 0; iter < max_iterations; iter++)
    {
        float chi2 = 0.0f, grad_A = 0.0f, grad_tau_c = 0.0f, grad_tau_e = 0.0f;

        // OPTIMIZATION: Pre-calculate peak value for current tau_c/tau_e
        float current_peak = FindPeakValue(tau_c, tau_e);

        for (int i = fSignalBegin; i <= fGateEnd; i += 2)
        {
            float t = i - fSignalBegin;

            // FAST MODEL CALCULATION with pre-calculated peak
            float raw_val = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
            float model = A * (raw_val / current_peak);
            float error = model - fWfm[i];
            chi2 += error * error;

            // FAST DERIVATIVES
            float dA = raw_val / current_peak;
            grad_A += 2.0f * error * dA;

            // Skip tau derivatives sometimes (they're less important)
            if (iter % 2 == 0 || i % 4 == 0)
            {
                grad_tau_c += 2.0f * error * DischargeValueDerivativeTauC(t, A, tau_c, tau_e);
                grad_tau_e += 2.0f * error * DischargeValueDerivativeTauE(t, A, tau_c, tau_e);
            }
        }

        chi2 /= dof;
        if (chi2 < best_chi2)
        {
            best_chi2 = chi2;
            best_A = A;
            best_tau_c = tau_c;
            best_tau_e = tau_e;
        }

        float step = 0.2f / (1.0f + iter * 0.1f);
        A -= step * grad_A;
        tau_c -= step * grad_tau_c;
        tau_e -= step * grad_tau_e;

        A = std::max(0.1f, std::min(A, 10000.0f));
        tau_c = std::clamp(tau_c, fTauCMin, fTauCMax);
        tau_e = std::clamp(tau_e, fTauEMin, fTauEMax);

        if (iter > 8 && fabs(chi2 - best_chi2) < 0.01f * best_chi2)
            break;
    }

    fAmplitude = best_A;
    fTauC = best_tau_c;
    fTauE = best_tau_e;

    for (int i = 0; i < (int)fFitWfm.size(); i++)
        fFitWfm[i] = (i >= fSignalBegin) ? DischargeValue(i - fSignalBegin, fAmplitude, fTauC, fTauE) : 0.0f;

    float ss_res = 0.0f, ss_tot = 0.0f, mean = 0.0f;
    int count = 0;
    for (int i = fSignalBegin; i <= fGateEnd; i++)
        mean += fWfm[i], count++;
    if (count > 0)
        mean /= count;

    for (int i = fSignalBegin; i <= fGateEnd; i++)
    {
        float error = fFitWfm[i] - fWfm[i];
        ss_res += error * error;
        ss_tot += (fWfm[i] - mean) * (fWfm[i] - mean);
    }

    fChi2 = (count > 0) ? ss_res / std::max(1.0f, (float)count - 3.0f) : 999.0f;
    fR2 = (ss_tot > 1e-10f) ? 1.0f - ss_res / ss_tot : 999.0f;
}

float DischargeFitter::GetFitValue(int sample)
{
    return (sample >= 0 && sample < (int)fFitWfm.size()) ? fFitWfm[sample] : 0.0f;
}

float DischargeFitter::GetFitValue(float x)
{
    int sample = (int)x;
    if (sample >= 0 && sample < (int)fFitWfm.size() && sample >= fSignalBegin)
        return DischargeValue(x - fSignalBegin, fAmplitude, fTauC, fTauE);
    return 0.0f;
}

float DischargeFitter::GetIntegral(int gate_beg, int gate_end)
{
    float t1 = std::max(0.0f, (float)(gate_beg - fSignalBegin));
    float t2 = std::max(0.0f, (float)(gate_end - fSignalBegin));
    if (t2 <= t1)
        return 0.0f;

    float sum_inv = 1.0f / fTauC + 1.0f / fTauE;
    float term1 = fAmplitude * fTauE * (exp(-t1 / fTauE) - exp(-t2 / fTauE));
    float term2 = -fAmplitude / sum_inv * ((1.0f + t2 / fTauC) * exp(-t2 * sum_inv) - (1.0f + t1 / fTauC) * exp(-t1 * sum_inv));

    return term1 + term2;
}

float DischargeFitter::GoToLevel(float Level, int &point, int iterator, int iLastPoint)
{
    while ((point > 0) && (point < iLastPoint))
    {
        if ((Level - fWfm.at(point)) * (Level - fWfm.at(point + iterator)) <= 0)
        {
            return LevelBy2Points((float)point, fWfm[point],
                                  (float)(point + iterator), fWfm[point + iterator], Level);
        }
        point += iterator;
    }
    point = -1;
    return 0;
}

float DischargeFitter::LevelBy2Points(float X1, float Y1, float X2, float Y2, float Y0)
{
    return (Y1 == Y2) ? X1 : (X1 * Y0 - X1 * Y2 - X2 * Y0 + X2 * Y1) / (Y1 - Y2);
}

int DischargeFitter::CalcSignalBeginStraight()
{
    if (fWfm.empty() || fGateBeg >= fGateEnd)
        return fGateBeg;

    auto max_iter = std::max_element(fWfm.begin() + fGateBeg, fWfm.begin() + fGateEnd);
    float amplitude = *max_iter - fZeroLevel;
    int timeMax = std::distance(fWfm.begin(), max_iter);
    if (amplitude <= 0)
        return fGateBeg;

    int point = timeMax;
    float front_time_beg_03 = GoToLevel(fZeroLevel + amplitude * 0.3f, point, -1, fWfm.size());
    point = timeMax;
    float front_time_end = GoToLevel(fZeroLevel + amplitude * 0.6f, point, -1, fWfm.size());

    int signal_begin = std::ceil((2 * front_time_beg_03 - front_time_end));
    return std::max(fGateBeg, std::min(signal_begin, timeMax - 2));
}