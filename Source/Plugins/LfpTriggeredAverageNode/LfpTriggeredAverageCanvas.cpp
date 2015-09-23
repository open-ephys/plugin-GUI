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

#include "LfpTriggeredAverageCanvas.h"

#include <math.h>

LfpTriggeredAverageCanvas::LfpTriggeredAverageCanvas(LfpTriggeredAverageNode* processor_) :
    screenBufferIndex(0), timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f),
    processor(processor_),
    displayBufferIndex(0)
{

    nChans = processor->getNumInputs();
    sampleRate = processor->getSampleRate();
    std::cout << "Setting num inputs on LfpTriggeredAverageCanvas to " << nChans << std::endl;

    displayBuffer = processor->getDisplayBufferAddress();
    displayBufferSize = displayBuffer->getNumSamples();
    std::cout << "Setting displayBufferSize on LfpTriggeredAverageCanvas to " << displayBufferSize << std::endl;

    screenBuffer = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
    screenBuffer->clear();

    viewport = new Viewport();
    display = new LfpTriggeredAverageDisplay(this, viewport);
    timescale = new LfpTriggeredAverageTimescale(this);

    timescale->setTimebase(timebase);

    viewport->setViewedComponent(display, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();


    //viewport->getVerticalScrollBar()->addListener(this->scrollBarMoved(viewport->getVerticalScrollBar(), 1.0));



    addAndMakeVisible(viewport);
    addAndMakeVisible(timescale);

    voltageRanges.add("50");
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


    spreads.add("10");
    spreads.add("20");
    spreads.add("30");
    spreads.add("40");
    spreads.add("50");
    spreads.add("60");


    rangeSelection = new ComboBox("Voltage range");
    rangeSelection->addItemList(voltageRanges, 1);
    rangeSelection->setSelectedId(4, dontSendNotification);
    rangeSelection->addListener(this);
    addAndMakeVisible(rangeSelection);

    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(3, dontSendNotification);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);


    spreadSelection = new ComboBox("Spread");
    spreadSelection->addItemList(spreads, 1);
    spreadSelection->setSelectedId(5, dontSendNotification);
    spreadSelection->addListener(this);
    addAndMakeVisible(spreadSelection);


    display->setNumChannels(nChans);
    display->setRange(1000.0f);

    // add event display-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 8; i++)
    {


        LfpTriggeredAverageEventInterface* eventOptions = new LfpTriggeredAverageEventInterface(display, this, i);
        LfpTriggeredAverageEventInterfaces.add(eventOptions);
        addAndMakeVisible(eventOptions);
        eventOptions->setBounds(500+(floor(i/2)*20), getHeight()-20-(i%2)*20, 40, 20);

        display->setEventDisplayState(i,true);

    }


}

LfpTriggeredAverageCanvas::~LfpTriggeredAverageCanvas()
{

    deleteAndZero(screenBuffer);
}

void LfpTriggeredAverageCanvas::resized()
{

    timescale->setBounds(leftmargin,0,getWidth()-scrollBarThickness-leftmargin,30);
    viewport->setBounds(0,30,getWidth(),getHeight()-90);

    display->setBounds(0,0,getWidth()-scrollBarThickness, getChannelHeight()*nChans);

    rangeSelection->setBounds(5,getHeight()-30,100,25);
    timebaseSelection->setBounds(175,getHeight()-30,100,25);
    spreadSelection->setBounds(345,getHeight()-30,100,25);

    for (int i = 0; i < 8; i++)
    {
        LfpTriggeredAverageEventInterfaces[i]->setBounds(500+(floor(i/2)*20), getHeight()-40+(i%2)*20, 40, 20); // arrange event channel buttons in two rows
        LfpTriggeredAverageEventInterfaces[i]->repaint();
    }


    // std::cout << "Canvas thinks LfpTriggeredAverageDisplay should be this high: "
    //  << LfpTriggeredAverageDisplay->getTotalHeight() << std::endl;

}

void LfpTriggeredAverageCanvas::beginAnimation()
{
    std::cout << "Beginning animation." << std::endl;

    displayBufferSize = displayBuffer->getNumSamples();

    screenBufferIndex = 0;

    startCallbacks();
}

void LfpTriggeredAverageCanvas::endAnimation()
{
    std::cout << "Ending animation." << std::endl;

    stopCallbacks();
}

void LfpTriggeredAverageCanvas::update()
{
    nChans = jmax(processor->getNumInputs(),1);
    sampleRate = processor->getSampleRate();

    std::cout << "Setting num inputs on LfpTriggeredAverageCanvas to " << nChans << std::endl;

    refreshScreenBuffer();

    display->setNumChannels(nChans);

    // update channel names
    for (int i = 0; i < processor->getNumInputs(); i++)
    {

        String chName = processor->channels[i]->getName();

        //std::cout << chName << std::endl;

        display->channelInfo[i]->setName(chName);

    }

    display->setBounds(0,0,getWidth()-scrollBarThickness*2, display->getTotalHeight());

}

void LfpTriggeredAverageCanvas::comboBoxChanged(ComboBox* cb)
{

    if (cb == timebaseSelection)
    {
        timebase = timebases[cb->getSelectedId()-1].getFloatValue();
    }
    else if (cb == rangeSelection)
    {
        display->setRange(voltageRanges[cb->getSelectedId()-1].getFloatValue());
        //std::cout << "Setting range to " << voltageRanges[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == spreadSelection)
    {
        //spread = spreads[cb->getSelectedId()-1].getFloatValue();
        display->setChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
        //display->resized();
        resized();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }

    timescale->setTimebase(timebase);
}




int LfpTriggeredAverageCanvas::getChannelHeight()
{
    return spreads[spreadSelection->getSelectedId()-1].getIntValue();

}


void LfpTriggeredAverageCanvas::setParameter(int param, float val)
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

void LfpTriggeredAverageCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    displayBufferIndex = processor->getDisplayBufferIndex();
    screenBufferIndex = 0;

}

void LfpTriggeredAverageCanvas::refreshScreenBuffer()
{

    screenBufferIndex = 0;

    screenBuffer->clear();

    // int w = display->getWidth();
    // //std::cout << "Refreshing buffer size to " << w << "pixels." << std::endl;

    // for (int i = 0; i < w; i++)
    // {
    //  float x = float(i);

    //  for (int n = 0; n < nChans; n++)
    //  {
    //      waves[n][i*2] = x;
    //      waves[n][i*2+1] = 0.5f; // line in center of display
    //  }
    // }

}

void LfpTriggeredAverageCanvas::updateScreenBuffer()
{


    // copy new samples from the displayBuffer into the screenBuffer (waves)
    int maxSamples = display->getWidth() - leftmargin;

    if (screenBufferIndex >= maxSamples) // wrap around if we reached right edge before
        screenBufferIndex = 0;

    lastScreenBufferIndex = screenBufferIndex;

    int index = processor->getDisplayBufferIndex();

    int nSamples =  index - displayBufferIndex; // N new samples to be addeddisplayBufferIndex

    if (nSamples < 0) // buffer has reset to 0
    {
        nSamples = (displayBufferSize - displayBufferIndex) + index;
    }

    float ratio = sampleRate * timebase / float(getWidth() - leftmargin - scrollBarThickness);

    // this number is crucial: converting from samples to values (in px) for the screen buffer
    int valuesNeeded = (int) float(nSamples) / ratio;


    if (screenBufferIndex + valuesNeeded > maxSamples)  // crop number of samples to fit cavas width
    {
        valuesNeeded = maxSamples - screenBufferIndex;
    }

    float subSampleOffset = 0.0;

    displayBufferIndex = displayBufferIndex % displayBufferSize; // make sure we're not overshooting
    int nextPos = (displayBufferIndex + 1) % displayBufferSize; //  position next to displayBufferIndex in display buffer to copy from

    if (valuesNeeded > 0 && valuesNeeded < 1000)
    {

        for (int i = 0; i < valuesNeeded; i++) // also fill one extra sample for line drawing interpolation to match across draws
        {
            float gain = 1.0;
            float alpha = (float) subSampleOffset;
            float invAlpha = 1.0f - alpha;

            screenBuffer->clear(screenBufferIndex, 1);

            for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
            {

                screenBuffer->addFrom(channel, // destChannel
                                      screenBufferIndex, // destStartSample
                                      displayBuffer->getWritePointer(channel, displayBufferIndex), // source
                                      1, // numSamples
                                      invAlpha*gain); // gain

                screenBuffer->addFrom(channel, // destChannel
                                      screenBufferIndex, // destStartSample
                                      displayBuffer->getWritePointer(channel, nextPos), // source
                                      1, // numSamples
                                      alpha*gain); // gain


            }

            subSampleOffset += ratio;

            while (subSampleOffset >= 1.0)
            {
                if (++displayBufferIndex > displayBufferSize)
                    displayBufferIndex = 0;

                nextPos = (displayBufferIndex + 1) % displayBufferSize;
                subSampleOffset -= 1.0;
            }

            screenBufferIndex++;

        }


    }
    else
    {
        //std::cout << "Skip." << std::endl;
    }
}

float LfpTriggeredAverageCanvas::getXCoord(int chan, int samp)
{
    return samp;
}

int LfpTriggeredAverageCanvas::getNumChannels()
{
    return nChans;
}

float LfpTriggeredAverageCanvas::getYCoord(int chan, int samp)
{
    return *screenBuffer->getReadPointer(chan, samp);
}

void LfpTriggeredAverageCanvas::paint(Graphics& g)
{

    //std::cout << "Painting" << std::endl;
    g.setColour(Colour(0,18,43)); //background color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setGradientFill(ColourGradient(Colour(50,50,50),0,0,
                                     Colour(25,25,25),0,30,
                                     false));

    g.fillRect(0, 0, getWidth()-scrollBarThickness, 30);

    g.setColour(Colours::black);

    g.drawLine(0,30,getWidth()-scrollBarThickness,30);

    g.setColour(Colour(25,25,60)); // timing grid color

    int w = getWidth()-scrollBarThickness-leftmargin;

    for (int i = 0; i < 10; i++)
    {
        if (i == 5 || i == 0)
            g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,3.0f);
        else
            g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);

    g.setFont(Font("Default", 16, Font::plain));

    g.setColour(Colour(100,100,100));

    g.drawText("Voltage range (uV)",5,getHeight()-55,300,20,Justification::left, false);
    g.drawText("Timebase (s)",175,getHeight()-55,300,20,Justification::left, false);
    g.drawText("Spread (px)",345,getHeight()-55,300,20,Justification::left, false);

    g.drawText("Event display",500,getHeight()-55,300,20,Justification::left, false);



}

void LfpTriggeredAverageCanvas::refresh()
{
    updateScreenBuffer();

    display->refresh(); // redraws only the new part of the screen buffer

    //getPeer()->performAnyPendingRepaintsNow();

}

void LfpTriggeredAverageCanvas::saveVisualizerParameters(XmlElement* xml)
{

    XmlElement* xmlNode = xml->createNewChildElement("LfpTriggeredAverageDisplay");


    xmlNode->setAttribute("Range",rangeSelection->getSelectedId());
    xmlNode->setAttribute("Timebase",timebaseSelection->getSelectedId());
    xmlNode->setAttribute("Spread",spreadSelection->getSelectedId());

    int eventButtonState = 0;

    for (int i = 0; i < 8; i++)
    {
        if (display->eventDisplayEnabled[i])
        {
            eventButtonState += (1 << i);
        }
    }

    xmlNode->setAttribute("EventButtonState", eventButtonState);

    xmlNode->setAttribute("ScrollX",viewport->getViewPositionX());
    xmlNode->setAttribute("ScrollY",viewport->getViewPositionY());
}


void LfpTriggeredAverageCanvas::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("LfpTriggeredAverageDisplay"))
        {
            rangeSelection->setSelectedId(xmlNode->getIntAttribute("Range"));
            timebaseSelection->setSelectedId(xmlNode->getIntAttribute("Timebase"));
            spreadSelection->setSelectedId(xmlNode->getIntAttribute("Spread"));

            viewport->setViewPosition(xmlNode->getIntAttribute("ScrollX"),
                                      xmlNode->getIntAttribute("ScrollY"));

            int eventButtonState = xmlNode->getIntAttribute("eventButtonState");

            for (int i = 0; i < 8; i++)
            {
                display->eventDisplayEnabled[i] = (eventButtonState >> i) & 1;

                LfpTriggeredAverageEventInterfaces[i]->checkEnabledState();
            }
        }
    }

}


// -------------------------------------------------------------

LfpTriggeredAverageTimescale::LfpTriggeredAverageTimescale(LfpTriggeredAverageCanvas* c) : canvas(c)
{

    font = Font("Default", 16, Font::plain);
}

LfpTriggeredAverageTimescale::~LfpTriggeredAverageTimescale()
{

}

void LfpTriggeredAverageTimescale::paint(Graphics& g)
{



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

void LfpTriggeredAverageTimescale::setTimebase(float t)
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

LfpTriggeredAverageDisplay::LfpTriggeredAverageDisplay(LfpTriggeredAverageCanvas* c, Viewport* v) :
    canvas(c), viewport(v), range(1000.0f)
{

    totalHeight = 0;

    addMouseListener(this, true);

    // hue cycle
    //for (int i = 0; i < 15; i++)
    //{
    //    channelColours.add(Colour(float(sin((3.14/2)*(float(i)/15))),float(1.0),float(1),float(1.0)));
    //}

    //hand-built palette
    channelColours.add(Colour(224,185,36));
    channelColours.add(Colour(214,210,182));
    channelColours.add(Colour(243,119,33));
    channelColours.add(Colour(186,157,168));
    channelColours.add(Colour(237,37,36));
    channelColours.add(Colour(179,122,79));
    channelColours.add(Colour(217,46,171));
    channelColours.add(Colour(217, 139,196));
    channelColours.add(Colour(101,31,255));
    channelColours.add(Colour(141,111,181));
    channelColours.add(Colour(48,117,255));
    channelColours.add(Colour(184,198,224));
    channelColours.add(Colour(116,227,156));
    channelColours.add(Colour(150,158,155));
    channelColours.add(Colour(82,173,0));
    channelColours.add(Colour(125,99,32));

}

LfpTriggeredAverageDisplay::~LfpTriggeredAverageDisplay()
{
    deleteAllChildren();
}


int LfpTriggeredAverageDisplay::getNumChannels()
{
    return numChans;
}

void LfpTriggeredAverageDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;

    deleteAllChildren();

    channels.clear();
    channelInfo.clear();

    totalHeight = 0;

    for (int i = 0; i < numChans; i++)
    {

        //std::cout << "Adding new channel display." << std::endl;

        LfpTriggeredAverageChannelDisplay* lfpChan = new LfpTriggeredAverageChannelDisplay(canvas, this, i);

        lfpChan->setColour(channelColours[i % channelColours.size()]);
        lfpChan->setRange(range);
        lfpChan->setChannelHeight(canvas->getChannelHeight());

        addAndMakeVisible(lfpChan);

        channels.add(lfpChan);

        LfpTriggeredAverageChannelDisplayInfo* lfpInfo = new LfpTriggeredAverageChannelDisplayInfo(canvas, this, i);

        lfpInfo->setColour(channelColours[i % channelColours.size()]);
        lfpInfo->setRange(range);
        lfpInfo->setChannelHeight(canvas->getChannelHeight());

        addAndMakeVisible(lfpInfo);

        channelInfo.add(lfpInfo);

        totalHeight += lfpChan->getChannelHeight();

    }

}

int LfpTriggeredAverageDisplay::getTotalHeight()
{
    return totalHeight;
}

void LfpTriggeredAverageDisplay::resized()
{

    int totalHeight = 0;

    for (int i = 0; i < channels.size(); i++)
    {

        LfpTriggeredAverageChannelDisplay* disp = channels[i];

        disp->setBounds(canvas->leftmargin,
                        totalHeight-disp->getChannelOverlap()/2,
                        getWidth(),
                        disp->getChannelHeight()+disp->getChannelOverlap());

        LfpTriggeredAverageChannelDisplayInfo* info = channelInfo[i];

        info->setBounds(0,
                        totalHeight-disp->getChannelOverlap()/2,
                        canvas->leftmargin,
                        disp->getChannelHeight()+disp->getChannelOverlap());

        totalHeight += disp->getChannelHeight();

    }

    canvas->fullredraw = true; //issue full redraw

    // std::cout << "Total height: " << totalHeight << std::endl;

}

void LfpTriggeredAverageDisplay::paint(Graphics& g)
{

}

void LfpTriggeredAverageDisplay::refresh()
{


    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    // ensure that only visible channels are redrawn
    for (int i = 0; i < numChans; i++)
    {

        int componentTop = channels[i]->getY();
        int componentBottom = channels[i]->getHeight() + componentTop;

        if ((topBorder <= componentBottom && bottomBorder >= componentTop))
        {
            if (canvas->fullredraw)
            {
                channels[i]->fullredraw = true;
                channels[i]->repaint();
                channelInfo[i]->repaint();

            }
            else
            {
                channels[i]->repaint(canvas->lastScreenBufferIndex-2, 0, (canvas->screenBufferIndex-canvas->lastScreenBufferIndex)+3, getChildComponent(i)->getHeight());  //repaint only the updated portion
                // we redraw from -2 to +1 relative to the real redraw window, the -2 makes sure that the lines join nicely, and the +1 draws the vertical update line
            }
            //std::cout << i << std::endl;
        }

    }

    canvas->fullredraw = false;
}

void LfpTriggeredAverageDisplay::setRange(float r)
{
    range = r;

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setRange(range);
    }
}

int LfpTriggeredAverageDisplay::getRange()
{
    return channels[0]->getRange();
}


void LfpTriggeredAverageDisplay::setChannelHeight(int r)
{

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight(r);
        channelInfo[i]->setChannelHeight(r);
    }

    resized();

}

int LfpTriggeredAverageDisplay::getChannelHeight()
{
    return channels[0]->getChannelHeight();
}



void LfpTriggeredAverageDisplay::mouseWheelMove(const MouseEvent&  e, const MouseWheelDetails&   wheel)
{

    //std::cout << "Mouse wheel " <<  e.mods.isCommandDown() << "  " << wheel.deltaY << std::endl;

    if (e.mods.isCommandDown())  // CTRL + scroll wheel -> change channel spacing
    {
        //
        // this should also scroll to keep the selected channel at a constant y!
        //
        int h = getChannelHeight();
        if (wheel.deltaY>0)
        {
            setChannelHeight(h+1);
        }
        else
        {
            if (h>5)
                setChannelHeight(h-1);
        }
    }
    else
    {
        if (e.mods.isShiftDown())  // SHIFT + scroll wheel -> change channel range
        {
            int h= getRange();
            if (wheel.deltaY>0)
            {
                setRange(h+10);
            }
            else
            {
                if (h>11)
                    setRange(h-10);
            }

        }
        else    // just scroll
        {
            //  passes the event up to the viewport so the screen scrolls
            if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
                viewport->mouseWheelMove(e.getEventRelativeTo(canvas), wheel);

        }
    }

    canvas->fullredraw = true;//issue full redraw

    refresh();

}


void LfpTriggeredAverageDisplay::mouseDown(const MouseEvent& event)
{
    //int y = event.getMouseDownY(); //relative to each channel pos
    MouseEvent canvasevent = event.getEventRelativeTo(viewport);
    int y = canvasevent.getMouseDownY() + viewport->getViewPositionY(); // need to account for scrolling

    int dist=0;
    int mindist=10000;
    int closest=5;
    for (int n = 0; n < numChans; n++) // select closest instead of relying ot eventComponent
    {
        channels[n]->deselect();

        int cpos=(channels[n]->getY() + (channels[n]->getHeight()/2));
        dist=int(abs(y - cpos));

        //std::cout << "Mouse down at " << y << " pos is "<< cpos << "n:" << n << "  dist " << dist << std::endl;

        if (dist<mindist)
        {
            mindist=dist-1;
            closest=n;
        }
    }

    //LfpTriggeredAverageChannelDisplay* lcd = (LfpTriggeredAverageChannelDisplay*) event.eventComponent;
    //lcd->select();

    channels[closest]->select();

    canvas->fullredraw = true;//issue full redraw

    refresh();

}


bool LfpTriggeredAverageDisplay::setEventDisplayState(int ch, bool state)
{
    eventDisplayEnabled[ch] = state;
    return eventDisplayEnabled[ch];
}


bool LfpTriggeredAverageDisplay::getEventDisplayState(int ch)
{
    return eventDisplayEnabled[ch];
}


// ------------------------------------------------------------------

LfpTriggeredAverageChannelDisplay::LfpTriggeredAverageChannelDisplay(LfpTriggeredAverageCanvas* c, LfpTriggeredAverageDisplay* d, int channelNumber) :
    canvas(c), display(d), isSelected(false), chan(channelNumber), channelOverlap(300), channelHeight(40), range(1000.0f)
{


    name = String(channelNumber+1); // default is to make the channelNumber the name


    channelHeightFloat = (float) channelHeight;

    channelFont = Font("Default", channelHeight*0.6, Font::plain);

    lineColour = Colour(255,255,255);

}

LfpTriggeredAverageChannelDisplay::~LfpTriggeredAverageChannelDisplay()
{

}

void LfpTriggeredAverageChannelDisplay::paint(Graphics& g)
{

    //g.fillAll(Colours::grey);

    g.setColour(Colours::yellow);   // draw most recent drawn sample position
    g.drawLine(canvas->screenBufferIndex+1, 0, canvas->screenBufferIndex+1, getHeight());




    //g.setColour(Colours::red); // draw oldest drawn sample position
    //g.drawLine(canvas->lastScreenBufferIndex, 0, canvas->lastScreenBufferIndex, getHeight()-channelOverlap);

    int center = getHeight()/2;

    if (isSelected)
    {

        g.setColour(Colours::lightgrey);
        g.fillRect(0,center-channelHeight/2,10,channelHeight);
        g.drawLine(0,center+channelHeight/2,getWidth(),center+channelHeight/2);
        g.drawLine(0,center-channelHeight/2,getWidth(),center-channelHeight/2);

        g.setColour(Colour(25,25,25));
        g.drawLine(0,center+channelHeight/4,10,center+channelHeight/4);
        g.drawLine(0,center-channelHeight/4,10,center-channelHeight/4);

    }


    g.setColour(Colour(40,40,40));
    g.drawLine(0, getHeight()/2, getWidth(), getHeight()/2);

    int stepSize = 1;
    int from = 0; // for vertical line drawing in the LFP data
    int to = 0;

    //for (int i = 0; i < getWidth()-stepSize; i += stepSize) // redraw entire display
    int ifrom = canvas->lastScreenBufferIndex - 3; // need to start drawing a bit before the actual redraw windowfor the interpolated line to join correctly

    if (ifrom < 0)
        ifrom = 0;

    int ito = canvas->screenBufferIndex - 1;

    if (fullredraw)
    {
        ifrom = 0; //canvas->leftmargin;
        ito = getWidth()-stepSize;
        fullredraw = false;
    }

    for (int i = ifrom; i < ito ; i += stepSize) // redraw only changed portion
    {

        // draw event markers
        int rawEventState = canvas->getYCoord(canvas->getNumChannels(), i);// get last channel+1 in buffer (represents events)
        for (int ev_ch = 0; ev_ch < 8 ; ev_ch++) // for all event channels
        {
            if (display->getEventDisplayState(ev_ch))  // check if plotting for this channel is enabled
            {
                if (rawEventState & (1 << ev_ch))    // events are  representet by a bit code, so we have to extract the individual bits with a mask
                {
                    g.setColour(display->channelColours[ev_ch*2]); // get color from lfp color scheme
                    g.setOpacity(0.35f);
                    g.drawLine(i, center-channelHeight/2 , i, center+channelHeight/2);
                }
            }
        }

        //std::cout << "e " << canvas->getYCoord(canvas->getNumChannels()-1, i) << std::endl;

        g.setColour(lineColour);
        g.setOpacity(1);

        // drawLine makes for ok anti-aliased plots, but is pretty slow
        g.drawLine(i,
                   (canvas->getYCoord(chan, i)/range*channelHeightFloat)+getHeight()/2,
                   i+stepSize,
                   (canvas->getYCoord(chan, i+stepSize)/range*channelHeightFloat)+getHeight()/2);

        if (false) // switched back to line drawing now that we only draw partial updates
        {

            // // pixel wise line plot has no anti-aliasing, but runs much faster
            double a = (canvas->getYCoord(chan, i)/range*channelHeightFloat)+getHeight()/2;
            double b = (canvas->getYCoord(chan, i+stepSize)/range*channelHeightFloat)+getHeight()/2;

            if (a<b)
            {
                from = (a);
                to = (b);
            }
            else
            {
                from = (b);
                to = (a);
            }

            if ((to-from) < 40)  // if there is too much vertical range in one pixel, don't draw the full line for speed reasons
            {
                for (int j = from; j <= to; j += 1)
                {
                    g.setPixel(i,j);
                }
            }
            else if ((to-from) < 100)
            {
                for (int j = from; j <= to; j += 2)
                {
                    g.setPixel(i,j);
                }
            }
            else
            {
                g.setPixel(i,to);
                g.setPixel(i,from);
            }

        }

    }

    // g.setColour(lineColour.withAlpha(0.7f)); // alpha on seems to decrease draw speed
    // g.setFont(channelFont);
    //  g.setFont(channelHeightFloat*0.6);

    // g.drawText(String(chan+1), 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);


}






void LfpTriggeredAverageChannelDisplay::setRange(float r)
{
    range = r;

    //std::cout << "Range: " << r << std::endl;
}

int LfpTriggeredAverageChannelDisplay::getRange()
{
    return range;
}


void LfpTriggeredAverageChannelDisplay::select()
{
    isSelected = true;
}

void LfpTriggeredAverageChannelDisplay::deselect()
{
    isSelected = false;
}

void LfpTriggeredAverageChannelDisplay::setColour(Colour c)
{
    lineColour = c;
}


void LfpTriggeredAverageChannelDisplay::setChannelHeight(int c)
{
    channelHeight = c;
    channelHeightFloat = (float) channelHeight;
    //channelOverlap = channelHeight / 2; //clips data too early,
    channelOverlap = channelHeight *5;
}

int LfpTriggeredAverageChannelDisplay::getChannelHeight()
{

    return channelHeight;
}

void LfpTriggeredAverageChannelDisplay::setChannelOverlap(int overlap)
{
    channelOverlap = overlap;
}


int LfpTriggeredAverageChannelDisplay::getChannelOverlap()
{
    return channelOverlap;
}

void LfpTriggeredAverageChannelDisplay::setName(String name_)
{
    name = name_;
}

// -------------------------------

LfpTriggeredAverageChannelDisplayInfo::LfpTriggeredAverageChannelDisplayInfo(LfpTriggeredAverageCanvas* canvas_, LfpTriggeredAverageDisplay* display_, int ch)
    : LfpTriggeredAverageChannelDisplay(canvas_, display_, ch)
{

}

void LfpTriggeredAverageChannelDisplayInfo::paint(Graphics& g)
{


    int center = getHeight()/2;

    g.setColour(lineColour);

    g.setFont(channelFont);
    g.setFont(channelHeightFloat*0.3);

    g.drawText(name, 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);

}



// Event display Options --------------------------------------------------------------------

LfpTriggeredAverageEventInterface::LfpTriggeredAverageEventInterface(LfpTriggeredAverageDisplay* display_, LfpTriggeredAverageCanvas* canvas_, int chNum):
    isEnabled(true), display(display_), canvas(canvas_)
{

    channelNumber = chNum;

    chButton = new UtilityButton(String(channelNumber+1), Font("Small Text", 13, Font::plain));
    chButton->setRadius(5.0f);
    chButton->setBounds(4,4,14,14);
    chButton->setEnabledState(true);
    chButton->setCorners(true, false, true, false);
    //chButton.color = display->channelColours[channelNumber*2];
    chButton->addListener(this);
    addAndMakeVisible(chButton);


    checkEnabledState();

}

LfpTriggeredAverageEventInterface::~LfpTriggeredAverageEventInterface()
{

}

void LfpTriggeredAverageEventInterface::checkEnabledState()
{
    isEnabled = display->getEventDisplayState(channelNumber);

    //repaint();
}

void LfpTriggeredAverageEventInterface::buttonClicked(Button* button)
{
    checkEnabledState();
    if (isEnabled)
    {
        display->setEventDisplayState(channelNumber, false);
    }
    else
    {
        display->setEventDisplayState(channelNumber, true);
    }

    repaint();

}


void LfpTriggeredAverageEventInterface::paint(Graphics& g)
{

    checkEnabledState();

    if (isEnabled)
    {
        g.setColour(display->channelColours[channelNumber*2]);
        g.fillRoundedRectangle(2,2,18,18,5.0f);
    }


    //g.drawText(String(channelNumber), 8, 2, 200, 15, Justification::left, false);

}
