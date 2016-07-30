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
class ElectrodeRateButton;

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



class RatePlot : public Component, public Button::Listener
{
public:
    RatePlot(RasterPlot*);
    virtual ~RatePlot();

    void paint(Graphics& g);
    void resized();
    void reset();

    void buttonClicked(Button* button);

    void setNumberOfElectrodes(int);

    void setLayout(int);

    OwnedArray<ElectrodeRateButton> electrodeButtons;

    int layout;
    int numElectrodes;
    RasterPlot* raster;

};

class ElectrodeRateButton : public Button
{
public:
    ElectrodeRateButton(RatePlot*, int chan);
    virtual ~ElectrodeRateButton();

    void paintButton(Graphics& g, bool, bool);
    void resized();
    int chan;
    float rate;
    bool isSelected;

    RatePlot* ratePlot;

};

class Timescale : public Component
{
public:
    Timescale(RasterPlot*);
    virtual ~Timescale();

    void paint(Graphics& g);
    void resized();

    void setRange(float, float);

    RasterPlot* raster;

    float min, max;
    float resolution;

};

class EventChannelButton : public Component,
    public Button::Listener
{
public:
    EventChannelButton(RasterPlot*, int chNum, Colour col);
    ~EventChannelButton();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    bool isEnabled;

private:

    int channelNumber;
    Colour colour;
    RasterPlot* rasterPlot;
    ScopedPointer<UtilityButton> chButton;

};

/**
 
 User interface for the SpikeRaster module.
 
 @see SpikeRaster
 
 */

class SpikeRasterEditor : public VisualizerEditor
{
public:
    SpikeRasterEditor(GenericProcessor*, bool useDefaultParameterEditors);
    ~SpikeRasterEditor();
    
    void updateSettings();
    
    Visualizer* createNewCanvas();
    
private:

    RasterPlot* rasterPlot;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeRasterEditor);
};


class SpikeRasterCanvas : public Visualizer, public Button::Listener, public Label::Listener
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

    void buttonClicked(Button*);
    void labelTextChanged(Label*);
    
    void paint(Graphics& g);

    void refresh();
    
    void resized();
    
    RasterPlot* getRasterPlot() {return rasterPlot.get();}

private:
    SpikeRaster* processor;
    ScopedPointer<RasterPlot> rasterPlot;
    ScopedPointer<PSTH> psth;
    ScopedPointer<RatePlot> ratePlot;
    ScopedPointer<Timescale> timescale;

    ScopedPointer<Label> triggerLabel;
    OwnedArray<EventChannelButton> eventChannelButtons;

    ScopedPointer<Label> viewLabel;
    ScopedPointer<UtilityButton> viewButton;

    ScopedPointer<Label> electrodeLayoutLabel;
    ScopedPointer<UtilityButton> electrodeLayoutSelector;

    ScopedPointer<UtilityButton> clearButton;

    ScopedPointer<Label> preSecsInput;
    ScopedPointer<Label> postSecsInput;
    ScopedPointer<Label> preSecsLabel;
    ScopedPointer<Label> postSecsLabel;

    int viewType;

    int currentMap;

    float psthHeight;

};

class RasterPlot : public Component
{
public:
    RasterPlot(SpikeRasterCanvas*);
    virtual ~RasterPlot();

    AudioSampleBuffer spikeBuffer;
    AudioSampleBuffer trialBuffer1;
    AudioSampleBuffer trialBuffer2;

    void paint(Graphics& g);
    void resized();
    void reset();

    void processSpikeObject(const SpikeObject& s);
    void processEvent(int eventChan, int64 ts);

    Random random;

    void setNumberOfElectrodes(int);
    void setSampleRate(float);
    void toggleElectrodeState(int);

    void setTimestamp(int64);
    void resetTimestamps();

    void setPreSecs(float);
    void setPostSecs(float);

    void setViewType(int);

    void clear();

    void setEventTrigger(int, bool);

    Array<float> getPSTH(int numBins);
    Array<float> getFiringRates();
    Array<int> triggerChannels;
    Array<int> electrodeChannels;
    float getMaxBufferPos();
    
    int rasterWidth; // number of pixels across raster
    float rasterTimebase; // timebase in s
    float preStimSecs; // pre-stimulus time

    int64 currentTimestamp;  // start time of data buffer (samples)
    int64 rasterStartTimestamp;  // start time of raster (samples)
    int64 triggerTimestamp;  // last trigger time

    int numElectrodes;
    int viewType;
    int trialIndex1;
    int trialIndex2;
    int totalTrials;

    Array<float> lastBufferPos;
    float sampleRate;

    Colour getColourForChannel(int ch);

};




#endif