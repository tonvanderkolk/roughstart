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

void InstanceRegistry::pingAll()
{
    const juce::ScopedLock sl (lock);
    for (auto* instance : instances)
        instance->receivePing();
}
