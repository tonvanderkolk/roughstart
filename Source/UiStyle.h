#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LevelMeter.h"

/** Kleuren en opmaak die editor en overzicht delen. */
namespace roughstart::ui
{
    inline const juce::Colour background    { 0xff1e2226 };
    inline const juce::Colour panel         { 0xff272c31 };
    inline const juce::Colour panelAlt      { 0xff2d3338 };
    inline const juce::Colour accent        { 0xff3ddc84 };
    inline const juce::Colour analyzeColour { 0xffe0583a };
    inline const juce::Colour warning       { 0xffe8b04a };
    inline const juce::Colour info          { 0xff7fb2e5 };   // neutrale melding, geen fout
    inline const juce::Colour dimText       { 0xffa8b0b8 };

    /**
        Kapt tekst van vóren af als hij niet past ("…Be With You_Drums"), zodat
        het onderscheidende einde van een spoornaam zichtbaar blijft.
    */
    inline juce::String fitFromStart (const juce::String& text, const juce::Font& font, float maxWidth)
    {
        if (juce::GlyphArrangement::getStringWidth (font, text) <= maxWidth)
            return text;

        const auto ellipsis = juce::String::fromUTF8 ("\xe2\x80\xa6");
        for (int i = 1; i < text.length(); ++i)
        {
            const auto candidate = ellipsis + text.substring (i).trimStart();
            if (juce::GlyphArrangement::getStringWidth (font, candidate) <= maxWidth)
                return candidate;
        }

        return ellipsis;
    }

    inline juce::String formatDb (float db, bool withSign)
    {
        if (db <= roughstart::minusInfinityDb)
            return "-inf dB";
        const auto text = juce::String (db, 1);
        return (withSign && db > 0.0f ? "+" : "") + text + " dB";
    }
}
