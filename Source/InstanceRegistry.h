#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include "PresetManager.h"

class RoughStartProcessor;

/**
    Gedeelde lijst van alle RoughStart-instances in hetzelfde hostproces.

    Wordt via juce::SharedResourcePointer<InstanceRegistry> gedeeld: de eerste
    instance maakt het object aan, de laatste ruimt het op. Werkt alleen als de
    host alle instances in één proces laadt (Logic doet dat voor AUv2).

    Let op: Logic laadt plug-ins op sporen zonder regio's pas als er iets op
    komt; die instances staan dan (nog) niet in de lijst.
*/
class InstanceRegistry
{
public:
    InstanceRegistry() = default;
    ~InstanceRegistry() = default;

    /** Meldt een instance aan en geeft een volgnummer terug (1, 2, 3...). */
    int add (RoughStartProcessor* instance);
    void remove (RoughStartProcessor* instance);

    int getNumInstances() const;

    /** Momentopname van de spoornamen van alle instances, in aanmeldvolgorde. */
    juce::StringArray getTrackNames() const;

    //==============================================================================
    /** Zet alle instances in analysestand onder één nieuw batchnummer. */
    void analyzeAll();

    /** Stopt de analyse van alle instances in de lopende batch (meting wordt vastgezet). */
    void stopAll();

    struct BatchSummary
    {
        int total = 0;
        int waiting = 0;
        int measuring = 0;
        int stored = 0;
        int noSignal = 0;

        bool isRunning() const noexcept { return waiting + measuring > 0; }
    };

    /**
        True als in deze batch minstens één instance het afspelen heeft gezien
        en inmiddels klaar is: het afspelen is dan gestopt.
    */
    bool hasBatchPlaybackEnded (int batch) const;

    /** Stand van de laatste Analyze All; total == 0 als er nog geen was. */
    BatchSummary getBatchSummary() const;

    //==============================================================================
    /** Eén regel in het overzichtsvenster: momentopname van één instance. */
    struct OverviewRow
    {
        int instanceNumber = 0;
        juce::String trackName;
        int numChannels = 2;          // 1 = mono, 2 = stereo
        int mode = 0;                 // 0 = Peak, 1 = RMS
        bool measured = false;
        float peakDb = 0.0f, rmsDb = 0.0f;
        float autoDb = 0.0f, trimDb = 0.0f, totalDb = 0.0f;
        bool limitedByCeiling = false;
        int analysisState = 0;        // RoughStartProcessor::AnalysisState
        int lastResult = 0;           // RoughStartProcessor::AnalysisResult
    };

    std::vector<OverviewRow> getOverview() const;

    /** Zet methode, doelen en plafond van alle instances gelijk. Alleen vanaf de berichtthread. */
    int applyTargetSettingsToAll (const roughstart::TargetSettings& settings);

    //==============================================================================
    /** Verhoogt bij elke aan- of afmelding; editors gebruiken dit om te verversen. */
    juce::uint32 getChangeCount() const noexcept { return changeCount.load(); }

    void notifyChanged() noexcept { ++changeCount; }

private:
    mutable juce::CriticalSection lock;
    std::vector<RoughStartProcessor*> instances;
    int nextInstanceNumber = 1;
    int currentBatch = 0;
    std::atomic<juce::uint32> changeCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InstanceRegistry)
};
