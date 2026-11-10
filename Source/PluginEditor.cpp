#include "PluginEditor.h"

namespace
{
    constexpr double pingFlashMs = 700.0;

    const juce::Colour background   { 0xff1e2226 };
    const juce::Colour panel        { 0xff272c31 };
    const juce::Colour accent       { 0xff3ddc84 };
    const juce::Colour analyzeColour{ 0xffe0583a };
    const juce::Colour dimText      { 0xffa8b0b8 };

    juce::String formatDb (float db, bool withSign)
    {
        if (db <= roughstart::minusInfinityDb)
            return "-inf dB";
        const auto text = juce::String (db, 1);
        return (withSign && db > 0.0f ? "+" : "") + text + " dB";
    }

    void styleValueLabel (juce::Label& l, float size)
    {
        l.setFont (juce::FontOptions (size));
        l.setJustificationType (juce::Justification::centredLeft);
    }

    void styleSlider (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 72, 22);
        s.setColour (juce::Slider::thumbColourId, accent);
        s.setColour (juce::Slider::trackColourId, accent.withAlpha (0.5f));
    }
}

RoughStartEditor::RoughStartEditor (RoughStartProcessor& p)
    : AudioProcessorEditor (&p), rsProcessor (p)
{
    titleLabel.setText ("RoughStart  v" JucePlugin_VersionString, juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    trackLabel.setJustificationType (juce::Justification::centredRight);
    trackLabel.setColour (juce::Label::textColourId, dimText);
    addAndMakeVisible (trackLabel);

    // Meetmethode en doel
    for (auto* l : { &modeLabel, &targetLabel, &ceilingLabel, &trimLabel })
    {
        l->setColour (juce::Label::textColourId, dimText);
        addAndMakeVisible (*l);
    }

    modeBox.addItemList ({ "Peak", "RMS" }, 1);
    addAndMakeVisible (modeBox);
    modeAttachment = std::make_unique<ComboAttachment> (rsProcessor.parameters, "mode", modeBox);

    styleSlider (targetPeakSlider);
    styleSlider (targetRmsSlider);
    addChildComponent (targetPeakSlider);
    addChildComponent (targetRmsSlider);
    targetPeakAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "targetPeak", targetPeakSlider);
    targetRmsAttachment  = std::make_unique<SliderAttachment> (rsProcessor.parameters, "targetRms", targetRmsSlider);

    styleSlider (ceilingSlider);
    addAndMakeVisible (ceilingSlider);
    ceilingAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "ceiling", ceilingSlider);

    // Analyse
    analyzeButton.onClick = [this] { toggleAnalysis(); };
    addAndMakeVisible (analyzeButton);

    statusLabel.setColour (juce::Label::textColourId, dimText);
    styleValueLabel (statusLabel, 13.0f);
    addAndMakeVisible (statusLabel);

    styleValueLabel (measuredLabel, 14.0f);
    styleValueLabel (autoLabel, 14.0f);
    styleValueLabel (totalLabel, 14.0f);
    addAndMakeVisible (measuredLabel);
    addAndMakeVisible (autoLabel);
    addAndMakeVisible (totalLabel);

    // Trim
    styleSlider (trimSlider);
    trimSlider.setDoubleClickReturnValue (true, 0.0);
    addAndMakeVisible (trimSlider);
    trimAttachment = std::make_unique<SliderAttachment> (rsProcessor.parameters, "trim", trimSlider);

    // Instances
    countLabel.setColour (juce::Label::textColourId, dimText);
    styleValueLabel (countLabel, 13.0f);
    addAndMakeVisible (countLabel);

    pingButton.onClick = [this] { rsProcessor.getRegistry().pingAll(); };
    addAndMakeVisible (pingButton);

    lastSeenPing = rsProcessor.getPingCount();
    refreshInstanceInfo();
    updateTargetVisibility();
    refreshLevels();

    setSize (440, 420);
    startTimerHz (30);
}

RoughStartEditor::~RoughStartEditor()
{
    stopTimer();
}

void RoughStartEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);

    g.setColour (panel);
    g.fillRoundedRectangle (getLocalBounds().reduced (10).withTrimmedTop (44).withTrimmedBottom (50).toFloat(), 6.0f);

    g.setColour (indicatorLit ? accent : juce::Colour (0xff3a4048));
    g.fillEllipse (indicatorBounds.toFloat());
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawEllipse (indicatorBounds.toFloat(), 1.5f);
}

void RoughStartEditor::resized()
{
    auto area = getLocalBounds().reduced (16, 12);

    auto top = area.removeFromTop (28);
    titleLabel.setBounds (top.removeFromLeft (200));
    trackLabel.setBounds (top);

    // Onderbalk: instances en ping
    auto bottom = area.removeFromBottom (30);
    indicatorBounds = bottom.removeFromRight (30).reduced (5);
    bottom.removeFromRight (6);
    pingButton.setBounds (bottom.removeFromRight (90));
    countLabel.setBounds (bottom);

    area.removeFromBottom (20);
    area.removeFromTop (18);
    area = area.reduced (10, 0);

    constexpr int labelWidth = 100, rowHeight = 28, gap = 8;

    auto row = area.removeFromTop (rowHeight);
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

void RoughStartEditor::toggleAnalysis()
{
    if (rsProcessor.isAnalyzing())
        rsProcessor.stopAnalysis();
    else
        rsProcessor.startAnalysis();

    refreshLevels();
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
                             analyzing ? analyzeColour : getLookAndFeel().findColour (juce::TextButton::buttonColourId));

    const auto m = analyzing ? rsProcessor.getLiveMeasurement() : rsProcessor.getStoredMeasurement();
    const juce::String prefix = analyzing ? "Live: " : "Measured: ";

    if (m.valid)
        measuredLabel.setText (prefix + "peak " + formatDb (m.peakDb, false)
                                   + "  ·  RMS " + formatDb (m.rmsDb, false),
                               juce::dontSendNotification);
    else
        measuredLabel.setText (prefix + (analyzing ? "no signal yet" : "not analyzed yet"),
                               juce::dontSendNotification);

    const auto autoGain = rsProcessor.getAutoGain();
    autoLabel.setText ("Auto: " + formatDb (autoGain.gainDb, true)
                           + (autoGain.limitedByCeiling ? "  (limited by ceiling)" : ""),
                       juce::dontSendNotification);
    autoLabel.setColour (juce::Label::textColourId,
                         autoGain.limitedByCeiling ? juce::Colour (0xffe8b04a) : juce::Colours::white);
    totalLabel.setText ("Total (Auto + Trim): " + formatDb (rsProcessor.getTotalGainDb(), true),
                        juce::dontSendNotification);
}

void RoughStartEditor::refreshInstanceInfo()
{
    countLabel.setText ("Instances in project: " + juce::String (rsProcessor.getRegistry().getNumInstances()),
                        juce::dontSendNotification);
    trackLabel.setText (rsProcessor.getDisplayName(), juce::dontSendNotification);
}

void RoughStartEditor::timerCallback()
{
    const auto change = rsProcessor.getRegistry().getChangeCount();
    if (change != lastSeenChange)
    {
        lastSeenChange = change;
        refreshInstanceInfo();
    }

    updateTargetVisibility();
    refreshLevels();

    const auto now = juce::Time::getMillisecondCounterHiRes();
    const auto ping = rsProcessor.getPingCount();
    if (ping != lastSeenPing)
    {
        lastSeenPing = ping;
        pingLitUntilMs = now + pingFlashMs;
    }

    const bool shouldBeLit = now < pingLitUntilMs;
    if (shouldBeLit != indicatorLit)
    {
        indicatorLit = shouldBeLit;
        repaint (indicatorBounds);
    }
}
