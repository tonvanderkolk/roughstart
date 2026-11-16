// Rendert de editor naar PNG's om het ontwerp te bekijken zonder Logic.
// Gebruik: RoughStartSnapshot <uitvoermap>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Source/PluginProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

static void save (juce::Component& c, const juce::File& file)
{
    auto img = c.createComponentSnapshot (c.getLocalBounds(), true, 2.0f);
    file.deleteFile();
    juce::FileOutputStream out (file);
    juce::PNGImageFormat().writeImageToStream (img, out);
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File dir (argc > 1 ? juce::String (argv[1]) : juce::File::getCurrentWorkingDirectory().getFullPathName());
    dir.createDirectory();

    std::unique_ptr<juce::AudioProcessor> base (createPluginFilter());
    auto& p = dynamic_cast<RoughStartProcessor&> (*base);
    p.setPlayConfigDetails (2, 2, 48000.0, 512);
    p.prepareToPlay (48000.0, 512);

    juce::AudioProcessor::TrackProperties props;
    props.name = "YachtPop 2 - Be With You_Drums";
    p.updateTrackProperties (props);
    p.setGroup ("Drums");

    // Nog niet geanalyseerd
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        save (*ed, dir.getChildFile ("track_empty.png"));
    }

    // Meting: drums (piek -1.0, RMS -18.6) via een stuk ruis-achtige sinus
    p.startAnalysis();
    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    juce::Random rnd (1);
    for (int b = 0; b < 400; ++b)
    {
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
                buf.setSample (ch, i, (rnd.nextFloat() * 2.0f - 1.0f) * ((b % 8 == 0 && i < 40) ? 0.89f : 0.12f));
        p.processBlock (buf, midi);
    }
    p.stopAnalysis();
    if (auto* t = p.parameters.getParameter ("trim"))
        t->setValueNotifyingHost (t->convertTo0to1 (1.5f));

    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        save (*ed, dir.getChildFile ("track_measured.png"));
    }

    // Extra sporen voor het overzicht
    struct Fake { const char* group; const char* name; float loud, quiet; int every; bool mono; bool analyze; float trim; };
    const Fake fakes[] = {
        { "Bass",   "YachtPop 2 - Be With You_Bass",   0.34f, 0.10f, 4,  false, true,  -1.0f },
        { "",       "YachtPop 2 - Be With You_Guitar", 0.64f, 0.04f, 12, false, true,   0.0f },
        { "Vocals", "YachtPop 2 - Be With You_Vocals", 0.79f, 0.09f, 10, false, true,   2.0f },
        { "Bass",   "Fender Pbass 66",                 0.75f, 0.14f, 6,  true,  true,   0.0f },
        { "Keys",   "Jazz Organ",                      0.0f,  0.0f,  1,  false, false,  0.0f },
    };
    std::vector<std::unique_ptr<juce::AudioProcessor>> others;
    for (const auto& f : fakes)
    {
        others.emplace_back (createPluginFilter());
        auto& o = dynamic_cast<RoughStartProcessor&> (*others.back());
        const int ch = f.mono ? 1 : 2;
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (f.mono ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
        layout.outputBuses.add (f.mono ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
        o.setBusesLayout (layout);
        o.prepareToPlay (48000.0, 512);
        juce::AudioProcessor::TrackProperties tp;
        tp.name = f.name;
        o.updateTrackProperties (tp);
        o.setGroup (f.group);

        if (f.analyze)
        {
            o.startAnalysis();
            juce::AudioBuffer<float> b2 (ch, 512);
            for (int b = 0; b < 400; ++b)
            {
                for (int c = 0; c < ch; ++c)
                    for (int i = 0; i < 512; ++i)
                        b2.setSample (c, i, (rnd.nextFloat() * 2.0f - 1.0f) * ((b % f.every == 0 && i < 60) ? f.loud : f.quiet));
                o.processBlock (b2, midi);
            }
            o.stopAnalysis();
        }
        if (auto* t = o.parameters.getParameter ("trim"))
            t->setValueNotifyingHost (t->convertTo0to1 (f.trim));
    }

    // Groepscontrole: Apply to group raakt alleen de eigen groep, groep overleeft opslaan/laden
    {
        roughstart::TargetSettings keys;
        keys.mode = 1; keys.targetPeakDb = -12.0f; keys.targetRmsDb = -20.0f; keys.ceilingDb = -6.0f;
        const int n = p.getRegistry().applyTargetSettingsToGroup ("Bass", keys);
        auto& bass = dynamic_cast<RoughStartProcessor&> (*others[0]);
        auto& guitar = dynamic_cast<RoughStartProcessor&> (*others[1]);

        juce::MemoryBlock mb;
        bass.getStateInformation (mb);
        std::unique_ptr<juce::AudioProcessor> copy (createPluginFilter());
        auto& c = dynamic_cast<RoughStartProcessor&> (*copy);
        c.setStateInformation (mb.getData(), (int) mb.getSize());

        const bool ok = n == 2 && bass.getTargetSettings() == keys && guitar.getTargetSettings() != keys
                        && c.getGroup() == "Bass" && guitar.getGroup().isEmpty()
                        && p.getRegistry().getGroupNames().joinIntoString (",") == "Bass,Drums,Keys,Vocals";
        std::printf ("Group check: %s (applied to %d)\n", ok ? "OK" : "FAILED", n);
        if (! ok)
            return 1;
    }

    p.lastEditorView = 1;
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        save (*ed, dir.getChildFile ("overview.png"));
    }

    return 0;
}
