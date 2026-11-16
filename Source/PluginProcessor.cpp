#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    namespace ids
    {
        const juce::String mode       = "mode";
        const juce::String targetPeak = "targetPeak";
        const juce::String targetRms  = "targetRms";
        const juce::String trim       = "trim";
        const juce::String ceiling    = "ceiling";

        const juce::Identifier stateType     { "RoughStartState" };
        const juce::Identifier measuredValid { "measuredValid" };
        const juce::Identifier measuredPeak  { "measuredPeakDb" };
        const juce::Identifier measuredRms   { "measuredRmsDb" };
        const juce::Identifier group         { "group" };
    }

    constexpr double gainRampSeconds = 0.05;


    juce::String dbText (float value, int)
    {
        return juce::String (value, 1) + " dB";
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout RoughStartProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    const auto nonAutomatable = AudioParameterFloatAttributes().withAutomatable (false)
                                                               .withStringFromValueFunction (dbText)
                                                               .withLabel ("dB");

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ids::mode, 1 }, "Mode", StringArray { "Peak", "RMS" }, 1,
        AudioParameterChoiceAttributes().withAutomatable (false)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ids::targetPeak, 1 }, "Peak Target",
        NormalisableRange<float> (-30.0f, 0.0f, 0.5f), -12.0f, nonAutomatable));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ids::targetRms, 1 }, "RMS Target",
        NormalisableRange<float> (-40.0f, 0.0f, 0.5f), -18.0f, nonAutomatable));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ids::trim, 1 }, "Trim",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText).withLabel ("dB")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ids::ceiling, 1 }, "Peak Ceiling",
        NormalisableRange<float> (-24.0f, 0.0f, 0.5f), -6.0f, nonAutomatable));

    return layout;
}

RoughStartProcessor::RoughStartProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, ids::stateType, createParameterLayout())
{
    modeParam       = parameters.getRawParameterValue (ids::mode);
    targetPeakParam = parameters.getRawParameterValue (ids::targetPeak);
    targetRmsParam  = parameters.getRawParameterValue (ids::targetRms);
    trimParam       = parameters.getRawParameterValue (ids::trim);
    ceilingParam    = parameters.getRawParameterValue (ids::ceiling);

    // Nieuwe instance: start met de standaardpreset van de gebruiker (of de
    // factory-standaard). Bij het laden van een project overschrijft
    // setStateInformation dit daarna met de opgeslagen waarden.
    applyTargetSettings (roughstart::PresetManager().getDefaultSettings());

    instanceNumber = registry->add (this);
    startTimerHz (20);
}

RoughStartProcessor::~RoughStartProcessor()
{
    stopTimer();
    registry->remove (this);
}

//==============================================================================
RoughStartProcessor::Mode RoughStartProcessor::getMode() const noexcept
{
    return modeParam->load() >= 0.5f ? Mode::rms : Mode::peak;
}

float RoughStartProcessor::getTargetDb() const noexcept
{
    return getMode() == Mode::rms ? targetRmsParam->load() : targetPeakParam->load();
}

float RoughStartProcessor::getTrimDb() const noexcept
{
    return trimParam->load();
}

RoughStartProcessor::Measurement RoughStartProcessor::getStoredMeasurement() const noexcept
{
    return { storedValid.load(), storedPeakDb.load(), storedRmsDb.load() };
}

RoughStartProcessor::Measurement RoughStartProcessor::getLiveMeasurement() const noexcept
{
    return { liveValid.load(), livePeakDb.load(), liveRmsDb.load() };
}

float RoughStartProcessor::getCeilingDb() const noexcept
{
    return ceilingParam->load();
}

roughstart::TargetSettings RoughStartProcessor::getTargetSettings() const noexcept
{
    roughstart::TargetSettings s;
    s.mode         = getMode() == Mode::rms ? 1 : 0;
    s.targetPeakDb = targetPeakParam->load();
    s.targetRmsDb  = targetRmsParam->load();
    s.ceilingDb    = ceilingParam->load();
    return s;
}

void RoughStartProcessor::applyTargetSettings (const roughstart::TargetSettings& s)
{
    auto set = [this] (const juce::String& id, float value)
    {
        if (auto* p = parameters.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };

    set (ids::mode, (float) s.mode);
    set (ids::targetPeak, s.targetPeakDb);
    set (ids::targetRms, s.targetRmsDb);
    set (ids::ceiling, s.ceilingDb);
}

roughstart::AutoGainResult RoughStartProcessor::getAutoGain() const noexcept
{
    if (! storedValid.load())
        return {};

    const auto peak = storedPeakDb.load();
    const auto measured = getMode() == Mode::rms ? storedRmsDb.load() : peak;
    return roughstart::computeAutoGain (measured, getTargetDb(), peak, getCeilingDb());
}

void RoughStartProcessor::startAnalysis (int batch)
{
    analysisBatch = batch;
    sawPlayback = false;
    lastResult = (int) AnalysisResult::none;
    liveValid = false;
    livePeakDb = roughstart::minusInfinityDb;
    liveRmsDb = roughstart::minusInfinityDb;
    resetMeterPending = true;
    analyzing = true;
}

bool RoughStartProcessor::stopAnalysis()
{
    if (! analyzing.exchange (false))
        return false;

    if (! liveValid.load())
    {
        lastResult = (int) AnalysisResult::noSignal;
        return false;   // geen signaal: vorige meting (en dus Auto) blijft staan
    }

    storedPeakDb = livePeakDb.load();
    storedRmsDb = liveRmsDb.load();
    storedValid = true;
    lastResult = (int) AnalysisResult::stored;
    return true;
}

RoughStartProcessor::AnalysisState RoughStartProcessor::getAnalysisState() const noexcept
{
    if (! analyzing.load())
        return AnalysisState::idle;

    return sawPlayback.load() ? AnalysisState::measuring : AnalysisState::waitingForPlayback;
}

void RoughStartProcessor::timerCallback()
{
    if (! analyzing.load())
        return;

    // Logic slaat plug-ins op sporen en bussen zonder geluid soms tijdelijk over,
    // ook midden in het nummer. Dat deze instance even geen audio krijgt, zegt dus
    // niets over het transport. Of het afspelen gestopt is, beslist de registry op
    // basis van alle instances (zie InstanceRegistry::hasPlaybackStopped).
    const int batch = analysisBatch.load();
    const bool started = batch > 0 ? registry->hasBatchSeenPlayback (batch) : sawPlayback.load();

    if (started && registry->hasPlaybackStopped())
        stopAnalysis();
}

//==============================================================================
void RoughStartProcessor::prepareToPlay (double sampleRate, int)
{
    setLatencySamples (0);
    numChannels = juce::jmax (1, getMainBusNumInputChannels());
    meter.prepare (sampleRate);
    resetMeterPending = false;

    smoothedGain.reset (sampleRate, gainRampSeconds);
    smoothedGain.setCurrentAndTargetValue (roughstart::dbToGain (getTotalGainDb()));
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

    const int numSamples = buffer.getNumSamples();
    const int numInputs = getTotalNumInputChannels();

    for (auto ch = numInputs; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    // 0. Transport volgen. Zonder transportinfo (bijv. auval) geldt: speelt.
    bool playing = true;
    if (auto* host = getPlayHead())
        if (auto position = host->getPosition())
            playing = position->getIsPlaying();

    hostPlaying = playing;
    const double now = juce::Time::getMillisecondCounterHiRes();
    lastBlockMs = now;
    (playing ? lastPlayingMs : lastStoppedMs) = now;

    // 1. Meten op het ingangssignaal (vóór de gain), zodat opnieuw analyseren
    //    niet afhangt van de huidige Auto- of Trim-stand. Alleen tijdens afspelen.
    if (analyzing.load() && playing)
    {
        sawPlayback = true;

        if (resetMeterPending.exchange (false))
            meter.reset();

        meter.process (buffer.getArrayOfReadPointers(), numInputs, numSamples);

        if (meter.hasSignal())
        {
            livePeakDb = meter.getPeakDb();
            liveRmsDb = meter.getRmsDb();
            liveValid = true;
        }
    }

    // 2. Gain (Auto + Trim) vloeiend toepassen.
    smoothedGain.setTargetValue (roughstart::dbToGain (getTotalGainDb()));

    if (smoothedGain.isSmoothing())
    {
        auto* const* channels = buffer.getArrayOfWritePointers();
        for (int i = 0; i < numSamples; ++i)
        {
            const float g = smoothedGain.getNextValue();
            for (int ch = 0; ch < numInputs; ++ch)
                channels[ch][i] *= g;
        }
    }
    else
    {
        const float g = smoothedGain.getTargetValue();
        if (g != 1.0f)
            buffer.applyGain (0, numSamples, g);
    }
}

//==============================================================================
void RoughStartProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty (ids::measuredValid, storedValid.load(), nullptr);
    state.setProperty (ids::measuredPeak, storedPeakDb.load(), nullptr);
    state.setProperty (ids::measuredRms, storedRmsDb.load(), nullptr);
    state.setProperty (ids::group, getGroup(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void RoughStartProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (parameters.state.getType()))
        return;

    auto state = juce::ValueTree::fromXml (*xml);

    storedValid  = (bool) state.getProperty (ids::measuredValid, false);
    storedPeakDb = (float) state.getProperty (ids::measuredPeak, roughstart::minusInfinityDb);
    storedRmsDb  = (float) state.getProperty (ids::measuredRms, roughstart::minusInfinityDb);
    setGroup (state.getProperty (ids::group, juce::String()).toString());
    state.removeProperty (ids::group, nullptr);

    parameters.replaceState (state);
}

void RoughStartProcessor::numChannelsChanged()
{
    numChannels = juce::jmax (1, getMainBusNumInputChannels());
    registry->notifyChanged();
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

juce::String RoughStartProcessor::getGroup() const
{
    const juce::ScopedLock sl (nameLock);
    return group;
}

void RoughStartProcessor::setGroup (const juce::String& newGroup)
{
    const auto cleaned = newGroup.trim().substring (0, 40);
    {
        const juce::ScopedLock sl (nameLock);
        if (group == cleaned)
            return;
        group = cleaned;
    }
    registry->notifyChanged();
}

juce::AudioProcessorEditor* RoughStartProcessor::createEditor()
{
    return new RoughStartEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoughStartProcessor();
}
