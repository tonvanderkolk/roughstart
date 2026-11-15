#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LevelMeter.h"

/** Kleuren en opmaak die editor, overzicht en LookAndFeel delen. */
namespace roughstart::ui
{
    inline const juce::Colour background    { 0xff16181b };   // vensterachtergrond
    inline const juce::Colour panel         { 0xff1f2226 };   // kaarten
    inline const juce::Colour panelAlt      { 0xff272b30 };   // afwisselende rijen, menu's
    inline const juce::Colour control       { 0xff2a2e34 };   // knoppen, vakjes
    inline const juce::Colour border        { 0xff363b42 };
    inline const juce::Colour accent        { 0xff3ddc84 };
    inline const juce::Colour analyzeColour { 0xffe0583a };
    inline const juce::Colour warning       { 0xffe8b04a };
    inline const juce::Colour info          { 0xff6fa8e6 };   // neutrale melding, geen fout; ook het plafond
    inline const juce::Colour text          { 0xffe8eaed };
    inline const juce::Colour dimText       { 0xff9aa2ab };
    inline const juce::Colour faintText     { 0xff6b737c };

    /**
        Kapt tekst van vóren af als hij niet past ("…Be With You_Drums"), zodat
        het onderscheidende einde van een spoornaam zichtbaar blijft.
    */
    inline juce::String fitFromStart (const juce::String& str, const juce::Font& font, float maxWidth)
    {
        if (juce::GlyphArrangement::getStringWidth (font, str) <= maxWidth)
            return str;

        const auto ellipsis = juce::String::fromUTF8 ("\xe2\x80\xa6");
        for (int i = 1; i < str.length(); ++i)
        {
            const auto candidate = ellipsis + str.substring (i).trimStart();
            if (juce::GlyphArrangement::getStringWidth (font, candidate) <= maxWidth)
                return candidate;
        }

        return ellipsis;
    }

    inline juce::String formatDb (float db, bool withSign)
    {
        if (db <= roughstart::minusInfinityDb)
            return "-inf dB";
        const auto value = juce::String (db, 1);
        return (withSign && db > 0.0f ? "+" : "") + value + " dB";
    }

    /** Het RoughStart-merkje: vijf verticale balkjes (golfvorm). */
    inline void drawLogo (juce::Graphics& g, juce::Rectangle<float> r)
    {
        static constexpr float heights[] = { 0.45f, 0.8f, 1.0f, 0.65f, 0.35f };
        const float barW = r.getWidth() / 9.0f;
        g.setColour (accent);
        for (int i = 0; i < 5; ++i)
        {
            const float h = r.getHeight() * heights[i];
            g.fillRoundedRectangle (r.getX() + (float) i * barW * 2.0f, r.getCentreY() - h * 0.5f, barW, h, barW * 0.5f);
        }
    }
}
