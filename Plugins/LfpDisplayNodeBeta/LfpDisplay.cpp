// LfpDisplay.cpp

#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpDisplayOptions.h"

using namespace LfpDisplayNodeBeta;

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v) :
    singleChan(-1), canvas(c), viewport(v)
{
    totalHeight = 0;
    colorGrouping=1;

    range[0] = 1000;
    range[1] = 500;
    range[2] = 500000;

    addMouseListener(this, true);

    // hue cycle
    //for (int i = 0; i < 15; i++)
    //{
    //    channelColours.add(Colour(float(sin((3.14/2)*(float(i)/15))),float(1.0),float(1),float(1.0)));
    //}

    backgroundColour = Colour(0,18,43);
    
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

    isPaused=false;

}

LfpDisplay::~LfpDisplay()
{
    deleteAllChildren();
}



int LfpDisplay::getNumChannels()
{
    return numChans;
}



int LfpDisplay::getColorGrouping()
{
    return colorGrouping;
}

void LfpDisplay::setColorGrouping(int i)
{
    colorGrouping=i;
    setColors(); // so that channel colors get re-assigned

}


void LfpDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;

    deleteAllChildren();

    channels.clear();
    channelInfo.clear();

    totalHeight = 0;

    for (int i = 0; i < numChans; i++)
    {
        //std::cout << "Adding new display for channel " << i << std::endl;

        LfpChannelDisplay* lfpChan = new LfpChannelDisplay(canvas, this, options, i);

        //lfpChan->setColour(channelColours[i % channelColours.size()]);
        lfpChan->setRange(range[options->getChannelType(i)]);
        lfpChan->setChannelHeight(canvas->getChannelHeight());
        
        addAndMakeVisible(lfpChan);

        channels.add(lfpChan);

        LfpChannelDisplayInfo* lfpInfo = new LfpChannelDisplayInfo(canvas, this, options, i);

        //lfpInfo->setColour(channelColours[i % channelColours.size()]);
        lfpInfo->setRange(range[options->getChannelType(i)]);
        lfpInfo->setChannelHeight(canvas->getChannelHeight());

        addAndMakeVisible(lfpInfo);

        channelInfo.add(lfpInfo);

        savedChannelState.add(true);

        totalHeight += lfpChan->getChannelHeight();

    }

    setColors();
    

    //std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;

}

void LfpDisplay::setColors()
{
    for (int i = 0; i < numChans; i++)
    {

        channels[i]->setColour(channelColours[(int(i/colorGrouping)+1) % channelColours.size()]);
        channelInfo[i]->setColour(channelColours[(int(i/colorGrouping)+1)  % channelColours.size()]);
    }

}


int LfpDisplay::getTotalHeight()
{
    return totalHeight;
}

void LfpDisplay::resized()
{
    
    //canvas->channelOverlapFactor

    int totalHeight = 0;

    for (int i = 0; i < channels.size(); i++)
    {

        LfpChannelDisplay* disp = channels[i];

        disp->setBounds(canvas->leftmargin,
                        totalHeight-(disp->getChannelOverlap()*canvas->channelOverlapFactor)/2,
                        getWidth(),
                        disp->getChannelHeight()+(disp->getChannelOverlap()*canvas->channelOverlapFactor));

        disp-> resized();
        
        LfpChannelDisplayInfo* info = channelInfo[i];

        info->setBounds(0,
                        totalHeight-disp->getChannelHeight()/4,
                        canvas->leftmargin + 50,
                        disp->getChannelHeight());

        totalHeight += disp->getChannelHeight();

    }

    canvas->fullredraw = true; //issue full redraw
    if (singleChan != -1)
        viewport->setViewPosition(juce::Point<int>(0,singleChan*getChannelHeight()));

  

    lfpChannelBitmap = Image(Image::ARGB, getWidth(), getHeight(), false);
    
    //inititalize black background
    Graphics gLfpChannelBitmap(lfpChannelBitmap);
    gLfpChannelBitmap.setColour(Colour(0,0,0)); //background color
    gLfpChannelBitmap.fillRect(0,0, getWidth(), getHeight());

    
    canvas->fullredraw = true;
    
    refresh();
    // std::cout << "Total height: " << totalHeight << std::endl;

}

void LfpDisplay::paint(Graphics& g)
{

    g.drawImageAt(lfpChannelBitmap, canvas->leftmargin,0);
    
}


void LfpDisplay::refresh()
{
    // Ensure the lfpChannelBitmap has been initialized
    if (lfpChannelBitmap.isNull()) { resized(); }

    // X-bounds of this update
    int fillfrom = canvas->lastScreenBufferIndex[0];
    int fillto = (canvas->screenBufferIndex[0]);
    
    if (fillfrom<0){fillfrom=0;};
    if (fillto>lfpChannelBitmap.getWidth()){fillto=lfpChannelBitmap.getWidth();};
    
    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    // clear appropriate section of the bitmap --
    // we need to do this before each channel draws its new section of data into lfpChannelBitmap
    Graphics gLfpChannelBitmap(lfpChannelBitmap);
    gLfpChannelBitmap.setColour(backgroundColour); //background color

    if (canvas->fullredraw)
    {
        gLfpChannelBitmap.fillRect(0,0, getWidth(), getHeight());
    } else {
        gLfpChannelBitmap.setColour(backgroundColour); //background color

        gLfpChannelBitmap.fillRect(fillfrom,0, (fillto-fillfrom)+1, getHeight());
    };
    
    
    for (int i = 0; i < numChans; i++)
    {

        int componentTop = channels[i]->getY();
        int componentBottom = channels[i]->getHeight() + componentTop;

        if ((topBorder <= componentBottom && bottomBorder >= componentTop)) // only draw things that are visible
        {
            if (canvas->fullredraw)
            {
                channels[i]->fullredraw = true;
                
                channels[i]->pxPaint();
                channelInfo[i]->repaint();
                
            }
            else
            {
                 channels[i]->pxPaint(); // draws to lfpChannelBitmap
                
                 // it's not clear why, but apparently because the pxPaint() in a child component of LfpDisplay, we also need to issue repaint() calls for each channel, even though there's nothin to repaint there. Otherwise, the repaint call in LfpDisplay::refresh(), a few lines down, lags behind the update line by ~60 px. This could ahev something to do with teh reopaint message passing in juce. In any case, this seemingly redundant repaint here seems to fix the issue.
                
                 // we redraw from 0 to +2 (px) relative to the real redraw window, the +1 draws the vertical update line
                 channels[i]->repaint(fillfrom, 0, (fillto-fillfrom)+2, channels[i]->getHeight());
                
                
            }
            //std::cout << i << std::endl;
        }

    }

    if (fillfrom == 0 && singleChan != -1)
    {
        channelInfo[singleChan]->repaint();
    }
    
    
    if (canvas->fullredraw)
    {
        repaint(0,topBorder,getWidth(),bottomBorder-topBorder);
    }else{
        //repaint(fillfrom, topBorder, (fillto-fillfrom)+1, bottomBorder-topBorder); // doesntb seem to be needed and results in duplicate repaint calls
    }
    
    canvas->fullredraw = false;
    
}



void LfpDisplay::setRange(float r, DataChannel::DataChannelTypes type)
{
    range[type] = r;
    
    if (channels.size() > 0)
    {

        for (int i = 0; i < numChans; i++)
        {
            if (channels[i]->getType() == type)
                channels[i]->setRange(range[type]);
        }
        canvas->fullredraw = true; //issue full redraw
    }
}

int LfpDisplay::getRange()
{
    return getRange(options->getSelectedType());
}

int LfpDisplay::getRange(DataChannel::DataChannelTypes type)
{
    for (int i=0; i < numChans; i++)
    {
        if (channels[i]->getType() == type)
            return channels[i]->getRange();
    }
    return 0;
}


void LfpDisplay::setChannelHeight(int r, bool resetSingle)
{

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight(r);
        channelInfo[i]->setChannelHeight(r);
    }
    if (resetSingle && singleChan != -1)
    {
        //std::cout << "width " <<  getWidth() << " numchans  " << numChans << " height " << getChannelHeight() << std::endl;
        setSize(getWidth(),numChans*getChannelHeight());
        viewport->setScrollBarsShown(true,false);
        viewport->setViewPosition(juce::Point<int>(0,singleChan*r));
        singleChan = -1;
        for (int n = 0; n < numChans; n++)
        {
            channelInfo[n]->setEnabledState(savedChannelState[n]);
        }
    }

    resized();

}

void LfpDisplay::setInputInverted(bool isInverted)
{

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setInputInverted(isInverted);
    }

    resized();

}

void LfpDisplay::setDrawMethod(bool isDrawMethod)
{
    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setDrawMethod(isDrawMethod);
    }
    resized();

}


int LfpDisplay::getChannelHeight()
{
    return channels[0]->getChannelHeight();
}



void LfpDisplay::mouseWheelMove(const MouseEvent&  e, const MouseWheelDetails&   wheel)
{

    //std::cout << "Mouse wheel " <<  e.mods.isCommandDown() << "  " << wheel.deltaY << std::endl;
    //TODO Changing ranges with the wheel is currently broken. With multiple ranges, most
    //of the wheel range code needs updating
    
    
    if (e.mods.isCommandDown() && singleChan == -1)  // CTRL + scroll wheel -> change channel spacing
    {
        int h = getChannelHeight();
        int hdiff=0;
        
        // std::cout << wheel.deltaY << std::endl;
        
        if (wheel.deltaY > 0)
        {
            hdiff = 2;
        }
        else
        {
            if (h > 5)
                hdiff = -2;
        }

        if (abs(h) > 100) // accelerate scrolling for large ranges
            hdiff *= 3;

        setChannelHeight(h+hdiff);
        int oldX=viewport->getViewPositionX();
        int oldY=viewport->getViewPositionY();

        setBounds(0,0,getWidth()-0, getChannelHeight()*canvas->nChans); // update height so that the scrollbar is correct

        int mouseY=e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
        int scrollBy = (mouseY/h)*hdiff*2;// compensate for motion of point under current mouse position
        viewport->setViewPosition(oldX,oldY+scrollBy); // set back to previous position plus offset

        options->setSpreadSelection(h+hdiff); // update combobox
        
        canvas->fullredraw = true;//issue full redraw - scrolling without modifier doesnt require a full redraw
    }
    else
    {
        if (e.mods.isAltDown())  // ALT + scroll wheel -> change channel range (was SHIFT but that clamps wheel.deltaY to 0 on OSX for some reason..)
        {
            int h = getRange();
            
            
            int step = options->getRangeStep(options->getSelectedType());
                       
            // std::cout << wheel.deltaY << std::endl;
            
            if (wheel.deltaY > 0)
            {
                setRange(h+step,options->getSelectedType());
            }
            else
            {
                if (h > step+1)
                    setRange(h-step,options->getSelectedType());
            }

            options->setRangeSelection(h); // update combobox
            canvas->fullredraw = true; //issue full redraw - scrolling without modifier doesnt require a full redraw
            
        }
        else    // just scroll
        {
            //  passes the event up to the viewport so the screen scrolls
            if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
                viewport->mouseWheelMove(e.getEventRelativeTo(canvas), wheel);

        }
      

    }
       //refresh(); // doesn't seem to be needed now that channels daraw to bitmap

}

void LfpDisplay::toggleSingleChannel(int chan)
{
    //std::cout << "Toggle channel " << chan << std::endl;

    if (chan != singleChan)
    {
        std::cout << "Single channel on" << std::endl;
        singleChan = chan;

        int newHeight = viewport->getHeight();
        channelInfo[chan]->setEnabledState(true);
        channelInfo[chan]->setSingleChannelState(true);
        setChannelHeight(newHeight, false);
        setSize(getWidth(), numChans*getChannelHeight());

        viewport->setScrollBarsShown(false,false);
        viewport->setViewPosition(juce::Point<int>(0,chan*newHeight));

        for (int i = 0; i < channels.size(); i++)
        {
            if (i != chan)
                channels[i]->setEnabledState(false);
        }

    }
    else if (chan == singleChan || chan == -2)
    {
        std::cout << "Single channel off" << std::endl;
        for (int n = 0; n < numChans; n++)
        {

            channelInfo[chan]->setSingleChannelState(false);
        }
        setChannelHeight(canvas->getChannelHeight());

        reactivateChannels();
    }
}

void LfpDisplay::reactivateChannels()
{

    for (int n = 0; n < channels.size(); n++)
       setEnabledState(savedChannelState[n], n);

}

bool LfpDisplay::getSingleChannelState()
{
    if (singleChan < 0) return false;
    else return true;
}


void LfpDisplay::mouseDown(const MouseEvent& event)
{
    //int y = event.getMouseDownY(); //relative to each channel pos
    MouseEvent canvasevent = event.getEventRelativeTo(viewport);
    int y = canvasevent.getMouseDownY() + viewport->getViewPositionY(); // need to account for scrolling
    int x = canvasevent.getMouseDownX();

    int dist = 0;
    int mindist = 10000;
    int closest = 5;
    for (int n = 0; n < numChans; n++) // select closest instead of relying on eventComponent
    {
        channels[n]->deselect();

        int cpos = (channels[n]->getY() + (channels[n]->getHeight()/2));
        dist = int(abs(y - cpos));

        //std::cout << "Mouse down at " << y << " pos is "<< cpos << "n:" << n << "  dist " << dist << std::endl;

        if (dist < mindist)
        {
            mindist = dist-1;
            closest = n;
        }
    }

    if (singleChan != -1)
    {
        //std::cout << y << " " << channels[singleChan]->getHeight() << " " << getRange() << std::endl;
        channelInfo[singleChan]->updateXY(
                float(x)/getWidth()*canvas->timebase, 
                (-(float(y)-viewport->getViewPositionY())/viewport->getViewHeight()*float(getRange()))+float(getRange()/2)
                );
    }

    channels[closest]->select();
    options->setSelectedType(channels[closest]->getType());

    if (event.getNumberOfClicks() == 2)
        toggleSingleChannel(closest);

    if (event.mods.isRightButtonDown())
    {
        PopupMenu channelMenu = channels[closest]->getOptions();
        const int result = channelMenu.show();
        channels[closest]->changeParameter(result);
    }

    canvas->fullredraw = true;//issue full redraw

    refresh();

}


bool LfpDisplay::setEventDisplayState(int ch, bool state)
{
    eventDisplayEnabled[ch] = state;
    return eventDisplayEnabled[ch];
}


bool LfpDisplay::getEventDisplayState(int ch)
{
    return eventDisplayEnabled[ch];
}


void LfpDisplay::setEnabledState(bool state, int chan, bool updateSaved)
{
    if (chan < numChans)
    {
        channels[chan]->setEnabledState(state);
        channelInfo[chan]->setEnabledState(state);

        if (updateSaved)
            savedChannelState.set(chan, state);

        canvas->isChannelEnabled.set(chan, state);
    }
}

bool LfpDisplay::getEnabledState(int chan)
{
    if (chan < numChans)
    {
        return channels[chan]->getEnabledState();
    }

    return false;
}

