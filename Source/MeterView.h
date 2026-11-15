#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UiStyle.h"

/**
    De meter in de Track-weergave: laat op één dBFS-schaal zien waar het spoor
    zit vóór RoughStart (grijs), waar het uitkomt na Auto (groen), en waar het
    doel (gestippeld) en het piekplafond (blauw) liggen.

    De balk volgt de gekozen meetmethode (piek of RMS); de piek staat er
    altijd als dun streepje bij, zodat je ziet waarom het plafond ingrijpt.
*/
class MeterView final : public juce::Component,
                        public juce::SettableTooltipClient
{
public:
    struct Data
    {
        bool measured = false;       // er is een meting (vastgezet of live)
        bool live = false;           // analyse loopt: toon live waarden
        bool rmsMode = true;
        float peakDb = 0.0f, rmsDb = 0.0f;
        float autoDb = 0.0f;
        float targetDb = -18.0f;
        float ceilingDb = -6.0f;
        bool limitedByCeiling = false;

        bool operator== (const Data& o) const noexcept
        {
            using juce::exactlyEqual;
            return measured == o.measured && live == o.live && rmsMode == o.rmsMode
                && exactlyEqual (peakDb, o.peakDb) && exactlyEqual (rmsDb, o.rmsDb) && exactlyEqual (autoDb, o.autoDb)
                && exactlyEqual (targetDb, o.targetDb) && exactlyEqual (ceilingDb, o.ceilingDb)
                && limitedByCeiling == o.limitedByCeiling;
        }
    };

    void setData (const Data& d)
    {
        if (d == data)
            return;
        data = d;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace roughstart::ui;

        auto area = getLocalBounds().toFloat();
        auto labelRow = area.removeFromTop (16.0f);
        auto scaleRow = area.removeFromBottom (16.0f);
        auto bar = area.reduced (0.0f, 4.0f);

        // Achtergrond van de balk
        g.setColour (control);
        g.fillRoundedRectangle (bar, 4.0f);

        auto xFor = [&] (float db) { return bar.getX() + bar.getWidth() * proportion (db); };

        // Schaalstreepjes en getallen
        g.setFont (juce::FontOptions (11.0f));
        for (float db = minDb; db <= maxDb + 0.01f; db += 6.0f)
        {
            const float x = xFor (db);
            g.setColour (border);
            g.drawVerticalLine ((int) x, bar.getBottom() - 3.0f, bar.getBottom());
            if (((int) db) % 12 == 0)
            {
                g.setColour (faintText);
                const auto label = db >= 0.0f ? juce::String ("0 dBFS") : juce::String ((int) db);
                const bool first = db <= minDb + 0.01f, last = db >= 0.0f;
                const float w = last ? 44.0f : 26.0f;
                const float left = last ? x - w : first ? x : x - w * 0.5f;
                g.drawText (label, juce::Rectangle<float> (left, scaleRow.getY(), w, scaleRow.getHeight()),
                            last ? juce::Justification::centredRight : first ? juce::Justification::centredLeft
                                                                         : juce::Justification::centred);
            }
        }

        if (data.measured)
        {
            const float before = data.rmsMode ? data.rmsDb : data.peakDb;
            const float after = before + (data.live ? 0.0f : data.autoDb);
            const auto innerBar = bar.reduced (0.0f, bar.getHeight() * 0.22f);

            // Vóór (grijs), na Auto (groen); tijdens analyse alleen live (oranje)
            g.setColour (data.live ? analyzeColour.withAlpha (0.85f) : juce::Colour (0xff5d646d));
            g.fillRoundedRectangle (innerBar.withRight (xFor (before)), 3.0f);

            if (! data.live)
            {
                g.setColour (accent);
                g.fillRoundedRectangle (innerBar.withRight (xFor (after)).reduced (0.0f, innerBar.getHeight() * 0.18f), 2.5f);

                // Pieken: vóór en na
                g.setColour (juce::Colour (0xff8a929b));
                g.fillRect (juce::Rectangle<float> (xFor (data.peakDb) - 1.0f, innerBar.getY(), 2.0f, innerBar.getHeight()));
                g.setColour (text);
                g.fillRect (juce::Rectangle<float> (xFor (data.peakDb + data.autoDb) - 1.0f, bar.getY() + 2.0f, 2.0f, bar.getHeight() - 4.0f));
            }
        }

        // Doel (gestippeld) en plafond (blauw), met labels erboven
        const float tx = xFor (data.targetDb), cx = xFor (data.ceilingDb);

        g.setColour (accent);
        for (float y = bar.getY() - 2.0f; y < bar.getBottom() + 2.0f; y += 5.0f)
            g.fillRect (juce::Rectangle<float> (tx - 1.0f, y, 2.0f, 3.0f));

        g.setColour (info);
        g.fillRect (juce::Rectangle<float> (cx - 1.25f, bar.getY() - 3.0f, 2.5f, bar.getHeight() + 6.0f));

        g.setFont (juce::FontOptions (11.5f));
        const auto targetText = juce::String ("target ") + juce::String (data.targetDb, 1);
        const auto ceilingText = juce::String ("ceiling ") + juce::String (data.ceilingDb, 1);
        auto targetBox = juce::Rectangle<float> (tx - 50.0f, labelRow.getY(), 100.0f, labelRow.getHeight());
        auto ceilingBox = juce::Rectangle<float> (cx - 50.0f, labelRow.getY(), 100.0f, labelRow.getHeight());

        // Labels niet over elkaar of buiten beeld
        if (ceilingBox.getRight() > bar.getRight()) ceilingBox.setX (bar.getRight() - ceilingBox.getWidth());
        if (ceilingBox.getX() < targetBox.getRight() - 12.0f)
        {
            targetBox.setX (juce::jmin (targetBox.getX(), ceilingBox.getX() - targetBox.getWidth() + 12.0f));
            ceilingBox.setX (juce::jmax (ceilingBox.getX(), targetBox.getRight() - 12.0f));
        }

        g.setColour (accent);
        g.drawText (targetText, targetBox, juce::Justification::centred);
        g.setColour (info);
        g.drawText (ceilingText, ceilingBox, juce::Justification::centred);
    }

private:
    static constexpr float minDb = -48.0f, maxDb = 0.0f;

    static float proportion (float db)
    {
        return juce::jlimit (0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
    }

    Data data;
};
