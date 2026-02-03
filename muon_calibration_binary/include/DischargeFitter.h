#ifndef DischargeFitter_H
#define DischargeFitter_H

#include <vector>
#include <complex>
#include <map>
#include <utility>

class DischargeFitter
{
private:
    std::vector<float> fWfm;
    float fZeroLevel;
    int fSignalBegin;
    int fGateBeg;
    int fGateEnd;

    // Fitted parameters
    float fAmplitude; // This now represents PEAK amplitude
    float fTauC;
    float fTauE;
    float fChi2;
    float fR2;

    // Fitted waveform
    std::vector<float> fFitWfm;

    // Bounds
    float fTauCMin, fTauCMax;
    float fTauEMin, fTauEMax;

    // Cache for peak normalization
    mutable std::map<std::pair<float, float>, float> fPeakCache;

    // Internal functions
    float DischargeValue(float t, float A_peak, float tau_c, float tau_e);
    float DischargeValueRaw(float t, float A_prefactor, float tau_c, float tau_e);
    float FindPeakValue(float tau_c, float tau_e);
    void CalculateInitialGuesses();

public:
    DischargeFitter(int gate_beg, int gate_end);

    void SetWaveform(const std::vector<float> &wfm, float zero_level);
    void SetSignalBegin(int signal_beg) { fSignalBegin = signal_beg; }
    void SetTauBounds(float tau_c_min, float tau_c_max,
                      float tau_e_min, float tau_e_max);

    void Fit();

    // Get results
    float GetFitValue(int sample);
    float GetFitValue(float x);
    float GetAmplitude() const { return fAmplitude; } // Peak amplitude
    float GetTauC() const { return fTauC; }
    float GetTauE() const { return fTauE; }
    float GetChiSquare() const { return fChi2; }
    float GetRSquare() const { return fR2; }
    float GetIntegral(int gate_beg, int gate_end);

    const std::vector<float> &GetFitWaveform() const { return fFitWfm; }
};

#endif