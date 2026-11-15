#include "OverviewComponent.h"
#include "UiStyle.h"

#include <algorithm>

using namespace roughstart;

namespace
{
    constexpr float meterMinDb = -48.0f;

    float meterProportion (float db)
    {
        return juce::jlimit (0.0f, 1.0f, (db - meterMinDb) / -meterMinDb);
    }

    using State = RoughStartProcessor::AnalysisState;
    using Result = RoughStartProcessor::AnalysisResult;

    bool isAnalyzing (const InstanceRegistry::OverviewRow& r)  { return (State) r.analysisState != State::idle; }
    bool hadNoSignal (const InstanceRegistry::OverviewRow& r)  { return r.lastResult == (int) Result::noSignal; }
}

//==============================================================================
/** Trim-vakje in een tabelcel: slepen of dubbelklikken om te typen. */
class OverviewComponent::TrimCell final : public juce::Slider
{
public:
    explicit TrimCell (OverviewComponent& o) : overview (o)
    {
        setSliderStyle (juce::Slider::LinearBar);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        setRange (-24.0, 24.0, 0.1);
        setDoubleClickReturnValue (true, 0.0);
        setScrollWheelEnabled (false);
        setMouseDragSensitivity (240);
        setTooltip ("Trim for this track. Drag left or right, double-click to reset. Alt-click to type a value.");

        textFromValueFunction = [] (double v) { return ui::formatDb ((float) v, true); };
        valueFromTextFunction = [] (const juce::String& s) { return s.retainCharacters ("-+0123456789.").getDoubleValue(); };

        onDragStart = [this] { overview.userIsEditingTrim = true;  overview.owner.getRegistry().setTrimGesture (instanceNumber, true); };
        onDragEnd   = [this] { overview.owner.getRegistry().setTrimGesture (instanceNumber, false); overview.userIsEditingTrim = false; };
        onValueChange = [this] { if (! updating) overview.owner.getRegistry().setTrimDb (instanceNumber, (float) getValue()); };
    }

    void setRow (int newInstanceNumber, float trimDb)
    {
        instanceNumber = newInstanceNumber;
        if (! isMouseButtonDown())
        {
            const juce::ScopedValueSetter<bool> svs (updating, true);
            setValue (trimDb, juce::dontSendNotification);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        valueAtMouseDown = (float) getValue();

        if (e.mods.isAltDown())
        {
            showTextEditor();
            return;
        }
        Slider::mouseDown (e);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        Slider::mouseUp (e);
        commit();
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        Slider::mouseDoubleClick (e);   // zet terug op 0
        commit();
    }

private:
    void showTextEditor()
    {
        auto* editor = new juce::TextEditor();
        editor->setText (juce::String (getValue(), 1), false);
        editor->setJustification (juce::Justification::centredRight);
        editor->setBounds (getLocalBounds());
        editor->selectAll();
        editor->onReturnKey = [this, editor]
        {
            valueAtMouseDown = (float) getValue();
            setValue (editor->getText().retainCharacters ("-+0123456789.").getDoubleValue(), juce::sendNotificationSync);
            commit();
            juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<juce::TextEditor> (editor)] { delete safe.getComponent(); });
        };
        editor->onEscapeKey = editor->onFocusLost = [editor]
        {
            juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<juce::TextEditor> (editor)] { delete safe.getComponent(); });
        };
        addAndMakeVisible (editor);
        editor->grabKeyboardFocus();
    }

    /** Legt de wijziging sinds de laatste muisklik vast voor Undo. */
    void commit()
    {
        const float now = (float) getValue();
        overview.owner.getRegistry().recordTrimChange (instanceNumber, valueAtMouseDown, now);
        valueAtMouseDown = now;
    }

    OverviewComponent& overview;
    int instanceNumber = 0;
    bool updating = false;
    float valueAtMouseDown = 0.0f;
};

//==============================================================================
OverviewComponent::OverviewComponent (RoughStartProcessor& o) : owner (o)
{
    auto& header = table.getHeader();
    constexpr int columnFlags = juce::TableHeaderComponent::visible | juce::TableHeaderComponent::sortable;

    header.addColumn ("Track",  trackCol,    160, 90, 400, columnFlags);
    header.addColumn ("Ch",     channelsCol,  44, 36, 70,  flags);
    header.addColumn ("Mode",   modeCol,      48, 40, 70,  flags);
    header.addColumn ("Level",  levelCol,    110, 70, 200, columnFlags);
    header.addColumn ("Peak",   peakCol,      56, 46, 90,  flags);
    header.addColumn ("RMS",    rmsCol,       56, 46, 90,  flags);
    header.addColumn ("Auto",   autoCol,      62, 50, 90,  flags);
    header.addColumn ("Trim",   trimCol,      84, 70, 110, columnFlags);
    header.addColumn ("Total",  totalCol,     64, 50, 90,  flags);
    header.addColumn ("Status", statusCol,   100, 70, 140, columnFlags);
    header.setStretchToFitActive (true);

    header.setColour (juce::TableHeaderComponent::backgroundColourId, ui::panel);
    header.setColour (juce::TableHeaderComponent::textColourId, ui::dimText);
    header.setColour (juce::TableHeaderComponent::outlineColourId, ui::border);
    header.setColour (juce::TableHeaderComponent::highlightColourId, ui::panelAlt);

    table.setColour (juce::ListBox::backgroundColourId, ui::panel);
    table.setColour (juce::ListBox::outlineColourId, ui::border);
    table.setOutlineThickness (1);
    table.setRowHeight (30);
    table.setHeaderHeight (28);
    addAndMakeVisible (table);

    refresh();
}

OverviewComponent::~OverviewComponent()
{
    stopTimer();
}

void OverviewComponent::resized()
{
    table.setBounds (getLocalBounds());
}

void OverviewComponent::visibilityChanged()
{
    if (isVisible())
    {
        refresh();
        startTimerHz (8);
    }
    else
    {
        stopTimer();
    }
}

void OverviewComponent::timerCallback()
{
    refresh();
}

void OverviewComponent::refresh()
{
    rows = owner.getRegistry().getOverview();

    // Tijdens slepen op Trim de volgorde vasthouden: anders verspringt de regel onder de muis.
    if (userIsEditingTrim)
    {
        std::vector<InstanceRegistry::OverviewRow> ordered;
        for (const auto& old : previousOrder)
            for (const auto& r : rows)
                if (r.instanceNumber == old)
                    ordered.push_back (r);
        if (ordered.size() == rows.size())
            rows = std::move (ordered);
    }
    else
    {
        sortRows();
    }

    previousOrder.clear();
    for (const auto& r : rows)
        previousOrder.push_back (r.instanceNumber);

    table.updateContent();
    table.repaint();
}

void OverviewComponent::sortOrderChanged (int newSortColumnId, bool isForwards)
{
    sortColumn = newSortColumnId;
    sortForwards = isForwards;
    sortRows();
    table.updateContent();
    table.repaint();
}

void OverviewComponent::sortRows()
{
    // Niet-gemeten sporen achteraan bij getalkolommen, ongeacht de richting.
    auto numeric = [] (const InstanceRegistry::OverviewRow& r, int col) -> float
    {
        switch (col)
        {
            case peakCol:  return r.peakDb;
            case rmsCol:   return r.rmsDb;
            case levelCol: return (r.mode == 1 ? r.rmsDb : r.peakDb) + r.autoDb;
            case autoCol:  return r.autoDb;
            case trimCol:  return r.trimDb;
            case totalCol: return r.totalDb;
            default:       return 0.0f;
        }
    };

    const int col = sortColumn;
    const bool forwards = sortForwards;

    std::stable_sort (rows.begin(), rows.end(), [&] (const auto& a, const auto& b)
    {
        int cmp = 0;

        switch (col)
        {
            case trackCol:    cmp = a.trackName.compareNatural (b.trackName); break;
            case modeCol:     cmp = a.mode - b.mode; break;
            case channelsCol: cmp = a.numChannels - b.numChannels; break;
            case statusCol:   cmp = statusText (a).compareNatural (statusText (b)); break;
            case peakCol: case rmsCol: case levelCol: case autoCol:
                if (a.measured != b.measured)
                    return a.measured;   // gemeten eerst
                [[fallthrough]];
            case trimCol: case totalCol:
            {
                const float x = numeric (a, col), y = numeric (b, col);
                cmp = x < y ? -1 : (x > y ? 1 : 0);
                break;
            }
            default:          cmp = a.instanceNumber - b.instanceNumber; break;
        }

        if (cmp == 0)
            return a.instanceNumber < b.instanceNumber;

        return forwards ? cmp < 0 : cmp > 0;
    });
}

juce::String OverviewComponent::getCellTooltip (int row, int columnId)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return {};

    const auto& r = rows[(size_t) row];

    switch (columnId)
    {
        case trackCol:  return r.trackName;
        case levelCol:
            return r.measured ? "Grey: level before RoughStart. Green: after Auto. White tick: peak after Auto."
                              : juce::String();
        case statusCol:
            if (isAnalyzing (r))   return "Analysis running for this track.";
            if (r.limitedByCeiling) return "Auto is limited so the peak stays at the Peak ceiling. Information, not an error.";
            if (hadNoSignal (r))    return "No signal during the last analysis. Auto was left unchanged.";
            if (! r.measured)       return "Not analyzed yet.";
            return "Measured and set to the target.";
        default:        return {};
    }
}

int OverviewComponent::getNumRows()
{
    return (int) rows.size();
}

void OverviewComponent::paintRowBackground (juce::Graphics& g, int row, int width, int height, bool)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;

    const bool isThis = rows[(size_t) row].instanceNumber == owner.getInstanceNumber();

    if (row % 2 == 1)
        g.fillAll (ui::panelAlt.withAlpha (0.55f));

    if (isThis)
    {
        g.fillAll (ui::accent.withAlpha (0.10f));
        g.setColour (ui::accent);
        g.fillRect (0, 0, 3, height);
    }

    g.setColour (ui::border.withAlpha (0.5f));
    g.drawHorizontalLine (height - 1, 0.0f, (float) width);
}

juce::String OverviewComponent::statusText (const InstanceRegistry::OverviewRow& r)
{
    switch ((State) r.analysisState)
    {
        case State::waitingForPlayback: return "Waiting";
        case State::measuring:          return "Measuring";
        case State::idle:               break;
    }

    if (! r.measured)          return hadNoSignal (r) ? "No signal" : "Not analyzed";
    if (hadNoSignal (r))       return "No signal";
    if (r.limitedByCeiling)    return "Ceiling";
    return "OK";
}

juce::String OverviewComponent::cellText (const InstanceRegistry::OverviewRow& r, int columnId)
{
    const auto dash = juce::String (juce::CharPointer_UTF8 ("\xe2\x80\x93"));

    switch (columnId)
    {
        case trackCol:    return r.trackName;
        case modeCol:     return r.mode == 1 ? "RMS" : "Peak";
        case channelsCol: return r.numChannels == 1 ? "Mono" : "St";
        case peakCol:     return r.measured ? juce::String (r.peakDb, 1) : dash;
        case rmsCol:      return r.measured ? juce::String (r.rmsDb, 1) : dash;
        case autoCol:     return r.measured ? (r.autoDb > 0.0f ? "+" : "") + juce::String (r.autoDb, 1) : dash;
        case totalCol:    return (r.totalDb > 0.0f ? "+" : "") + juce::String (r.totalDb, 1);
        case statusCol:   return statusText (r);
        default:          return {};
    }
}

juce::Component* OverviewComponent::refreshComponentForCell (int row, int columnId, bool, juce::Component* existing)
{
    if (columnId != trimCol || ! juce::isPositiveAndBelow (row, (int) rows.size()))
    {
        delete existing;
        return nullptr;
    }

    auto* cell = dynamic_cast<TrimCell*> (existing);
    if (cell == nullptr)
    {
        delete existing;
        cell = new TrimCell (*this);
    }

    const auto& r = rows[(size_t) row];
    cell->setRow (r.instanceNumber, r.trimDb);
    return cell;
}

void OverviewComponent::paintLevel (juce::Graphics& g, const InstanceRegistry::OverviewRow& r, int width, int height) const
{
    const auto bar = juce::Rectangle<float> (6.0f, (float) height * 0.5f - 5.0f, (float) width - 12.0f, 10.0f);
    g.setColour (ui::control);
    g.fillRoundedRectangle (bar, 3.0f);

    if (isAnalyzing (r))
    {
        g.setColour (ui::analyzeColour.withAlpha (0.6f));
        g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * 0.15f), 3.0f);
        return;
    }

    if (! r.measured)
        return;

    auto xFor = [&] (float db) { return bar.getX() + bar.getWidth() * meterProportion (db); };
    const float before = r.mode == 1 ? r.rmsDb : r.peakDb;
    const float after = before + r.autoDb;

    g.setColour (juce::Colour (0xff5d646d));
    g.fillRoundedRectangle (bar.withRight (xFor (before)).reduced (0.0f, 1.0f), 2.5f);
    g.setColour (ui::accent);
    g.fillRoundedRectangle (bar.withRight (xFor (after)).reduced (0.0f, 3.0f), 2.0f);
    g.setColour (ui::text);
    g.fillRect (juce::Rectangle<float> (xFor (r.peakDb + r.autoDb) - 1.0f, bar.getY() - 1.0f, 2.0f, bar.getHeight() + 2.0f));
}

void OverviewComponent::paintStatus (juce::Graphics& g, const InstanceRegistry::OverviewRow& r, int width, int height) const
{
    juce::Colour colour = ui::accent;
    if (isAnalyzing (r))                         colour = ui::analyzeColour;
    else if (! r.measured || hadNoSignal (r))    colour = ui::dimText;
    else if (r.limitedByCeiling)                 colour = ui::info;

    const auto label = statusText (r);
    const juce::Font font (juce::FontOptions (12.0f, juce::Font::bold));
    const float w = juce::jmin ((float) width - 10.0f, juce::GlyphArrangement::getStringWidth (font, label) + 16.0f);
    const auto pill = juce::Rectangle<float> (6.0f, (float) height * 0.5f - 10.0f, w, 20.0f);

    g.setColour (colour.withAlpha (0.16f));
    g.fillRoundedRectangle (pill, 10.0f);
    g.setColour (colour);
    g.setFont (font);
    g.drawText (label, pill, juce::Justification::centred, true);
}

void OverviewComponent::paintCell (juce::Graphics& g, int row, int columnId, int width, int height, bool)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;

    const auto& r = rows[(size_t) row];

    if (columnId == levelCol)  { paintLevel (g, r, width, height);  return; }
    if (columnId == statusCol) { paintStatus (g, r, width, height); return; }

    const bool isThis = r.instanceNumber == owner.getInstanceNumber();
    const juce::Font font (juce::FontOptions (13.5f, isThis ? juce::Font::bold : juce::Font::plain));
    g.setFont (font);

    if (columnId == trackCol)
    {
        const int available = width - 14;
        g.setColour (ui::text);
        g.drawText (ui::fitFromStart (r.trackName, font, (float) available), 8, 0, available, height,
                    juce::Justification::centredLeft, false);
        return;
    }

    const bool numeric = columnId == peakCol || columnId == rmsCol || columnId == autoCol || columnId == totalCol;
    juce::Colour colour = ui::text;
    if (columnId == channelsCol || columnId == modeCol || columnId == peakCol || columnId == rmsCol)
        colour = ui::dimText;
    if (numeric && ! r.measured && columnId != totalCol)
        colour = ui::faintText;

    g.setColour (colour);
    g.drawText (cellText (r, columnId), 6, 0, width - 12, height,
                numeric ? juce::Justification::centredRight : juce::Justification::centredLeft, true);
}
