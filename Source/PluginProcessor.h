#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "InstanceRegistry.h"

class RoughStartProcessor final : public juce::AudioProcessor
{
public:
    RoughStartProcessor();
    ~RoughStartProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    void updateTrackProperties (const TrackProperties& properties) override;

    //==============================================================================
    InstanceRegistry& getRegistry() noexcept { return *registry; }

    int getInstanceNumber() const noexcept { return instanceNumber; }

    /** Spoornaam van de host, of "Instance N" als de host die niet doorgeeft. */
    juce::String getDisplayName() const;

    /** Wordt door de registry aangeroepen; mag van elke thread komen. */
    void receivePing() noexcept { ++pingCount; }
    juce::uint32 getPingCount() const noexcept { return pingCount.load(); }

private:
    juce::SharedResourcePointer<InstanceRegistry> registry;
    int instanceNumber = 0;

    mutable juce::CriticalSection nameLock;
    juce::String trackName;

    std::atomic<juce::uint32> pingCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoughStartProcessor)
};
