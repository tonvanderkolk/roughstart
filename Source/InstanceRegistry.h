#pragma once

#include <juce_core/juce_core.h>
#include <vector>

class RoughStartProcessor;

/**
    Gedeelde lijst van alle RoughStart-instances in hetzelfde hostproces.

    Wordt via juce::SharedResourcePointer<InstanceRegistry> gedeeld: de eerste
    instance maakt het object aan, de laatste ruimt het op. Werkt alleen als de
    host alle instances in één proces laadt (Logic doet dat voor AUv2).
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

    /** Laat de ping-indicator in alle instances oplichten. */
    void pingAll();

    /** Verhoogt bij elke aan- of afmelding; editors gebruiken dit om te verversen. */
    juce::uint32 getChangeCount() const noexcept { return changeCount.load(); }

    void notifyChanged() noexcept { ++changeCount; }

private:
    mutable juce::CriticalSection lock;
    std::vector<RoughStartProcessor*> instances;
    int nextInstanceNumber = 1;
    std::atomic<juce::uint32> changeCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InstanceRegistry)
};
