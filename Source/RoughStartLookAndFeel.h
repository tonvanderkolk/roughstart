#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UiStyle.h"

/**
    Eigen donkere stijl voor RoughStart: platte knoppen met afgeronde hoeken,
    één groene accentkleur, een boogvormige draaiknop en compacte
    "waardevakjes" (LinearBar-schuiven met naam en waarde erin).

    Een knop wordt de hoofdknop (gevuld groen) met
    button.getProperties().set ("primary", true).
*/
class RoughStartLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RoughStartLookAndFeel()
    {
        using namespace roughstart::ui;

        setColour (juce::ResizableWindow::backgroundColourId, background);
        setColour (juce::TextButton::buttonColourId, control);
        setColour (juce::TextButton::buttonOnColourId, accent.withAlpha (0.22f));
        setColour (juce::TextButton::textColourOffId, text);
        setColour (juce::TextButton::textColourOnId, accent);
        setColour (juce::ComboBox::backgroundColourId, control);
        setColour (juce::ComboBox::outlineColourId, border);
        setColour (juce::ComboBox::textColourId, text);
        setColour (juce::ComboBox::arrowColourId, dimText);
        setColour (juce::PopupMenu::backgroundColourId, panelAlt);
        setColour (juce::PopupMenu::textColourId, text);
        setColour (juce::PopupMenu::headerTextColourId, dimText);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha (0.25f));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour (juce::Label::textColourId, text);
        setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxTextColourId, text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::TextEditor::backgroundColourId, background);
        setColour (juce::TextEditor::outlineColourId, border);
        setColour (juce::TextEditor::focusedOutlineColourId, accent);
        setColour (juce::TooltipWindow::backgroundColourId, panelAlt);
        setColour (juce::TooltipWindow::textColourId, text);
        setColour (juce::TooltipWindow::outlineColourId, border);
        setColour (juce::ScrollBar::thumbColourId, border.brighter (0.3f));
    }

    //==============================================================================
    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                               bool over, bool down) override
    {
        using namespace roughstart::ui;

        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        const bool primary = (bool) b.getProperties().getWithDefault ("primary", false);

        juce::Colour fill = primary ? accent : b.findColour (juce::TextButton::buttonColourId);
        if (b.getToggleState())
            fill = b.findColour (juce::TextButton::buttonOnColourId);
        if (over)  fill = fill.brighter (primary ? 0.08f : 0.12f);
        if (down)  fill = fill.darker (0.12f);

        const auto edges = b.isConnectedOnLeft() || b.isConnectedOnRight();
        const float radius = 6.0f;

        juce::Path p;
        p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), radius, radius,
                               ! b.isConnectedOnLeft(), ! b.isConnectedOnRight(),
                               ! b.isConnectedOnLeft(), ! b.isConnectedOnRight());
        g.setColour (fill);
        g.fillPath (p);

        if (! primary)
        {
            g.setColour (b.getToggleState() ? accent.withAlpha (0.6f) : border);
            g.strokePath (p, juce::PathStrokeType (1.0f));
        }
        juce::ignoreUnused (edges);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        return juce::FontOptions (juce::jmin (14.0f, (float) buttonHeight * 0.48f));
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        using namespace roughstart::ui;
        const bool primary = (bool) b.getProperties().getWithDefault ("primary", false);

        g.setFont (getTextButtonFont (b, b.getHeight()));
        g.setColour (primary ? juce::Colour (0xff0d1f15)
                             : (b.getToggleState() ? accent : text).withMultipliedAlpha (b.isEnabled() ? 1.0f : 0.5f));
        g.drawFittedText (b.getButtonText(), b.getLocalBounds().reduced (6, 0), juce::Justification::centred, 1);
    }

    //==============================================================================
    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) override
    {
        using namespace roughstart::ui;
        const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
        g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (box.isMouseOver (true) ? border.brighter (0.4f) : border);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        juce::Path arrow;
        const float cx = (float) w - 14.0f, cy = (float) h * 0.5f;
        arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
        arrow.lineTo (cx, cy + 2.0f);
        arrow.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (dimText);
        g.strokePath (arrow, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override        { return juce::FontOptions (13.5f); }
    juce::Font getPopupMenuFont() override                        { return juce::FontOptions (14.0f); }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (8, 1, box.getWidth() - 30, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
    }

    //==============================================================================
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider& s) override
    {
        using namespace roughstart::ui;

        const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (6.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const float lineW = juce::jmax (4.0f, radius * 0.14f);
        const float arcR = radius - lineW * 0.5f;

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
        g.setColour (border);
        g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Bipolair (Trim): vul vanaf het midden
        const float zeroPos = (float) s.valueToProportionOfLength (0.0);
        const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
        const float from = startAngle + (bipolar ? zeroPos : 0.0f) * (endAngle - startAngle);
        const float to = startAngle + pos * (endAngle - startAngle);

        if (std::abs (to - from) > 0.001f)
        {
            juce::Path value;
            value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, juce::jmin (from, to), juce::jmax (from, to), true);
            g.setColour (accent);
            g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        const float knobR = arcR - lineW * 1.4f;
        g.setColour (control);
        g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f);
        g.setColour (border);
        g.drawEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f, 1.0f);

        const float a = to - juce::MathConstants<float>::halfPi;
        const juce::Point<float> p1 (centre.x + std::cos (a) * knobR * 0.35f, centre.y + std::sin (a) * knobR * 0.35f);
        const juce::Point<float> p2 (centre.x + std::cos (a) * knobR * 0.85f, centre.y + std::sin (a) * knobR * 0.85f);
        g.setColour (text);
        g.drawLine ({ p1, p2 }, 2.5f);
    }

    //==============================================================================
    // LinearBar: compact waardevakje met naam links en waarde rechts
    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float minPos,
                           float maxPos, juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        using namespace roughstart::ui;

        if (style != juce::Slider::LinearBar)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, minPos, maxPos, style, s);
            return;
        }

        const auto r = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (0.5f);
        g.setColour (control);
        g.fillRoundedRectangle (r, 6.0f);

        // Vulling geeft de stand binnen het bereik aan
        const float fillW = juce::jlimit (0.0f, r.getWidth(), sliderPos - r.getX());
        g.saveState();
        g.reduceClipRegion (r.withWidth (fillW).toNearestInt());
        g.setColour (accent.withAlpha (0.16f));
        g.fillRoundedRectangle (r, 6.0f);
        g.restoreState();

        g.setColour (s.isMouseOverOrDragging() ? border.brighter (0.4f) : border);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        auto textArea = r.reduced (8.0f, 0.0f);
        g.setFont (juce::FontOptions (12.5f));
        g.setColour (dimText);
        g.drawText (s.getName(), textArea, juce::Justification::centredLeft);
        g.setFont (juce::FontOptions (13.5f, juce::Font::bold));
        g.setColour (text);
        g.drawText (s.getTextFromValue (s.getValue()), textArea, juce::Justification::centredRight);
    }

    juce::Label* createSliderTextBox (juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (s);
        l->setFont (juce::FontOptions (18.0f, juce::Font::bold));
        l->setJustificationType (juce::Justification::centredLeft);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineWhenEditingColourId, roughstart::ui::accent);
        l->setColour (juce::Label::backgroundWhenEditingColourId, roughstart::ui::background);
        return l;
    }

    //==============================================================================
    void drawTooltip (juce::Graphics& g, const juce::String& t, int w, int h) override
    {
        using namespace roughstart::ui;
        const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
        g.setColour (panelAlt);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (border);
        g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);

        juce::AttributedString s;
        s.setJustification (juce::Justification::centredLeft);
        s.append (t, juce::FontOptions (13.0f), text);
        juce::TextLayout layout;
        layout.createLayoutWithBalancedLineLengths (s, (float) w - 16.0f);
        layout.draw (g, r.reduced (8.0f, 5.0f));
    }

    juce::Rectangle<int> getTooltipBounds (const juce::String& t, juce::Point<int> pos, juce::Rectangle<int> parent) override
    {
        juce::AttributedString s;
        s.append (t, juce::FontOptions (13.0f));
        juce::TextLayout layout;
        layout.createLayoutWithBalancedLineLengths (s, 260.0f);

        const int w = (int) layout.getWidth() + 18, h = (int) layout.getHeight() + 12;
        return juce::Rectangle<int> (pos.x > parent.getCentreX() ? pos.x - (w + 12) : pos.x + 12,
                                     pos.y > parent.getCentreY() ? pos.y - (h + 6) : pos.y + 18, w, h)
                   .constrainedWithin (parent);
    }
};
