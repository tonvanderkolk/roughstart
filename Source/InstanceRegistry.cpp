#include "InstanceRegistry.h"
#include "PluginProcessor.h"

#include <algorithm>

int InstanceRegistry::add (RoughStartProcessor* instance)
{
    int number = 0;
    {
        const juce::ScopedLock sl (lock);
        if (std::find (instances.begin(), instances.end(), instance) == instances.end())
            instances.push_back (instance);
        number = nextInstanceNumber++;
    }
    notifyChanged();
    return number;
}

void InstanceRegistry::remove (RoughStartProcessor* instance)
{
    {
        const juce::ScopedLock sl (lock);
        instances.erase (std::remove (instances.begin(), instances.end(), instance), instances.end());
    }
    notifyChanged();
}

int InstanceRegistry::getNumInstances() const
{
    const juce::ScopedLock sl (lock);
    return (int) instances.size();
}

juce::StringArray InstanceRegistry::getTrackNames() const
{
    const juce::ScopedLock sl (lock);
    juce::StringArray names;
    for (auto* instance : instances)
        names.add (instance->getDisplayName());
    return names;
}

//==============================================================================
void InstanceRegistry::analyzeAll()
{
    const juce::ScopedLock sl (lock);
    const int batch = ++currentBatch;

    for (auto* instance : instances)
        instance->startAnalysis (batch);
}

void InstanceRegistry::stopAll()
{
    const juce::ScopedLock sl (lock);

    for (auto* instance : instances)
        if (instance->getAnalysisBatch() == currentBatch && instance->isAnalyzing())
            instance->stopAnalysis();
}

namespace
{
    constexpr double allQuietTimeoutMs = 3000.0;  // niemand krijgt nog audio: host is gestopt
}

bool InstanceRegistry::hasBatchSeenPlayback (int batch) const
{
    const juce::ScopedLock sl (lock);

    for (auto* instance : instances)
        if (instance->getAnalysisBatch() == batch && instance->hasSeenPlayback())
            return true;

    return false;
}

bool InstanceRegistry::hasPlaybackStopped() const
{
    const juce::ScopedLock sl (lock);

    double latestPlaying = 0.0, latestStopped = 0.0, latestBlock = 0.0;

    for (auto* instance : instances)
    {
        latestPlaying = juce::jmax (latestPlaying, instance->getLastPlayingBlockMs());
        latestStopped = juce::jmax (latestStopped, instance->getLastStoppedBlockMs());
        latestBlock   = juce::jmax (latestBlock,   instance->getLastBlockMs());
    }

    if (latestStopped > latestPlaying)
        return true;   // de nieuwste melding van de host: staat stil

    return juce::Time::getMillisecondCounterHiRes() - latestBlock > allQuietTimeoutMs;
}

InstanceRegistry::BatchSummary InstanceRegistry::getBatchSummary() const
{
    using State = RoughStartProcessor::AnalysisState;
    using Result = RoughStartProcessor::AnalysisResult;

    const juce::ScopedLock sl (lock);
    BatchSummary s;

    if (currentBatch == 0)
        return s;

    for (auto* instance : instances)
    {
        if (instance->getAnalysisBatch() != currentBatch)
            continue;

        ++s.total;

        switch (instance->getAnalysisState())
        {
            case State::waitingForPlayback: ++s.waiting;   break;
            case State::measuring:          ++s.measuring; break;
            case State::idle:
                if (instance->getLastAnalysisResult() == Result::stored)
                    ++s.stored;
                else
                    ++s.noSignal;
                break;
        }
    }

    return s;
}

//==============================================================================
std::vector<InstanceRegistry::OverviewRow> InstanceRegistry::getOverview() const
{
    const juce::ScopedLock sl (lock);
    std::vector<OverviewRow> rows;
    rows.reserve (instances.size());

    for (auto* instance : instances)
    {
        const auto m = instance->getStoredMeasurement();
        const auto autoGain = instance->getAutoGain();

        OverviewRow r;
        r.instanceNumber   = instance->getInstanceNumber();
        r.trackName        = instance->getDisplayName();
        r.numChannels      = instance->getNumChannels();
        r.mode             = instance->getMode() == RoughStartProcessor::Mode::rms ? 1 : 0;
        r.measured         = m.valid;
        r.peakDb           = m.peakDb;
        r.rmsDb            = m.rmsDb;
        r.autoDb           = autoGain.gainDb;
        r.limitedByCeiling = autoGain.limitedByCeiling;
        r.trimDb           = instance->getTrimDb();
        r.totalDb          = autoGain.gainDb + r.trimDb;
        r.analysisState    = (int) instance->getAnalysisState();
        r.lastResult       = (int) instance->getLastAnalysisResult();

        const auto live    = instance->getLiveMeasurement();
        r.liveValid        = instance->isAnalyzing() && live.valid;
        r.livePeakDb       = live.peakDb;
        r.liveRmsDb        = live.rmsDb;
        rows.push_back (r);
    }

    return rows;
}

int InstanceRegistry::applyTargetSettingsToAll (const roughstart::TargetSettings& settings)
{
    const juce::ScopedLock sl (lock);

    for (auto* instance : instances)
        instance->applyTargetSettings (settings);

    return (int) instances.size();
}

void InstanceRegistry::setTrimDb (int instanceNumber, float trimDb)
{
    const juce::ScopedLock sl (lock);

    for (auto* instance : instances)
        if (instance->getInstanceNumber() == instanceNumber)
            if (auto* p = instance->parameters.getParameter ("trim"))
                p->setValueNotifyingHost (p->convertTo0to1 (trimDb));
}

void InstanceRegistry::setTrimGesture (int instanceNumber, bool starting)
{
    const juce::ScopedLock sl (lock);

    for (auto* instance : instances)
        if (instance->getInstanceNumber() == instanceNumber)
            if (auto* p = instance->parameters.getParameter ("trim"))
            {
                if (starting) p->beginChangeGesture();
                else          p->endChangeGesture();
            }
}

namespace
{
    /** Eén Trim-wijziging vanuit het overzicht; zoekt de instance op volgnummer. */
    class TrimChangeAction final : public juce::UndoableAction
    {
    public:
        TrimChangeAction (InstanceRegistry& r, int instance, float from, float to)
            : registry (r), instanceNumber (instance), oldDb (from), newDb (to) {}

        bool perform() override { registry.setTrimDb (instanceNumber, newDb); return true; }
        bool undo() override    { registry.setTrimDb (instanceNumber, oldDb); return true; }
        int getSizeInUnits() override { return 1; }

    private:
        InstanceRegistry& registry;
        int instanceNumber;
        float oldDb, newDb;
    };
}

void InstanceRegistry::recordTrimChange (int instanceNumber, float oldDb, float newDb)
{
    if (std::abs (oldDb - newDb) < 0.01f)
        return;

    undoManager.beginNewTransaction ("Trim");
    undoManager.perform (new TrimChangeAction (*this, instanceNumber, oldDb, newDb));
}
