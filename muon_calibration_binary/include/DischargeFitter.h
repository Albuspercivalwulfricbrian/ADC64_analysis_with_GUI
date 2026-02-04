#ifndef DISCHARGEFITTER_H
#define DISCHARGEFITTER_H

#include <vector>
#include <map>
#include <utility>

class DischargeFitter
{
private:
    int fGateBeg;
    int fGateEnd;
    int fSignalBegin;

    float fTauCMin;
    float fTauCMax;
    float fTauEMin;
    float fTauEMax;

    float fAmplitude;
    float fTauC;
    float fTauE;
    float fChi2;
    float fR2;

    float fZeroLevel;
    std::vector<float> fWfm;
    std::vector<float> fFitWfm;

    // Simple cache for FindPeakValue (much faster than map)
    static const int CACHE_SIZE = 64;
    float fTauCCache[CACHE_SIZE];
    float fTauECache[CACHE_SIZE];
    float fPeakCache[CACHE_SIZE];
    int fCacheIndex;

    float DischargeValueRaw(float t, float A_prefactor, float tau_c, float tau_e);
    float FindPeakValue(float tau_c, float tau_e);
    void CalculateInitialGuesses();

    float DischargeValueDerivativeA(float t, float A, float tau_c, float tau_e);
    float DischargeValueDerivativeTauC(float t, float A, float tau_c, float tau_e);
    float DischargeValueDerivativeTauE(float t, float A, float tau_c, float tau_e);

    float GoToLevel(float Level, int &point, int iterator, int iLastPoint);
    float LevelBy2Points(float X1, float Y1, float X2, float Y2, float Y0);

public:
    DischargeFitter(int gate_beg, int gate_end);

    void SetWaveform(const std::vector<float> &wfm, float zero_level);
    void SetTauBounds(float tau_c_min, float tau_c_max,
                      float tau_e_min, float tau_e_max);
    void SetSignalBegin(int signal_begin) { fSignalBegin = signal_begin; }

    float DischargeValue(float t, float A_peak, float tau_c, float tau_e);

    void Fit(int max_iterations = 8);

    float GetFitValue(int sample);
    float GetFitValue(float x);
    float GetIntegral(int gate_beg, int gate_end);

    float GetAmplitude() const { return fAmplitude; }
    float GetTauC() const { return fTauC; }
    float GetTauE() const { return fTauE; }
    float GetChiSquare() const { return fChi2; }
    float GetRSquare() const { return fR2; }

    int CalcSignalBeginStraight();
};

#endif // DISCHARGEFITTER_H