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
        r.mode             = instance->getMode() == RoughStartProcessor::Mode::rms ? 1 : 0;
        r.measured         = m.valid;
        r.peakDb           = m.peakDb;
        r.rmsDb            = m.rmsDb;
        r.autoDb           = autoGain.gainDb;
        r.limitedByCeiling = autoGain.limitedByCeiling;
        r.trimDb           = instance->getTrimDb();
        r.totalDb          = autoGain.gainDb + r.trimDb;
        r.analysisState    = (int) instance->getAnalysisState();
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
