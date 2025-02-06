#include "FourierFilter.h"
#include <cmath>
#include <iostream>

FourierFilter::FourierFilter(const std::vector<int16_t>& signal, const int32_t zl, const int32_t gate) : signal(signal), zl(zl), gate(gate) {
    N = signal.size();
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    filtered_signal.resize(N);
}

FourierFilter::~FourierFilter() {
    fftw_free(in);
    fftw_free(out);
}

void FourierFilter::forwardTransform() {
    // Perform FFT
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    for (size_t i = 0; i < N; ++i) {
        in[i][0] = (signal[i]-zl); // Real part
        in[i][1] = 0.0;       // Imaginary part
    }
    fftw_execute(p);
    fftw_destroy_plan(p);
}

void FourierFilter::applyLowPassFilter(double cutoffFrequency) {
    // Apply low-pass filter
    for (size_t i = 0; i < N; ++i) {
        double frequency = static_cast<double>(i); // Normalized frequency
        if (frequency > cutoffFrequency) {
            out[i][0] = 0.0; // Zero out real part
            out[i][1] = 0.0; // Zero out imaginary part
        }
    }
}

void FourierFilter::backwardTransform() {
    // Perform inverse FFT
    fftw_plan p = fftw_plan_dft_1d(N, out, in, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);

    // Store the filtered signal

    for (size_t i = 0; i < N; ++i) in[i][0] = 2*in[i][0] / N;
    float zl_local = 0;
    for (size_t i = 0; i < gate; ++i) zl_local+=in[i][0];
    zl_local/=gate;
    for (size_t i = 0; i < N; ++i) filtered_signal[i] = static_cast<int16_t>((in[i][0])-zl_local+zl); // Normalize by N
}

const std::vector<int16_t>& FourierFilter::getFilteredSignal() const {
    return filtered_signal;
}

std::vector<double> FourierFilter::getFourierTransformedSignal() const {
    std::vector<double> fourierSignal(N);
    for (size_t i = 0; i < N; ++i) {
        fourierSignal[i] = std::sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]); // Magnitude
    }
    return fourierSignal;
}

// int main() {
//     // Example usage
//     std::vector<int16_t> signal = { /* your signal data here */ };
//     FourierFilter filter(signal);

//     double cutoffFrequency = 0.1; // Example cutoff frequency
//     filter.applyLowPassFilter(cutoffFrequency);

//     std::vector<int16_t> filteredSignal = filter.getFilteredSignal();

//     // Output the filtered signal
//     for (const auto& sample : filteredSignal) {
//         std::cout << sample << std::endl;
//     }

//     return 0;
// }