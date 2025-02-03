#include "FFTFilter.h"
#include <iostream>
#include <cmath>

FFTFilter::FFTFilter(const std::vector<int16_t>& input_signal, double sample_rate, double cutoff_frequency)
    : wf(input_signal), sample_rate(sample_rate), cutoff_frequency(cutoff_frequency) {
    // Prepare FFT output array
    // out.resize((static_cast<int16_t>(wf.size()) / 2+ 1) );
    fourier_transform();
    // apply_frequency_cutoff();
    // backward_transform();
}

const std::vector<int16_t>& FFTFilter::get_signal() const {
    return wf; // Return the filtered signal
}

std::vector<double> FFTFilter::get_transformed_signal() const {
    std::vector<double> real_parts(out.size());
    for (size_t i = 0; i < out.size(); ++i) {
        real_parts[i] = out[i][0]; // Extract the real part
    }
    return real_parts; // Return the real parts of the transformed signal
}

void FFTFilter::set_cutoff_frequency(double new_cutoff_frequency) {
    cutoff_frequency = new_cutoff_frequency;
    apply_frequency_cutoff(); // Reapply filtering
    backward_transform(); // Transform back to time domain
}

void FFTFilter::fourier_transform() {
    int N = wf.size();
    std::vector<double> in(N);

    // Convert int16_t signal to double
    for (int i = 0; i < N; ++i) {
        in[i] = static_cast<double>(wf[i]);
    }

    // Create FFTW plan for forward FFT
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d(N, in.data(), out.data(), FFTW_ESTIMATE);

    // Execute the forward FFT
    fftw_execute(plan_forward);

    // Cleanup
    fftw_destroy_plan(plan_forward);
}

void FFTFilter::apply_frequency_cutoff() {
    int N = out.size() * 2 - 2; // Since out has N/2 + 1 elements
    double nyquist = sample_rate / 2.0;

    for (int i = 0; i < out.size(); ++i) {
        double freq = static_cast<double>(i) * sample_rate / N;
        if (freq > cutoff_frequency) {
            out[i][0] = 0; // Zero real part
            out[i][1] = 0; // Zero imaginary part
        }
    }
}

void FFTFilter::backward_transform() {
    int N = wf.size();
    std::vector<double> in(N);

    // Create FFTW plan for inverse FFT
    fftw_plan plan_backward = fftw_plan_dft_c2r_1d(N, const_cast<fftw_complex*>(out.data()), in.data(), FFTW_ESTIMATE);

    // Execute the inverse FFT
    fftw_execute(plan_backward);

    // Normalize the output
    for (int i = 0; i < N; ++i) {
        wf[i] = static_cast<int16_t>(in[i] / N); // Scale down by N
    }

    // Cleanup
    fftw_destroy_plan(plan_backward);
}


// #include "FFTFilter.h"
// #include <iostream>
// #include <vector>

// int main() {
//     // Example signal
//     std::vector<int16_t> input_signal = { /* Your signal data here */ };    
//     double sample_rate = 44100; // Sample rate in Hz
//     double cutoff_frequency = 1000; // Initial cutoff frequency in Hz

//     // Create an instance of FFTFilter
//     FFTFilter filter(input_signal, sample_rate, cutoff_frequency);

//     // Get and display filtered signal
//     const auto& filtered_signal = filter.get_signal();
//     std::cout << "Filtered Signal: ";
//     for (const auto& value : filtered_signal) {
//         std::cout << value << " ";
//     }
//     std::cout << std::endl;

//     // Set a new cutoff frequency and reapply filtering
//     double new_cutoff_frequency = 500; // New cutoff frequency in Hz
//     filter.set_cutoff_frequency(new_cutoff_frequency);

//     // Get and display filtered signal after changing cutoff frequency
//     const auto& updated_filtered_signal = filter.get_signal();
//     std::cout << "Updated Filtered Signal: ";
//     for (const auto& value : updated_filtered_signal) {
//         std::cout << value << " ";
//     }
//     std::cout << std::endl;

//     return 0;
// }