#pragma once

#include <juce_data_structures/juce_data_structures.h>
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

    /** True als minstens één instance in deze batch het afspelen heeft gezien. */
    bool hasBatchSeenPlayback (int batch) const;

    /**
        True als het transport van de host stilstaat. Over alle instances heen:
        is de nieuwste melding "staat stil" recenter dan de nieuwste melding
        "speelt af"? Of krijgt geen enkele instance nog audio (hosts die bij stop
        niets meer verwerken)? Dat één instance even geen audio krijgt, telt niet:
        Logic slaat stille sporen tijdens het afspelen soms over.
    */
    bool hasPlaybackStopped() const;

    /** Stand van de laatste Analyze All; total == 0 als er nog geen was. */
    BatchSummary getBatchSummary() const;

    //==============================================================================
    /** Eén regel in het overzichtsvenster: momentopname van één instance. */
    struct OverviewRow
    {
        int instanceNumber = 0;
        juce::String trackName;
        juce::String group;           // leeg = geen groep
        int numChannels = 2;          // 1 = mono, 2 = stereo
        int mode = 0;                 // 0 = Peak, 1 = RMS
        bool measured = false;
        float peakDb = 0.0f, rmsDb = 0.0f;
        float autoDb = 0.0f, trimDb = 0.0f, totalDb = 0.0f;
        bool limitedByCeiling = false;
        int analysisState = 0;        // RoughStartProcessor::AnalysisState
        int lastResult = 0;           // RoughStartProcessor::AnalysisResult
        bool liveValid = false;       // tijdens meten: al signaal gezien
        float livePeakDb = 0.0f, liveRmsDb = 0.0f;
    };

    std::vector<OverviewRow> getOverview() const;

    /** Zet methode, doelen en plafond van alle instances gelijk. Alleen vanaf de berichtthread. */
    int applyTargetSettingsToAll (const roughstart::TargetSettings& settings);

    /** Als hierboven, maar alleen voor de instances in deze groep. Geeft het aantal terug. */
    int applyTargetSettingsToGroup (const juce::String& group, const roughstart::TargetSettings& settings);

    /** Alle groepsnamen die nu in het project voorkomen, alfabetisch, zonder lege. */
    juce::StringArray getGroupNames() const;

    /** Zet Trim van één instance (op volgnummer). Alleen vanaf de berichtthread. */
    void setTrimDb (int instanceNumber, float trimDb);

    /** Begin/einde van een sleepbeweging op Trim (voor automatisering in de host). */
    void setTrimGesture (int instanceNumber, bool starting);

    /**
        Legt een afgeronde Trim-wijziging vast in de gedeelde undo-geschiedenis.
        Logic's eigen Undo geldt per plug-invenster, dus wijzigingen vanuit het
        overzicht aan andere sporen krijgen hier hun eigen undo.
    */
    void recordTrimChange (int instanceNumber, float oldDb, float newDb);

    juce::UndoManager& getUndoManager() noexcept { return undoManager; }

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
    juce::UndoManager undoManager { 0, 200 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InstanceRegistry)
};
