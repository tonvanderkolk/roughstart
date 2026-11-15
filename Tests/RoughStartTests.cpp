#include <juce_core/juce_core.h>
#include "../Source/LevelMeter.h"
#include "../Source/PresetManager.h"

#include <vector>

using namespace roughstart;

namespace
{
    constexpr double sampleRate = 48000.0;

    std::vector<float> makeSine (float amplitude, double freq, int numSamples)
    {
        std::vector<float> v ((size_t) numSamples);
        for (int i = 0; i < numSamples; ++i)
            v[(size_t) i] = amplitude * (float) std::sin (2.0 * juce::MathConstants<double>::pi * freq * i / sampleRate);
        return v;
    }

    void feedStereo (LevelMeter& meter, const std::vector<float>& signal, int blockSize)
    {
        const int total = (int) signal.size();
        for (int pos = 0; pos < total; pos += blockSize)
        {
            const float* chans[2] = { signal.data() + pos, signal.data() + pos };
            meter.process (chans, 2, std::min (blockSize, total - pos));
        }
    }
}

struct LevelMeterTests final : juce::UnitTest
{
    LevelMeterTests() : juce::UnitTest ("LevelMeter", "RoughStart") {}

    void runTest() override
    {
        beginTest ("Sinus 0.5: piek -6.02 dBFS, RMS -9.03 dBFS");
        {
            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, makeSine (0.5f, 1000.0, (int) sampleRate), 512);
            expect (m.hasSignal());
            expectWithinAbsoluteError (m.getPeakDb(), -6.02f, 0.05f);
            expectWithinAbsoluteError (m.getRmsDb(), -9.03f, 0.05f);
        }

        beginTest ("Alleen stilte: geen signaal");
        {
            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, std::vector<float> ((size_t) sampleRate, 0.0f), 512);
            expect (! m.hasSignal());
            expectEquals (m.getRmsDb(), minusInfinityDb);
        }

        beginTest ("Stille passages tellen niet mee in RMS");
        {
            auto signal = makeSine (0.5f, 1000.0, (int) sampleRate);
            signal.insert (signal.end(), (size_t) sampleRate * 2, 0.0f);

            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, signal, 512);
            expectWithinAbsoluteError (m.getRmsDb(), -9.03f, 0.05f);
        }

        beginTest ("Signaal onder -60 dBFS telt als stilte");
        {
            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, makeSine (0.0005f, 1000.0, (int) sampleRate), 512);
            expect (! m.hasSignal());
        }

        beginTest ("Uitkomst hangt niet af van de blokgrootte");
        {
            const auto signal = makeSine (0.3f, 440.0, (int) sampleRate * 2);
            LevelMeter a, b;
            a.prepare (sampleRate);
            b.prepare (sampleRate);
            feedStereo (a, signal, 37);
            feedStereo (b, signal, 4096);
            expectWithinAbsoluteError (a.getRmsDb(), b.getRmsDb(), 0.001f);
            expectWithinAbsoluteError (a.getPeakDb(), b.getPeakDb(), 0.001f);
        }

        beginTest ("Reset wist de meting");
        {
            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, makeSine (0.5f, 1000.0, (int) sampleRate), 512);
            m.reset();
            expect (! m.hasSignal());
        }

        beginTest ("Zachte staart (-40 dB onder het signaal) telt niet mee in RMS");
        {
            // Luid deel 0.5 (RMS -9.03) en een even lang zacht deel 0.005 (-49 dBFS,
            // boven de absolute drempel). Zonder relatieve drempel zou de RMS
            // rond -12 dB uitkomen.
            auto signal = makeSine (0.5f, 1000.0, (int) sampleRate);
            const auto quiet = makeSine (0.005f, 1000.0, (int) sampleRate);
            signal.insert (signal.end(), quiet.begin(), quiet.end());

            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, signal, 512);
            expectWithinAbsoluteError (m.getRmsDb(), -9.03f, 0.1f);
            expectWithinAbsoluteError (m.getPeakDb(), -6.02f, 0.05f);
        }

        beginTest ("Dynamisch signaal (-10 dB verschil) telt wel volledig mee");
        {
            auto signal = makeSine (0.5f, 1000.0, (int) sampleRate);
            const auto softer = makeSine (0.158f, 1000.0, (int) sampleRate);   // -10 dB
            signal.insert (signal.end(), softer.begin(), softer.end());

            // verwachte RMS: gemiddelde vermogen van beide delen
            const double expected = 10.0 * std::log10 ((0.125 + 0.158 * 0.158 / 2.0) / 2.0);

            LevelMeter m;
            m.prepare (sampleRate);
            feedStereo (m, signal, 512);
            expectWithinAbsoluteError (m.getRmsDb(), (float) expected, 0.1f);
        }

        beginTest ("Auto-gain: verschil met doel, begrensd op ±24 dB");
        {
            constexpr float noCeiling = 100.0f;
            expectWithinAbsoluteError (computeAutoGain (-6.0f, -12.0f, -6.0f, noCeiling).gainDb, -6.0f, 0.001f);
            expectWithinAbsoluteError (computeAutoGain (-30.0f, -18.0f, -20.0f, noCeiling).gainDb, 12.0f, 0.001f);
            expectEquals (computeAutoGain (-80.0f, -12.0f, -80.0f, noCeiling).gainDb, 24.0f);
            expectEquals (computeAutoGain (0.0f, -40.0f, 0.0f, noCeiling).gainDb, -24.0f);
        }

        beginTest ("Piekplafond begrenst Auto");
        {
            // Drums uit de Logic-test: piek -0.8, RMS -20.6, doel RMS -18, plafond -6
            auto r = computeAutoGain (-20.6f, -18.0f, -0.8f, -6.0f);
            expectWithinAbsoluteError (r.gainDb, -5.2f, 0.001f);
            expect (r.limitedByCeiling);

            // Ruim onder het plafond: geen begrenzing
            r = computeAutoGain (-30.0f, -18.0f, -20.0f, -6.0f);
            expectWithinAbsoluteError (r.gainDb, 12.0f, 0.001f);
            expect (! r.limitedByCeiling);

            // Piekdoel boven het plafond: plafond wint
            r = computeAutoGain (-20.0f, -3.0f, -20.0f, -6.0f);
            expectWithinAbsoluteError (r.gainDb, 14.0f, 0.001f);
            expect (r.limitedByCeiling);
        }

        beginTest ("dB-omrekening");
        {
            expectWithinAbsoluteError (gainToDb (0.5), -6.0206f, 0.001f);
            expectWithinAbsoluteError (dbToGain (-6.0206f), 0.5f, 0.0001f);
            expectEquals (gainToDb (0.0), minusInfinityDb);
        }
    }
};

static LevelMeterTests levelMeterTests;

struct PresetManagerTests final : juce::UnitTest
{
    PresetManagerTests() : juce::UnitTest ("PresetManager", "RoughStart") {}

    void runTest() override
    {
        const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                             .getChildFile ("RoughStartTests_" + juce::Uuid().toString());
        PresetManager pm (dir);

        TargetSettings rms;
        rms.mode = 1;
        rms.targetRmsDb = -20.0f;
        rms.ceilingDb = -8.0f;

        beginTest ("Opslaan en laden geeft dezelfde instellingen");
        {
            expect (pm.save ("Drums RMS", rms));
            const auto loaded = pm.load ("Drums RMS");
            expect (loaded.has_value());
            expect (*loaded == rms);
            expect (pm.getPresetNames().contains ("Drums RMS"));
        }

        beginTest ("Ongeldige tekens in de naam worden verwijderd");
        {
            expectEquals (PresetManager::sanitiseName (" a/b:c? "), juce::String ("abc"));
            expect (! pm.save ("///", rms));
        }

        beginTest ("Export en import, met unieke naam bij dubbel");
        {
            const auto exported = dir.getChildFile ("export_test.xml");
            expect (pm.exportPreset ("Drums RMS", exported));

            const auto imported = pm.importFile (exported);
            expectEquals (imported, juce::String ("Drums RMS (2)"));
            expect (*pm.load (imported) == rms);
        }

        beginTest ("Ongeldige bestanden worden geweigerd");
        {
            const auto bad = dir.getChildFile ("bad.rspreset");
            bad.replaceWithText ("<SomethingElse/>");
            expect (pm.importFile (bad).isEmpty());
            expect (! pm.getPresetNames().contains ("bad"));
        }

        beginTest ("Waarden buiten bereik worden begrensd");
        {
            juce::XmlElement xml ("RoughStartPreset");
            xml.setAttribute ("mode", "rms");
            xml.setAttribute ("ceilingDb", 12.0);
            xml.setAttribute ("targetRmsDb", -99.0);
            const auto s = PresetManager::fromXml (xml);
            expect (s.has_value());
            expectEquals (s->ceilingDb, 0.0f);
            expectEquals (s->targetRmsDb, -40.0f);
            expectEquals (s->targetPeakDb, -12.0f);
            expectEquals (s->mode, 1);
        }

        beginTest ("Factory presets en standaard");
        {
            PresetManager fresh (dir.getChildFile ("fresh"));
            expect (PresetManager::getFactoryPresets().size() >= 3);

            // Zonder instelling: eerste factory preset (RMS -18, plafond -6)
            const auto def = fresh.getDefaultPreset();
            expect (def.factory);
            const auto s = fresh.getDefaultSettings();
            expectEquals (s.mode, 1);
            expectEquals (s.targetRmsDb, -18.0f);
            expectEquals (s.ceilingDb, -6.0f);

            // Eigen preset als standaard
            TargetSettings mine;
            mine.mode = 1;
            mine.targetRmsDb = -30.0f;
            expect (fresh.save ("Mine", mine));
            expect (fresh.setDefaultPreset ({ false, "Mine" }));
            expect (fresh.getDefaultPreset() == PresetManager::PresetRef { false, "Mine" });
            expect (fresh.getDefaultSettings() == mine);

            // Venstergrootte naast de standaard bewaren
            expectEquals (fresh.getUiScale(), 1.0f);
            expect (fresh.setUiScale (1.5f));
            expectEquals (fresh.getUiScale(), 1.5f);
            expect (fresh.getDefaultPreset() == PresetManager::PresetRef { false, "Mine" });

            // Settings-bestand telt niet als preset
            expect (! fresh.getPresetNames().contains ("settings"));

            // Eigen standaard verwijderd: terug naar factory
            expect (fresh.remove ("Mine"));
            expect (fresh.getDefaultPreset().factory);
            expect (fresh.getDefaultSettings() == PresetManager::getFactoryPresets().front().settings);
        }

        beginTest ("Verwijderen");
        {
            expect (pm.remove ("Drums RMS"));
            expect (! pm.exists ("Drums RMS"));
        }

        dir.deleteRecursively();
    }
};

static PresetManagerTests presetManagerTests;

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    return failures > 0 ? 1 : 0;
}
