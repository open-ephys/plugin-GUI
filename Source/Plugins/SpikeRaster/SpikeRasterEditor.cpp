/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2016 Open Ephys

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

#include "SpikeRasterEditor.h"

SpikeRasterEditor::SpikeRasterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
: VisualizerEditor(parentNode, useDefaultParameterEditors)

{
    
    tabText = "Raster";
    
    desiredWidth = 180;

    electrodeSelector = new ComboBox();
    electrodeSelector->setBounds(35,40,100,20);
    electrodeSelector->addListener(this);

    addAndMakeVisible(electrodeSelector);

    unitSelector = new ComboBox();
    unitSelector->setBounds(35,70,100,20);
    unitSelector->addListener(this);

    addAndMakeVisible(unitSelector);

    eventChannelSelector = new ComboBox();
    eventChannelSelector->setBounds(35,100,100,20);
    eventChannelSelector->addListener(this);

    addAndMakeVisible(eventChannelSelector);
    
}

SpikeRasterEditor::~SpikeRasterEditor()
{
}


Visualizer* SpikeRasterEditor::createNewCanvas()
{
    
    SpikeRaster* processor = (SpikeRaster*) getProcessor();
    canvas = new SpikeRasterCanvas(processor);
    return canvas;
    
}

void SpikeRasterEditor::comboBoxChanged(ComboBox* c)
{

    SpikeRasterCanvas* rfmc = (SpikeRasterCanvas*) canvas.get();
    rasterPlot = rfmc->getRasterPlot();
    
    if (c == electrodeSelector)
    {
        rasterPlot->setCurrentElectrode(c->getSelectedId()-1);
    } else if (c == unitSelector)
    {
        rasterPlot->setCurrentUnit(c->getSelectedId()-1);
    } else if (c == unitSelector)
    {
        rasterPlot->setEventChannel(c->getSelectedId()-1);
    }
    
}

void SpikeRasterEditor::updateSettings()
{
    SpikeRaster* processor = (SpikeRaster*) getProcessor();

    int numElectrodes = processor->getNumElectrodes();
    
    electrodeSelector->clear();
    unitSelector->clear();
    eventChannelSelector->clear();

    if (numElectrodes > 0)
    {

        for (int i = 1; i <= numElectrodes; i++)
        {
            String itemName = "Electrode ";
            itemName += String(i);
            std::cout << i << " " << itemName << std::endl;
            electrodeSelector->addItem(itemName, i);
        }

        electrodeSelector->setSelectedId(1, dontSendNotification);

        unitSelector->addItem("MUA", 1);
        
        for (int i = 1; i <= 5; i++)
        {
            String itemName = "Unit ";
            itemName += String(i);
            unitSelector->addItem(itemName, i + 1);
        }

        unitSelector->setSelectedId(1, dontSendNotification);

        for (int i = 1; i <= 8; i++)
        {
            String itemName = "TTL ";
            itemName += String(i);
            eventChannelSelector->addItem(itemName, i);
        }

        eventChannelSelector->setSelectedId(1, dontSendNotification);
    }

    updateVisualizer();
}

// ==============================================================


SpikeRasterCanvas::SpikeRasterCanvas(SpikeRaster* sr) : processor(sr), currentMap(0)
{

    rasterPlot = new RasterPlot(this);
    addAndMakeVisible(rasterPlot);
    resized();
    repaint();
    
    processor->setRasterPlot(rasterPlot);
}


SpikeRasterCanvas::~SpikeRasterCanvas(){
    
}
    
void SpikeRasterCanvas::beginAnimation()
{
    startCallbacks();
    std::cout << "Spike Raster beginning animation." << std::endl;

}

void SpikeRasterCanvas::endAnimation()
{
    stopCallbacks();
}
    
void SpikeRasterCanvas::refreshState()
{
}

void SpikeRasterCanvas::update()
{
    

}
    
void SpikeRasterCanvas::setParameter(int, float) {}

void SpikeRasterCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::grey);
}
    
void SpikeRasterCanvas::refresh()
{
    processor->setParameter(2, 0.0f); // request redraw
    
    repaint();

    //::cout << "Refresh" << std::endl;
}
    
void SpikeRasterCanvas::resized()
{
    rasterPlot->setBounds(0, 0, getWidth(), getHeight());

    repaint();
}
    
// =====================================================



RasterPlot::RasterPlot(SpikeRasterCanvas*)
{
    spikes = AudioSampleBuffer(1000,30);

    random = Random();

    unitId = 0;
    electrodeId = 0;
    eventId = 0;

    reset();
}

RasterPlot::~RasterPlot()
{

}

void RasterPlot::reset()
{
    float numXPixels = spikes.getNumChannels();
    float numYPixels = spikes.getNumSamples();

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            spikes.setSample(n,m, 0.);
        }
    }

    repaint();
}

void RasterPlot::paint(Graphics& g)
{
    g.fillAll(Colours::purple);
}

void RasterPlot::resized()
{

}

void RasterPlot::setCurrentElectrode(int elec)
{
    electrodeId = elec;
}


void RasterPlot::setEventChannel(int event)
{
    eventId = event;
}


void RasterPlot::setCurrentUnit(int unit)
{
    unitId = unit;
}

void RasterPlot::processSpikeObject(const SpikeObject& s)
{

    std::cout << "Got spike." << std::endl;

    if (s.sortedId == unitId)
    {

    }

    

}
