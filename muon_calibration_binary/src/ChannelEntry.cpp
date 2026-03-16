#include "ChannelEntry.h"
#include "PronyFitter.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <limits>
#include <algorithm>

using namespace std;

void IntegralInfo::Initialize()
{
    signal_length = 0;
    end_amplitude = 0;
}

void FitParameters::Initialize()
{
    fit_charge = 0.0f;
    chi2 = 999.0f;
    r2 = 0.0f;
    tau_c = 0.0f;
    tau_e = 0.0f;
    fit_amplitude = 0.0f;
}

void short_energy_ChannelEntry::Initialize()
{
    charge = 0.;
    time = 0.;
    amp = 0;
    zl_rms = 0.;
    zl = 0.;
    ADC_ID = 0;
    ADCTimeStamp = 0.;
    II.Initialize();
    FP.Initialize();
}

void SinglePeakInfo::Initialize()
{
    charge = 0.;
    time = 0.;
    amp = 0;
    II.Initialize();
    FP.Initialize();
}

void PeaksInfo::Initialize()
{
    ADC_ID = 0;
    ADCTimeStamp = 0;
    zl = 0;
    zl_rms = 0;
    peaks.clear();
}

void PeaksInfo::ResetVector()
{
    peaks = {};
}

int PeaksInfo::GetCurrentSize()
{
    return peaks.size();
}

void PeaksInfo::AddPeak(const SinglePeakInfo &peak)
{
    peaks.push_back(peak);
}

uint32_t PeaksInfo::amp()
{
    if (peaks.size() > 0)
        return peaks[0].amp;
    else
        return 0;
}

float PeaksInfo::time()
{
    if (peaks.size() > 0)
        return peaks[0].time;
    else
        return 0;
}

float PeaksInfo::charge()
{
    float charge = 0;
    if (peaks.size() > 0)
        for (auto el : peaks)
            charge += el.charge;
    return charge;
}

void diff_short_energy_ChannelEntry::Initialize()
{
    min_diff = 0;
    min_diff_time = 0;
    max_diff = 0;
    max_diff_time = 0;
}

void ChannelEntry::Initialize()
{
    ADCID = -1000;
    channel = -1000;
    wf.clear();
    dwf.clear();
    wf_size = 0;
}

void ChannelEntry::GetWfSize() { wf_size = wf.size(); }

void ChannelEntry::SplineWf()
{
    vector<float> wf1;
    const int32_t SplineWidth = 2;
    for (int32_t i = 0; i < wf.size(); i++)
    {
        float point = 0;
        int32_t il = i - SplineWidth;
        int32_t ir = i + SplineWidth;
        if (il < 0)
            il = 0;
        if (ir > wf.size() - 1)
            ir = wf.size() - 1;
        float counter = 0;
        for (int32_t in = il; in <= ir; in++)
        {
            point += wf[in];
            counter++;
        }
        point /= counter;
        wf1.push_back(point);
    }
    for (int32_t i = 0; i < wf.size(); i++)
        wf[i] = wf1[i];
}

int32_t ChannelEntry::PointAmpl(int32_t i)
{
    int32_t v = zl - wf[i];
    return v;
}

int32_t ChannelEntry::CountCoincidencePeaks(int32_t threshold1, int32_t threshold2)
{
    int32_t nCoincidencePeaks = 0;
    int32_t epsilon1 = 10;
    int32_t epsilon2 = 100;
    for (int32_t i = fGATE_BEG; i < fGATE_END; i++)
    {
        int32_t v = PointAmpl(i);
        int32_t vl = PointAmpl(i - 1);
        int32_t vll = PointAmpl(i - 2);
        int32_t vr = PointAmpl(i + 1);
        int32_t vrr = PointAmpl(i + 2);

        if ((v - vr > epsilon1) && (v - vl > epsilon1) && (v - vrr > epsilon2) && (v - vll > epsilon2) && (v > threshold1 && v < threshold2))
            nCoincidencePeaks++;
    }
    return nCoincidencePeaks;
}

void ChannelEntry::CalculateDiffWf()
{
    const float Diff_window = 4;
    dwf.clear();
    for (int32_t i = 0; i < wf.size(); i++)
    {
        int32_t il = i - Diff_window;
        int32_t ir = i + Diff_window;
        if (il < 0)
            il = 0;
        if (ir > wf.size() - 1)
            ir = wf.size() - 1;
        dwf.push_back((int32_t)((float)(wf[ir] - wf[il]) / (float)(ir - il)));
    }
}

float ChannelEntry::CalculateZlwithNoisePeaks(int a)
{
    CalculateDiffWf();
    float sum = 0;
    float counter = 0;
    for (int s = fZlLeft + 1; s < fZlRight; ++s)
    {
        if (abs(dwf[s]) < a && abs(dwf[s - 1]) < a && abs(dwf[s + 1]) < a)
        {
            sum += wf[s];
            counter++;
        }
    }
    zl = sum / counter;
    return zl;
}

void ChannelEntry::AssumeSmartScope()
{
    fGATE_BEG = peak_position;
    fGATE_END = peak_position;

    while (1)
    {
        fGATE_BEG--;
        if (fGATE_BEG < 0)
        {
            fGATE_BEG++;
            break;
        }
        if (wf[fGATE_BEG] > zl - cutoff_level * amp)
            break;
    }
    while (1)
    {
        fGATE_END++;
        if (fGATE_END >= wf.size())
        {
            fGATE_END--;
            break;
        }
        if (wf[fGATE_END] > zl - cutoff_level * amp)
            break;
    }
}

float ChannelEntry::LevelBy2Points(float X1, float Y1, float X2, float Y2, float Y0)
{
    if (Y1 == Y2)
        return X1;
    return (X1 * Y0 - X1 * Y2 - X2 * Y0 + X2 * Y1) / (Y1 - Y2);
}

float ChannelEntry::GoToLevel(float Level)
{
    int point = peak_position;
    float targetADC = zl - Level * amp;

    while (point > fGATE_BEG)
    {
        float currentValue = static_cast<float>(wf[point]);
        float prevValue = static_cast<float>(wf[point - 1]);

        if ((targetADC - currentValue) * (targetADC - prevValue) <= 0)
        {
            return LevelBy2Points(static_cast<float>(point), currentValue,
                                  static_cast<float>(point - 1), prevValue, targetADC);
        }
        point--;
    }
    return 0.0f;
}

PronyFitResult ChannelEntry::PerformPronyFitWithOverrideHarmonics()
{
    PronyFitResult result = {};

    if (amp == 0 || fGATE_BEG < 0 || fGATE_END >= wf.size() || fGATE_BEG >= fGATE_END)
        return result;

    std::vector<float> positive_wf(wf.size());
    for (size_t i = 0; i < wf.size(); i++)
    {
        positive_wf[i] = zl - (float)wf[i];
    }

    // Create PronyFitter on heap
    PronyFitter *pfitter = new PronyFitter(2, 2, fGATE_BEG, fGATE_END);

    pfitter->SetWaveform(positive_wf, 0.0f);
    int SignalBeg = pfitter->CalcSignalBeginStraight();

    if (SignalBeg < 0)
        SignalBeg = 0;

    pfitter->SetSignalBegin(SignalBeg + 2);

    // OVERRIDE HARMONICS - NO DC TERM, SetExternalHarmonics adds it internally
    std::vector<std::complex<float>> override_harmonics = {
        {0.881099f, 0.0f},  // This will be harmonic 1
        {0.9617225f, 0.0f}, // This will be harmonic 2
    };

    pfitter->SetExternalHarmonics(override_harmonics);

    // Store harmonics in result (SetExternalHarmonics already added DC)
    std::complex<float> *all_harmonics = pfitter->GetHarmonics();
    int num_harmonics = pfitter->GetNumberOfHarmonics(); // Should return 3

    if (all_harmonics && num_harmonics > 0)
    {
        result.harmonics.assign(all_harmonics, all_harmonics + num_harmonics);
    }

    result.fitter = pfitter;
    result.signal_begin = SignalBeg;

    pfitter->CalculateFitAmplitudes();

    // Get amplitudes and store them
    std::complex<float> *amplitudes = pfitter->GetAmplitudes();

    if (amplitudes && num_harmonics > 0)
    {
        result.amplitudes.assign(amplitudes, amplitudes + num_harmonics);
    }

    result.integral = pfitter->GetIntegral(fGATE_BEG, fGATE_END);
    result.chi2 = pfitter->GetChiSquare(fGATE_BEG, fGATE_END, peak_position);
    result.r2 = pfitter->GetRSquare(fGATE_BEG, fGATE_END);

    return result;
}

PronyFitResult ChannelEntry::PerformPronyFit()
{
    PronyFitResult result = {};

    if (amp == 0 || fGATE_BEG < 0 || fGATE_END >= wf.size() || fGATE_BEG >= fGATE_END)
        return result;

    std::vector<float> positive_wf(wf.size());
    for (size_t i = 0; i < wf.size(); i++)
    {
        positive_wf[i] = zl - (float)wf[i];
    }

    // Create PronyFitter on heap
    PronyFitter *pfitter = new PronyFitter(5, 3, fGATE_BEG, fGATE_END);
    // std::cout << fGATE_BEG << " " << fGATE_END << endl;
    pfitter->SetWaveform(positive_wf, 0.0f);
    // pfitter->SetDebugMode(1);
    int SignalBeg = pfitter->CalcSignalBeginStraight();

    if (SignalBeg < 0)
        SignalBeg = 0;

    pfitter->SetSignalBegin(SignalBeg + 2);
    pfitter->CalculateFitHarmonics();

    std::complex<float> *harmonics = pfitter->GetHarmonics();
    int num_harmonics = pfitter->GetNumberOfHarmonics();

    if (harmonics && num_harmonics > 0)
    {
        result.harmonics.assign(harmonics, harmonics + num_harmonics);
    }
    result.fitter = pfitter;
    result.signal_begin = SignalBeg;

    pfitter->CalculateFitAmplitudes();
    std::complex<float> *amplitudes = pfitter->GetAmplitudes();

    if (amplitudes && num_harmonics > 0)
        result.amplitudes.assign(amplitudes, amplitudes + num_harmonics);

    result.integral = pfitter->GetIntegral(fGATE_BEG, fGATE_END);
    result.chi2 = pfitter->GetChiSquare(fGATE_BEG, fGATE_END, peak_position);
    result.r2 = pfitter->GetRSquare(fGATE_BEG, fGATE_END);

    return result;
}

FitParameters ChannelEntry::CalculateDischargeFit()
{
    FitParameters result;
    result.Initialize();

    if (amp == 0 || fGATE_BEG < 0 || fGATE_END >= wf.size() || fGATE_BEG >= fGATE_END)
        return result;

    std::vector<float> positive_wf(wf.size());
    for (size_t i = 0; i < wf.size(); i++)
    {
        positive_wf[i] = zl - (float)wf[i];
    }

    // Create DischargeFitter
    DischargeFitter dischargeFit(fGATE_BEG, fGATE_END);
    dischargeFit.SetWaveform(positive_wf, 0.0f);
    dischargeFit.SetFixedTauValues(6.4, 19.5);
    // dischargeFit.SetTauBounds(5.0f, 10.0f, 15.0f, 20.0f);
    int SignalBeg = dischargeFit.CalcSignalBeginStraight();
    if (SignalBeg < 2)
        SignalBeg = 2;
    dischargeFit.SetSignalBegin(GetLeftBoarder() - 1);
    dischargeFit.Fit(15);

    // Store results
    result.fit_charge = dischargeFit.GetIntegral(fGATE_BEG, fGATE_END);
    result.chi2 = dischargeFit.GetChiSquare();
    result.r2 = dischargeFit.GetRSquare();
    result.tau_c = dischargeFit.GetTauC();
    result.tau_e = dischargeFit.GetTauE();
    result.fit_amplitude = dischargeFit.GetAmplitude();

    return result;
}

FitParameters ChannelEntry::CalculatePronyFit()
{
    FitParameters result;
    result.Initialize();

    PronyFitResult pronyResult = PerformPronyFit();
    if (pronyResult.fitter)
    {
        result.chi2 = pronyResult.chi2;
        result.r2 = pronyResult.r2;
        result.fit_charge = pronyResult.integral;
        result.fit_amplitude = (pronyResult.amplitudes.size() > 0) ? std::real(pronyResult.amplitudes[0]) : 0.0f;
        delete pronyResult.fitter;
    }

    return result;
}

FitParameters ChannelEntry::CalculatePronyFitWithOverrideHarmonics()
{
    FitParameters result;
    result.Initialize();

    PronyFitResult pronyResult = PerformPronyFitWithOverrideHarmonics();
    if (pronyResult.fitter)
    {
        result.chi2 = pronyResult.chi2;
        result.r2 = pronyResult.r2;
        result.fit_charge = pronyResult.integral;
        result.fit_amplitude = (pronyResult.amplitudes.size() > 0) ? std::real(pronyResult.amplitudes[0]) : 0.0f;
        delete pronyResult.fitter;
    }

    return result;
}

int32_t ChannelEntry::GetLeftBoarder()
{
    return fGATE_BEG;
}

int32_t ChannelEntry::GetRightBoarder()
{
    return fGATE_END;
}

void ChannelEntry::DeleteCurrentPeak()
{
    if (fGATE_BEG != fGATE_END)
    {
        for (int32_t i = fGATE_BEG; i < fGATE_END; i++)
            wf[i] = zl;
    }
}

void ChannelEntry::SetBoarders(int32_t BEG, int32_t END)
{
    fGATE_BEG = BEG;
    fGATE_END = END;
}

void ChannelEntry::FindDiffWfPars(int32_t &min_diff, int32_t &min_time, int32_t &max_diff, int32_t &max_time)
{
    for (int32_t s = fGATE_BEG; s < fGATE_END; ++s)
    {
        int32_t v = wf[s];
        if (v < min_diff)
        {
            min_diff = v;
            min_time = 16 * s;
        }
        if (v > max_diff)
        {
            max_diff = v;
            max_time = 16 * s;
        }
    }
}

void ChannelEntry::Set_Zero_Level_Area(int32_t i)
{
    fZlLeft = 0;
    fZlRight = i;
}

void ChannelEntry::Set_Zero_Level(int EZL)
{
    zl = EZL;
}

float ChannelEntry::Get_Zero_Level()
{
    const int32_t interv_num = 1;
    int zero_lvl = 0;
    int best_spread = -1;
    for (int i = 0; i < interv_num; ++i)
    {
        int vmin = numeric_limits<int>::max();
        int vmax = numeric_limits<int>::min();
        int sum = 0;
        for (int s = fZlRight / interv_num * i; s < fZlRight / interv_num * (i + 1); ++s)
        {
            int v = wf[s];
            sum += v;
            if (v < vmin)
                vmin = v;
            if (v > vmax)
                vmax = v;
        }
        int spread = vmax - vmin;
        if (best_spread < 0)
            best_spread = spread;
        if (spread <= best_spread)
        {
            best_spread = spread;
            zero_lvl = sum / (fZlRight / interv_num);
        }
        zl = zero_lvl;
    }
    return zero_lvl;
}

float ChannelEntry::Get_Zero_Level_RMS()
{
    const int32_t interv_num = 1;
    float best_spread = -1;
    float rms_zl = -1;
    for (int32_t i = 0; i < interv_num; ++i)
    {
        int32_t vmin = numeric_limits<int>::max();
        int32_t vmax = numeric_limits<int>::min();
        float sum = 0;
        float sumsquare = 0;
        float sum_counter = 0;
        for (int32_t s = fZlRight / interv_num * i; s < fZlRight / interv_num * (i + 1); ++s)
        {
            int32_t v = wf[s];
            sum += (float)v;
            sum_counter++;
        }
        sum /= sum_counter;
        sumsquare = 0.;

        for (int32_t s = fZlRight / interv_num * i; s < fZlRight / interv_num * (i + 1); ++s)
        {
            sumsquare += (float)(wf[s] - sum) * (wf[s] - sum) / sum_counter;
        }
        rms_zl = sqrt(sumsquare);
        if (best_spread < 0)
            best_spread = rms_zl;
    }
    return best_spread;
}

float ChannelEntry::Get_Charge()
{
    float gateInteg = 0;
    for (int s = fGATE_BEG; s <= fGATE_END; ++s)
    {
        gateInteg += ((float)zl - (float)wf[s]);
    }
    II.signal_length = fGATE_END - fGATE_BEG;
    II.end_amplitude = (float)zl - (float)wf[fGATE_END];

    return gateInteg;
}

IntegralInfo ChannelEntry::GetIntegralInfo()
{
    return II;
}

int32_t ChannelEntry::Get_time()
{
    amp = 0;
    peak_position = 0;
    if (wf.size() == 0)
    {
        return peak_position;
    }
    for (int s = fGATE_BEG; s < fGATE_END; ++s)
    {
        int32_t v = zl - wf[s];
        if (v > amp)
        {
            amp = v;
            peak_position = s;
        }
    }
    return peak_position;
}

float ChannelEntry::Get_time_gauss()
{
    if (wf.size() == 0)
        return 0;
    float peak_search = 0.;
    float ampl_sum = 0;
    for (int32_t s = fGATE_BEG; s < fGATE_END; ++s)
    {
        int32_t v = zl - wf[s];
        if (v > amp * cutoff_level)
        {
            ampl_sum += (float)v;
            peak_search += (float)v * s;
        }
    }
    peak_search /= ampl_sum;
    return 16.0 * peak_search;
}

uint32_t ChannelEntry::Get_Amplitude()
{
    return (uint32_t)amp;
}

void ChannelEntry::InvertSignal()
{
    for (int i = 0; i < wf_size; i++)
        wf[i] = -wf[i];
}