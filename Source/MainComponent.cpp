#include "MainComponent.h"
#include "IRExtractorDSP.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(loadDryButton);
    addAndMakeVisible(loadWetButton);
    addAndMakeVisible(generateIRButton);
    addAndMakeVisible(saveIRButton);
    addAndMakeVisible(statusLabel);

    loadDryButton.addListener(this);
    loadWetButton.addListener(this);
    generateIRButton.addListener(this);
    saveIRButton.addListener(this);

    setSize(200, 200);
}

MainComponent::~MainComponent() {}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto rowH = 30;

    loadDryButton.setBounds(area.removeFromTop(rowH).removeFromLeft(180));
    loadWetButton.setBounds(area.removeFromTop(rowH).removeFromLeft(180));
    generateIRButton.setBounds(area.removeFromTop(rowH).removeFromLeft(180));
    saveIRButton.setBounds(area.removeFromTop(rowH).removeFromLeft(180));
    statusLabel.setBounds(area);
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &loadDryButton)
    {
        juce::Logger::outputDebugString("Clicked: Load Dry");

        fileChooserDry = std::make_unique<juce::FileChooser>("Select the dry file...", juce::File(), "*.wav");

        fileChooserDry->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                juce::Logger::outputDebugString("Dry FileChooser callback triggered!");
                auto file = fc.getResult();

                if (file.existsAsFile())
                {
                    dryFile = file;
                    juce::AudioFormatManager formatManager;
                    formatManager.registerBasicFormats();

                    if (auto* reader = formatManager.createReaderFor(dryFile))
                    {
                        dryBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                        reader->read(&dryBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
                        delete reader;

                        juce::Logger::outputDebugString("Dry audio loaded: " + dryFile.getFullPathName());
                        statusLabel.setText("Dry file loaded", juce::dontSendNotification);
                    }
                    else
                    {
                        juce::Logger::outputDebugString("Failed to read dry file");
                        statusLabel.setText("Failed to load dry file", juce::dontSendNotification);
                    }
                }
                else
                {
                    statusLabel.setText("No dry file selected", juce::dontSendNotification);
                }

                fileChooserDry = nullptr;
            });
    }
    else if (button == &loadWetButton)
    {
        juce::Logger::outputDebugString("Clicked: Load Wet");

        fileChooserWet = std::make_unique<juce::FileChooser>("Select the wet file...", juce::File(), "*.wav");

        fileChooserWet->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                juce::Logger::outputDebugString("Wet FileChooser callback triggered!");
                auto file = fc.getResult();

                if (file.existsAsFile())
                {
                    wetFile = file;
                    juce::AudioFormatManager formatManager;
                    formatManager.registerBasicFormats();

                    if (auto* reader = formatManager.createReaderFor(wetFile))
                    {
                        wetBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                        reader->read(&wetBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
                        delete reader;

                        juce::Logger::outputDebugString("Wet audio loaded: " + wetFile.getFullPathName());
                        statusLabel.setText("Wet file loaded", juce::dontSendNotification);
                    }
                    else
                    {
                        juce::Logger::outputDebugString("Failed to read wet file");
                        statusLabel.setText("Failed to load wet file", juce::dontSendNotification);
                    }
                }
                else
                {
                    statusLabel.setText("No wet file selected", juce::dontSendNotification);
                }

                fileChooserWet = nullptr;
            });
    }
    else if (button == &generateIRButton)
    {
        if (dryBuffer.getNumSamples() > 0 && wetBuffer.getNumSamples() > 0)
        {
            irBuffer = IRExtractorDSP::performDeconvolution(dryBuffer, wetBuffer, sampleRate);

            //irBuffer.applyGain(5.0f); // Boost the gain


            statusLabel.setText("IR generated", juce::dontSendNotification);
        }
        else
        {
            statusLabel.setText("Please load both files first", juce::dontSendNotification);
        }
    }
    else if (button == &saveIRButton)
    {
        if (irBuffer.getNumSamples() > 0)
        {
            fileChooserSave = std::make_unique<juce::FileChooser>(
                "Save IR as WAV",
                juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                "*.wav");

            fileChooserSave->launchAsync(
                juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc)
                {
                    auto file = fc.getResult();

                    if (!file.existsAsFile() && file != juce::File{})
                    {
                      
                        juce::File outputFile = file;
                        if (!outputFile.hasFileExtension(".wav"))
                            outputFile = outputFile.withFileExtension(".wav");

                        juce::WavAudioFormat wavFormat;

                        std::unique_ptr<juce::FileOutputStream> outputStream(outputFile.createOutputStream());

                        if (outputStream != nullptr && outputStream->openedOk())
                        {
                            juce::AudioFormatWriter* rawWriter = wavFormat.createWriterFor(
                                outputStream.get(),           // stream pointer
                                sampleRate,                   // sample rate (double)
                                1,                            // numChannels
                                24,                           // bitsPerSample
                                {},                           // metadata
                                0);                           // quality (unused)

                            if (rawWriter != nullptr)
                            {
                                std::unique_ptr<juce::AudioFormatWriter> writer(rawWriter);
                                outputStream.release(); // ownership goes to writer

                                writer->writeFromAudioSampleBuffer(irBuffer, 0, irBuffer.getNumSamples());

                                statusLabel.setText("IR saved to: " + outputFile.getFileName(), juce::dontSendNotification);
                                juce::Logger::outputDebugString("Saved IR to: " + outputFile.getFullPathName());
                            }
                            else
                            {
                                statusLabel.setText("Failed to create writer", juce::dontSendNotification);
                            }
                        }
                        else
                        {
                            statusLabel.setText("Could not create output file", juce::dontSendNotification);
                        }
                    }
                    else
                    {
                        statusLabel.setText("No file selected", juce::dontSendNotification);
                    }

                    fileChooserSave = nullptr;
                });
        }
        else
        {
            statusLabel.setText("No IR to save", juce::dontSendNotification);
        }
    }

}
