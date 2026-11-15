// Speelt Analyze All na zoals Logic het doet, met sporen die Logic tijdens het
// afspelen tijdelijk overslaat. Gebruik: RoughStartAnalyzeSim
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Source/PluginProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

namespace
{
    struct FakeHead final : juce::AudioPlayHead
    {
        bool playing = false;
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo p;
            p.setIsPlaying (playing);
            return p;
        }
    };

    int failures = 0;
    void check (bool ok, const juce::String& what)
    {
        std::cout << (ok ? "  OK    " : "  FOUT  ") << what << std::endl;
        if (! ok) ++failures;
    }
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    FakeHead head;

    // drums: altijd audio. horns: speelt, valt na 1 s 3 s stil (Logic slaat over).
    // bass: krijgt pas na 2 s audio. pad: krijgt nooit audio (leeg spoor).
    const char* names[] = { "drums", "horns", "bass", "pad" };
    std::vector<std::unique_ptr<juce::AudioProcessor>> owned;
    std::vector<RoughStartProcessor*> p;
    for (auto* n : names)
    {
        owned.emplace_back (createPluginFilter());
        auto* rp = dynamic_cast<RoughStartProcessor*> (owned.back().get());
        rp->setPlayConfigDetails (2, 2, 48000.0, 512);
        rp->prepareToPlay (48000.0, 512);
        rp->setPlayHead (&head);
        juce::AudioProcessor::TrackProperties tp; tp.name = n; rp->updateTrackProperties (tp);
        p.push_back (rp);
    }

    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    auto feed = [&] (RoughStartProcessor* rp, float level)
    {
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < 512; ++i)
                buf.setSample (c, i, level * std::sin ((float) i * 0.07f));
        rp->processBlock (buf, midi);
    };

    // Stap van ~10 ms: blokken leveren en timers laten lopen
    auto run = [&] (double seconds, std::function<void (double)> perStep)
    {
        const int steps = (int) (seconds * 100.0);
        for (int s = 0; s < steps; ++s)
        {
            perStep ((double) s / 100.0);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (10);
        }
    };

    p[0]->getRegistry().analyzeAll();

    // Stilstand vóór afspelen: blokken met playing = false
    run (0.5, [&] (double) { for (auto* rp : p) if (rp != p[3]) feed (rp, 0.0f); });
    check (p[0]->getAnalysisState() == RoughStartProcessor::AnalysisState::waitingForPlayback, "wacht op afspelen");

    // Afspelen, 6 seconden
    head.playing = true;
    run (6.0, [&] (double t)
    {
        feed (p[0], 0.5f);
        if (t < 1.0 || t > 4.0) feed (p[1], 0.4f);     // horns: 3 s overgeslagen
        if (t > 2.0)            feed (p[2], 0.3f);     // bass: komt later binnen
    });

    check (p[1]->isAnalyzing(), "horns meet nog na een gat van 3 s");
    check (p[2]->isAnalyzing(), "bass meet nog");

    // Stop: Logic levert weer blokken met playing = false
    head.playing = false;
    run (0.5, [&] (double) { for (auto* rp : p) if (rp != p[3]) feed (rp, 0.0f); });

    using R = RoughStartProcessor::AnalysisResult;
    check (! p[0]->isAnalyzing() && p[0]->getLastAnalysisResult() == R::stored, "drums opgeslagen na stop");
    check (! p[1]->isAnalyzing() && p[1]->getLastAnalysisResult() == R::stored, "horns opgeslagen na stop");
    check (! p[2]->isAnalyzing() && p[2]->getLastAnalysisResult() == R::stored, "bass opgeslagen na stop");
    check (! p[3]->isAnalyzing() && p[3]->getLastAnalysisResult() == R::noSignal, "leeg spoor: geen signaal");

    // Host die bij stop helemaal niets meer verwerkt: na ~3 s toch afgerond
    p[0]->getRegistry().analyzeAll();
    head.playing = true;
    run (1.0, [&] (double) { for (auto* rp : p) if (rp != p[3]) feed (rp, 0.4f); });
    head.playing = false;
    run (3.5, [&] (double) {});
    check (! p[0]->isAnalyzing() && ! p[2]->isAnalyzing(), "zonder blokken na stop: na 3 s afgerond");

    // Losse Analyze track op een spoor met een gat
    p[1]->startAnalysis();
    head.playing = true;
    run (4.0, [&] (double t) { feed (p[0], 0.5f); if (t < 1.0 || t > 3.0) feed (p[1], 0.4f); });
    check (p[1]->isAnalyzing(), "Analyze track blijft meten over een gat van 2 s");
    head.playing = false;
    run (0.5, [&] (double) { feed (p[0], 0.0f); feed (p[1], 0.0f); });
    check (! p[1]->isAnalyzing() && p[1]->getLastAnalysisResult() == R::stored, "Analyze track opgeslagen na stop");

    for (auto* rp : p) rp->setPlayHead (nullptr);
    std::cout << (failures == 0 ? "Alles in orde." : juce::String (failures) + " fout(en).") << std::endl;
    return failures == 0 ? 0 : 1;
}
