#include "PluginEditor.h"
#include "UiStyle.h"
#include "BinaryData.h"

using namespace roughstart;

namespace
{
    constexpr int trackWidth = 460, trackHeight = 540;
    constexpr int overviewWidth = 760, overviewHeight = 500;
    constexpr int headerHeight = 58, footerHeight = 76;
    constexpr double noticeMs = 3500.0;

    void styleLabel (juce::Label& l, float size, juce::Colour colour, bool bold = false,
                     juce::Justification just = juce::Justification::centredLeft)
    {
        l.setFont (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
        l.setColour (juce::Label::textColourId, colour);
        l.setJustificationType (just);
        l.setInterceptsMouseClicks (false, false);
    }

    void styleValueBox (juce::Slider& s, const juce::String& name)
    {
        s.setName (name);
        s.setSliderStyle (juce::Slider::LinearBar);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setScrollWheelEnabled (false);   // voorkomt per ongeluk verschuiven bij scrollen
        s.setMouseDragSensitivity (300);
        s.setVelocityBasedMode (false);
    }

    enum PresetMenuItem { saveItem = 1, applyAllItem, defaultItem, importItem, exportItem, deleteItem, revealItem, resetItem };
}

//==============================================================================
RoughStartEditor::RoughStartEditor (RoughStartProcessor& p)
    : AudioProcessorEditor (&p), rsProcessor (p), overview (p),
      help (juce::String::fromUTF8 (BinaryData::MANUAL_md, BinaryData::MANUAL_mdSize))
{
    setLookAndFeel (&lookAndFeel);

    // Kop: merkje + naam (getekend in paint), spoornaam, weergave-tabs, help
    styleLabel (trackLabel, 12.5f, ui::dimText);
    addAndMakeVisible (trackLabel);

    for (auto* b : { &trackViewButton, &overviewButton })
    {
        b->setClickingTogglesState (false);
        addAndMakeVisible (*b);
    }
    trackViewButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    overviewButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    trackViewButton.onClick = [this] { setView (View::track); };
    overviewButton.onClick  = [this] { setView (View::overview); };

    helpButton.onClick = [this] { help.setVisible (! help.isVisible()); help.toFront (false); };
    addAndMakeVisible (helpButton);

    // Resultaat: meting, Auto en meter
    styleLabel (measuredLabel, 12.5f, ui::dimText);
    styleLabel (autoCaption, 12.5f, ui::dimText, false, juce::Justification::bottomRight);
    styleLabel (autoLabel, 26.0f, ui::text, true, juce::Justification::centredRight);
    styleLabel (meterNote, 12.0f, ui::info, false, juce::Justification::centredRight);

    // Trim (grote draaiknop) en totaal
    trimSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    trimSlider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    trimSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    trimSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 96, 28);
    trimSlider.setDoubleClickReturnValue (true, 0.0);
    trimSlider.setScrollWheelEnabled (false);
    trimSlider.setMouseDragSensitivity (220);
    trimAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "trim", trimSlider);
    styleLabel (trimLabel, 12.5f, ui::dimText);
    styleLabel (totalCaption, 12.5f, ui::dimText, false, juce::Justification::bottomRight);
    styleLabel (totalLabel, 20.0f, ui::text, true, juce::Justification::centredRight);

    // Instellingen: preset, Peak/RMS, doel, plafond, Apply to all
    presetBox.setTextWhenNothingSelected ("(custom)");
    presetBox.setTextWhenNoChoicesAvailable ("(no presets yet)");
    presetBox.onChange = [this] { loadSelectedPreset(); };
    presetMenuButton.setButtonText ("Presets");
    presetMenuButton.onClick = [this] { showPresetMenu(); };
    applyAllButton.onClick = [this] { applySettingsToAll(); };

    modeBox.addItemList ({ "Peak", "RMS" }, 1);   // onzichtbaar; koppelt de parameter
    modeAttachment = std::make_unique<ComboAttachment> (rsProcessor.parameters, "mode", modeBox);
    peakButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    rmsButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    peakButton.onClick = [this] { setMode (0); };
    rmsButton.onClick  = [this] { setMode (1); };

    styleValueBox (targetPeakSlider, "Target");
    styleValueBox (targetRmsSlider, "Target");
    styleValueBox (ceilingSlider, "Ceiling");
    targetPeakAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "targetPeak", targetPeakSlider);
    targetRmsAttachment  = std::make_unique<SliderAttachment> (rsProcessor.parameters, "targetRms", targetRmsSlider);
    ceilingAttachment    = std::make_unique<SliderAttachment> (rsProcessor.parameters, "ceiling", ceilingSlider);

    // Analyse
    analyzeButton.setButtonText ("Analyze track");
    analyzeButton.onClick = [this] { toggleAnalysis(); };
    styleLabel (statusLabel, 12.0f, ui::dimText);

    // Tooltips
    helpButton.setTooltip ("Open the manual: how RoughStart works.");
    trackViewButton.setTooltip ("Settings and result for this track.");
    overviewButton.setTooltip ("All RoughStart tracks in this project in one table.");
    presetBox.setTooltip ("Choose a preset. Shows (custom) when the settings match no preset.");
    presetMenuButton.setTooltip ("Save, import, export or delete presets, choose the default for new instances.");
    peakButton.setTooltip ("Peak: aim the loudest moment of the track at the target.");
    rmsButton.setTooltip ("RMS: aim the average level of the track at the target. Closer to how loud it sounds.");
    applyAllButton.setTooltip ("Copy Mode, both targets and Peak ceiling to every RoughStart in the project. Measurements and Trim stay per track.");
    targetPeakSlider.setTooltip ("Level the loudest moment should end up at. Drag left or right; double-click to type.");
    targetRmsSlider.setTooltip ("Average level the track should end up at. Drag left or right; double-click to type.");
    ceilingSlider.setTooltip ("Auto never raises the track's peak above this level.");
    analyzeButton.setTooltip ("Measure only this track while Logic plays. Stops by itself when playback stops.");
    trimSlider.setTooltip ("Your own adjustment on top of Auto. Drag up or down; double-click for 0 dB. Can be automated.");
    analyzeAllButton.setTooltip ("Measure every RoughStart track at once during the next playback.");
    meter.setTooltip ("Grey: level before RoughStart. Green: level after Auto. Dashed: target. Blue: peak ceiling. White tick: peak after Auto.");

    trackViewComponents = { &measuredLabel, &autoCaption, &autoLabel, &meter, &meterNote,
                            &trimLabel, &trimSlider, &totalCaption, &totalLabel,
                            &presetBox, &presetMenuButton, &peakButton, &rmsButton,
                            &targetPeakSlider, &targetRmsSlider, &ceilingSlider, &applyAllButton,
                            &analyzeButton, &statusLabel };

    for (auto* c : trackViewComponents)
        addChildComponent (c);

    addChildComponent (overview);

    // Onderbalk
    analyzeAllButton.getProperties().set ("primary", true);
    analyzeAllButton.onClick = [this] { toggleAnalyzeAll(); };
    addAndMakeVisible (analyzeAllButton);
    styleLabel (batchLabel, 12.0f, ui::dimText);
    addAndMakeVisible (batchLabel);

    addChildComponent (help);
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
    setLookAndFeel (nullptr);
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

void RoughStartEditor::setMode (int modeIndex)
{
    modeBox.setSelectedId (modeIndex + 1, juce::sendNotificationSync);
    updateTargetVisibility();
}

void RoughStartEditor::paint (juce::Graphics& g)
{
    g.fillAll (ui::background);

    // Kop: merkje en naam
    ui::drawLogo (g, { 18.0f, 13.0f, 18.0f, 20.0f });
    g.setColour (ui::text);
    g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    g.drawText ("RoughStart", 44, 10, 140, 26, juce::Justification::centredLeft);

    g.setColour (ui::border);
    g.drawHorizontalLine (headerHeight, 0.0f, (float) getWidth());
    g.drawHorizontalLine (getHeight() - footerHeight, 0.0f, (float) getWidth());

    if (view == View::track)
    {
        // Twee kaarten: resultaat (meter + Trim) en instellingen
        for (auto r : { resultCard, settingsCard })
        {
            g.setColour (ui::panel);
            g.fillRoundedRectangle (r.toFloat(), 8.0f);
            g.setColour (ui::border.withAlpha (0.7f));
            g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 8.0f, 1.0f);
        }

        g.setColour (ui::border);
        g.drawHorizontalLine (trimDividerY, (float) resultCard.getX() + 14.0f, (float) resultCard.getRight() - 14.0f);

        g.setColour (ui::faintText);
        g.setFont (juce::FontOptions (11.5f));
        g.drawText ("Settings", settingsCard.getX() + 14, settingsCard.getY() + 8, 120, 14, juce::Justification::centredLeft);
    }
}

void RoughStartEditor::resized()
{
    prompt.setBounds (getLocalBounds());
    help.setBounds (getLocalBounds());

    // Kop
    auto header = getLocalBounds().removeFromTop (headerHeight).reduced (16, 0);
    auto topRow = header.removeFromTop (46).withTrimmedTop (10).withHeight (26);
    helpButton.setBounds (topRow.removeFromRight (26));
    topRow.removeFromRight (8);
    overviewButton.setBounds (topRow.removeFromRight (82));
    trackViewButton.setBounds (topRow.removeFromRight (64));
    trackLabel.setBounds (getLocalBounds().removeFromTop (headerHeight).withTrimmedTop (34).withTrimmedLeft (44).withTrimmedRight (16));

    // Onderbalk
    auto footer = getLocalBounds().removeFromBottom (footerHeight).reduced (16, 12);
    auto footerRow = footer.removeFromTop (34);
    analyzeAllButton.setBounds (footerRow.removeFromRight (view == View::track ? 200 : 180));

    if (view == View::track)
    {
        footerRow.removeFromRight (8);
        analyzeButton.setBounds (footerRow);
        footer.removeFromTop (4);
        auto statusRow = footer;
        statusLabel.setBounds (statusRow.removeFromLeft (statusRow.getWidth() / 2));
        batchLabel.setBounds (statusRow);
        batchLabel.setJustificationType (juce::Justification::centredRight);
    }
    else
    {
        batchLabel.setBounds (footerRow);
        batchLabel.setJustificationType (juce::Justification::centredLeft);
    }

    auto area = getLocalBounds().withTrimmedTop (headerHeight).withTrimmedBottom (footerHeight).reduced (12, 12);

    if (view == View::overview)
    {
        overview.setBounds (area);
        return;
    }

    // Resultaatkaart
    resultCard = area.removeFromTop (262);
    auto r = resultCard.reduced (14, 12);

    auto autoRow = r.removeFromTop (38);
    auto autoBox = autoRow.removeFromRight (170);
    autoCaption.setBounds (autoBox.removeFromLeft (44).withTrimmedBottom (8));
    autoLabel.setBounds (autoBox);
    measuredLabel.setBounds (autoRow);

    r.removeFromTop (6);
    meter.setBounds (r.removeFromTop (70));
    meterNote.setBounds (r.removeFromTop (18));

    r.removeFromTop (8);
    trimDividerY = r.getY();
    r.removeFromTop (10);

    auto trimRow = r;
    trimLabel.setBounds (trimRow.removeFromLeft (38));
    trimSlider.setBounds (trimRow.removeFromLeft (190).withSizeKeepingCentre (190, juce::jmin (84, trimRow.getHeight())));
    auto totalBox = trimRow;
    totalCaption.setBounds (totalBox.removeFromTop (totalBox.getHeight() / 2).withTrimmedBottom (2));
    totalLabel.setBounds (totalBox.withHeight (26));

    // Instellingenkaart
    area.removeFromTop (10);
    settingsCard = area;
    auto s = settingsCard.reduced (14, 0).withTrimmedTop (28).withTrimmedBottom (12);

    auto row = s.removeFromTop (30);
    applyAllButton.setBounds (row.removeFromRight (96));
    row.removeFromRight (6);
    presetMenuButton.setBounds (row.removeFromRight (74));
    row.removeFromRight (6);
    presetBox.setBounds (row);

    s.removeFromTop (8);
    row = s.removeFromTop (30);
    peakButton.setBounds (row.removeFromLeft (52));
    rmsButton.setBounds (row.removeFromLeft (52));
    row.removeFromLeft (8);
    const int half = (row.getWidth() - 6) / 2;
    targetPeakSlider.setBounds (row.removeFromLeft (half));
    targetRmsSlider.setBounds (targetPeakSlider.getBounds());
    row.removeFromLeft (6);
    ceilingSlider.setBounds (row);
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
    trackLabel.setText (rsProcessor.getDisplayName() + juce::String (juce::CharPointer_UTF8 ("  \xc2\xb7  "))
                            + rsProcessor.getChannelFormatName(),
                        juce::dontSendNotification);
}

void RoughStartEditor::updateTargetVisibility()
{
    const int modeIndex = rsProcessor.getMode() == RoughStartProcessor::Mode::rms ? 1 : 0;
    if (modeIndex == lastModeIndex)
        return;

    lastModeIndex = modeIndex;
    peakButton.setToggleState (modeIndex == 0, juce::dontSendNotification);
    rmsButton.setToggleState (modeIndex == 1, juce::dontSendNotification);

    if (view == View::track)
    {
        targetPeakSlider.setVisible (modeIndex == 0);
        targetRmsSlider.setVisible (modeIndex == 1);
    }
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
        case State::measuring:          status = "Measuring this track..."; break;
        case State::idle:
            switch (rsProcessor.getLastAnalysisResult())
            {
                case Result::stored:   status = "Measurement stored."; break;
                case Result::noSignal: status = "No signal, Auto unchanged."; break;
                case Result::none:     break;
            }
            break;
    }
    statusLabel.setText (status, juce::dontSendNotification);

    analyzeButton.setButtonText (state == State::waitingForPlayback ? "Cancel" : analyzing ? "Stop" : "Analyze track");
    analyzeButton.setColour (juce::TextButton::buttonColourId,
                             analyzing ? ui::analyzeColour : lookAndFeel.findColour (juce::TextButton::buttonColourId));

    const auto m = analyzing ? rsProcessor.getLiveMeasurement() : rsProcessor.getStoredMeasurement();
    const auto autoGain = rsProcessor.getAutoGain();
    const bool rms = rsProcessor.getMode() == RoughStartProcessor::Mode::rms;

    if (m.valid)
        measuredLabel.setText ((analyzing ? juce::String ("Live  ") : juce::String ("Measured  "))
                                   + "RMS " + ui::formatDb (m.rmsDb, false)
                                   + juce::String (juce::CharPointer_UTF8 ("  \xc2\xb7  "))
                                   + "peak " + ui::formatDb (m.peakDb, false),
                               juce::dontSendNotification);
    else
        measuredLabel.setText (analyzing ? "Play to measure..." : "Not analyzed yet. Click Analyze all and play the song.",
                               juce::dontSendNotification);

    const bool hasStored = rsProcessor.getStoredMeasurement().valid;
    autoLabel.setText (hasStored ? ui::formatDb (autoGain.gainDb, true) : juce::String (juce::CharPointer_UTF8 ("\xe2\x80\x93")),
                       juce::dontSendNotification);
    autoLabel.setColour (juce::Label::textColourId, hasStored ? ui::text : ui::faintText);

    meterNote.setText (hasStored && autoGain.limitedByCeiling && ! analyzing ? "Peak at ceiling" : juce::String(),
                       juce::dontSendNotification);

    totalLabel.setText (ui::formatDb (rsProcessor.getTotalGainDb(), true), juce::dontSendNotification);

    MeterView::Data d;
    d.measured = m.valid;
    d.live = analyzing;
    d.rmsMode = rms;
    d.peakDb = m.peakDb;
    d.rmsDb = m.rmsDb;
    d.autoDb = autoGain.gainDb;
    d.targetDb = rsProcessor.getTargetDb();
    d.ceilingDb = rsProcessor.getCeilingDb();
    d.limitedByCeiling = autoGain.limitedByCeiling;
    meter.setData (d);
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
    const int count = rsProcessor.getRegistry().getNumInstances();

    analyzeAllButton.setButtonText (running ? "Stop all" : "Analyze all (" + juce::String (count) + ")");
    analyzeAllButton.getProperties().set ("primary", ! running);
    analyzeAllButton.setColour (juce::TextButton::buttonColourId,
                                running ? ui::analyzeColour : lookAndFeel.findColour (juce::TextButton::buttonColourId));

    juce::String text;

    if (juce::Time::getMillisecondCounterHiRes() < noticeUntilMs)
        text = noticeText;
    else if (s.total == 0)
        text = {};
    else if (s.waiting > 0 && s.measuring == 0)
        text = juce::String (s.waiting) + " tracks waiting for playback...";
    else if (running)
        text = "Measuring " + juce::String (s.measuring) + " of " + juce::String (s.total) + " tracks...";
    else
        text = "Last analyze all: " + juce::String (s.stored) + " stored"
               + (s.noSignal > 0 ? ", " + juce::String (s.noSignal) + " without signal" : juce::String());

    batchLabel.setText (text, juce::dontSendNotification);
    analyzeAllButton.repaint();
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

    const auto defaultRef = presets.getDefaultPreset();

    auto addEntry = [this, &defaultRef] (const juce::String& name, const TargetSettings& s, bool factory)
    {
        presetCache.push_back ({ name, s, factory });
        const bool isDefault = defaultRef == PresetManager::PresetRef { factory, name };
        presetBox.addItem (name + (isDefault ? "  (default)" : ""), (int) presetCache.size());
    };

    presetBox.addSectionHeading ("Factory");
    for (const auto& f : PresetManager::getFactoryPresets())
        addEntry (f.name, f.settings, true);

    const auto userNames = presets.getPresetNames();
    if (! userNames.isEmpty())
    {
        presetBox.addSeparator();
        presetBox.addSectionHeading ("My presets");

        for (const auto& name : userNames)
            if (auto s = presets.load (name))
                addEntry (name, *s, false);
    }

    syncPresetSelection();
}

void RoughStartEditor::syncPresetSelection()
{
    // Toon de preset die precies overeenkomt met de huidige instellingen, anders "(custom)".
    const auto current = rsProcessor.getTargetSettings();

    // Voorkeur voor de preset die al geselecteerd is (bij twee identieke presets).
    if (auto* selected = getSelectedEntry(); selected != nullptr && selected->settings == current)
        return;

    // Eigen presets gaan voor factory presets met dezelfde waarden.
    int match = 0;
    for (int pass = 0; pass < 2 && match == 0; ++pass)
    {
        const bool wantFactory = pass == 1;
        for (size_t i = 0; i < presetCache.size(); ++i)
        {
            if (presetCache[i].factory == wantFactory && presetCache[i].settings == current)
            {
                match = (int) i + 1;
                break;
            }
        }
    }

    presetBox.setSelectedId (match, juce::dontSendNotification);
}

const RoughStartEditor::PresetEntry* RoughStartEditor::getSelectedEntry() const
{
    const int index = presetBox.getSelectedId() - 1;
    return juce::isPositiveAndBelow (index, (int) presetCache.size()) ? &presetCache[(size_t) index] : nullptr;
}

juce::String RoughStartEditor::getSelectedPresetName() const
{
    if (auto* e = getSelectedEntry())
        return e->name;
    return {};
}

void RoughStartEditor::loadSelectedPreset()
{
    if (auto* e = getSelectedEntry())
        rsProcessor.applyTargetSettings (e->settings);
}

void RoughStartEditor::setSelectedAsDefault()
{
    auto* e = getSelectedEntry();
    if (e == nullptr)
        return;

    const auto name = e->name;
    if (presets.setDefaultPreset ({ e->factory, name }))
    {
        reloadPresets();
        showNotice ("\"" + name + "\" is now the default for new instances.");
    }
}

void RoughStartEditor::showPresetMenu()
{
    const auto* selected = getSelectedEntry();
    const bool hasSelection = selected != nullptr;
    const bool isUserPreset = hasSelection && ! selected->factory;

    juce::PopupMenu menu;
    menu.addItem (saveItem, "Save current settings as preset...");
    menu.addItem (applyAllItem, "Apply current settings to all tracks");
    menu.addItem (defaultItem, "Use selected preset as default for new instances", hasSelection);
    menu.addSeparator();
    menu.addItem (importItem, "Import preset...");
    menu.addItem (exportItem, "Export selected preset...", hasSelection);
    menu.addItem (deleteItem, "Delete selected preset", isUserPreset);
    menu.addSeparator();
    menu.addItem (revealItem, "Show presets folder");
    menu.addItem (resetItem, "Reset to default preset");

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
                                case defaultItem:  safeThis->setSelectedAsDefault(); break;
                                case importItem:   safeThis->importPreset(); break;
                                case exportItem:   safeThis->exportSelectedPreset(); break;
                                case deleteItem:   safeThis->deleteSelectedPreset(); break;
                                case revealItem:
                                    safeThis->presets.getDirectory().createDirectory();
                                    safeThis->presets.getDirectory().revealToUser();
                                    break;
                                case resetItem:
                                    safeThis->rsProcessor.applyTargetSettings (safeThis->presets.getDefaultSettings());
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
    const auto* entry = getSelectedEntry();
    if (entry == nullptr)
        return;

    const auto name = entry->name;
    const auto settings = entry->settings;

    const auto suggested = juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                               .getChildFile (name + PresetManager::fileExtension);

    fileChooser = std::make_unique<juce::FileChooser> ("Export RoughStart preset", suggested,
                                                       juce::String ("*") + PresetManager::fileExtension);

    juce::Component::SafePointer<RoughStartEditor> safeThis (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting,
                              [safeThis, name, settings] (const juce::FileChooser& chooser)
                              {
                                  if (safeThis == nullptr)
                                      return;

                                  auto file = chooser.getResult();
                                  if (file == juce::File())
                                      return;

                                  if (! file.hasFileExtension (PresetManager::fileExtension))
                                      file = file.withFileExtension (PresetManager::fileExtension);

                                  safeThis->showNotice (PresetManager::toXml (name, settings)->writeTo (file)
                                                            ? "Preset exported to " + file.getFileName() + "."
                                                            : "Could not export preset.");
                              });
}

void RoughStartEditor::deleteSelectedPreset()
{
    const auto* entry = getSelectedEntry();
    if (entry == nullptr || entry->factory)
        return;

    const auto name = entry->name;

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
    showNotice ("Mode, target and ceiling applied to " + juce::String (count) + " tracks.");
}
