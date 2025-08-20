#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component,
	public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button*) override;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::TextButton loadDryButton{ "Load Dry File" };
    juce::TextButton loadWetButton{ "Load Wet File" };
    juce::TextButton generateIRButton{ "Generate IR" };
    juce::TextButton saveIRButton{ "Save IR" };
    std::unique_ptr<juce::FileChooser> fileChooserDry;
    std::unique_ptr<juce::FileChooser> fileChooserWet;
    std::unique_ptr<juce::FileChooser> fileChooserSave;


    juce::Label statusLabel;
    juce::AudioBuffer<float> dryBuffer, wetBuffer;
    juce::AudioBuffer<float> irBuffer;

    juce::File dryFile;
    juce::File wetFile;
   
    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
