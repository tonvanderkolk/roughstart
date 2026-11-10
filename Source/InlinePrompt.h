#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UiStyle.h"

/**
    Vraagvenster binnen de plugin-editor zelf (geen apart desktopvenster).

    Losse vensters (AlertWindow) flikkeren in Logic omdat ze met het
    plugin-venster om de voorgrond vechten. Dit component ligt als laag over
    de hele editor en blokkeert zolang het zichtbaar is de rest van de UI.
*/
class InlinePrompt final : public juce::Component
{
public:
    InlinePrompt()
    {
        titleLabel.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        messageLabel.setColour (juce::Label::textColourId, roughstart::ui::dimText);
        messageLabel.setJustificationType (juce::Justification::topLeft);

        textEditor.setColour (juce::TextEditor::backgroundColourId, roughstart::ui::background);
        textEditor.setColour (juce::TextEditor::outlineColourId, roughstart::ui::dimText.withAlpha (0.5f));
        textEditor.setColour (juce::TextEditor::focusedOutlineColourId, roughstart::ui::accent);
        textEditor.onReturnKey = [this] { confirm(); };
        textEditor.onEscapeKey = [this] { hidePrompt(); };

        confirmButton.onClick = [this] { confirm(); };
        cancelButton.onClick  = [this] { hidePrompt(); };

        for (auto* c : std::initializer_list<juce::Component*> { &titleLabel, &messageLabel, &textEditor,
                                                                  &confirmButton, &cancelButton })
            addAndMakeVisible (c);
    }

    /** Vraagt om tekst; onConfirm krijgt de ingevulde tekst. */
    void showTextInput (const juce::String& title, const juce::String& message,
                        const juce::String& initialText, const juce::String& confirmText,
                        std::function<void (const juce::String&)> onConfirm)
    {
        callback = std::move (onConfirm);
        withText = true;
        textEditor.setText (initialText, false);
        textEditor.setVisible (true);
        open (title, message, confirmText);
        textEditor.grabKeyboardFocus();
        textEditor.selectAll();
    }

    /** Ja/nee-vraag; onConfirm wordt alleen bij bevestigen aangeroepen. */
    void showConfirm (const juce::String& title, const juce::String& message,
                      const juce::String& confirmText, std::function<void()> onConfirm)
    {
        callback = [cb = std::move (onConfirm)] (const juce::String&) { if (cb) cb(); };
        withText = false;
        textEditor.setVisible (false);
        open (title, message, confirmText);
    }

    void hidePrompt()
    {
        setVisible (false);
        callback = nullptr;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black.withAlpha (0.55f));

        const auto box = getBoxBounds().toFloat();
        g.setColour (roughstart::ui::panelAlt);
        g.fillRoundedRectangle (box, 8.0f);
        g.setColour (roughstart::ui::dimText.withAlpha (0.3f));
        g.drawRoundedRectangle (box, 8.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getBoxBounds().reduced (18, 14);

        titleLabel.setBounds (area.removeFromTop (24));
        area.removeFromTop (4);
        messageLabel.setBounds (area.removeFromTop (withText ? 22 : 40));
        area.removeFromTop (6);

        if (withText)
        {
            textEditor.setBounds (area.removeFromTop (28));
            area.removeFromTop (12);
        }

        auto buttons = area.removeFromBottom (30);
        confirmButton.setBounds (buttons.removeFromRight (90));
        buttons.removeFromRight (8);
        cancelButton.setBounds (buttons.removeFromRight (90));
    }

private:
    void open (const juce::String& title, const juce::String& message, const juce::String& confirmText)
    {
        titleLabel.setText (title, juce::dontSendNotification);
        messageLabel.setText (message, juce::dontSendNotification);
        confirmButton.setButtonText (confirmText);
        setVisible (true);
        toFront (false);
        resized();
        repaint();
    }

    void confirm()
    {
        auto cb = std::move (callback);
        const auto text = textEditor.getText();
        hidePrompt();

        if (cb)
            cb (text);
    }

    juce::Rectangle<int> getBoxBounds() const
    {
        const int w = juce::jmin (340, getWidth() - 32);
        const int h = withText ? 156 : 150;
        return getLocalBounds().withSizeKeepingCentre (w, h);
    }

    juce::Label titleLabel, messageLabel;
    juce::TextEditor textEditor;
    juce::TextButton confirmButton { "OK" }, cancelButton { "Cancel" };
    std::function<void (const juce::String&)> callback;
    bool withText = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InlinePrompt)
};
