#include <juce_core/juce_core.h>
#include "../Source/LevelMeter.h"

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

        beginTest ("Auto-gain: verschil met doel, begrensd op ±24 dB");
        {
            expectWithinAbsoluteError (computeAutoGainDb (-6.0f, -12.0f), -6.0f, 0.001f);
            expectWithinAbsoluteError (computeAutoGainDb (-30.0f, -18.0f), 12.0f, 0.001f);
            expectEquals (computeAutoGainDb (-80.0f, -12.0f), 24.0f);
            expectEquals (computeAutoGainDb (0.0f, -40.0f), -24.0f);
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

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    return failures > 0 ? 1 : 0;
}
