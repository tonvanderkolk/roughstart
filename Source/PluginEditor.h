#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class RoughStartEditor final : public juce::AudioProcessorEditor,
                               private juce::Timer
{
public:
    explicit RoughStartEditor (RoughStartProcessor&);
    ~RoughStartEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshInstanceInfo();

    RoughStartProcessor& processor;

    juce::Label titleLabel, countLabel, namesLabel;
    juce::TextButton pingButton { "Ping all" };

    juce::Rectangle<int> indicatorBounds;
    juce::uint32 lastSeenPing = 0;
    juce::uint32 lastSeenChange = 0;
    double pingLitUntilMs = 0.0;
    bool indicatorLit = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoughStartEditor)
};
