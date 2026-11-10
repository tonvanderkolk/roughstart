#include "PluginEditor.h"
#include "UiStyle.h"

using namespace roughstart;

namespace
{
    constexpr int trackWidth = 440, trackHeight = 490;
    constexpr int overviewWidth = 720, overviewHeight = 480;
    constexpr double noticeMs = 3500.0;

    void styleValueLabel (juce::Label& l, float size)
    {
        l.setFont (juce::FontOptions (size));
        l.setJustificationType (juce::Justification::centredLeft);
    }

    void styleSlider (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 72, 22);
        s.setColour (juce::Slider::thumbColourId, ui::accent);
        s.setColour (juce::Slider::trackColourId, ui::accent.withAlpha (0.5f));
        s.setScrollWheelEnabled (false);   // voorkomt per ongeluk verschuiven bij scrollen
    }

    enum PresetMenuItem { saveItem = 1, applyAllItem, importItem, exportItem, deleteItem, revealItem, resetItem };
}

//==============================================================================
RoughStartEditor::RoughStartEditor (RoughStartProcessor& p)
    : AudioProcessorEditor (&p), rsProcessor (p), overview (p)
{
    // Kop
    titleLabel.setText ("RoughStart  v" JucePlugin_VersionString, juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    trackLabel.setColour (juce::Label::textColourId, ui::dimText);
    styleValueLabel (trackLabel, 13.0f);
    addAndMakeVisible (trackLabel);

    for (auto* b : { &trackViewButton, &overviewButton })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonOnColourId, ui::accent.withAlpha (0.35f));
        addAndMakeVisible (*b);
    }
    trackViewButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    overviewButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    trackViewButton.onClick = [this] { setView (View::track); };
    overviewButton.onClick  = [this] { setView (View::overview); };

    // Labels
    for (auto* l : { &presetLabel, &modeLabel, &targetLabel, &ceilingLabel, &trimLabel })
        l->setColour (juce::Label::textColourId, ui::dimText);

    // Presets
    presetBox.setTextWhenNothingSelected ("(custom)");
    presetBox.setTextWhenNoChoicesAvailable ("(no presets yet)");
    presetBox.onChange = [this] { loadSelectedPreset(); };
    presetMenuButton.onClick = [this] { showPresetMenu(); };

    // Meetmethode, doel, plafond
    modeBox.addItemList ({ "Peak", "RMS" }, 1);
    modeAttachment = std::make_unique<ComboAttachment> (rsProcessor.parameters, "mode", modeBox);

    styleSlider (targetPeakSlider);
    styleSlider (targetRmsSlider);
    styleSlider (ceilingSlider);
    targetPeakAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "targetPeak", targetPeakSlider);
    targetRmsAttachment  = std::make_unique<SliderAttachment> (rsProcessor.parameters, "targetRms", targetRmsSlider);
    ceilingAttachment    = std::make_unique<SliderAttachment> (rsProcessor.parameters, "ceiling", ceilingSlider);

    // Analyse
    analyzeButton.onClick = [this] { toggleAnalysis(); };
    statusLabel.setColour (juce::Label::textColourId, ui::dimText);
    styleValueLabel (statusLabel, 13.0f);
    styleValueLabel (measuredLabel, 14.0f);
    styleValueLabel (autoLabel, 14.0f);
    styleValueLabel (totalLabel, 14.0f);

    // Trim
    styleSlider (trimSlider);
    trimSlider.setDoubleClickReturnValue (true, 0.0);
    trimAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "trim", trimSlider);

    trackViewComponents = { &presetLabel, &presetBox, &presetMenuButton,
                            &modeLabel, &modeBox, &targetLabel, &targetPeakSlider, &targetRmsSlider,
                            &ceilingLabel, &ceilingSlider, &analyzeButton, &statusLabel,
                            &measuredLabel, &autoLabel, &trimLabel, &trimSlider, &totalLabel };

    for (auto* c : trackViewComponents)
        addChildComponent (c);

    addChildComponent (overview);

    // Onderbalk
    countLabel.setColour (juce::Label::textColourId, ui::dimText);
    styleValueLabel (countLabel, 13.0f);
    addAndMakeVisible (countLabel);

    batchLabel.setColour (juce::Label::textColourId, ui::dimText);
    styleValueLabel (batchLabel, 13.0f);
    addAndMakeVisible (batchLabel);

    analyzeAllButton.onClick = [this] { toggleAnalyzeAll(); };
    addAndMakeVisible (analyzeAllButton);

    addChildComponent (prompt);

    reloadPresets();
    refreshInstanceInfo();
    refreshLevels();
    refreshBatchStatus();

    setView (rsProcessor.lastEditorView == 1 ? View::overview : View::track);
    startTimerHz (30);
}

RoughStartEditor::~RoughStartEditor()
{
    stopTimer();
}

//==============================================================================
void RoughStartEditor::setView (View newView)
{
    view = newView;
    rsProcessor.lastEditorView = (int) newView;

    const bool isTrack = newView == View::track;

    for (auto* c : trackViewComponents)
        c->setVisible (isTrack);

    lastModeIndex = -1;
    if (isTrack)
        updateTargetVisibility();

    overview.setVisible (! isTrack);

    trackViewButton.setToggleState (isTrack, juce::dontSendNotification);
    overviewButton.setToggleState (! isTrack, juce::dontSendNotification);

    if (isTrack)
        setSize (trackWidth, trackHeight);
    else
        setSize (overviewWidth, overviewHeight);

    resized();
    repaint();
}

void RoughStartEditor::paint (juce::Graphics& g)
{
    g.fillAll (ui::background);

    if (view == View::track)
    {
        g.setColour (ui::panel);
        g.fillRoundedRectangle (getLocalBounds().reduced (10).withTrimmedTop (60).withTrimmedBottom (78).toFloat(), 6.0f);
    }
}

void RoughStartEditor::resized()
{
    prompt.setBounds (getLocalBounds());

    auto area = getLocalBounds().reduced (16, 12);

    // Kop: titel + weergaveknoppen, daaronder de spoornaam
    auto top = area.removeFromTop (28);
    overviewButton.setBounds (top.removeFromRight (90).reduced (0, 2));
    trackViewButton.setBounds (top.removeFromRight (70).reduced (0, 2));
    titleLabel.setBounds (top);
    trackLabel.setBounds (area.removeFromTop (20));

    // Onderbalk: Analyze All voor alle instances
    auto bottom = area.removeFromBottom (58);
    auto bottomRow = bottom.removeFromTop (32);
    analyzeAllButton.setBounds (bottomRow.removeFromRight (130));
    countLabel.setBounds (bottomRow);
    batchLabel.setBounds (bottom);

    area.removeFromBottom (14);
    area.removeFromTop (12);

    if (view == View::overview)
    {
        overview.setBounds (area.withTrimmedLeft (-6).withTrimmedRight (-6));
        return;
    }

    area = area.reduced (10, 0);
    constexpr int labelWidth = 100, rowHeight = 28, gap = 8;

    auto row = area.removeFromTop (rowHeight);
    presetLabel.setBounds (row.removeFromLeft (labelWidth));
    presetMenuButton.setBounds (row.removeFromRight (90));
    row.removeFromRight (6);
    presetBox.setBounds (row);
    area.removeFromTop (gap);

    row = area.removeFromTop (rowHeight);
    modeLabel.setBounds (row.removeFromLeft (labelWidth));
    modeBox.setBounds (row.removeFromLeft (120));
    area.removeFromTop (gap);

    row = area.removeFromTop (rowHeight);
    targetLabel.setBounds (row.removeFromLeft (labelWidth));
    targetPeakSlider.setBounds (row);
    targetRmsSlider.setBounds (row);
    area.removeFromTop (gap);

    row = area.removeFromTop (rowHeight);
    ceilingLabel.setBounds (row.removeFromLeft (labelWidth));
    ceilingSlider.setBounds (row);
    area.removeFromTop (gap * 2);

    row = area.removeFromTop (rowHeight + 4);
    analyzeButton.setBounds (row.removeFromLeft (130));
    row.removeFromLeft (10);
    statusLabel.setBounds (row);
    area.removeFromTop (gap);

    measuredLabel.setBounds (area.removeFromTop (22));
    autoLabel.setBounds (area.removeFromTop (22));
    area.removeFromTop (gap);

    row = area.removeFromTop (rowHeight);
    trimLabel.setBounds (row.removeFromLeft (labelWidth));
    trimSlider.setBounds (row);
    area.removeFromTop (gap);

    totalLabel.setBounds (area.removeFromTop (22));
}

//==============================================================================
void RoughStartEditor::timerCallback()
{
    const auto change = rsProcessor.getRegistry().getChangeCount();
    if (change != lastSeenChange)
    {
        lastSeenChange = change;
        refreshInstanceInfo();
    }

    if (view == View::track)
    {
        updateTargetVisibility();
        refreshLevels();
        syncPresetSelection();
    }

    refreshBatchStatus();
}

void RoughStartEditor::refreshInstanceInfo()
{
    countLabel.setText ("Instances in project: " + juce::String (rsProcessor.getRegistry().getNumInstances()),
                        juce::dontSendNotification);
    trackLabel.setText (rsProcessor.getDisplayName(), juce::dontSendNotification);
}

void RoughStartEditor::updateTargetVisibility()
{
    const int modeIndex = rsProcessor.getMode() == RoughStartProcessor::Mode::rms ? 1 : 0;
    if (modeIndex == lastModeIndex)
        return;

    lastModeIndex = modeIndex;
    targetPeakSlider.setVisible (modeIndex == 0);
    targetRmsSlider.setVisible (modeIndex == 1);
    targetLabel.setText (modeIndex == 0 ? "Target (peak)" : "Target (RMS)", juce::dontSendNotification);
}

//==============================================================================
void RoughStartEditor::toggleAnalysis()
{
    if (rsProcessor.isAnalyzing())
        rsProcessor.stopAnalysis();
    else
        rsProcessor.startAnalysis();

    refreshLevels();
}

void RoughStartEditor::refreshLevels()
{
    using State = RoughStartProcessor::AnalysisState;
    using Result = RoughStartProcessor::AnalysisResult;

    const auto state = rsProcessor.getAnalysisState();
    const bool analyzing = state != State::idle;

    juce::String status;
    switch (state)
    {
        case State::waitingForPlayback: status = "Waiting for playback..."; break;
        case State::measuring:          status = "Measuring... stops when playback stops."; break;
        case State::idle:
            switch (rsProcessor.getLastAnalysisResult())
            {
                case Result::stored:   status = "Measurement stored."; break;
                case Result::noSignal: status = "No signal measured, Auto unchanged."; break;
                case Result::none:     break;
            }
            break;
    }
    statusLabel.setText (status, juce::dontSendNotification);

    analyzeButton.setButtonText (state == State::waitingForPlayback ? "Cancel" : analyzing ? "Stop" : "Analyze");
    analyzeButton.setColour (juce::TextButton::buttonColourId,
                             analyzing ? ui::analyzeColour : getLookAndFeel().findColour (juce::TextButton::buttonColourId));

    const auto m = analyzing ? rsProcessor.getLiveMeasurement() : rsProcessor.getStoredMeasurement();
    const juce::String prefix = analyzing ? "Live: " : "Measured: ";

    if (m.valid)
        measuredLabel.setText (prefix + "peak " + ui::formatDb (m.peakDb, false)
                                   + "  ·  RMS " + ui::formatDb (m.rmsDb, false),
                               juce::dontSendNotification);
    else
        measuredLabel.setText (prefix + (analyzing ? "no signal yet" : "not analyzed yet"),
                               juce::dontSendNotification);

    const auto autoGain = rsProcessor.getAutoGain();
    autoLabel.setText ("Auto: " + ui::formatDb (autoGain.gainDb, true)
                           + (autoGain.limitedByCeiling ? "  (peak at ceiling)" : ""),
                       juce::dontSendNotification);
    autoLabel.setColour (juce::Label::textColourId,
                         autoGain.limitedByCeiling ? ui::info : juce::Colours::white);
    totalLabel.setText ("Total (Auto + Trim): " + ui::formatDb (rsProcessor.getTotalGainDb(), true),
                        juce::dontSendNotification);
}

void RoughStartEditor::toggleAnalyzeAll()
{
    auto& registry = rsProcessor.getRegistry();

    if (registry.getBatchSummary().isRunning())
        registry.stopAll();
    else
        registry.analyzeAll();

    refreshBatchStatus();
    refreshLevels();
}

void RoughStartEditor::refreshBatchStatus()
{
    const auto s = rsProcessor.getRegistry().getBatchSummary();
    const bool running = s.isRunning();

    analyzeAllButton.setButtonText (running ? "Stop All" : "Analyze All");
    analyzeAllButton.setColour (juce::TextButton::buttonColourId,
                                running ? ui::analyzeColour : getLookAndFeel().findColour (juce::TextButton::buttonColourId));

    juce::String text;

    if (juce::Time::getMillisecondCounterHiRes() < noticeUntilMs)
        text = noticeText;
    else if (s.total == 0)
        text = {};
    else if (s.waiting > 0 && s.measuring == 0)
        text = "Analyze All: " + juce::String (s.waiting) + " tracks waiting for playback...";
    else if (running)
        text = "Analyze All: measuring " + juce::String (s.measuring) + " of " + juce::String (s.total)
               + " tracks... stops when playback stops.";
    else
        text = "Last Analyze All: " + juce::String (s.stored) + " stored"
               + (s.noSignal > 0 ? ", " + juce::String (s.noSignal) + " without signal (Auto unchanged)" : juce::String())
               + ".";

    batchLabel.setText (text, juce::dontSendNotification);
}

void RoughStartEditor::showNotice (const juce::String& text)
{
    noticeText = text;
    noticeUntilMs = juce::Time::getMillisecondCounterHiRes() + noticeMs;
    refreshBatchStatus();
}

//==============================================================================
void RoughStartEditor::reloadPresets()
{
    presetCache.clear();
    presetBox.clear (juce::dontSendNotification);

    int id = 1;
    for (const auto& name : presets.getPresetNames())
    {
        if (auto s = presets.load (name))
        {
            presetCache.emplace_back (name, *s);
            presetBox.addItem (name, id++);
        }
    }

    syncPresetSelection();
}

void RoughStartEditor::syncPresetSelection()
{
    // Toon de preset die precies overeenkomt met de huidige instellingen, anders "(custom)".
    const auto current = rsProcessor.getTargetSettings();

    // Voorkeur voor de preset die al geselecteerd is (bij twee identieke presets).
    const int selectedIndex = presetBox.getSelectedId() - 1;
    if (juce::isPositiveAndBelow (selectedIndex, (int) presetCache.size())
        && presetCache[(size_t) selectedIndex].second == current)
        return;

    int match = 0;
    for (size_t i = 0; i < presetCache.size(); ++i)
    {
        if (presetCache[i].second == current)
        {
            match = (int) i + 1;
            break;
        }
    }

    presetBox.setSelectedId (match, juce::dontSendNotification);
}

juce::String RoughStartEditor::getSelectedPresetName() const
{
    const int index = presetBox.getSelectedId() - 1;
    return juce::isPositiveAndBelow (index, (int) presetCache.size()) ? presetCache[(size_t) index].first
                                                                     : juce::String();
}

void RoughStartEditor::loadSelectedPreset()
{
    const int index = presetBox.getSelectedId() - 1;
    if (juce::isPositiveAndBelow (index, (int) presetCache.size()))
        rsProcessor.applyTargetSettings (presetCache[(size_t) index].second);
}

void RoughStartEditor::showPresetMenu()
{
    const bool hasSelection = getSelectedPresetName().isNotEmpty();

    juce::PopupMenu menu;
    menu.addItem (saveItem, "Save current settings as preset...");
    menu.addItem (applyAllItem, "Apply current settings to all tracks");
    menu.addSeparator();
    menu.addItem (importItem, "Import preset...");
    menu.addItem (exportItem, "Export selected preset...", hasSelection);
    menu.addItem (deleteItem, "Delete selected preset", hasSelection);
    menu.addSeparator();
    menu.addItem (revealItem, "Show presets folder");
    menu.addItem (resetItem, "Reset settings to defaults");

    juce::Component::SafePointer<RoughStartEditor> safeThis (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (presetMenuButton),
                        [safeThis] (int result)
                        {
                            if (safeThis == nullptr)
                                return;

                            switch (result)
                            {
                                case saveItem:     safeThis->savePresetAs(); break;
                                case applyAllItem: safeThis->applySettingsToAll(); break;
                                case importItem:   safeThis->importPreset(); break;
                                case exportItem:   safeThis->exportSelectedPreset(); break;
                                case deleteItem:   safeThis->deleteSelectedPreset(); break;
                                case revealItem:
                                    safeThis->presets.getDirectory().createDirectory();
                                    safeThis->presets.getDirectory().revealToUser();
                                    break;
                                case resetItem:
                                    safeThis->rsProcessor.applyTargetSettings (TargetSettings {});
                                    break;
                                default: break;
                            }
                        });
}

void RoughStartEditor::savePresetAs()
{
    // Het prompt is een kind van de editor, dus 'this' leeft zolang de callback kan komen.
    prompt.showTextInput ("Save preset", "Name for this preset:", getSelectedPresetName(), "Save",
                          [this] (const juce::String& text)
                          {
                              const auto name = PresetManager::sanitiseName (text);
                              if (name.isEmpty())
                                  return;

                              if (presets.save (name, rsProcessor.getTargetSettings()))
                              {
                                  reloadPresets();
                                  showNotice ("Preset \"" + name + "\" saved.");
                              }
                              else
                              {
                                  showNotice ("Could not save preset \"" + name + "\".");
                              }
                          });
}

void RoughStartEditor::importPreset()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Import RoughStart preset",
                                                       juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                       juce::String ("*") + PresetManager::fileExtension);

    juce::Component::SafePointer<RoughStartEditor> safeThis (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [safeThis] (const juce::FileChooser& chooser)
                              {
                                  if (safeThis == nullptr)
                                      return;

                                  const auto file = chooser.getResult();
                                  if (file == juce::File())
                                      return;

                                  const auto name = safeThis->presets.importFile (file);
                                  if (name.isEmpty())
                                  {
                                      safeThis->showNotice ("Not a valid RoughStart preset: " + file.getFileName());
                                      return;
                                  }

                                  safeThis->reloadPresets();
                                  safeThis->showNotice ("Preset \"" + name + "\" imported.");
                              });
}

void RoughStartEditor::exportSelectedPreset()
{
    const auto name = getSelectedPresetName();
    if (name.isEmpty())
        return;

    const auto suggested = juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                               .getChildFile (name + PresetManager::fileExtension);

    fileChooser = std::make_unique<juce::FileChooser> ("Export RoughStart preset", suggested,
                                                       juce::String ("*") + PresetManager::fileExtension);

    juce::Component::SafePointer<RoughStartEditor> safeThis (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting,
                              [safeThis, name] (const juce::FileChooser& chooser)
                              {
                                  if (safeThis == nullptr)
                                      return;

                                  auto file = chooser.getResult();
                                  if (file == juce::File())
                                      return;

                                  if (! file.hasFileExtension (PresetManager::fileExtension))
                                      file = file.withFileExtension (PresetManager::fileExtension);

                                  safeThis->showNotice (safeThis->presets.exportPreset (name, file)
                                                            ? "Preset exported to " + file.getFileName() + "."
                                                            : "Could not export preset.");
                              });
}

void RoughStartEditor::deleteSelectedPreset()
{
    const auto name = getSelectedPresetName();
    if (name.isEmpty())
        return;

    prompt.showConfirm ("Delete preset", "Delete preset \"" + name + "\"?\nThis cannot be undone.", "Delete",
                        [this, name]
                        {
                            if (presets.remove (name))
                            {
                                reloadPresets();
                                showNotice ("Preset \"" + name + "\" deleted.");
                            }
                        });
}

void RoughStartEditor::applySettingsToAll()
{
    const int count = rsProcessor.getRegistry().applyTargetSettingsToAll (rsProcessor.getTargetSettings());
    showNotice ("Settings applied to " + juce::String (count) + " tracks.");
}
