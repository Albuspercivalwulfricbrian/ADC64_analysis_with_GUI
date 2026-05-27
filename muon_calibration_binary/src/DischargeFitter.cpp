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
      fZeroLevel(0.0f), fCacheIndex(0),
      best_one_minus_r2(999.0f),
      fFixedTauMode(false),
      fDebugMode(false)
{
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

void DischargeFitter::SetFixedTauValues(float tau_c, float tau_e)
{
    fTauC = tau_c;
    fTauE = tau_e;
    fFixedTauMode = true;
}

float DischargeFitter::DischargeValueRaw(float t, float A_prefactor, float tau_c, float tau_e)
{
    if (t < 0)
        return 0.0f;
    return A_prefactor * (1.0f - (1.0f + t / tau_c) * exp(-t / tau_c)) * exp(-t / tau_e);
}

float DischargeFitter::FindPeakValue(float tau_c, float tau_e)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (fabs(fTauCCache[i] - tau_c) < 0.001f &&
            fabs(fTauECache[i] - tau_e) < 0.001f)
        {
            return fPeakCache[i];
        }
    }

    float ratio = tau_c / (tau_c + tau_e + 1e-6f);
    float peak_approx = ratio * exp(-ratio);

    float t_peak = tau_c * (1.0f + tau_c / (tau_e + 1e-6f));
    float max_val = peak_approx;

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

    // Skip if in fixed tau mode

    float data_peak = 0.0f;
    int peak_idx = fSignalBegin;
    for (int i = fSignalBegin; i <= fGateEnd && i < (int)fWfm.size(); i++)
        if (fWfm[i] > data_peak)
        {
            data_peak = fWfm[i];
            peak_idx = i;
        }

    fAmplitude = data_peak;

    if (fFixedTauMode)
        return;
    // τ_e guess
    float decay_target = data_peak * 0.368f;
    fTauE = fTauEMax;
    for (int i = peak_idx + 1; i <= fGateEnd && i < (int)fWfm.size(); i++)
        if (fWfm[i] <= decay_target)
        {
            float t_1e = (float)(i - 1 - peak_idx) + (decay_target - fWfm[i - 1]) / (fWfm[i] - fWfm[i - 1]);
            fTauE = t_1e * 0.7f;
            break;
        }
    fTauE = std::clamp(fTauE, fTauEMin, fTauEMax);

    // τ_c guess
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
        fTauC = (t80 - t20) / 2.2f;
    }
    else
    {
        fTauC = 15.0f;
    }
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
    if (t < 0 || tau_c <= 0)
        return 0.0f;

    float eps = 0.001f * tau_c;
    float f_plus = DischargeValue(t, A, tau_c + eps, tau_e);
    float f_minus = DischargeValue(t, A, tau_c - eps, tau_e);
    float deriv = (f_plus - f_minus) / (2.0f * eps);

    return deriv;
}

float DischargeFitter::DischargeValueDerivativeTauE(float t, float A, float tau_c, float tau_e)
{
    if (t < 0 || tau_e <= 0)
        return 0.0f;

    float eps = 0.001f * tau_e;
    float f_plus = DischargeValue(t, A, tau_c, tau_e + eps);
    float f_minus = DischargeValue(t, A, tau_c, tau_e - eps);
    float deriv = (f_plus - f_minus) / (2.0f * eps);

    return deriv;
}

void DischargeFitter::Fit(int max_iterations)
{
    if (fWfm.empty() || fSignalBegin >= fGateEnd)
        return;

    CalculateInitialGuesses();

    float A = fAmplitude;
    float tau_c = fTauC;
    float tau_e = fTauE;

    if (fDebugMode)
        printf("Initial guesses: A=%.2f, tc=%.2f, te=%.2f, fixed=%d\n",
               A, tau_c, tau_e, fFixedTauMode);

    float best_chi2 = std::numeric_limits<float>::max();
    float best_A = A;
    float best_tau_c = tau_c;
    float best_tau_e = tau_e;
    best_one_minus_r2 = 999.0f;

    int n_points = 0;
    for (int i = fSignalBegin; i <= fGateEnd; i += 2)
        n_points++;

    if (n_points < 4)
    {
        if (fDebugMode)
            printf("Warning: Not enough fit points (%d)\n", n_points);
        return;
    }

    float norm = (float)n_points;
    float dof = std::max(1.0f, (float)n_points - (fFixedTauMode ? 1.0f : 3.0f));

    if (fDebugMode)
        printf("Fit start: n_points=%d, A=%.2f, tc=%.2f, te=%.2f\n",
               n_points, A, tau_c, tau_e);

    for (int iter = 0; iter < max_iterations; iter++)
    {
        float chi2 = 0.0f;
        float grad_A = 0.0f;
        float grad_tau_c = 0.0f;
        float grad_tau_e = 0.0f;

        float current_peak = FindPeakValue(tau_c, tau_e);
        if (current_peak <= 0)
        {
            if (fDebugMode)
                printf("Warning: current_peak = %f\n", current_peak);
            break;
        }

        for (int i = fSignalBegin; i <= fGateEnd; i += 2)
        {
            float t = i - fSignalBegin;
            float y = fWfm[i];

            float raw_val = DischargeValueRaw(t, 1.0f, tau_c, tau_e);
            float model = A * (raw_val / current_peak);
            float error = model - y;
            chi2 += error * error;

            float dA = raw_val / current_peak;
            grad_A += 2.0f * error * dA;

            if (!fFixedTauMode)
            {
                grad_tau_c += 2.0f * error * DischargeValueDerivativeTauC(t, A, tau_c, tau_e);
                grad_tau_e += 2.0f * error * DischargeValueDerivativeTauE(t, A, tau_c, tau_e);
            }
        }

        chi2 /= norm;
        grad_A /= norm;

        if (!fFixedTauMode)
        {
            grad_tau_c /= norm;
            grad_tau_e /= norm;
        }

        // Calculate R² and 1-R²
        float ss_res = chi2 * norm;
        float ss_tot = 0;
        float mean_y = 0;

        for (int i = fSignalBegin; i <= fGateEnd; i += 2)
            mean_y += fWfm[i];
        mean_y /= n_points;

        for (int i = fSignalBegin; i <= fGateEnd; i += 2)
            ss_tot += (fWfm[i] - mean_y) * (fWfm[i] - mean_y);

        float r2 = (ss_tot > 1e-10f) ? 1.0f - ss_res / ss_tot : 0.0f;
        float one_minus_r2 = 1.0f - r2;

        if (fDebugMode)
            printf("Iter %d: A=%.2f, tc=%.2f, te=%.2f, chi2=%.2f, 1-R2=%.4f\n",
                   iter, A, tau_c, tau_e, chi2, one_minus_r2);

        if (iter > 5 &&
            (fabs(one_minus_r2 - best_one_minus_r2) < 0.02f * best_one_minus_r2 || one_minus_r2 < 0.07))
            break;

        if (one_minus_r2 < best_one_minus_r2)
        {
            best_one_minus_r2 = one_minus_r2;
            best_chi2 = chi2;
            best_A = A;
            best_tau_c = tau_c;
            best_tau_e = tau_e;
        }

        float step_scale = 1.0f;
        if (one_minus_r2 < 0.4f)
            step_scale = 0.6f;
        if (one_minus_r2 < 0.3f)
            step_scale = 0.4f;
        if (one_minus_r2 < 0.15f)
            step_scale = 0.2f;

        // Update amplitude
        float grad_norm = sqrt(grad_A * grad_A + 1e-10f);
        A -= 0.1f * A * step_scale * (grad_A / grad_norm);

        if (!fFixedTauMode)
        {
            float step_size_tc = 0.2f * (fTauCMax - fTauCMin) * step_scale;
            float step_size_te = 0.2f * (fTauEMax - fTauEMin) * step_scale;

            tau_c -= step_size_tc * (grad_tau_c / sqrt(grad_tau_c * grad_tau_c + 1e-10f));
            tau_e -= step_size_te * (grad_tau_e / sqrt(grad_tau_e * grad_tau_e + 1e-10f));

            tau_c = std::clamp(tau_c, fTauCMin, fTauCMax);
            tau_e = std::clamp(tau_e, fTauEMin, fTauEMax);
        }

        if (A < 0)
            A = 0;
    }

    fAmplitude = best_A;
    fTauC = best_tau_c;
    fTauE = best_tau_e;
    fChi2 = best_chi2;
    fR2 = 1.0f - best_one_minus_r2;

    if (fDebugMode)
        printf("Fit done: A=%.2f, tc=%.2f, te=%.2f, chi2=%.2f, R2=%.4f\n\n",
               fAmplitude, fTauC, fTauE, fChi2, fR2);

    // Precompute fit waveform
    float final_peak = FindPeakValue(fTauC, fTauE);
    for (int i = 0; i < (int)fFitWfm.size(); i++)
    {
        if (i >= fSignalBegin)
        {
            float t = i - fSignalBegin;
            float raw_val = DischargeValueRaw(t, 1.0f, fTauC, fTauE);
            fFitWfm[i] = fAmplitude * (raw_val / final_peak);
        }
        else
        {
            fFitWfm[i] = 0.0f;
        }
    }

    // Calculate final statistics
    float ss_res = 0.0f, ss_tot_final = 0.0f, mean_final = 0.0f;
    int count = 0;
    for (int i = fSignalBegin; i <= fGateEnd; i++)
    {
        mean_final += fWfm[i];
        count++;
    }
    if (count > 0)
        mean_final /= count;

    for (int i = fSignalBegin; i <= fGateEnd; i++)
    {
        float error = fFitWfm[i] - fWfm[i];
        ss_res += error * error;
        ss_tot_final += (fWfm[i] - mean_final) * (fWfm[i] - mean_final);
    }

    fChi2 = ss_res / dof;
    fR2 = (ss_tot_final > 1e-10f) ? 1.0f - ss_res / ss_tot_final : 999.0f;
}

float DischargeFitter::GetFitValue(int sample)
{
    return (sample >= 0 && sample < (int)fFitWfm.size()) ? fFitWfm[sample] : 0.0f;
}

float DischargeFitter::GetFitValue(float x)
{
    int sample = (int)(x + 0.5f);
    return GetFitValue(sample);
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