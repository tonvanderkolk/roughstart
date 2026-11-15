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

    p.lastEditorView = 1;
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        save (*ed, dir.getChildFile ("overview.png"));
    }

    return 0;
}
