#ifndef CHANNEL_ENTRY_H
#define CHANNEL_ENTRY_H

#include <stdint.h>
#include <math.h>
#include <vector>
#include <array>
#include <complex>
#include "PronyFitter.h"

const int MAX_N_SAMPLES = 2048;
constexpr size_t MAX_HARMONICS = 10; // Define maximum expected harmonics

struct IntegralInfo
{
    int32_t signal_length = 0;
    int32_t end_amplitude = 0;
    void Initialize();
};

struct short_energy_ChannelEntry
{
    float charge = 0;
    float time = 0;
    float ADCTimeStamp = 0;
    uint32_t amp = 0;
    float zl = 0;
    float zl_rms = 0;
    IntegralInfo II;
    uint32_t ADC_ID = 0;
    void Initialize();
};

struct SinglePeakInfo
{
    float charge = 0;
    float time = 0;
    uint32_t amp = 0;
    IntegralInfo II;
    void Initialize();
    SinglePeakInfo() { Initialize(); }
};

struct PeaksInfo
{
    uint32_t ADC_ID = 0;
    float ADCTimeStamp = 0;
    float zl = 0;
    float zl_rms = 0;
    std::vector<SinglePeakInfo> peaks = {};

    void Initialize();
    void ResetVector();
    int GetCurrentSize();
    void AddPeak(const SinglePeakInfo &peak);
    PeaksInfo() { Initialize(); }
    uint32_t amp();
    float time();
    float charge();
};

struct diff_short_energy_ChannelEntry
{
    int32_t min_diff;
    int32_t min_diff_time;
    int32_t max_diff;
    int32_t max_diff_time;
    void Initialize();
};

struct PronyFitResult
{
    float integral;
    float chi2;
    float r2;
    std::vector<std::complex<float>> harmonics;
    PronyFitter *fitter = nullptr;
    int signal_begin;
};
class ChannelEntry
{
public:
    int32_t ADCID = 0;
    int32_t channel = 0;
    int32_t wf_size;
    std::vector<int32_t> wf;

private:
    std::vector<int32_t> dwf;
    int32_t fZlLeft = 0;
    int32_t fZlRight = 200;
    float zl = 0;
    IntegralInfo II;
    int32_t amp = 0;
    int32_t peak_position = 0;
    int32_t fGATE_BEG = 1000000;
    int32_t fGATE_END = -1000000;
    float cutoff_level = 0.02;

public:
    void GetWfSize();
    void Initialize();
    void SplineWf();
    void CalculateDiffWf();
    void AssumeSmartScope();
    float GoToLevel(float Level);
    float LevelBy2Points(float X1, float Y1, float X2, float Y2, float Y0);
    PronyFitResult PerformPronyFit();
    void DeleteCurrentPeak();
    void SetBoarders(int32_t, int32_t);
    void FindDiffWfPars(int32_t &min_diff, int32_t &min_time, int32_t &max_diff, int32_t &max_time);
    void Set_Zero_Level(int);
    void Set_Zero_Level_Area(int32_t i);
    int32_t CountCoincidencePeaks(int32_t, int32_t);
    int32_t PointAmpl(int32_t);
    int32_t GetLeftBoarder();
    int32_t GetRightBoarder();
    float CalculateZlwithNoisePeaks(int);
    float Get_Zero_Level();
    float Get_Zero_Level_RMS();
    float Get_Charge();
    int32_t Get_time();
    float Get_time_gauss();
    uint32_t Get_Amplitude();
    IntegralInfo GetIntegralInfo();
    void InvertSignal();
};

#endif // CHANNEL_ENTRY_H