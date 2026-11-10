#include "PluginEditor.h"

namespace
{
    constexpr double pingFlashMs = 700.0;
}

RoughStartEditor::RoughStartEditor (RoughStartProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    titleLabel.setText ("RoughStart  v" JucePlugin_VersionString, juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    countLabel.setFont (juce::FontOptions (15.0f));
    addAndMakeVisible (countLabel);

    namesLabel.setFont (juce::FontOptions (13.0f));
    namesLabel.setJustificationType (juce::Justification::topLeft);
    namesLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (namesLabel);

    pingButton.onClick = [this] { processor.getRegistry().pingAll(); };
    addAndMakeVisible (pingButton);

    lastSeenPing = processor.getPingCount();
    refreshInstanceInfo();

    setSize (360, 260);
    startTimerHz (30);
}

RoughStartEditor::~RoughStartEditor()
{
    stopTimer();
}

void RoughStartEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e2226));

    const auto colour = indicatorLit ? juce::Colour (0xff3ddc84) : juce::Colour (0xff3a4048);
    g.setColour (colour);
    g.fillEllipse (indicatorBounds.toFloat());
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawEllipse (indicatorBounds.toFloat(), 1.5f);
}

void RoughStartEditor::resized()
{
    auto area = getLocalBounds().reduced (16);

    titleLabel.setBounds (area.removeFromTop (28));
    area.removeFromTop (8);

    auto row = area.removeFromTop (32);
    indicatorBounds = row.removeFromRight (32).reduced (4);
    row.removeFromRight (8);
    pingButton.setBounds (row.removeFromRight (110));
    countLabel.setBounds (row);

    area.removeFromTop (12);
    namesLabel.setBounds (area);
}

void RoughStartEditor::refreshInstanceInfo()
{
    auto& registry = processor.getRegistry();
    const auto names = registry.getTrackNames();

    countLabel.setText ("Gevonden instances: " + juce::String (names.size()),
                        juce::dontSendNotification);
    namesLabel.setText ("Deze instance: " + processor.getDisplayName() + "\n\n"
                            + names.joinIntoString ("\n"),
                        juce::dontSendNotification);
}

void RoughStartEditor::timerCallback()
{
    const auto change = processor.getRegistry().getChangeCount();
    if (change != lastSeenChange)
    {
        lastSeenChange = change;
        refreshInstanceInfo();
    }

    const auto now = juce::Time::getMillisecondCounterHiRes();
    const auto ping = processor.getPingCount();
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
