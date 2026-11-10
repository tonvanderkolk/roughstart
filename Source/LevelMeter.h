#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

/*
    Pure DSP zonder JUCE-afhankelijkheid, zodat het los te testen is
    (zie Tests/RoughStartTests.cpp).
*/
namespace roughstart
{
    constexpr float silenceThresholdDb = -60.0f;  // absolute drempel: vensters met een piek hieronder tellen niet mee
    constexpr float relativeGateDb     = -20.0f;  // vensters zoveel onder het gemiddelde tellen niet mee in RMS
    constexpr float maxAutoGainDb      = 24.0f;   // Auto blijft binnen ±24 dB
    constexpr float minusInfinityDb    = -100.0f;

    inline float gainToDb (double gain) noexcept
    {
        return gain > 0.0 ? std::max (minusInfinityDb, (float) (20.0 * std::log10 (gain)))
                          : minusInfinityDb;
    }

    inline float powerToDb (double meanSquare) noexcept
    {
        return meanSquare > 0.0 ? std::max (minusInfinityDb, (float) (10.0 * std::log10 (meanSquare)))
                                : minusInfinityDb;
    }

    inline float dbToGain (float db) noexcept
    {
        return std::pow (10.0f, db * 0.05f);
    }

    struct AutoGainResult
    {
        float gainDb = 0.0f;
        bool limitedByCeiling = false;
    };

    /**
        Auto-gain: het verschil tussen doel en meting, maar nooit zoveel dat de
        gemeten piek boven het plafond uitkomt. Daarna begrensd op ±24 dB.
    */
    inline AutoGainResult computeAutoGain (float measuredDb, float targetDb,
                                           float measuredPeakDb, float ceilingDb) noexcept
    {
        const float wanted  = targetDb - measuredDb;
        const float allowed = ceilingDb - measuredPeakDb;

        AutoGainResult r;
        r.limitedByCeiling = allowed < wanted;
        r.gainDb = std::clamp (std::min (wanted, allowed), -maxAutoGainDb, maxAutoGainDb);
        return r;
    }

    /**
        Meet piek en RMS over een langere periode en slaat stille passages over.

        Het signaal wordt in vensters van ~50 ms verdeeld. Twee drempels:
        - absoluut: een venster telt alleen mee als de piek erin boven -60 dBFS komt;
        - relatief (alleen RMS): vensters waarvan de RMS meer dan 20 dB onder het
          gemiddelde ligt, tellen niet mee. Zo trekken uitklinkende staarten
          tussen drumklappen de RMS niet omlaag. (Zelfde idee als de relatieve
          gate in EBU R128.)

        Daarvoor worden vensters per 0,5 dB in een histogram bijgehouden, zodat
        de relatieve drempel zonder tweede doorgang te bepalen is.

        Piek en RMS worden over alle kanalen samen bepaald. Een onvolledig
        laatste venster telt niet mee. Niet thread-safe: gebruik vanaf één
        thread (de audiothread).
    */
    class LevelMeter
    {
    public:
        void prepare (double sampleRate, double windowSeconds = 0.05) noexcept
        {
            windowSize = std::max<std::int64_t> (1, (std::int64_t) std::lround (sampleRate * windowSeconds));
            reset();
        }

        void reset() noexcept
        {
            windowPos = 0;
            windowPeak = 0.0f;
            windowSumSq = 0.0;
            windowSamples = 0;
            totalPeak = 0.0f;
            activeWindows = 0;
            bins.fill ({});
        }

        void process (const float* const* channels, int numChannels, int numSamples) noexcept
        {
            if (numChannels <= 0)
                return;

            for (int i = 0; i < numSamples; ++i)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    const float s = channels[ch][i];
                    windowPeak = std::max (windowPeak, std::abs (s));
                    windowSumSq += (double) s * (double) s;
                }

                windowSamples += numChannels;

                if (++windowPos >= windowSize)
                    finishWindow();
            }
        }

        bool hasSignal() const noexcept                   { return activeWindows > 0; }
        std::int64_t getNumActiveWindows() const noexcept { return activeWindows; }

        float getPeakDb() const noexcept { return gainToDb (totalPeak); }

        float getRmsDb() const noexcept
        {
            if (activeWindows == 0)
                return minusInfinityDb;

            // 1. Gemiddelde over alle vensters boven de absolute drempel
            double sumSq = 0.0;
            std::int64_t samples = 0;
            for (const auto& b : bins)
            {
                sumSq += b.sumSq;
                samples += b.samples;
            }

            const float threshold = powerToDb (sumSq / (double) samples) + relativeGateDb;

            // 2. Opnieuw, maar alleen vensters boven de relatieve drempel
            double gatedSumSq = 0.0;
            std::int64_t gatedSamples = 0;
            for (size_t i = 0; i < bins.size(); ++i)
            {
                if (binCentreDb (i) >= threshold)
                {
                    gatedSumSq += bins[i].sumSq;
                    gatedSamples += bins[i].samples;
                }
            }

            return gatedSamples > 0 ? powerToDb (gatedSumSq / (double) gatedSamples)
                                    : powerToDb (sumSq / (double) samples);
        }

    private:
        struct Bin
        {
            double sumSq = 0.0;
            std::int64_t samples = 0;
        };

        static constexpr float binLowDb  = -100.0f;
        static constexpr float binStepDb = 0.5f;
        static constexpr size_t numBins  = 230;   // -100 .. +15 dB

        static float binCentreDb (size_t index) noexcept
        {
            return binLowDb + ((float) index + 0.5f) * binStepDb;
        }

        static size_t binIndexFor (float db) noexcept
        {
            const auto i = (long) std::floor ((db - binLowDb) / binStepDb);
            return (size_t) std::clamp<long> (i, 0, (long) numBins - 1);
        }

        void finishWindow() noexcept
        {
            if (windowPeak >= silenceThresholdGain && windowSamples > 0)
            {
                totalPeak = std::max (totalPeak, windowPeak);

                auto& b = bins[binIndexFor (powerToDb (windowSumSq / (double) windowSamples))];
                b.sumSq += windowSumSq;
                b.samples += windowSamples;
                ++activeWindows;
            }

            windowPos = 0;
            windowPeak = 0.0f;
            windowSumSq = 0.0;
            windowSamples = 0;
        }

        static constexpr float silenceThresholdGain = 0.001f; // -60 dBFS

        std::int64_t windowSize = 2400;
        std::int64_t windowPos = 0;
        float windowPeak = 0.0f;
        double windowSumSq = 0.0;
        std::int64_t windowSamples = 0;

        float totalPeak = 0.0f;
        std::int64_t activeWindows = 0;
        std::array<Bin, numBins> bins {};
    };
}
