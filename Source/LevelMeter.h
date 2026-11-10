#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

/*
    Pure DSP zonder JUCE-afhankelijkheid, zodat het los te testen is
    (zie Tests/RoughStartTests.cpp).
*/
namespace roughstart
{
    constexpr float silenceThresholdDb = -60.0f;  // vensters stiller dan dit tellen niet mee
    constexpr float maxAutoGainDb      = 24.0f;   // Auto blijft binnen ±24 dB
    constexpr float minusInfinityDb    = -100.0f;

    inline float gainToDb (double gain) noexcept
    {
        return gain > 0.0 ? std::max (minusInfinityDb, (float) (20.0 * std::log10 (gain)))
                          : minusInfinityDb;
    }

    inline float dbToGain (float db) noexcept
    {
        return std::pow (10.0f, db * 0.05f);
    }

    /** Hoeveel dB er bij moet om van het gemeten niveau op het doel te komen, begrensd. */
    inline float computeAutoGainDb (float measuredDb, float targetDb) noexcept
    {
        return std::clamp (targetDb - measuredDb, -maxAutoGainDb, maxAutoGainDb);
    }

    /**
        Meet piek en RMS over een langere periode en slaat stille passages over.

        Het signaal wordt in vensters van ~50 ms verdeeld. Een venster telt alleen
        mee als de piek erin boven de stiltedrempel komt. Piek en RMS worden over
        alle kanalen samen bepaald. Een onvolledig laatste venster telt niet mee.

        Niet thread-safe: gebruik vanaf één thread (de audiothread).
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
            totalSumSq = 0.0;
            totalSamples = 0;
            activeWindows = 0;
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

        bool hasSignal() const noexcept               { return activeWindows > 0; }
        std::int64_t getNumActiveWindows() const noexcept { return activeWindows; }

        float getPeakDb() const noexcept { return gainToDb (totalPeak); }

        float getRmsDb() const noexcept
        {
            return totalSamples > 0 ? gainToDb (std::sqrt (totalSumSq / (double) totalSamples))
                                    : minusInfinityDb;
        }

    private:
        void finishWindow() noexcept
        {
            if (windowPeak >= silenceThresholdGain)
            {
                totalPeak = std::max (totalPeak, windowPeak);
                totalSumSq += windowSumSq;
                totalSamples += windowSamples;
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
        double totalSumSq = 0.0;
        std::int64_t totalSamples = 0;
        std::int64_t activeWindows = 0;
    };
}
