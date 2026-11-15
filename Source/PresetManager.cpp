#include "PresetManager.h"

namespace roughstart
{
    namespace
    {
        const juce::String rootTag = "RoughStartPreset";
        constexpr int formatVersion = 1;

        float readClamped (const juce::XmlElement& xml, const char* attribute,
                           float fallback, float low, float high)
        {
            return juce::jlimit (low, high, (float) xml.getDoubleAttribute (attribute, fallback));
        }
    }

    PresetManager::PresetManager (juce::File presetDirectory)
        : directory (std::move (presetDirectory))
    {
    }

    juce::File PresetManager::getDefaultDirectory()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Application Support")
                   .getChildFile ("RoughStart")
                   .getChildFile ("Presets");
    }

    juce::String PresetManager::sanitiseName (const juce::String& name)
    {
        return name.removeCharacters ("/\\:*?\"<>|").trim().substring (0, 64);
    }

    juce::File PresetManager::getFile (const juce::String& name) const
    {
        return directory.getChildFile (sanitiseName (name) + fileExtension);
    }

    juce::StringArray PresetManager::getPresetNames() const
    {
        juce::StringArray names;

        for (const auto& file : directory.findChildFiles (juce::File::findFiles, false,
                                                          juce::String ("*") + fileExtension))
        {
            if (auto xml = juce::XmlDocument::parse (file))
                if (fromXml (*xml).has_value())
                    names.add (file.getFileNameWithoutExtension());
        }

        names.sortNatural();
        return names;
    }

    bool PresetManager::exists (const juce::String& name) const
    {
        return getFile (name).existsAsFile();
    }

    bool PresetManager::save (const juce::String& name, const TargetSettings& settings)
    {
        const auto clean = sanitiseName (name);
        if (clean.isEmpty() || ! directory.createDirectory())
            return false;

        return toXml (clean, settings)->writeTo (getFile (clean));
    }

    std::optional<TargetSettings> PresetManager::load (const juce::String& name) const
    {
        if (auto xml = juce::XmlDocument::parse (getFile (name)))
            return fromXml (*xml);

        return std::nullopt;
    }

    bool PresetManager::remove (const juce::String& name)
    {
        const auto file = getFile (name);
        return file.existsAsFile() && file.deleteFile();
    }

    juce::String PresetManager::importFile (const juce::File& source)
    {
        auto xml = juce::XmlDocument::parse (source);
        if (xml == nullptr)
            return {};

        const auto settings = fromXml (*xml);
        if (! settings.has_value())
            return {};

        auto base = sanitiseName (xml->getStringAttribute ("name", source.getFileNameWithoutExtension()));
        if (base.isEmpty())
            base = "Imported";

        auto name = base;
        for (int i = 2; exists (name); ++i)
            name = base + " (" + juce::String (i) + ")";

        return save (name, *settings) ? name : juce::String();
    }

    bool PresetManager::exportPreset (const juce::String& name, const juce::File& destination) const
    {
        const auto settings = load (name);
        if (! settings.has_value())
            return false;

        return toXml (sanitiseName (name), *settings)->writeTo (destination);
    }

    //==============================================================================
    const std::vector<PresetManager::NamedPreset>& PresetManager::getFactoryPresets()
    {
        auto make = [] (int mode, float peak, float rms, float ceiling)
        {
            TargetSettings s;
            s.mode = mode;
            s.targetPeakDb = peak;
            s.targetRmsDb = rms;
            s.ceilingDb = ceiling;
            return s;
        };

        static const std::vector<NamedPreset> presets {
            { "RMS -18 (standard)",   make (1, -12.0f, -18.0f, -6.0f) },
            { "RMS -24 (dynamic)",    make (1, -12.0f, -24.0f, -6.0f) },
            { "Peak -12 (classic)",   make (0, -12.0f, -18.0f, -6.0f) },
        };

        return presets;
    }

    std::optional<TargetSettings> PresetManager::getFactoryPreset (const juce::String& name)
    {
        for (const auto& p : getFactoryPresets())
            if (p.name == name)
                return p.settings;

        return std::nullopt;
    }

    PresetManager::PresetRef PresetManager::getDefaultPreset() const
    {
        const PresetRef fallback { true, getFactoryPresets().front().name };

        auto xml = juce::XmlDocument::parse (getSettingsFile());
        if (xml == nullptr || ! xml->hasTagName ("RoughStartSettings"))
            return fallback;

        PresetRef ref { xml->getStringAttribute ("defaultType") != "user",
                        xml->getStringAttribute ("defaultPreset") };

        if (ref.name.isEmpty())
            return fallback;

        if (ref.factory ? ! getFactoryPreset (ref.name).has_value() : ! exists (ref.name))
            return fallback;

        return ref;
    }

    bool PresetManager::setDefaultPreset (const PresetRef& ref)
    {
        if (! directory.createDirectory())
            return false;

        auto xml = readSettings();
        xml.setAttribute ("defaultType", ref.factory ? "factory" : "user");
        xml.setAttribute ("defaultPreset", ref.name);
        return writeSettings (xml);
    }

    juce::XmlElement PresetManager::readSettings() const
    {
        if (auto xml = juce::XmlDocument::parse (getSettingsFile()))
            if (xml->hasTagName ("RoughStartSettings"))
                return *xml;

        return juce::XmlElement ("RoughStartSettings");
    }

    bool PresetManager::writeSettings (const juce::XmlElement& xml)
    {
        return directory.createDirectory() && xml.writeTo (getSettingsFile());
    }

    float PresetManager::getUiScale() const
    {
        return juce::jlimit (1.0f, 2.0f, (float) readSettings().getDoubleAttribute ("uiScale", 1.0));
    }

    bool PresetManager::setUiScale (float scale)
    {
        auto xml = readSettings();
        xml.setAttribute ("uiScale", (double) juce::jlimit (1.0f, 2.0f, scale));
        return writeSettings (xml);
    }

    TargetSettings PresetManager::getDefaultSettings() const
    {
        const auto ref = getDefaultPreset();

        if (ref.factory)
            return getFactoryPreset (ref.name).value_or (getFactoryPresets().front().settings);

        return load (ref.name).value_or (getFactoryPresets().front().settings);
    }

    //==============================================================================
    std::unique_ptr<juce::XmlElement> PresetManager::toXml (const juce::String& name, const TargetSettings& s)
    {
        auto xml = std::make_unique<juce::XmlElement> (rootTag);
        xml->setAttribute ("version", formatVersion);
        xml->setAttribute ("name", name);
        xml->setAttribute ("mode", s.mode == 1 ? "rms" : "peak");
        xml->setAttribute ("targetPeakDb", (double) s.targetPeakDb);
        xml->setAttribute ("targetRmsDb", (double) s.targetRmsDb);
        xml->setAttribute ("ceilingDb", (double) s.ceilingDb);
        return xml;
    }

    std::optional<TargetSettings> PresetManager::fromXml (const juce::XmlElement& xml)
    {
        if (! xml.hasTagName (rootTag))
            return std::nullopt;

        TargetSettings defaults, s;
        s.mode         = xml.getStringAttribute ("mode").equalsIgnoreCase ("rms") ? 1 : 0;
        s.targetPeakDb = readClamped (xml, "targetPeakDb", defaults.targetPeakDb, -30.0f, 0.0f);
        s.targetRmsDb  = readClamped (xml, "targetRmsDb",  defaults.targetRmsDb,  -40.0f, 0.0f);
        s.ceilingDb    = readClamped (xml, "ceilingDb",    defaults.ceilingDb,    -24.0f, 0.0f);
        return s;
    }
}
