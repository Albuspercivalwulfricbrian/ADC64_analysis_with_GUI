#ifndef FOURIERFILTER_H
#define FOURIERFILTER_H

#include <vector>
#include <fftw3.h>
#include <cstdint>
class FourierFilter {
public:
    FourierFilter(const std::vector<int32_t>& signal, const int32_t zl, const int32_t gate);
    ~FourierFilter();

    void forwardTransform();
    void applyLowPassFilter(double cutoffFrequency);
    void backwardTransform();

    const std::vector<int32_t>& getFilteredSignal() const;
    std::vector<double> getFourierTransformedSignal() const;

private:
    std::vector<int32_t> signal;
    std::vector<int32_t> filtered_signal;
    size_t N;
    fftw_complex *in, *out;
    int32_t zl, gate;
};

#endif // FOURIERFILTER_H