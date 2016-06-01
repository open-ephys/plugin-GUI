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

#ifndef SPIKERASTEREDITOR_H_INCLUDED
#define SPIKERASTEREDITOR_H_INCLUDED

#include <VisualizerEditorHeaders.h>
#include <VisualizerWindowHeaders.h>
#include <SpikeLib.h>

#include "SpikeRaster.h"

class Visualizer;

class PSTH : public Component
{
public:
    PSTH(RasterPlot*);
    virtual ~PSTH();

    void paint(Graphics& g);
    void resized();
    void reset();

    int numBins;
    RasterPlot* raster;

};

class RatePlot : public Component
{
public:
    RatePlot(RasterPlot*);
    virtual ~RatePlot();

    void paint(Graphics& g);
    void resized();
    void reset();

    void setNumberOfElectrodes(int);

    int layout;
    int numElectrodes;
    RasterPlot* raster;

};

/**
 
 User interface for the SpikeRaster module.
 
 @see SpikeRaster
 
 */

class SpikeRasterEditor : public VisualizerEditor, public ComboBox::Listener
{
public:
    SpikeRasterEditor(GenericProcessor*, bool useDefaultParameterEditors);
    ~SpikeRasterEditor();
    
    void updateSettings();

    void comboBoxChanged(ComboBox* c);
    
    Visualizer* createNewCanvas();
    
private:

    ScopedPointer<ComboBox> electrodeSelector;
    ScopedPointer<ComboBox> unitSelector;
    ScopedPointer<ComboBox> eventChannelSelector;

    RasterPlot* rasterPlot;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeRasterEditor);
};


class SpikeRasterCanvas : public Visualizer
{
public:
    SpikeRasterCanvas(SpikeRaster* n);
    ~SpikeRasterCanvas();
    
    void beginAnimation();
    void endAnimation();
    
    void refreshState();
    void update();
    
    void setParameter(int, float);
    void setParameter(int, int, int, float) {}
    
    void paint(Graphics& g);

    void refresh();
    
    void resized();
    
    RasterPlot* getRasterPlot() {return rasterPlot.get();}

private:
    SpikeRaster* processor;
    ScopedPointer<RasterPlot> rasterPlot;
    ScopedPointer<PSTH> psth;
    ScopedPointer<RatePlot> ratePlot;

    int currentMap;

};

class RasterPlot : public Component
{
public:
    RasterPlot(SpikeRasterCanvas*);
    virtual ~RasterPlot();

    AudioSampleBuffer spikeBuffer;

    void paint(Graphics& g);
    void resized();
    void reset();

    void processSpikeObject(const SpikeObject& s);

    Random random;

    void setCurrentUnit(int);
    void setCurrentElectrode(int);
    void setEventChannel(int);

    void setNumberOfElectrodes(int);
    void setSampleRate(float);

    void setTimestamp(int64);
    void resetTimestamps();

    Array<float> getPSTH(int numBins);
    Array<float> getFiringRates();
    float getMaxBufferPos();
    
    int unitId;
    int electrodeId;
    int eventId;
    int rasterWidth; // number of pixels across raster
    float rasterTimebase; // timebase in s

    int64 currentTimestamp;  // start time of data buffer (samples)
    int64 rasterStartTimestamp;  // start time of raster (samples)
    int numElectrodes;

    Array<float> lastBufferPos;
    float sampleRate;

};




#endif