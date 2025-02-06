#ifndef FOURIERFILTER_H
#define FOURIERFILTER_H

#include <vector>
#include <fftw3.h>
#include <cstdint>
class FourierFilter {
public:
    FourierFilter(const std::vector<int16_t>& signal, const int32_t zl, const int32_t gate);
    ~FourierFilter();

    void forwardTransform();
    void applyLowPassFilter(double cutoffFrequency);
    void backwardTransform();

    const std::vector<int16_t>& getFilteredSignal() const;
    std::vector<double> getFourierTransformedSignal() const;

private:
    std::vector<int16_t> signal;
    std::vector<int16_t> filtered_signal;
    size_t N;
    fftw_complex *in, *out;
    int32_t zl, gate;
};

#endif // FOURIERFILTER_H