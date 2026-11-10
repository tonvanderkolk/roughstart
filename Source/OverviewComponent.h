#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

/**
    Tabel met alle RoughStart-instances in het project: spoornaam, methode,
    gemeten piek/RMS, Auto, Trim, totaal en status. Kolommen zijn sorteerbaar
    (klik op de kolomkop). Ververst zichzelf zolang hij zichtbaar is.
*/
class OverviewComponent final : public juce::Component,
                                private juce::TableListBoxModel,
                                private juce::Timer
{
public:
    explicit OverviewComponent (RoughStartProcessor& owner);
    ~OverviewComponent() override;

    void resized() override;
    void visibilityChanged() override;

private:
    enum ColumnId { trackCol = 1, modeCol, peakCol, rmsCol, autoCol, trimCol, totalCol, statusCol, channelsCol };

    int getNumRows() override;
    void paintRowBackground (juce::Graphics&, int row, int width, int height, bool selected) override;
    void paintCell (juce::Graphics&, int row, int columnId, int width, int height, bool selected) override;
    void sortOrderChanged (int newSortColumnId, bool isForwards) override;
    juce::String getCellTooltip (int row, int columnId) override;

    void timerCallback() override;
    void refresh();
    void sortRows();

    static juce::String cellText (const InstanceRegistry::OverviewRow&, int columnId);
    static juce::String statusText (const InstanceRegistry::OverviewRow&);

    RoughStartProcessor& owner;
    juce::TableListBox table { "Overview", this };
    std::vector<InstanceRegistry::OverviewRow> rows;
    int sortColumn = 0;          // 0 = volgorde van aanmelden
    bool sortForwards = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverviewComponent)
};
