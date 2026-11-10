#include "PluginProcessor.h"
#include "PluginEditor.h"

RoughStartProcessor::RoughStartProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    instanceNumber = registry->add (this);
}

RoughStartProcessor::~RoughStartProcessor()
{
    registry->remove (this);
}

void RoughStartProcessor::prepareToPlay (double, int)
{
    setLatencySamples (0);
}

bool RoughStartProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == out;
}

void RoughStartProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // v0.1: alleen doorgeven. Extra uitgangskanalen zonder ingang leegmaken.
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

void RoughStartProcessor::updateTrackProperties (const TrackProperties& properties)
{
    {
        const juce::ScopedLock sl (nameLock);
        trackName = properties.name.value_or (juce::String());
    }
    registry->notifyChanged();
}

juce::String RoughStartProcessor::getDisplayName() const
{
    const juce::ScopedLock sl (nameLock);
    return trackName.isNotEmpty() ? trackName : "Instance " + juce::String (instanceNumber);
}

juce::AudioProcessorEditor* RoughStartProcessor::createEditor()
{
    return new RoughStartEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoughStartProcessor();
}
