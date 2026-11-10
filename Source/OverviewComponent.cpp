#include "OverviewComponent.h"
#include "UiStyle.h"

#include <algorithm>

using namespace roughstart;

OverviewComponent::OverviewComponent (RoughStartProcessor& o) : owner (o)
{
    auto& header = table.getHeader();
    constexpr int flags = juce::TableHeaderComponent::visible | juce::TableHeaderComponent::sortable;

    header.addColumn ("Track",  trackCol,  160, 80, 400, flags);
    header.addColumn ("Ch",     channelsCol, 58, 44, 80, flags);
    header.addColumn ("Mode",   modeCol,    52, 40, 80,  flags);
    header.addColumn ("Peak",   peakCol,    68, 50, 100, flags);
    header.addColumn ("RMS",    rmsCol,     68, 50, 100, flags);
    header.addColumn ("Auto",   autoCol,    68, 50, 100, flags);
    header.addColumn ("Trim",   trimCol,    68, 50, 100, flags);
    header.addColumn ("Total",  totalCol,   68, 50, 100, flags);
    header.addColumn ("Status", statusCol, 110, 60, 200, flags);
    header.setStretchToFitActive (true);

    header.setColour (juce::TableHeaderComponent::backgroundColourId, ui::panelAlt);
    header.setColour (juce::TableHeaderComponent::textColourId, ui::dimText);
    header.setColour (juce::TableHeaderComponent::outlineColourId, ui::background);

    table.setColour (juce::ListBox::backgroundColourId, ui::panel);
    table.setColour (juce::ListBox::outlineColourId, ui::background);
    table.setRowHeight (24);
    table.setHeaderHeight (26);
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
        startTimerHz (4);
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
    sortRows();
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
            case peakCol:  return r.measured ? r.peakDb : -1000.0f;
            case rmsCol:   return r.measured ? r.rmsDb  : -1000.0f;
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
            case trackCol:  cmp = a.trackName.compareNatural (b.trackName); break;
            case modeCol:   cmp = a.mode - b.mode; break;
            case channelsCol: cmp = a.numChannels - b.numChannels; break;
            case statusCol: cmp = statusText (a).compareNatural (statusText (b)); break;
            case peakCol: case rmsCol:
                if (a.measured != b.measured)
                    return a.measured;   // gemeten eerst
                [[fallthrough]];
            case autoCol: case trimCol: case totalCol:
            {
                const float x = numeric (a, col), y = numeric (b, col);
                cmp = x < y ? -1 : (x > y ? 1 : 0);
                break;
            }
            default:        cmp = a.instanceNumber - b.instanceNumber; break;
        }

        if (cmp == 0)
            return a.instanceNumber < b.instanceNumber;

        return forwards ? cmp < 0 : cmp > 0;
    });
}

int OverviewComponent::getNumRows()
{
    return (int) rows.size();
}

void OverviewComponent::paintRowBackground (juce::Graphics& g, int row, int, int, bool)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;

    const bool isThis = rows[(size_t) row].instanceNumber == owner.getInstanceNumber();

    if (isThis)
        g.fillAll (ui::accent.withAlpha (0.15f));
    else if (row % 2 == 1)
        g.fillAll (ui::panelAlt);
}

juce::String OverviewComponent::statusText (const InstanceRegistry::OverviewRow& r)
{
    using State = RoughStartProcessor::AnalysisState;

    switch ((State) r.analysisState)
    {
        case State::waitingForPlayback: return "Waiting";
        case State::measuring:          return "Measuring";
        case State::idle:               break;
    }

    if (! r.measured)
        return r.lastResult == (int) RoughStartProcessor::AnalysisResult::noSignal ? "No signal" : "Not analyzed";
    if (r.lastResult == (int) RoughStartProcessor::AnalysisResult::noSignal)
        return "No signal (kept)";
    if (r.limitedByCeiling)    return "Peak at ceiling";
    return "OK";
}

juce::String OverviewComponent::cellText (const InstanceRegistry::OverviewRow& r, int columnId)
{
    switch (columnId)
    {
        case trackCol:  return r.trackName;
        case modeCol:   return r.mode == 1 ? "RMS" : "Peak";
        case channelsCol: return r.numChannels == 1 ? "Mono" : "Stereo";
        case peakCol:   return r.measured ? ui::formatDb (r.peakDb, false) : "-";
        case rmsCol:    return r.measured ? ui::formatDb (r.rmsDb, false)  : "-";
        case autoCol:   return r.measured ? ui::formatDb (r.autoDb, true)  : "-";
        case trimCol:   return ui::formatDb (r.trimDb, true);
        case totalCol:  return ui::formatDb (r.totalDb, true);
        case statusCol: return statusText (r);
        default:        return {};
    }
}

void OverviewComponent::paintCell (juce::Graphics& g, int row, int columnId, int width, int height, bool)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;

    const auto& r = rows[(size_t) row];
    const bool isThis = r.instanceNumber == owner.getInstanceNumber();

    auto colour = juce::Colours::white;
    if (columnId == statusCol)
    {
        const auto state = (RoughStartProcessor::AnalysisState) r.analysisState;
        if (state != RoughStartProcessor::AnalysisState::idle)  colour = ui::analyzeColour;
        else if (! r.measured
                 || r.lastResult == (int) RoughStartProcessor::AnalysisResult::noSignal) colour = ui::dimText;
        else if (r.limitedByCeiling)                            colour = ui::info;
        else                                                    colour = ui::accent;
    }

    g.setColour (colour);
    g.setFont (juce::FontOptions (13.5f, isThis ? juce::Font::bold : juce::Font::plain));

    const bool numeric = columnId >= peakCol && columnId <= totalCol;
    g.drawText (cellText (r, columnId), 6, 0, width - 12, height,
                numeric ? juce::Justification::centredRight : juce::Justification::centredLeft, true);
}
