#pragma once

#include <juce_core/juce_core.h>
#include <optional>

namespace roughstart
{
    /** De doelinstellingen die een preset vastlegt (geen meting, geen Trim). */
    struct TargetSettings
    {
        int   mode         = 1;        // 0 = Peak, 1 = RMS (standaard: RMS -18, plafond -6)
        float targetPeakDb = -12.0f;
        float targetRmsDb  = -18.0f;
        float ceilingDb    = -6.0f;

        bool operator== (const TargetSettings& other) const noexcept
        {
            return mode == other.mode
                && std::abs (targetPeakDb - other.targetPeakDb) < 0.01f
                && std::abs (targetRmsDb  - other.targetRmsDb)  < 0.01f
                && std::abs (ceilingDb    - other.ceilingDb)    < 0.01f;
        }

        bool operator!= (const TargetSettings& other) const noexcept { return ! (*this == other); }
    };

    /**
        Presets als losse XML-bestanden (.rspreset) in een map, standaard
        ~/Library/Application Support/RoughStart/Presets. Een preset is gewoon
        een bestand, dus importeren/exporteren is kopiëren met controle.
    */
    class PresetManager
    {
    public:
        explicit PresetManager (juce::File presetDirectory = getDefaultDirectory());

        static juce::File getDefaultDirectory();
        static constexpr const char* fileExtension = ".rspreset";

        const juce::File& getDirectory() const noexcept { return directory; }

        /** Namen van alle geldige presets, alfabetisch. */
        juce::StringArray getPresetNames() const;

        bool exists (const juce::String& name) const;
        bool save (const juce::String& name, const TargetSettings& settings);
        std::optional<TargetSettings> load (const juce::String& name) const;
        bool remove (const juce::String& name);

        /** Leest een presetbestand en slaat het op in de map. Geeft de naam terug, of leeg bij een ongeldig bestand. */
        juce::String importFile (const juce::File& source);

        bool exportPreset (const juce::String& name, const juce::File& destination) const;

        //==============================================================================
        // Factory presets: ingebouwd, altijd aanwezig, niet te wijzigen of te verwijderen.
        struct NamedPreset
        {
            juce::String name;
            TargetSettings settings;
        };

        static const std::vector<NamedPreset>& getFactoryPresets();
        static std::optional<TargetSettings> getFactoryPreset (const juce::String& name);

        //==============================================================================
        // Standaard voor nieuwe instances: een factory- of eigen preset.
        struct PresetRef
        {
            bool factory = true;
            juce::String name;

            bool operator== (const PresetRef& o) const noexcept { return factory == o.factory && name == o.name; }
        };

        /** De ingestelde standaard; valt terug op de eerste factory preset. */
        PresetRef getDefaultPreset() const;
        bool setDefaultPreset (const PresetRef& ref);

        /** Instellingen voor een nieuwe instance. Bestaat de eigen standaard niet meer, dan de factory-standaard. */
        TargetSettings getDefaultSettings() const;

        static std::unique_ptr<juce::XmlElement> toXml (const juce::String& name, const TargetSettings& settings);
        static std::optional<TargetSettings> fromXml (const juce::XmlElement& xml);
        static juce::String sanitiseName (const juce::String& name);

    private:
        juce::File getFile (const juce::String& name) const;
        juce::File getSettingsFile() const { return directory.getChildFile ("settings.xml"); }

        juce::File directory;
    };
}
