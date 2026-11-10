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
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void timerCallback() override;
    void refreshInstanceInfo();
    void refreshLevels();
    void updateTargetVisibility();
    void toggleAnalysis();
    void toggleAnalyzeAll();
    void refreshBatchStatus();

    RoughStartProcessor& rsProcessor;

    juce::Label titleLabel, trackLabel;

    juce::Label modeLabel { {}, "Mode" }, targetLabel { {}, "Target" }, ceilingLabel { {}, "Peak ceiling" };
    juce::ComboBox modeBox;
    juce::Slider targetPeakSlider, targetRmsSlider, ceilingSlider;

    juce::TextButton analyzeButton { "Analyze" };
    juce::Label statusLabel;

    juce::Label measuredLabel, autoLabel, totalLabel;

    juce::Label trimLabel { {}, "Trim" };
    juce::Slider trimSlider;

    juce::Label countLabel, batchLabel;
    juce::TextButton analyzeAllButton { "Analyze All" };

    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<SliderAttachment> targetPeakAttachment, targetRmsAttachment, ceilingAttachment, trimAttachment;

    juce::uint32 lastSeenChange = 0;
    int lastModeIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoughStartEditor)
};
