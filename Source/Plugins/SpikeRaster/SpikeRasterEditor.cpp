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
    rasterPlot->setNumberOfElectrodes(sr->getNumElectrodes());
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
    spikeBuffer = AudioSampleBuffer(30,1001);

    random = Random();

    unitId = 0;
    electrodeId = 0;
    eventId = 0;

    reset();

    displayStartTimestamp = 0;
    
}

RasterPlot::~RasterPlot()
{

}

void RasterPlot::setNumberOfElectrodes(int n)
{
    numElectrodes = n;

    lastSpikeTime.clear();

    for (int i = 0; i < numElectrodes; i++)
    {
        lastSpikeTime.add(0);
    }
    //spikeBuffer.setSize(numElectrodes, 1000);
}

void RasterPlot::reset()
{
    float numXPixels = spikeBuffer.getNumChannels();
    float numYPixels = spikeBuffer.getNumSamples();

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            spikeBuffer.setSample(n,m, 0.);
        }
    }

    repaint();
}

void RasterPlot::paint(Graphics& g)
{
    float numYPixels = spikeBuffer.getNumChannels();
    float numXPixels = spikeBuffer.getNumSamples();

    float xHeight = getWidth()/numXPixels;
    float yHeight = getHeight()/numYPixels;

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            float colourIndex = spikeBuffer.getSample(m,n);

            if (colourIndex == 0.)
            {
                g.setColour(Colours::grey);
            } else if (colourIndex == 1.)
            {
                g.setColour(Colours::white);
            } else {
                g.setColour(Colours::black);
            }

            g.fillRect(n*xHeight, m*yHeight, xHeight, yHeight);
        }
    }
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

void RasterPlot::setTimestamp(int64 ts)
{
    currentTimestamp = ts;
}

void RasterPlot::setSampleRate(float sr)
{
    sampleRate = sr;
}

void RasterPlot::processSpikeObject(const SpikeObject& s)
{
    int electrode = s.source;
    int unit = s.sortedId;
    int timestamp = s.timestamp;

    std::cout << "Got spike." << std::endl;

    if (currentTimestamp - displayStartTimestamp > sampleRate)
    {
        displayStartTimestamp = currentTimestamp;
        spikeBuffer.clear(electrode, 0, 1001);
    }

    // if (timestamp - displayStartTimestamp > sampleRate)
    // {
    //     displayStartTimestamp = timestamp;
    //     spikeBuffer.clear(electrode, 0, 1001);
    //     //spikeBuffer.clear();
    // }

    float relativeTimestamp = float(timestamp - displayStartTimestamp) / sampleRate;

    //std::cout << spikeBuffer.getNumSamples() << " " << spikeBuffer.getNumChannels() << " " << (int) (relativeTimestamp*1000) << " " << electrode << std::endl;

    if (relativeTimestamp > 0 && relativeTimestamp < 1)
    {
        //std::cout << relativeTimestamp << " " << displayStartTimestamp << std::endl;

        //spikeBuffer.clear(electrode, (int) (lastSpikeTime[electrode]*1000) + 1, (int) (relativeTimestamp - lastSpikeTime[electrode])*1000);
        spikeBuffer.setSample(electrode, (int) (relativeTimestamp*1000), 1);

        lastSpikeTime.set(electrode, relativeTimestamp);
    }
    

}
