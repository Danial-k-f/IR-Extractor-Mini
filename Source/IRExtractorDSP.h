/*
  ==============================================================================

    IRExtractorDSP.h
    Created: 12 Aug 2025 3:23:49pm
    Author:  dania

  ==============================================================================
*/

#pragma once



#include <JuceHeader.h>

class IRExtractorDSP
{
public:
    static juce::AudioBuffer<float> performDeconvolution(const juce::AudioBuffer<float>& dry,
        const juce::AudioBuffer<float>& wet,
        double sampleRate);
};
