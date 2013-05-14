/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "LfpDisplayCanvas.h"

#include <math.h>

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_) :
    screenBufferIndex(0), timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f),
    processor(processor_),
     displayBufferIndex(0)
{

    nChans = processor->getNumInputs();
    sampleRate = processor->getSampleRate();
    std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

    displayBuffer = processor->getDisplayBufferAddress();
    displayBufferSize = displayBuffer->getNumSamples();
    std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;

    screenBuffer = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);

    viewport = new Viewport();
    lfpDisplay = new LfpDisplay(this, viewport);
    timescale = new LfpTimescale(this);

    timescale->setTimebase(timebase);

    viewport->setViewedComponent(lfpDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    addAndMakeVisible(viewport);
    addAndMakeVisible(timescale);

    voltageRanges.add("100");
    voltageRanges.add("500");
    voltageRanges.add("1000");
    voltageRanges.add("2000");
    voltageRanges.add("5000");

    timebases.add("0.2");
    timebases.add("0.5");
    timebases.add("1.0");
    timebases.add("2.0");
    timebases.add("5.0");
    timebases.add("10.0");

    rangeSelection = new ComboBox("Voltage range");
    rangeSelection->addItemList(voltageRanges, 1);
    rangeSelection->setSelectedId(3,false);
    rangeSelection->addListener(this);
    addAndMakeVisible(rangeSelection);

    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(3,false);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);

    lfpDisplay->setNumChannels(nChans);
    lfpDisplay->setRange(1000.0f);

}

LfpDisplayCanvas::~LfpDisplayCanvas()
{

    deleteAndZero(screenBuffer);
}

void LfpDisplayCanvas::resized()
{

    timescale->setBounds(0,0,getWidth()-scrollBarThickness,30);
    viewport->setBounds(0,30,getWidth(),getHeight()-90);

    lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness, lfpDisplay->getTotalHeight());

    rangeSelection->setBounds(5,getHeight()-30,100,25);
    timebaseSelection->setBounds(175,getHeight()-30,100,25);

}

void LfpDisplayCanvas::beginAnimation()
{
    std::cout << "Beginning animation." << std::endl;

    displayBufferSize = displayBuffer->getNumSamples();

    screenBufferIndex = 0;

    startCallbacks();
}

void LfpDisplayCanvas::endAnimation()
{
    std::cout << "Ending animation." << std::endl;

    stopCallbacks();
}

void LfpDisplayCanvas::update()
{
    nChans = processor->getNumInputs();
    sampleRate = processor->getSampleRate();

    std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

    refreshScreenBuffer();

    lfpDisplay->setNumChannels(nChans);
    lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness*2, lfpDisplay->getTotalHeight());


    repaint();

}

void LfpDisplayCanvas::comboBoxChanged(ComboBox* cb)
{

    if (cb == timebaseSelection)
    {
        timebase = timebases[cb->getSelectedId()-1].getFloatValue();
    }
    else if (cb == rangeSelection)
    {
        lfpDisplay->setRange(voltageRanges[cb->getSelectedId()-1].getFloatValue());
        //std::cout << "Setting range to " << voltageRanges[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }

    timescale->setTimebase(timebase);
}


void LfpDisplayCanvas::setParameter(int param, float val)
{
    // if (param == 0)
    // {
    //     timebase = val;
    //     refreshScreenBuffer();
    // }
    // else
    // {
    //     displayGain = val; //* 0.0001f;
    // }

    // repaint();
}

void LfpDisplayCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    displayBufferIndex = processor->getDisplayBufferIndex();
    screenBufferIndex = 0;

}

void LfpDisplayCanvas::refreshScreenBuffer()
{

    screenBufferIndex = 0;

    screenBuffer->clear();

    // int w = lfpDisplay->getWidth();
    // //std::cout << "Refreshing buffer size to " << w << "pixels." << std::endl;

    // for (int i = 0; i < w; i++)
    // {
    // 	float x = float(i);

    // 	for (int n = 0; n < nChans; n++)
    // 	{
    // 		waves[n][i*2] = x;
    // 		waves[n][i*2+1] = 0.5f; // line in center of display
    // 	}
    // }

}

void LfpDisplayCanvas::updateScreenBuffer()
{
    // copy new samples from the displayBuffer into the screenBuffer (waves)

    lastScreenBufferIndex = screenBufferIndex;

    int maxSamples = lfpDisplay->getWidth();

    int index = processor->getDisplayBufferIndex();

    int nSamples = index - displayBufferIndex;

    if (nSamples < 0) // buffer has reset to 0
    {
        nSamples = (displayBufferSize - displayBufferIndex) + index;
    }

    float ratio = sampleRate * timebase / float(getWidth());

    // this number is crucial:
    int valuesNeeded = (int) float(nSamples) / ratio;

    float subSampleOffset = 0.0;
    int nextPos = (displayBufferIndex + 1) % displayBufferSize;

    if (valuesNeeded > 0 && valuesNeeded < 1000)
    {

        for (int i = 0; i < valuesNeeded; i++)
        {
            float gain = 1.0;
            float alpha = (float) subSampleOffset;
            float invAlpha = 1.0f - alpha;

            screenBuffer->clear(screenBufferIndex, 1);

            for (int channel = 0; channel < nChans; channel++)
            {

                gain = 1.0f / (processor->channels[channel]->bitVolts * float(0x7fff));

                screenBuffer->addFrom(channel, // destChannel
                                      screenBufferIndex, // destStartSample
                                      displayBuffer->getSampleData(channel, displayBufferIndex), // source
                                      1, // numSamples
                                      invAlpha*gain); // gain

                screenBuffer->addFrom(channel, // destChannel
                                      screenBufferIndex, // destStartSample
                                      displayBuffer->getSampleData(channel, nextPos), // source
                                      1, // numSamples
                                      alpha*gain); // gain

                //waves[channel][screenBufferIndex*2+1] =
                //	*(displayBuffer->getSampleData(channel, displayBufferIndex))*invAlpha*gain*displayGain;

                //waves[channel][screenBufferIndex*2+1] +=
                //	*(displayBuffer->getSampleData(channel, nextPos))*alpha*gain*displayGain;

                //waves[channel][screenBufferIndex*2+1] += 0.5f; // to center in viewport

            }

            //// now do the event channel
            ////	waves[nChans][screenBufferIndex*2+1] =
            //		*(displayBuffer->getSampleData(nChans, displayBufferIndex));


            subSampleOffset += ratio;

            while (subSampleOffset >= 1.0)
            {
                if (++displayBufferIndex >= displayBufferSize)
                    displayBufferIndex = 0;

                nextPos = (displayBufferIndex + 1) % displayBufferSize;
                subSampleOffset -= 1.0;
            }

            screenBufferIndex++;
            screenBufferIndex %= maxSamples;

        }

    }
    else
    {
        //std::cout << "Skip." << std::endl;
    }
}

float LfpDisplayCanvas::getXCoord(int chan, int samp)
{
    return samp;
}

float LfpDisplayCanvas::getYCoord(int chan, int samp)
{
    return *screenBuffer->getSampleData(chan, samp);
}

void LfpDisplayCanvas::paint(Graphics& g)
{

    //std::cout << "Painting" << std::endl;
    g.setColour(Colour(25,25,25));

    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(Colour(40,40,40));

    int w = getWidth()-scrollBarThickness;

    for (int i = 1; i < 10; i++)
    {
        if (i == 5)
            g.drawLine(w/10*i,0,w/10*i,getHeight()-60,3.0f);
        else
            g.drawLine(w/10*i,0,w/10*i,getHeight()-60,1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);

    g.setFont(Font("Default", 16, Font::plain));

    g.setColour(Colour(100,100,100));

    g.drawText("Voltage range (uV)",5,getHeight()-55,300,20,Justification::left, false);

    g.drawText("Timebase (s)",175,getHeight()-55,300,20,Justification::left, false);

}

void LfpDisplayCanvas::refresh()
{
    updateScreenBuffer();

    lfpDisplay->refresh();

    //getPeer()->performAnyPendingRepaintsNow();

}

void LfpDisplayCanvas::saveVisualizerParameters(XmlElement* xml)
{

    XmlElement* xmlNode = xml->createNewChildElement("LFPDISPLAY");

    xmlNode->setAttribute("Range",rangeSelection->getSelectedId());
    xmlNode->setAttribute("Timebase",timebaseSelection->getSelectedId());

    xmlNode->setAttribute("ScrollX",viewport->getViewPositionX());
    xmlNode->setAttribute("ScrollY",viewport->getViewPositionY());
}


void LfpDisplayCanvas::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("LFPDISPLAY"))
        {
            rangeSelection->setSelectedId(xmlNode->getIntAttribute("Range"));
            timebaseSelection->setSelectedId(xmlNode->getIntAttribute("Timebase"));
            viewport->setViewPosition(xmlNode->getIntAttribute("ScrollX"),
                                      xmlNode->getIntAttribute("ScrollY"));
        }
    }

}


// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplayCanvas* c) : canvas(c)
{

    font = Font("Default", 16, Font::plain);
}

LfpTimescale::~LfpTimescale()
{

}

void LfpTimescale::paint(Graphics& g)
{

    g.setGradientFill(ColourGradient(Colour(50,50,50),0,0,
                                     Colour(25,25,25),0,getHeight(),
                                     false));

    g.fillAll();

    g.setColour(Colours::black);

    g.drawLine(0,getHeight(),getWidth(),getHeight());

    g.setFont(font);

    g.setColour(Colour(100,100,100));

    g.drawText("ms:",5,0,100,getHeight(),Justification::left, false);

    for (int i = 1; i < 10; i++)
    {
        if (i == 5)
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),3.0f);
        else
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),1.0f);

        g.drawText(labels[i-1],getWidth()/10*i+3,0,100,getHeight(),Justification::left, false);
    }

}

void LfpTimescale::setTimebase(float t)
{
    timebase = t;

    labels.clear();

    for (float i = 1.0f; i < 10.0; i++)
    {
        String labelString = String(timebase/10.0f*1000.0f*i);

        labels.add(labelString.substring(0,4));
    }

    repaint();

}


// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v) :
    canvas(c), viewport(v), range(1000.0f)
{

    totalHeight = 0;

    addMouseListener(this, true);

    for (int i = 0; i < 10; i++)
    {
        channelColours.add(Colour(200,200,255-i*25));
    }

    for (int i = 10; i > -1; i--)
    {
        channelColours.add(Colour(200,200,255-i*25));
    }

}

LfpDisplay::~LfpDisplay()
{
    deleteAllChildren();
}

void LfpDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;

    deleteAllChildren();

    channels.clear();

    totalHeight = 0;

    for (int i = 0; i < numChans; i++)
    {

        //std::cout << "Adding new channel display." << std::endl;

        LfpChannelDisplay* lfpChan = new LfpChannelDisplay(canvas, i);

        lfpChan->setColour(channelColours[i % channelColours.size()]);
        lfpChan->setRange(range);

        addAndMakeVisible(lfpChan);

        channels.add(lfpChan);

        totalHeight += lfpChan->getChannelHeight();

    }

}

void LfpDisplay::resized()
{

    int totalHeight = 0;

    for (int i = 0; i < numChans; i++)
    {

        LfpChannelDisplay* disp = channels[i];

        disp->setBounds(0,
                totalHeight-disp->getChannelOverlap()/2,
                getWidth(),
                disp->getChannelHeight()+disp->getChannelOverlap());

        totalHeight += disp->getChannelHeight();

    }

}

void LfpDisplay::paint(Graphics& g)
{

}

void LfpDisplay::refresh()
{


    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    // ensure that only visible channels are redrawn
    for (int i = 0; i < numChans; i++)
    {

        int componentTop = getChildComponent(i)->getY();
        int componentBottom = getChildComponent(i)->getHeight() + componentTop;

        if ((topBorder <= componentBottom && bottomBorder >= componentTop))
        {
            getChildComponent(i)->repaint();

            //std::cout << i << std::endl;
        }

    }

}

void LfpDisplay::setRange(float r)
{

    range = r;

    for (int i = 0; i < numChans; i++)
    {

        channels[i]->setRange(range);

    }

}

void LfpDisplay::mouseDown(const MouseEvent& event)
{
    //int x = event.getMouseDownX();
    //int y = event.getMouseDownY();

    //std::cout << "Mouse down at " << x << ", " << y << std::endl;


    for (int n = 0; n < numChans; n++)
    {
        channels[n]->deselect();
    }

    LfpChannelDisplay* lcd = (LfpChannelDisplay*) event.eventComponent;

    lcd->select();

    repaint();

}

// ------------------------------------------------------------------

LfpChannelDisplay::LfpChannelDisplay(LfpDisplayCanvas* c, int channelNumber) :
    canvas(c), isSelected(false), chan(channelNumber), channelHeight(100), channelOverlap(60), range(1000.0f)
{

    ch = (float) channelHeight;

    channelFont = Font("Default", 50, Font::plain);

    lineColour = Colour(255,255,255);

}

LfpChannelDisplay::~LfpChannelDisplay()
{

}

void LfpChannelDisplay::paint(Graphics& g)
{

    //g.fillAll(Colours::grey);

    g.setColour(Colours::yellow);

    g.drawLine(canvas->screenBufferIndex, 0, canvas->screenBufferIndex, getHeight()-channelOverlap);

    int center = getHeight()/2;

    if (isSelected)
    {
        g.setColour(Colours::lightgrey);
        g.fillRect(0,center-50,10,100);
        g.drawLine(0,center+50,getWidth(),center+50);
        g.drawLine(0,center-50,getWidth(),center-50);

        g.setColour(Colour(25,25,25));
        g.drawLine(0,center+25,10,center+25);
        g.drawLine(0,center-25,10,center-25);

    }


    g.setColour(Colour(40,40,40));
    g.drawLine(0, getHeight()/2, getWidth(), getHeight()/2);

    int stepSize = 1;

    g.setColour(lineColour);

    for (int i = 0; i < getWidth()-stepSize; i += stepSize)
    {

        g.drawLine(i,
                   (canvas->getYCoord(chan, i)/range*ch)+getHeight()/2,
                   i+stepSize,
                   (canvas->getYCoord(chan, i+stepSize)/range*ch)+getHeight()/2);
    }

    g.setColour(lineColour.withAlpha(0.7f));
    g.setFont(channelFont);

    g.drawText(String(chan+1), 10, channelHeight/2, 200, 50, Justification::left, false);


}

void LfpChannelDisplay::setRange(float r)
{
    range = r;
}

void LfpChannelDisplay::select()
{
    isSelected = true;
}

void LfpChannelDisplay::deselect()
{
    isSelected = false;
}

void LfpChannelDisplay::setColour(Colour c)
{
    lineColour = c;
}


void LfpChannelDisplay::setChannelHeight(int c)
{
    channelHeight = c;
    ch = (float) channelHeight;
}

int LfpChannelDisplay::getChannelHeight()
{

    return channelHeight;
}

void LfpChannelDisplay::setChannelOverlap(int overlap)
{
    channelOverlap = overlap;
}


int LfpChannelDisplay::getChannelOverlap()
{
    return channelOverlap;
}