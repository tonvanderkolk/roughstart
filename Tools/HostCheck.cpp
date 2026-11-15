// Laadt RoughStart als VST3 (of AU) zoals een DAW dat doet en controleert de basis:
// laden, mono en stereo, audio verwerken, state opslaan en terugzetten, editor openen,
// en twee instances in hetzelfde proces. Gebruik: RoughStartHostCheck <pad naar .vst3>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

static int failures = 0;
static void check (bool ok, const juce::String& what)
{
    std::cout << (ok ? "  OK    " : "  FOUT  ") << what << std::endl;
    if (! ok) ++failures;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    if (argc < 2) { std::cout << "Gebruik: RoughStartHostCheck <plugin>" << std::endl; return 2; }

    juce::AudioPluginFormatManager formats;
    juce::addDefaultFormatsToManager (formats);

    juce::OwnedArray<juce::PluginDescription> found;
    for (auto* f : formats.getFormats())
        if (f->fileMightContainThisPluginType (argv[1]))
            f->findAllTypesForFile (found, argv[1]);

    check (found.size() == 1, "plugin gevonden (" + juce::String (found.size()) + ")");
    if (found.isEmpty()) return 1;
    std::cout << "  " << found[0]->pluginFormatName << ": " << found[0]->name << " " << found[0]->version
              << " van " << found[0]->manufacturerName << std::endl;

    juce::String error;
    auto a = formats.createPluginInstance (*found[0], 48000.0, 512, error);
    auto b = formats.createPluginInstance (*found[0], 48000.0, 512, error);
    check (a != nullptr && b != nullptr, "twee instances aangemaakt " + error);
    if (a == nullptr || b == nullptr) return 1;

    // Mono en stereo
    for (auto set : { juce::AudioChannelSet::mono(), juce::AudioChannelSet::stereo() })
    {
        juce::AudioProcessor::BusesLayout l;
        l.inputBuses.add (set);
        l.outputBuses.add (set);
        check (a->checkBusesLayoutSupported (l), "layout " + set.getDescription());
    }

    a->prepareToPlay (48000.0, 512);
    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    for (int i = 0; i < 512; ++i)
        buf.setSample (0, i, 0.5f * std::sin ((float) i * 0.1f)), buf.setSample (1, i, buf.getSample (0, i));
    a->processBlock (buf, midi);
    check (buf.getMagnitude (0, 512) > 0.4f && buf.getMagnitude (0, 512) < 0.6f, "audio komt ongewijzigd door (0 dB)");
    check (a->getLatencySamples() == 0, "nul latency");

    // Parameters en state
    auto& params = a->getParameters();
    check (params.size() >= 5, juce::String (params.size()) + " parameters");
    juce::AudioProcessorParameter* trim = nullptr;
    for (auto* p : params)
        if (p->getName (32) == "Trim") trim = p;
    check (trim != nullptr, "Trim-parameter aanwezig");

    if (trim != nullptr)
    {
        trim->setValueNotifyingHost (0.75f);   // +12 dB
        juce::MemoryBlock state;
        a->getStateInformation (state);
        trim->setValueNotifyingHost (0.5f);
        a->setStateInformation (state.getData(), (int) state.getSize());
        check (std::abs (trim->getValue() - 0.75f) < 0.01f, "state opslaan en terugzetten");

        juce::AudioBuffer<float> b2 (2, 512);
        float last = 0.0f;
        for (int k = 0; k < 40; ++k)
        {
            for (int i = 0; i < 512; ++i) b2.setSample (0, i, 0.1f), b2.setSample (1, i, 0.1f);
            a->processBlock (b2, midi);
            last = b2.getSample (0, 511);
        }
        check (std::abs (last - 0.1f * 3.98f) < 0.02f, "Trim +12 dB wordt toegepast (" + juce::String (last / 0.1f, 2) + "x)");
    }

    // Editor (alleen als een venster mogelijk is; zonder venster laadt een VST3-editor niet)
    if (argc > 2 && juce::String (argv[2]) == "--editor")
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (a->createEditorIfNeeded());
        check (ed != nullptr && ed->getWidth() > 300, "editor opent (" + juce::String (ed ? ed->getWidth() : 0) + " px breed)");
        a->editorBeingDeleted (ed.get());
    }

    a->releaseResources();
    std::cout << (failures == 0 ? "Alles in orde." : juce::String (failures) + " fout(en).") << std::endl;
    return failures == 0 ? 0 : 1;
}
