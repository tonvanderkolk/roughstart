#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "OverviewComponent.h"
#include "PresetManager.h"
#include "InlinePrompt.h"
#include "HelpOverlay.h"
#include "MeterView.h"
#include "RoughStartLookAndFeel.h"

class RoughStartEditor final : public juce::AudioProcessorEditor,
                               private juce::Timer
{
public:
    explicit RoughStartEditor (RoughStartProcessor&);
    ~RoughStartEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    enum class View { track = 0, overview = 1 };

    struct PresetEntry
    {
        juce::String name;
        roughstart::TargetSettings settings;
        bool factory = false;
    };


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
    const PresetEntry* getSelectedEntry() const;
    void setSelectedAsDefault();

    RoughStartProcessor& rsProcessor;
    View view = View::track;
    RoughStartLookAndFeel lookAndFeel;   // vóór alle componenten: leeft het langst

    void setMode (int modeIndex);

    // Kop
    juce::Label titleLabel, trackLabel;
    juce::TextButton trackViewButton { "Track" }, overviewButton { "Overview" };
    juce::TextButton helpButton { "?" };

    // Track-weergave
    juce::Label presetLabel { {}, "Preset" };
    juce::ComboBox presetBox;
    juce::TextButton presetMenuButton { "Presets..." };

    juce::Label modeLabel { {}, "Mode" }, targetLabel { {}, "Target" }, ceilingLabel { {}, "Peak ceiling" };
    juce::ComboBox modeBox;
    juce::TextButton applyAllButton { "Apply to all" };
    juce::Slider targetPeakSlider, targetRmsSlider, ceilingSlider;

    juce::TextButton analyzeButton { "Analyze" };
    juce::Label statusLabel;
    juce::Label measuredLabel, autoLabel, totalLabel;
    juce::Label autoCaption { {}, "Auto" }, totalCaption { {}, "Total gain" }, meterNote;
    juce::TextButton peakButton { "Peak" }, rmsButton { "RMS" };
    MeterView meter;

    juce::Label trimLabel { {}, "Trim" };
    juce::Slider trimSlider;

    std::vector<juce::Component*> trackViewComponents;

    // Overzicht
    OverviewComponent overview;

    // Onderbalk (beide weergaven)
    juce::Label countLabel, batchLabel;
    juce::TextButton analyzeAllButton { "Analyze All" };
    juce::TextButton undoButton { "Undo" }, redoButton { "Redo" };
    void undoTrim (bool redo);

    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<SliderAttachment> targetPeakAttachment, targetRmsAttachment, ceilingAttachment, trimAttachment;

    roughstart::PresetManager presets;
    std::vector<PresetEntry> presetCache;
    std::unique_ptr<juce::FileChooser> fileChooser;

    HelpOverlay help;
    juce::TooltipWindow tooltipWindow { this, 600 };   // kind van de editor, geen los venster
    InlinePrompt prompt;   // als laatste: ligt boven alles

    juce::String noticeText;
    double noticeUntilMs = 0.0;

    juce::uint32 lastSeenChange = 0;
    juce::Rectangle<int> resultCard, settingsCard;
    int trimDividerY = 0;
    int lastModeIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoughStartEditor)
};
