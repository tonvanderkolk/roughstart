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
