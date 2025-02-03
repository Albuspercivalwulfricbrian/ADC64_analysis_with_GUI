#ifndef FFTFILTER_H
#define FFTFILTER_H

#include <vector>
#include <fftw3.h>
#include <cstdint>

class FFTFilter {
public:
    FFTFilter(const std::vector<int16_t>& input_signal, double sample_rate, double cutoff_frequency);
    const std::vector<int16_t>& get_signal() const;
    std::vector<double> get_transformed_signal() const;
    void set_cutoff_frequency(double new_cutoff_frequency);

private:
    std::vector<int16_t> wf; // Original signal (filtered)
    std::vector<fftw_complex> out; // Transformed signal (frequency domain)
    double sample_rate; // Sample rate in Hz
    double cutoff_frequency; // Cutoff frequency in Hz

    void fourier_transform();
    void apply_frequency_cutoff();
    void backward_transform();
};

#endif // FFTFILTER_H