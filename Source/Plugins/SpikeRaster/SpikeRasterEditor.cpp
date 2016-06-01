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

    ratePlot = new RatePlot(rasterPlot);
    addAndMakeVisible(ratePlot);

    psth = new PSTH(rasterPlot);
    addAndMakeVisible(psth);

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
    rasterPlot->resetTimestamps();
}
    
void SpikeRasterCanvas::refreshState()
{
}

void SpikeRasterCanvas::update()
{
    rasterPlot->setNumberOfElectrodes(processor->getNumElectrodes());
    ratePlot->setNumberOfElectrodes(processor->getNumElectrodes());

}
    
void SpikeRasterCanvas::setParameter(int, float) {}

void SpikeRasterCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}
    
void SpikeRasterCanvas::refresh()
{
    processor->setParameter(2, 0.0f); // request redraw
    
    repaint();

    //::cout << "Refresh" << std::endl;
}
    
void SpikeRasterCanvas::resized()
{
    rasterPlot->setBounds(100, 10, getWidth()-250, getHeight()-100);

    ratePlot->setBounds(10, 10, 80, getHeight()-100);

    psth->setBounds(100, getHeight()-80, getWidth()-250, 70);

}
    
// =====================================================



RasterPlot::RasterPlot(SpikeRasterCanvas*)
{

    unitId = 0;
    electrodeId = 0;
    eventId = 0;

    rasterWidth = 500;
    rasterTimebase = 2.0f;
    rasterStartTimestamp = 0;

    spikeBuffer = AudioSampleBuffer(60,rasterWidth);

    random = Random();

    reset();
    
}

RasterPlot::~RasterPlot()
{

}

void RasterPlot::setNumberOfElectrodes(int n)
{
    numElectrodes = n;

    lastBufferPos.clear();

    for (int i = 0; i < numElectrodes; i++)
    {
        lastBufferPos.add(0); // 0.0 to 1.0
    }

}

void RasterPlot::reset()
{
    spikeBuffer.clear();
    rasterStartTimestamp = 0;

    repaint();
}

void RasterPlot::paint(Graphics& g)
{

    //std::cout << "Drawing raster" << std::endl;
    g.fillAll(Colours::grey);

    float numYPixels = spikeBuffer.getNumChannels();
    float numXPixels = spikeBuffer.getNumSamples();

    float xHeight = getWidth()/numXPixels;
    float yHeight = getHeight()/numYPixels;

    g.setColour(Colours::white);

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            float colourIndex = spikeBuffer.getSample(m,n);

            if (colourIndex == 1.)
            {
                g.fillRect(n*xHeight, m*yHeight, xHeight, yHeight);
            }
        }
    }

    

    //std::cout << lastBufferPos[0] << std::endl;

    g.setColour(Colours::black);
    g.fillRect(getMaxBufferPos() * getWidth(), 0.0f, xHeight*2, float(getHeight()));
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

    //std::cout << "Setting timestamp." << std::endl;

    currentTimestamp = ts;

    float bufferStart = (currentTimestamp - rasterStartTimestamp) / (sampleRate * rasterTimebase); // (0 to 1)

    if (bufferStart < 1.0) // no overflow
    {
        for (int i = 0; i < numElectrodes; i++)
        {
            spikeBuffer.clear(i, lastBufferPos[i] * rasterWidth + 1,
                                 (bufferStart - lastBufferPos[i]) * rasterWidth);

            lastBufferPos.set(i, bufferStart);
        }

    } else {
        
        float bufferRemaining = bufferStart - 1.0;

        //std::cout << "Buffer remaining: " << bufferRemaining * rasterWidth << std::endl;

        for (int i = 0; i < numElectrodes; i++)
        {
            int startSample = lastBufferPos[i] * rasterWidth + 1;
            int endSample = (1.0 - lastBufferPos[i]) * rasterWidth;

            if (startSample + endSample > spikeBuffer.getNumSamples())
                endSample = spikeBuffer.getNumSamples() - startSample;

           // std::cout << "Clearing " << startSample << " to " << endSample << std::endl;

            spikeBuffer.clear(i, startSample, endSample);

            startSample = 0;
            endSample = bufferRemaining * rasterWidth;

            if (startSample + endSample > spikeBuffer.getNumSamples())
                endSample = spikeBuffer.getNumSamples() - startSample;

        //    std::cout << "Clearing " << startSample << " to " << endSample << std::endl;
            spikeBuffer.clear(i, startSample, endSample);

            lastBufferPos.set(i, bufferRemaining);
        }

        rasterStartTimestamp = currentTimestamp - int( bufferRemaining * sampleRate * rasterTimebase );
    }

    //std::cout << "Raster start: " << rasterStartTimestamp << std::endl;

}

void RasterPlot::resetTimestamps()
{

    //rasterStartTimestamp = 0;
    //setNumberOfElectrodes(numElectrodes);
}

void RasterPlot::setSampleRate(float sr)
{
    sampleRate = sr;
}

void RasterPlot::processSpikeObject(const SpikeObject& s)
{
    int electrode = s.source;
    int unit = s.sortedId;
    int timestamp = s.timestamp; // absolute time

    float bufferPos = float(timestamp - rasterStartTimestamp) / (sampleRate * rasterTimebase);

    //std::cout << "Spike time: " << bufferPos << std::endl;

    //std::cout << spikeBuffer.getNumSamples() << " " << spikeBuffer.getNumChannels() << " " << (int) (relativeTimestamp*1000) << " " << electrode << std::endl;

    if (bufferPos > 0. && bufferPos < 1.)
    {
        //std::cout << relativeTimestamp << " " << displayStartTimestamp << std::endl;

        int startSample = lastBufferPos[electrode] * rasterWidth + 1;
        int numSamples = (bufferPos - lastBufferPos[electrode]) * rasterWidth;

        if (numSamples > 0)
        {
            //std::cout << electrode << " " << startSample << " " << numSamples << std::endl;
            spikeBuffer.clear(electrode, startSample, numSamples);
        } else {
            spikeBuffer.clear(electrode, 0, bufferPos);
        }

        spikeBuffer.setSample(electrode, bufferPos * rasterWidth, 1);

        lastBufferPos.set(electrode, bufferPos);
    }
    
}

Array<float> RasterPlot::getPSTH(int numBins)
{
    int samplesPerBin = rasterWidth / numBins;

    Array<float> psth;

    int startBin; //= samplesPerBin * i;

    for (int i = 0; i < numBins; i++)
    {
        startBin = samplesPerBin * i;

        float totalSpikes = 0;

        for (int m = startBin; m < startBin + samplesPerBin; m++)
        {
            for (int n = 0; n < numElectrodes; n++)
            {
                if (m < spikeBuffer.getNumSamples())
                    totalSpikes += spikeBuffer.getSample(n,m);
            }
        }

        float spikeRate = totalSpikes / float(numElectrodes) / (rasterTimebase / numBins);

        //std::cout << totalSpikes << " ";

        psth.add(spikeRate);
    }

    //std::cout << std::endl;

    //std::cout << startBin << " " << samplesPerBin << std::endl;

    return psth;
}


float RasterPlot::getMaxBufferPos()
{
    float maxBufferPos = 0.0f;

    for (int i = 0; i < numElectrodes; i++)
        maxBufferPos = jmax(lastBufferPos[i], maxBufferPos);

    return maxBufferPos; 
}

Array<float> RasterPlot::getFiringRates()
{
    Array<float> rates;

    for (int n = 0; n < numElectrodes; n++)
    {
        float totalSpikes = 0;

        for (int m = 0; m < spikeBuffer.getNumSamples(); m++)
        {
            totalSpikes += spikeBuffer.getSample(n,m);
        }

        float spikeRate = totalSpikes / rasterTimebase;

        rates.add(spikeRate);
    }

    return rates;
}

//===========================================================

PSTH::PSTH(RasterPlot* r) : raster(r)
{
    numBins = 50;
}

PSTH::~PSTH()
{

}

void PSTH::paint(Graphics& g)
{
    g.fillAll(Colours::lightgrey);

    Array<float> psth = raster->getPSTH(numBins);

    g.setColour(Colours::purple);

    float maxBufferPos = raster->getMaxBufferPos();

    float barWidth = float(getWidth())/float(numBins);

    for (int i = 0; i < numBins; i++)
    {
        if (!(maxBufferPos*getWidth() > barWidth*i && maxBufferPos*getWidth() < barWidth*i + barWidth))
        {
            g.fillRect(barWidth*i, getHeight()-psth[i], barWidth, float(getHeight()));
        }
        
    }

    g.setColour(Colours::black);
    g.fillRect(maxBufferPos * getWidth(), 0.0f, 2.0, float(getHeight()));
}

void PSTH::resized()
{

}

void PSTH::reset()
{

}


//===========================================================

RatePlot::RatePlot(RasterPlot* r) : raster(r)
{
    layout = 0;
}

RatePlot::~RatePlot()
{

}

void RatePlot::paint(Graphics& g)
{
    g.fillAll(Colours::lightgrey);

    Array<float> rates = raster->getFiringRates();

    float maxBufferPos = raster->getMaxBufferPos();

    float electrodeSize = 10.0f;
    float electrodeSpacing = 5.0f;

    for (int i = 0; i < numElectrodes; i++)
    {
        g.setColour(Colours::black);
        g.fillRect(10.0f, 10.0f + (electrodeSize + electrodeSpacing) * i, electrodeSize, electrodeSize);

        Colour fillColour = Colour(jmin(rates[i]/20.0f, 1.0f)*255, 54.0f, jmin(rates[i]/20.0f, 1.0f)*255);

        g.setColour(fillColour);
        g.fillRect(11.0f, 10.0f + (electrodeSize + electrodeSpacing) * i +1.0f, electrodeSize-2.0f, electrodeSize-2.0f);

    }

    //std::cout << "Rate: " << rates[0] << std::endl;
}

void RatePlot::resized()
{

}

void RatePlot::reset()
{
    
}


void RatePlot::setNumberOfElectrodes(int n)
{
    numElectrodes = n;
}

