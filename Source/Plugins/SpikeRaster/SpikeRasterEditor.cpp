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

void SpikeRasterEditor::updateSettings()
{
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

    timescale = new Timescale(rasterPlot);
    addAndMakeVisible(timescale);

    for (int i = 0; i < 8; i++)
    {
        EventChannelButton* ecb = new EventChannelButton(rasterPlot, i, rasterPlot->getColourForChannel(i));
        eventChannelButtons.add(ecb);
        addAndMakeVisible(ecb);
    }

    triggerLabel = new Label("Triggers:","Triggers:");
    triggerLabel->setFont(Font("Small Text", 11, Font::plain));
    triggerLabel->setColour(Label::textColourId, Colour(200,200,200));
    addAndMakeVisible(triggerLabel);

    viewLabel = new Label("View:","View:");
    viewLabel->setFont(Font("Small Text", 11, Font::plain));
    viewLabel->setColour(Label::textColourId, Colour(200,200,200));
    addAndMakeVisible(viewLabel);

    viewButton = new UtilityButton("Continuous", Font("Small Text", 13, Font::plain));
    viewButton->setRadius(5.0f);
    viewButton->setEnabledState(true);
    viewButton->setCorners(true, true, true, true);
    viewButton->addListener(this);
    viewButton->setToggleState(false, dontSendNotification);
    addAndMakeVisible(viewButton);

    clearButton = new UtilityButton("Clear", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(5.0f);
    clearButton->setEnabledState(true);
    clearButton->setCorners(true, true, true, true);
    clearButton->addListener(this);
    clearButton->setToggleState(false, dontSendNotification);
    addAndMakeVisible(clearButton);

    preSecsLabel = new Label("Pre:","Pre:");
    preSecsLabel->setFont(Font("Small Text", 13, Font::plain));
    preSecsLabel->setColour(Label::textColourId, Colour(200, 200, 200));
    addAndMakeVisible(preSecsLabel);

    postSecsLabel = new Label("Post:","Post:");
    postSecsLabel->setFont(Font("Small Text", 13, Font::plain));
    postSecsLabel->setColour(Label::textColourId, Colour(200, 200, 200));
    addAndMakeVisible(postSecsLabel);

    preSecsInput = new Label("0.5","0.5");
    preSecsInput->setFont(Font("Small Text", 13, Font::plain));
    preSecsInput->setColour(Label::textColourId, Colour(250, 250, 250));
    preSecsInput->setEditable(true);
    preSecsInput->addListener(this);
    addAndMakeVisible(preSecsInput);

    postSecsInput = new Label("1.5","1.5");
    postSecsInput->setFont(Font("Small Text", 13, Font::plain));
    postSecsInput->setColour(Label::textColourId, Colour(250, 250, 250));
    postSecsInput->setEditable(true);
    postSecsInput->addListener(this);
    addAndMakeVisible(postSecsInput);

    electrodeLayoutLabel = new Label("Layout:","Layout:");
    electrodeLayoutLabel->setFont(Font("Small Text", 11, Font::plain));
    electrodeLayoutLabel->setColour(Label::textColourId, Colour(200,200,200));
    addAndMakeVisible(electrodeLayoutLabel);

    electrodeLayoutSelector = new UtilityButton("Default", Font("Small Text", 13, Font::plain));
    electrodeLayoutSelector->setRadius(5.0f);
    electrodeLayoutSelector->setEnabledState(true);
    electrodeLayoutSelector->setCorners(true, true, true, true);
    electrodeLayoutSelector->addListener(this);
    electrodeLayoutSelector->setToggleState(false, dontSendNotification);
    addAndMakeVisible(electrodeLayoutSelector);

    resized();
    repaint();
    
    processor->setRasterPlot(rasterPlot);

    viewType = 0;
    
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
}
    
void SpikeRasterCanvas::resized()
{
    rasterPlot->setBounds(140, 10, getWidth()-300, getHeight()-120);

    ratePlot->setBounds(10, 10, 120, getHeight()-120);

    psth->setBounds(140, getHeight()-100, getWidth()-300, 70);

    timescale->setBounds(125, getHeight()-25, getWidth()-270, 20);

    for (int i = 0; i < eventChannelButtons.size(); i++)
    {
         eventChannelButtons[i]->setBounds(getWidth()-140 + (i % 4) * 20, 40 + (i/4) * 20, 35, 35);
    }

    triggerLabel->setBounds(getWidth()-140, 15, 140, 30);
    viewLabel->setBounds(getWidth()-140, 100, 140, 20);
    viewButton->setBounds(getWidth()-135, 120, 105, 20);
    clearButton->setBounds(getWidth()-135, 250, 74, 20);
    preSecsLabel->setBounds(getWidth()-140, 160, 105, 15);
    preSecsInput->setBounds(getWidth()-90, 160, 45, 15);
    postSecsLabel->setBounds(getWidth()-140, 180, 105, 15);
    postSecsInput->setBounds(getWidth()-90, 180, 45, 15);
    electrodeLayoutLabel->setBounds(10, getHeight()-90, 140, 20);
    electrodeLayoutSelector->setBounds(10, getHeight()-70, 105, 20);

}

void SpikeRasterCanvas::labelTextChanged(Label* l)
{
    String text = l->getText();
    float value = text.getFloatValue();

    if (value < 0.05)
        value = 0.05;
    else if (value > 5)
        value = 5;

    if (l == preSecsInput)
        rasterPlot->setPreSecs(value);
    else if (l == postSecsInput)
        rasterPlot->setPostSecs(value);

    l->setText(String(value), dontSendNotification);

    float min = preSecsInput->getText().getFloatValue()*-1;
    float max = postSecsInput->getText().getFloatValue();

    timescale->setRange(min, max);

}

void SpikeRasterCanvas::buttonClicked(Button* b)
{
    if (b == viewButton)
    {
        if (viewType == 0)
        {
            viewType = 1;
            viewButton->setLabel("All");
        } else if (viewType == 1)
        {
            viewType = 2;
            viewButton->setLabel("Single");
        } else {
            viewType = 0;
            viewButton->setLabel("Continuous");
        }

        rasterPlot->setViewType(viewType);
    } 
    else if (b == clearButton)
    {
        rasterPlot->clear();
    } else if (b == electrodeLayoutSelector)
    {
        if (ratePlot->layout == 0)
        {
            ratePlot->setLayout(1);
            electrodeLayoutSelector->setLabel("Neuropix");
        } else if (ratePlot->layout == 1)
        {
            ratePlot->setLayout(2);
            electrodeLayoutSelector->setLabel("Neuroseeker");
        } else {
            ratePlot->setLayout(0);
            electrodeLayoutSelector->setLabel("Default");
        }

    }

    repaint();
}
    
// =====================================================



RasterPlot::RasterPlot(SpikeRasterCanvas*)
{

    rasterWidth = 500;
    rasterTimebase = 2.0f;
    preStimSecs = 0.5f;
    rasterStartTimestamp = 0;
    triggerTimestamp = -1;

    viewType = 0;
    trialIndex2 = 0;
    totalTrials = 0;

    electrodeChannels.add(0);

    spikeBuffer = AudioSampleBuffer(100,rasterWidth);
    trialBuffer1 = AudioSampleBuffer(100,rasterWidth);
    trialBuffer2 = AudioSampleBuffer(100,rasterWidth);

    spikeBuffer.clear();
    trialBuffer1.clear();
    trialBuffer2.clear();

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

void RasterPlot::clear()
{
    trialBuffer1.clear();
    trialBuffer2.clear();
    trialIndex1 = 0;
    trialIndex2 = 0;

    setNumberOfElectrodes(numElectrodes); // clear last sample
}

void RasterPlot::setViewType(int v)
{
    viewType = v;
}

void RasterPlot::paint(Graphics& g)
{

    //std::cout << "Drawing raster" << std::endl;

    AudioSampleBuffer* buffer;

    switch (viewType)
    {
        case 0:
            buffer = &spikeBuffer;
            break;
        case 1:
            buffer = &trialBuffer1;
            break;
        case 2:
            buffer = &trialBuffer2;
            break;
        default:
			buffer = &spikeBuffer;
            break;
    }

    g.fillAll(Colours::grey);

    float numYPixels = buffer->getNumChannels();
    float numXPixels = buffer->getNumSamples();

    float xHeight = getWidth()/numXPixels;
    float yHeight = getHeight()/numYPixels;

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            float colourIndex = buffer->getSample(m,n);

            if (viewType == 1)
            {
                colourIndex /= (totalTrials);
            }

            if (colourIndex > 0)
            {
                g.setColour(Colour(colourIndex*128+127, colourIndex*128+127, colourIndex*128+127));
                g.fillRect(n*xHeight, getHeight() - m*yHeight - yHeight, xHeight, yHeight);
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

void RasterPlot::setPreSecs(float value)
{
    float postStimSecs = rasterTimebase - preStimSecs;

    preStimSecs = value;
    rasterTimebase = postStimSecs + preStimSecs;

    clear();
    getParentComponent()->repaint();
}

void RasterPlot::setPostSecs(float value)
{

    rasterTimebase = value + preStimSecs;

    clear();

    getParentComponent()->repaint();

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

    if (triggerTimestamp > 0)
    {
        // copy data
        int offset = int((rasterTimebase - preStimSecs) * sampleRate);

        if (currentTimestamp > triggerTimestamp + offset)
        {
            
            std::cout << "Trigger time: " <<  triggerTimestamp << 
                        ", Current time: " <<  currentTimestamp << 
                        ", Offset: " << offset << std::endl;

            // copy data to all channels buffer

            for (int n = 0; n < numElectrodes; n++)
            {
                trialBuffer1.addFrom(n, 0, spikeBuffer, n, 0, spikeBuffer.getNumSamples());
            }

            trialBuffer2.clear(trialIndex2, 0, trialBuffer2.getNumSamples());

            for (int n = 0; n < electrodeChannels.size(); n++)
            {
                trialBuffer2.addFrom(trialIndex2, 0, spikeBuffer, electrodeChannels[n], 0, spikeBuffer.getNumSamples());
            }

            trialIndex2++;
            totalTrials++;

            if (trialIndex2 == spikeBuffer.getNumChannels())
                trialIndex2 = 0;

            triggerTimestamp = -1;
        } 
    }

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
    //int unit = s.sortedId;
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

void RasterPlot::processEvent(int chan, int64 timestamp)
{
    //std::cout << "Event chan: " << chan << ", ts: " << timestamp << std::endl;

    std::cout << std::endl;

    if (triggerChannels.contains(chan))
        triggerTimestamp = timestamp;
}

Array<float> RasterPlot::getPSTH(int numBins)
{
    int samplesPerBin = rasterWidth / numBins;

    AudioSampleBuffer* buffer;

    Array<float> psth;

    switch (viewType)
    {
        case 0:
        {
            buffer = &spikeBuffer;
            break;
        }
        case 1:
        {
            buffer = &trialBuffer1;
            break;
        }   
        case 2:
        {
            buffer = &trialBuffer2;
            break;
        }   
        default:
            buffer = &spikeBuffer;
            break;
    }

    int startBin; //= samplesPerBin * i;

    float maxBin = 0;

    for (int i = 0; i < numBins; i++)
    {
        startBin = samplesPerBin * i;

        float totalSpikes = 0;

        for (int m = startBin; m < startBin + samplesPerBin; m++)
        {
            for (int n = 0; n < numElectrodes; n++)
            {
                if (m < spikeBuffer.getNumSamples())
                    totalSpikes += buffer->getSample(n,m);
            }
        }

        float spikeRate = totalSpikes;

        // normalize it!

        switch (viewType)
        {
            case 0:
            {
                spikeRate /= float(numElectrodes);
                
                break;
            }
            case 1:
            {
                if (totalTrials > 0)
                {
                    spikeRate /= float(numElectrodes);
                    spikeRate /= float(totalTrials);
                }
                    
                break;
            }   
            case 2:
            {
                if (trialIndex2 > 0)
                {
                    
                    spikeRate /= float(jmin(trialIndex2, trialBuffer2.getNumChannels()));

                }
                    
                break;
            }   
            default:
                break;
        }

        spikeRate /= (rasterTimebase / numBins);

        psth.add(spikeRate);

        maxBin = jmax(maxBin, spikeRate);

        //std::cout << spikeRate << std::endl;
    }

    float upperBound = int(maxBin) / int(50) * int(50) + 40; 

    for (int i = 0; i < numBins; i++)
    {
        psth.set(i, psth[i]/upperBound);
    }

    psth.add(upperBound);

    return psth;
}

void RasterPlot::setEventTrigger(int ch, bool trigger)
{

    if (trigger)
    {
        triggerChannels.add(ch);
    }
    else
    {
        triggerChannels.remove(triggerChannels.indexOf(ch));
    }
}

void RasterPlot::toggleElectrodeState(int ch)
{
    if (electrodeChannels.contains(ch))
    {
        electrodeChannels.remove(electrodeChannels.indexOf(ch));
    } else {
        electrodeChannels.add(ch);
    }

    trialBuffer2.clear();
    trialIndex2 = 0;
}


float RasterPlot::getMaxBufferPos()
{
    float maxBufferPos = 0.0f;



    switch (viewType)
    {
        case 0:
        {
            for (int i = 0; i < numElectrodes; i++)
                maxBufferPos = jmax(lastBufferPos[i], maxBufferPos);
            break;
        }
        
        case 1:
        {
            maxBufferPos = preStimSecs / rasterTimebase;
            break;
        }
            
        case 2:
        {
            maxBufferPos = preStimSecs / rasterTimebase;
            break;
        }
            
        default:
            break;
    }

    

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

Colour RasterPlot::getColourForChannel(int ch)
{

    if (ch == 0)
        return Colour(224,185,36);
    else if (ch == 1)
        return Colour(243,119,33);
    else if (ch == 2)
        return Colour(237,37,36);
    else if (ch == 3)
        return Colour(217,46,171);
    else if (ch == 4)
        return Colour(101,31,255);
    else if (ch == 5)
        return Colour(48,117,255);
    else if (ch == 6)
        return Colour(116,227,156);
    else
        return Colour(82,173,0);
 
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

    g.setColour(Colours::grey);

    //if (psth[numBins] > 0)
    //{
        //std::cout << psth[numBins] << std::endl;
    float barHeight = 10.0f;

    while (barHeight < psth[numBins])
    {
         float h = getHeight() - (barHeight/psth[numBins])*getHeight();
         g.drawLine(0.0f, h, getWidth(), h);
         barHeight += 10.0f;
     }
    //}

    g.setColour(Colour(225, 54, 125));

    float maxBufferPos = raster->getMaxBufferPos();

    float barWidth = float(getWidth())/float(numBins);

    for (int i = 0; i < numBins; i++)
    {

        g.fillRect(barWidth*i, float(getHeight()*(1-psth[i])), barWidth+0.5f, psth[i]*getHeight());
 
    }

    g.setColour(Colours::black);
    g.fillRect(maxBufferPos * getWidth(), 0.0f, 2.0, float(getHeight()));

    float textHeight = 9.0f;
    g.setFont(7);
    while (textHeight < psth[numBins])
    {
         float h = getHeight() - (textHeight/psth[numBins])*getHeight();
         g.drawText(String(textHeight + 1.0f), 2, (int) h, 8, 7, Justification::left, false);
         textHeight += 10.0f;
     }
}

void PSTH::resized()
{

}

void PSTH::reset()
{

}

// ==========================================================


ElectrodeRateButton::ElectrodeRateButton(RatePlot* r, int chan_) : Button(String(chan_)), ratePlot(r)
{
    std::cout << "created button for channel " << chan_ << std::endl;
    chan = chan_;
    rate = 0.0f;

    setClickingTogglesState(true);
}


ElectrodeRateButton::~ElectrodeRateButton()
{

}

void ElectrodeRateButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState())
    {
        //std::cout << "hi" << std::endl;
        g.fillAll(Colours::white);
    }

    Colour fillColour = Colour(jmin(rate/20.0f, 1.0f)*225, 54.0f, jmin(rate/20.0f, 1.0f)*125);
    g.setColour(fillColour);
    g.fillRect(1,1,getHeight()-2,getWidth()-2);
}

void ElectrodeRateButton::resized()
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

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        electrodeButtons[i]->rate = rates[i];
    }

    g.setFont(Font("Small Text", 12, Font::plain));

    float xDist;
    if (layout == 0)
        xDist = 20;
    else
        xDist = 38;

    g.drawText("0", 0., getHeight() - 20, xDist, 15., Justification::right, false);

    for (int i = 9; i < electrodeButtons.size(); i += 10)
    {
        g.drawText(String(i+1), 0., getHeight() - (float(i)/4.*10. + 20.), xDist, 15., Justification::right, false);
    }

}

void RatePlot::resized()
{
    float electrodeSize = 10.0f;
    float xLoc;
    float yLoc;
    float xDist;

    if (layout == 0)
        xDist = 25;
    else
        xDist = 40;

    float yDist = 20;

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        switch (layout)
        {
            case 0:
            {
                if (i % 5 == 0){
                    xLoc = 0;
                    yLoc = (i / 5) * electrodeSize;
                }
                else if (i % 5 == 1){
                    xLoc = electrodeSize * 3;
                    yLoc = (i / 5)*electrodeSize;
                }
                else if (i % 5 == 2){
                    xLoc = electrodeSize * 6;
                    yLoc = (i / 5) * electrodeSize;
                }
                else if (i % 5 == 3){
                    xLoc = electrodeSize*1.5;
                    yLoc = (i / 5)*electrodeSize + electrodeSize/2;
                }
                else{
                    xLoc = electrodeSize * 4.5;
                    yLoc = (i / 5)*electrodeSize + electrodeSize/2;
                }
                break;
            }
            case 1:
            {
                if (i % 4 == 0){
                    xLoc = 0;
                    yLoc = (i / 4)*electrodeSize;
                }
                else if (i % 4 == 1){
                    xLoc = electrodeSize * 2;
                    yLoc = (i / 4)*electrodeSize;
                }
                else if (i % 4 == 2){
                    xLoc = electrodeSize;
                    yLoc = (i / 4)*electrodeSize + electrodeSize/2;
                }
                else{
                    xLoc = electrodeSize * 3;
                    yLoc = (i / 4)*electrodeSize + electrodeSize/2;
                }
                break;
            }
            case 2:
            {
                xLoc = (i % 4) * electrodeSize;
                yLoc = (i / 4) * electrodeSize;
                break;
            }
        }

        electrodeButtons[i]->setBounds(xDist + xLoc, getHeight() - yDist - yLoc, electrodeSize, electrodeSize);
        
    }
}

void RatePlot::reset()
{
    
}

void RatePlot::buttonClicked(Button* button)
{
    ElectrodeRateButton* b = (ElectrodeRateButton*) button;

    std::cout << b->chan << " was clicked." << std::endl;
    b->repaint();

    raster->toggleElectrodeState(b->chan);

}

void RatePlot::setLayout(int layout_)
{
    layout = layout_;

    resized();

}

void RatePlot::setNumberOfElectrodes(int n)
{
    electrodeButtons.clear();

    for (int i = 0; i < n; i++)
    {
        ElectrodeRateButton* button = new ElectrodeRateButton(this, i);
        electrodeButtons.add(button);
        addAndMakeVisible(button);
        button->addListener(this);
    }

    if (electrodeButtons.size() > 0)
        electrodeButtons[0]->setToggleState(true, dontSendNotification);

    resized();
}

// =========================================================

EventChannelButton::EventChannelButton(RasterPlot* rp, int chNum, Colour col):
    isEnabled(false), colour(col), rasterPlot(rp)
{

    channelNumber = chNum;

    chButton = new UtilityButton(String(channelNumber+1), Font("Small Text", 13, Font::plain));
    chButton->setRadius(5.0f);
    chButton->setBounds(4,4,14,14);
    chButton->setEnabledState(true);
    chButton->setCorners(true, false, true, false);

    chButton->addListener(this);
    addAndMakeVisible(chButton);

}

EventChannelButton::~EventChannelButton()
{

}

void EventChannelButton::buttonClicked(Button* button)
{

    isEnabled = !isEnabled;

    rasterPlot->setEventTrigger(channelNumber, isEnabled);

    repaint();

}


void EventChannelButton::paint(Graphics& g)
{

    if (isEnabled)
    {
        g.setColour(colour);
        g.fillRoundedRectangle(2,2,18,18,6.0f);
    }

}

// ============================================================================



Timescale::Timescale(RasterPlot* r) : raster(r)
{
    min = -0.5f;
    max = 1.5f;

    resolution = 0.5f;

}

Timescale::~Timescale()
{

}

void Timescale::paint(Graphics& g)
{
    g.fillAll(Colour(55,55,55));
    g.setColour(Colours::grey);
    g.fillRect(2,2,getWidth()-4,getHeight()-4);
    g.setColour(Colours::lightgrey);
    g.setFont(Font("Small Text", 12, Font::plain));

    float pt = min;

    //g.drawText("0", 2, (int) h, 8, 7, Justification::left, false);

    //std::cout << "max: " << max << ", min: " << min << std::endl;

    while (pt <= max + resolution)
    {
        float xLoc = (pt - min)/(max-min);
         //std::cout << " x: " << xLoc << " , val: " << pt << std::endl;
         g.drawText(String(pt), xLoc*(getWidth()-30)-4, 4, 40, 13, Justification::centred, false);
         pt += resolution;
     }
}

void Timescale::resized()
{

}

void Timescale::setRange(float mn, float mx)
{
    min = mn;
    max = mx;

    if (max - min > 4)
    {
        resolution = 1;
    } else if (max - min < 4 && max - min > 1)
    {
        resolution = 0.5f;
    } else if (max - min < 1)
    {
        resolution = 0.25f;
    }

    repaint();
}

