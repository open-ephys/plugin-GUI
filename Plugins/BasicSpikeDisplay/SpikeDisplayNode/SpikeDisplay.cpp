/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

SpikeDisplay::SpikeDisplay(SpikeDisplayCanvas* sdc, Viewport* v) :
    canvas(sdc), 
    viewport(v), 
    shouldInvert(false), 
    thresholdCoordinator(nullptr)
{

    totalHeight = 1000;

}


void SpikeDisplay::removePlots()
{
    spikePlots.clear();
}


void SpikeDisplay::clear()
{
    if (spikePlots.size() > 0)
    {
        for (int i = 0; i < spikePlots.size(); i++)
        {
            spikePlots[i]->clear();
        }
    }

}

SpikePlot* SpikeDisplay::addSpikePlot(int numChannels, int electrodeNum, String name_, std::string identifier_)
{

    LOGD("Adding spike plot for electrode: ", electrodeNum, " named: ", name_, " with ", numChannels, " channels");

    SpikePlot* spikePlot = new SpikePlot(canvas, electrodeNum, 1000 + numChannels, name_, identifier_);
    spikePlots.add(spikePlot);
    addAndMakeVisible(spikePlot);
    spikePlot->invertSpikes(shouldInvert);
    if (thresholdCoordinator)
    {
        spikePlot->registerThresholdCoordinator(thresholdCoordinator);
    }

    return spikePlot;
}

SpikePlot* SpikeDisplay::getSpikePlot(int index)
{
    return spikePlots[index];
}

void SpikeDisplay::paint(Graphics& g)
{

    g.fillAll(Colours::darkgrey);

}

void SpikeDisplay::resized()
{

    if (spikePlots.size() > 0)
    {

        int w = getWidth();

        int numColumns = 1;
        int column, row;

        int stereotrodeStart = 0;
        int tetrodeStart = 0;

        int singlePlotIndex = -1;
        int stereotrodePlotIndex = -1;
        int tetrodePlotIndex = -1;
        int index = -1;

        float width = 0;
        float height = 0;

        float maxHeight = 0;

        for (int i = 0; i < spikePlots.size(); i++)
        {

            if (spikePlots[i]->nChannels == 1)
            {
                index = ++singlePlotIndex;
                numColumns = (int) jmax(w / spikePlots[i]->minWidth / scaleFactor, 1.0f);
                width = jmin((float) w / (float) numColumns, (float) getWidth());
                height = width * spikePlots[i]->aspectRatio;

            }
            else if (spikePlots[i]->nChannels == 2)
            {
                index = ++stereotrodePlotIndex;
                numColumns = (int) jmax(w / spikePlots[i]->minWidth / scaleFactor, 1.0f);
                width = jmin((float) w / (float) numColumns, (float) getWidth());
                height = width * spikePlots[i]->aspectRatio;

            }
            else if (spikePlots[i]->nChannels == 4)
            {
                index = ++tetrodePlotIndex;
                numColumns = (int) jmax(w / spikePlots[i]->minWidth / scaleFactor, 1.0f);
                width = jmin((float) w / (float) numColumns, (float) getWidth());
                height = width * spikePlots[i]->aspectRatio;
            }

            column = index % numColumns;

            row = index / numColumns;

            spikePlots[i]->setBounds(width*column, row*height, width, height);

            maxHeight = jmax(maxHeight, row*height + height);

            if (spikePlots[i]->nChannels == 1)
            {
                stereotrodeStart = (int)(height*(float(row)+1));
            }
            else if (spikePlots[i]->nChannels == 2)
            {
                tetrodeStart = (int)(height*(float(row)+1));
            }

        }

        for (int i = 0; i < spikePlots.size(); i++)
        {

            int x = spikePlots[i]->getX();
            int y = spikePlots[i]->getY();
            int w2 = spikePlots[i]->getWidth();
            int h2 = spikePlots[i]->getHeight();

            if (spikePlots[i]->nChannels == 2)
            {
                spikePlots[i]->setBounds(x, y+stereotrodeStart, w2, h2);
                maxHeight = jmax(maxHeight, (float) y+stereotrodeStart+h2);

            }
            else if (spikePlots[i]->nChannels == 4)
            {
                spikePlots[i]->setBounds(x, y+stereotrodeStart+tetrodeStart, w2, h2);
                maxHeight = jmax(maxHeight, (float) y+stereotrodeStart+tetrodeStart+h2);
            }

        }

        totalHeight = (int) maxHeight;

        setBounds(0, 0, getWidth(), totalHeight);
    }

}

void SpikeDisplay::refresh()
{
    for (auto plot : spikePlots)
    {
        plot->refresh();
    }
}


void SpikeDisplay::plotSpike(const Spike* spike, int electrodeNum)
{
    spikePlots[electrodeNum]->processSpikeObject(spike);
}


void SpikeDisplay::invertSpikes(bool shouldInvert_)
{

    shouldInvert = shouldInvert_;

    for (int i = 0; i < spikePlots.size(); i++)
    {
        spikePlots[i]->invertSpikes(shouldInvert_);
    }

}

void SpikeDisplay::resetAudioMonitorState()
{
    for (int i = 0; i < spikePlots.size(); i++)
    {
        spikePlots[i]->resetAudioMonitorState();
    }
}

void SpikeDisplay::setPlotScaleFactor(float scale)
{
    scaleFactor = scale;

    resized();
}

void SpikeDisplay::registerThresholdCoordinator(SpikeThresholdCoordinator* stc)
{
    
    thresholdCoordinator = stc;

    for (int i = 0; i < spikePlots.size(); i++)
    {
        spikePlots[i]->registerThresholdCoordinator(stc);
    }
}

int SpikeDisplay::getNumPlots()
{
    return spikePlots.size();
}

int SpikeDisplay::getNumChannelsForPlot(int plotNum)
{
    return spikePlots[plotNum]->nChannels;
}

float SpikeDisplay::getThresholdForWaveAxis(int plotNum, int axisNum)
{
    return spikePlots[plotNum]->getDisplayThresholdForChannel(axisNum);
}

float SpikeDisplay::getRangeForWaveAxis(int plotNum, int axisNum)
{
    return spikePlots[plotNum]->getRangeForChannel(axisNum);
}

void SpikeDisplay::setThresholdForWaveAxis(int plotNum, int axisNum, float range)
{
    if (spikePlots.size() > plotNum)
        spikePlots[plotNum]->setDisplayThresholdForChannel(axisNum, range);
}

void SpikeDisplay::setRangeForWaveAxis(int plotNum, int axisNum, float range)
{
    if (spikePlots.size() > plotNum)
        spikePlots[plotNum]->setRangeForChannel(axisNum, range);
}

bool SpikeDisplay::getMonitorStateForPlot(int plotNum)
{
    if (spikePlots.size() > plotNum)
        return spikePlots[plotNum]->getMonitorState();
}

void SpikeDisplay::setMonitorStateForPlot(int plotNum, bool state)
{
    if (spikePlots.size() > plotNum)
        spikePlots[plotNum]->setMonitorState(state);
}