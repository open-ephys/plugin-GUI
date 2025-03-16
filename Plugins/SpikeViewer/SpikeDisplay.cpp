/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "SpikeDisplay.h"

#include "SpikeDisplayCanvas.h"
#include "SpikePlots.h"


SpikeDisplay::SpikeDisplay (SpikeDisplayCanvas* sdc, Viewport* v) : canvas (sdc),
                                                                    viewport (v),
                                                                    shouldInvert (false),
                                                                    thresholdCoordinator (nullptr)
{
    totalHeight = 1000;

    grid.alignContent = Grid::AlignContent::start;
    grid.justifyContent = Grid::JustifyContent::center;
    grid.alignItems = Grid::AlignItems::center;
    grid.justifyItems = Grid::JustifyItems::center;
    grid.autoFlow = Grid::AutoFlow::rowDense;
    
    grid.columnGap = Grid::Px (5); // Gap between columns
    grid.rowGap = Grid::Px (5); // Gap between rows
    
}

void SpikeDisplay::removePlots()
{
    singleElectrodePlots.clear();
    stereotrodePlots.clear();
    tetrodePlots.clear();
    spikePlots.clear();
}

void SpikeDisplay::clear()
{

    for (auto spikePlot : spikePlots)
    {
        spikePlot->clear();
    }

}

SpikePlot* SpikeDisplay::addSpikePlot (int numChannels, int electrodeNum, String name_, Array<String> channelNames, std::string identifier_)
{
    LOGD ("Adding spike plot for electrode: ", electrodeNum, " named: ", name_, " with ", numChannels, " channels");

    SpikePlot* spikePlot = new SpikePlot (canvas, electrodeNum, 1000 + numChannels, name_, channelNames, identifier_);

    if (numChannels == 1)
        singleElectrodePlots.add (spikePlot);
    else if (numChannels == 2)
        stereotrodePlots.add (spikePlot);
    else if (numChannels == 4)
        tetrodePlots.add (spikePlot);

    spikePlots.add (spikePlot);
    
    addAndMakeVisible (spikePlot);
    spikePlot->invertSpikes (shouldInvert);
    if (thresholdCoordinator)
    {
        spikePlot->registerThresholdCoordinator (thresholdCoordinator);
    }

    resized();
    setBounds (0, 0, getWidth(), totalHeight);

    return spikePlot;
}

SpikePlot* SpikeDisplay::getSpikePlot (int numChannels, int index)
{

    if (index < 0)
        return nullptr;

    if (numChannels == 1 && index < singleElectrodePlots.size())
        return singleElectrodePlots[index];
    else if (numChannels == 2 && index < stereotrodePlots.size())
        return stereotrodePlots[index];
    else if (numChannels == 4 && index < tetrodePlots.size())
        return tetrodePlots[index];
    
    return nullptr;
}

void SpikeDisplay::paint (Graphics& g)
{
    //g.fillAll (findColour (ThemeColours::componentBackground));
}

void SpikeDisplay::resized()
{
    totalHeight = 0;
    
    if (singleElectrodePlots.size() > 0)
    {
        int columnHeight = 200 * scaleFactor;
        int columnWidth = columnHeight / singleElectrodePlots[0]->aspectRatio;

        int numColumns = getWidth() / columnWidth;
        numColumns = jmax (1, numColumns);
        columnWidth = getWidth() / numColumns;
        columnHeight = columnWidth * singleElectrodePlots[0]->aspectRatio;

        Array<GridItem> items;
        
        for (auto spikePlot : singleElectrodePlots)
        {
            items.add (GridItem (spikePlot).withSize (columnWidth, columnHeight));
        }
        
        grid.items = items;
        grid.templateColumns.clear();

        for (int i = 0; i < numColumns; ++i)
            grid.templateColumns.add (Grid::TrackInfo (Grid::Fr (1)));

        grid.templateRows = juce::Grid::TrackInfo (juce::Grid::Px (columnHeight)); // Fixed row height

        grid.performLayout (getLocalBounds().withY(totalHeight));

        totalHeight = singleElectrodePlots.getLast()->getBottom() + 20;
    
    }

    if (stereotrodePlots.size() > 0)
    {
        int columnHeight = 200 * scaleFactor;
        int columnWidth = columnHeight / stereotrodePlots[0]->aspectRatio;
        
        int numColumns = getWidth() / columnWidth;
        numColumns = jmax (1, numColumns);
        columnWidth = getWidth() / numColumns;
        columnHeight = columnWidth * stereotrodePlots[0]->aspectRatio;

        Array<GridItem> items;

        for (auto spikePlot : stereotrodePlots)
        {
            items.add (GridItem (spikePlot).withSize (columnWidth, columnHeight));
        }

        grid.items = items;
        grid.templateColumns.clear();

        for (int i = 0; i < numColumns; ++i)
            grid.templateColumns.add (Grid::TrackInfo (Grid::Fr (1)));

        grid.templateRows = juce::Grid::TrackInfo (juce::Grid::Px (columnHeight)); // Fixed row height

        grid.performLayout (getLocalBounds().withY(totalHeight));

        totalHeight = stereotrodePlots.getLast()->getBottom() + 20;
    }
        
    if (tetrodePlots.size() > 0)
    {
        int columnHeight = 400 * scaleFactor;
        int columnWidth = columnHeight / tetrodePlots[0]->aspectRatio;

        int numColumns = getWidth() / columnWidth;
        numColumns = jmax (1, numColumns);
        columnWidth = getWidth() / numColumns;
        columnHeight = columnWidth * tetrodePlots[0]->aspectRatio;


        Array<GridItem> items;

        for (auto spikePlot : tetrodePlots)
        {
            items.add (GridItem (spikePlot).withSize (columnWidth, columnHeight));
        }

        grid.items = items;
        grid.templateColumns.clear();

        for (int i = 0; i < numColumns; ++i)
            grid.templateColumns.add (Grid::TrackInfo (Grid::Fr (1)));

        grid.templateRows = juce::Grid::TrackInfo (juce::Grid::Px (columnHeight)); // Fixed row height

        grid.performLayout (getLocalBounds().withY (totalHeight));

        totalHeight = tetrodePlots.getLast()->getBottom() + 20;
    }
}

void SpikeDisplay::refresh()
{
    for (auto plot : spikePlots)
    {
        plot->refresh();
    }
}

void SpikeDisplay::plotSpike (const Spike* spike, int electrodeNum)
{
    spikePlots[electrodeNum]->processSpikeObject (spike);
}

void SpikeDisplay::invertSpikes (bool shouldInvert_)
{
    shouldInvert = shouldInvert_;

    for (int i = 0; i < spikePlots.size(); i++)
    {
        spikePlots[i]->invertSpikes (shouldInvert_);
    }
}

void SpikeDisplay::resetAudioMonitorState()
{
    for (int i = 0; i < spikePlots.size(); i++)
    {
        spikePlots[i]->resetAudioMonitorState();
    }
}

void SpikeDisplay::setPlotScaleFactor (float scale)
{
    scaleFactor = scale;

    resized();

    setBounds (0, 0, getWidth(), totalHeight);
}

void SpikeDisplay::setRange (int rangeInMicrovolts)
{
    for (auto plot : spikePlots)
        plot->setRange (rangeInMicrovolts);
}

void SpikeDisplay::setHistorySize (int history)
{
    for (auto plot : spikePlots)
        plot->setHistorySize (history);
}

void SpikeDisplay::registerThresholdCoordinator (SpikeThresholdCoordinator* stc)
{
    thresholdCoordinator = stc;

    for (int i = 0; i < spikePlots.size(); i++)
    {
        spikePlots[i]->registerThresholdCoordinator (stc);
    }
}

int SpikeDisplay::getNumPlots()
{
    return spikePlots.size();
}

int SpikeDisplay::getTotalHeight()
{
    return totalHeight;
}

int SpikeDisplay::getNumChannelsForPlot (int plotNum)
{
    return spikePlots[plotNum]->nChannels;
}

float SpikeDisplay::getThresholdForWaveAxis (int plotNum, int axisNum)
{
    return spikePlots[plotNum]->getDisplayThresholdForChannel (axisNum);
}

float SpikeDisplay::getRangeForWaveAxis (int plotNum, int axisNum)
{
    return spikePlots[plotNum]->getRangeForChannel (axisNum);
}

void SpikeDisplay::setThresholdForWaveAxis (int plotNum, int axisNum, float range)
{
    if (spikePlots.size() > plotNum)
        spikePlots[plotNum]->setDisplayThresholdForChannel (axisNum, range);
}

void SpikeDisplay::setRangeForWaveAxis (int plotNum, int axisNum, float range)
{
    if (spikePlots.size() > plotNum)
        spikePlots[plotNum]->setRangeForChannel (axisNum, range);
}

bool SpikeDisplay::getMonitorStateForPlot (int plotNum)
{
    if (spikePlots.size() > plotNum)
        return spikePlots[plotNum]->getMonitorState();
    else
        return false;
}

void SpikeDisplay::setMonitorStateForPlot (int plotNum, bool state)
{
    if (spikePlots.size() > plotNum)
        spikePlots[plotNum]->setMonitorState (state);
}
