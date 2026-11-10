#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "OverviewComponent.h"
#include "PresetManager.h"
#include "InlinePrompt.h"

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

    enum class View { track = 0, overview = 1 };

    void timerCallback() override;
    void setView (View newView);

    void refreshInstanceInfo();
    void refreshLevels();
    void refreshBatchStatus();
    void updateTargetVisibility();
    void toggleAnalysis();
    void toggleAnalyzeAll();

    // Presets
    void reloadPresets();
    void syncPresetSelection();
    void loadSelectedPreset();
    void showPresetMenu();
    void savePresetAs();
    void importPreset();
    void exportSelectedPreset();
    void deleteSelectedPreset();
    void applySettingsToAll();
    void showNotice (const juce::String& text);
    juce::String getSelectedPresetName() const;

    RoughStartProcessor& rsProcessor;
    View view = View::track;

    // Kop
    juce::Label titleLabel, trackLabel;
    juce::TextButton trackViewButton { "Track" }, overviewButton { "Overview" };

    // Track-weergave
    juce::Label presetLabel { {}, "Preset" };
    juce::ComboBox presetBox;
    juce::TextButton presetMenuButton { "Presets..." };

    juce::Label modeLabel { {}, "Mode" }, targetLabel { {}, "Target" }, ceilingLabel { {}, "Peak ceiling" };
    juce::ComboBox modeBox;
    juce::Slider targetPeakSlider, targetRmsSlider, ceilingSlider;

    juce::TextButton analyzeButton { "Analyze" };
    juce::Label statusLabel;
    juce::Label measuredLabel, autoLabel, totalLabel;

    juce::Label trimLabel { {}, "Trim" };
    juce::Slider trimSlider;

    std::vector<juce::Component*> trackViewComponents;

    // Overzicht
    OverviewComponent overview;

    // Onderbalk (beide weergaven)
    juce::Label countLabel, batchLabel;
    juce::TextButton analyzeAllButton { "Analyze All" };

    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<SliderAttachment> targetPeakAttachment, targetRmsAttachment, ceilingAttachment, trimAttachment;

    roughstart::PresetManager presets;
    std::vector<std::pair<juce::String, roughstart::TargetSettings>> presetCache;
    std::unique_ptr<juce::FileChooser> fileChooser;

    InlinePrompt prompt;   // als laatste: ligt boven alles

    juce::String noticeText;
    double noticeUntilMs = 0.0;

    juce::uint32 lastSeenChange = 0;
    int lastModeIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoughStartEditor)
};
