#ifndef CHANNEL_ENTRY_H
#define CHANNEL_ENTRY_H
// #include<TTree.h>
#include <stdint.h>
#include <math.h>
#include <vector>
using namespace std;
const int MAX_N_SAMPLES = 2048;

struct IntegralInfo
{
    int32_t signal_length = 0;
    int32_t npeaks = 0;
    int32_t end_amplitude = 0;
    void Initialize();
};

struct short_energy_ChannelEntry
{
    float charge;
    float time;
    float ADCTimeStamp;
    uint32_t amp;
    float zl;
    float zl_rms;
    IntegralInfo II;
    uint32_t ADC_ID;
    void Initialize();
};

struct diff_short_energy_ChannelEntry
{
    int32_t min_diff;
    int32_t min_diff_time;
    int32_t max_diff;
    int32_t max_diff_time;
    void Initialize();
};

class ChannelEntry {

    public:
    int32_t ADCID = 0;
    int32_t channel = 0;
    int32_t wf_size;
    vector<int32_t> wf;

    private:
    vector<int32_t> dwf;

    int32_t fZlLeft = 0;
    int32_t fZlRight = 200;
    float zl = 0;
    IntegralInfo II;
    int32_t amp = 0;
    int32_t peak_position = 0;
    int32_t fGATE_BEG = 1000000;
    int32_t fGATE_END = -1000000;
    public:
    void GetWfSize();
    // static TString GetChName(int32_t channel_num);
    void Initialize();
    void SplineWf();
    void CalculateDiffWf();
    void AssumeSmartScope();
    void SetBoarders(int32_t,int32_t);
    void FindDiffWfPars(int32_t &min_diff, int32_t &min_time, int32_t &max_diff, int32_t &max_time);
    void Set_Zero_Level(int);
    void Set_Zero_Level_Area(int32_t i);
    int32_t CountCoincidencePeaks(int32_t, int32_t);
    int32_t PointAmpl(int32_t);
    float CalculateZlwithNoisePeaks(int);
    int32_t Get_Zero_Level();
    float Get_Zero_Level_RMS();
    float Get_Charge();
    int32_t Get_time();
    float Get_time_gauss();
    uint32_t Get_Amplitude();
    IntegralInfo GetIntegralInfo();
    void FillWf(int32_t *Ewf);
    void InvertSignal();

};


#endif CHANNEL_ENTRY_H
