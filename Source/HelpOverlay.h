#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UiStyle.h"

/**
    Toont de handleiding (docs/MANUAL.md, ingebakken als BinaryData) als laag
    binnen de editor. Eenvoudige Markdown-weergave: koppen (#, ##), opsommingen
    (- en 1.) en gewone alinea's.
*/
class HelpOverlay final : public juce::Component
{
public:
    explicit HelpOverlay (const juce::String& markdown)
    {
        text.setMultiLine (true, true);
        text.setReadOnly (true);
        text.setCaretVisible (false);
        text.setScrollbarsShown (true);
        text.setColour (juce::TextEditor::backgroundColourId, roughstart::ui::panel);
        text.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        text.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        text.setColour (juce::TextEditor::textColourId, juce::Colours::white);
        text.setIndents (14, 10);
        fill (markdown);
        addAndMakeVisible (text);

        closeButton.onClick = [this] { setVisible (false); };
        addAndMakeVisible (closeButton);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (roughstart::ui::background);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (12);
        auto bottom = area.removeFromBottom (32);
        closeButton.setBounds (bottom.removeFromRight (100));
        area.removeFromBottom (10);
        text.setBounds (area);
    }

private:
    void fill (const juce::String& markdown)
    {
        text.clear();

        auto add = [this] (const juce::String& s, float size, bool bold, juce::Colour colour)
        {
            text.setFont (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
            text.setColour (juce::TextEditor::textColourId, colour);
            text.insertTextAtCaret (s);
        };

        for (auto line : juce::StringArray::fromLines (markdown))
        {
            line = line.trimEnd();

            if (line.startsWith ("# "))
                add (line.substring (2) + "\n\n", 20.0f, true, juce::Colours::white);
            else if (line.startsWith ("## "))
                add ("\n" + line.substring (3) + "\n", 16.0f, true, roughstart::ui::accent);
            else if (line.startsWith ("- "))
                add (juce::String (juce::CharPointer_UTF8 ("\xe2\x80\xa2  ")) + line.substring (2) + "\n", 14.0f, false, juce::Colours::white);
            else if (line.isNotEmpty() && juce::CharacterFunctions::isDigit (line[0]) && line.contains (". "))
                add (line + "\n", 14.0f, false, juce::Colours::white);
            else if (line.isEmpty())
                add ("\n", 8.0f, false, juce::Colours::white);
            else
                add (line + "\n", 14.0f, false, roughstart::ui::dimText);
        }

        text.moveCaretToTop (false);
    }

    juce::TextEditor text;
    juce::TextButton closeButton { "Close" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpOverlay)
};
