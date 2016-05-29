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

    int currentMap;

};

class RasterPlot : public Component
{
public:
    RasterPlot(SpikeRasterCanvas*);
    virtual ~RasterPlot();

    AudioSampleBuffer spikes;

    void paint(Graphics& g);
    void resized();
    void reset();

    void processSpikeObject(const SpikeObject& s);

    Random random;

    void setCurrentUnit(int);
    void setCurrentElectrode(int);
    void setEventChannel(int);
    
    int unitId;
    int electrodeId;
    int eventId;

};


#endif