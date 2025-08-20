#include "IRExtractorDSP.h"

juce::AudioBuffer<float> IRExtractorDSP::performDeconvolution(const juce::AudioBuffer<float>& dry,
    const juce::AudioBuffer<float>& wet,
    double sampleRate)
{
    if (dry.getNumChannels() == 0 || wet.getNumChannels() == 0)
    {
        DBG("Error: One of the buffers has zero channels.");
        return {};
    }

    const int drySamples = dry.getNumSamples();
    const int wetSamples = wet.getNumSamples();
    //const double sampleRate = 44100.0; // ? ?? ???????? ?? ??? ???? ??
    const int desiredSamples = std::max(std::max(dry.getNumSamples(), wet.getNumSamples()),
        static_cast<int>(sampleRate * 4.0));

    // Desired output length in seconds
    const double irSeconds = 2.0;
    const int desiredOutputSamples = static_cast<int>(sampleRate * irSeconds);

    // Linear convolution length = N + M - 1
    const int convLength = drySamples + wetSamples - 1;
    const int fftOrder = static_cast<int>(std::ceil(std::log2(convLength)));
    const int fftSize = 1 << fftOrder;

    // Allocate zero-padded buffers (twice fftSize for real-to-complex transform)
    juce::HeapBlock<float> dryTime(fftSize * 2, true);
    juce::HeapBlock<float> wetTime(fftSize * 2, true);

    std::memcpy(dryTime.get(), dry.getReadPointer(0), sizeof(float) * drySamples);
    std::memcpy(wetTime.get(), wet.getReadPointer(0), sizeof(float) * wetSamples);

    juce::dsp::FFT fft(fftOrder);

    fft.performRealOnlyForwardTransform(dryTime.get());
    fft.performRealOnlyForwardTransform(wetTime.get());

    auto* dryFreq = reinterpret_cast<juce::dsp::Complex<float>*>(dryTime.get());
    auto* wetFreq = reinterpret_cast<juce::dsp::Complex<float>*>(wetTime.get());

    // Deconvolution in frequency domain
    juce::HeapBlock<float> irFreqTime(fftSize * 2, true);
    auto* irFreq = reinterpret_cast<juce::dsp::Complex<float>*>(irFreqTime.get());

    const float epsilon = 1e-8f;

    for (int i = 0; i < fftSize / 2; ++i)
    {
        float denom = dryFreq[i].real() * dryFreq[i].real() + dryFreq[i].imag() * dryFreq[i].imag() + epsilon;
        float real = (wetFreq[i].real() * dryFreq[i].real() + wetFreq[i].imag() * dryFreq[i].imag()) / denom;
        float imag = (wetFreq[i].imag() * dryFreq[i].real() - wetFreq[i].real() * dryFreq[i].imag()) / denom;
        irFreq[i] = { real, imag };
    }

    fft.performRealOnlyInverseTransform(irFreqTime.get());

    // Create output IR buffer with desired trimmed length
    juce::AudioBuffer<float> result(1, desiredOutputSamples);
    result.clear();

    result.copyFrom(0, 0, irFreqTime.get(), desiredOutputSamples);

    // Optional: Apply Hann window to avoid sharp cutoff
    /*for (int i = 0; i < desiredOutputSamples; ++i)
    {
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (desiredOutputSamples - 1)));
        result.getWritePointer(0)[i] *= window;
    }*/

    // Optional: Boost gain (~+6 dB)
    /*result.applyGain(2.0f);*/

    return result;
}
