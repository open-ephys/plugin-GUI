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

#include "LfpDisplay.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"
#include "LfpChannelColourScheme.h"
#include "LfpDefaultColourScheme.h"
#include "LfpMonochromaticColourScheme.h"
#include "LfpGradientColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpDisplay -
// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v)
    : singleChan(-1)
    , canvas(c)
    , viewport(v)
    , channelsReversed(false)
    , displaySkipAmt(0)
    , m_SpikeRasterPlottingFlag(false)
{
    perPixelPlotter = new PerPixelBitmapPlotter(this);
    supersampledPlotter = new SupersampledBitmapPlotter(this);
    
//    colorScheme = new LfpDefaultColourScheme();
    colourSchemeList.add(new LfpDefaultColourScheme(this, canvas));
    colourSchemeList.add(new LfpMonochromaticColourScheme(this, canvas));
    colourSchemeList.add(new LfpGradientColourScheme(this, canvas));
    
    activeColourScheme = 0;
    
    plotter = perPixelPlotter;
    m_MedianOffsetPlottingFlag = false;
    
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
    
//    setBufferedToImage(true); // TODO: (kelly) test

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
//    deleteAllChildren();
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
    getColourSchemePtr()->setColourGrouping(i);
    setColors(); // so that channel colors get re-assigned

}

LfpChannelColourScheme * LfpDisplay::getColourSchemePtr()
{
    return colourSchemeList[activeColourScheme];
}

void LfpDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;
    
//    deleteAllChildren();
    removeAllChildren();

    channels.clear();
    channelInfo.clear();
    drawableChannels.clear();

    totalHeight = 0;
    cachedDisplayChannelHeight = canvas->getChannelHeight();

	if (numChans > 0)
	{
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
			lfpInfo->setSubprocessorIdx(canvas->getChannelSubprocessorIdx(i));

			addAndMakeVisible(lfpInfo);

			channelInfo.add(lfpInfo);

			drawableChannels.add(LfpChannelTrack{
				lfpChan,
				lfpInfo
			});

			savedChannelState.add(true);

			totalHeight += lfpChan->getChannelHeight();

		}

	}
    
    setColors();
    
    std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;

}

void LfpDisplay::setColors()
{
    for (int i = 0; i < drawableChannels.size(); i++)
    {

//        channels[i]->setColour(channelColours[(int(i/colorGrouping)+1) % channelColours.size()]);
//        channelInfo[i]->setColour(channelColours[(int(i/colorGrouping)+1)  % channelColours.size()]);
        drawableChannels[i].channel->setColour(getColourSchemePtr()->getColourForIndex(i));
        drawableChannels[i].channelInfo->setColour(getColourSchemePtr()->getColourForIndex(i));
    }

}

void LfpDisplay::setActiveColourSchemeIdx(int index)
{
    activeColourScheme = index;
}

int LfpDisplay::getActiveColourSchemeIdx()
{
    return activeColourScheme;
}

int LfpDisplay::getNumColourSchemes()
{
    return colourSchemeList.size();
}

StringArray LfpDisplay::getColourSchemeNameArray()
{
    StringArray nameList;
    for (auto scheme : colourSchemeList)
        nameList.add(scheme->getName());
    
    return nameList;
}

int LfpDisplay::getTotalHeight()
{
    return totalHeight;
}

void LfpDisplay::resized()
{
    int totalHeight = 0;
    
    for (int i = 0; i < drawableChannels.size(); i++)
    {
        
        LfpChannelDisplay* disp = drawableChannels[i].channel;
        
        if (disp->getHidden()) continue;
        
        disp->setBounds(canvas->leftmargin,
                        totalHeight-(disp->getChannelOverlap()*canvas->channelOverlapFactor)/2,
                        getWidth(),
                        disp->getChannelHeight()+(disp->getChannelOverlap()*canvas->channelOverlapFactor));
        
        disp-> resized();
        
        LfpChannelDisplayInfo* info = drawableChannels[i].channelInfo;
        
        info->setBounds(0,
                        totalHeight-disp->getChannelHeight() + (disp->getChannelOverlap()*canvas->channelOverlapFactor)/4.0,
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
    if (lfpChannelBitmap.isNull())
    {
        resized();
    }

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
//    for (int i = 0; i < drawableChannels.size(); ++i)
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
    if (!getSingleChannelState()) cachedDisplayChannelHeight = r;
    
    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight(r);
        channelInfo[i]->setChannelHeight(r);
    }
    if (resetSingle && singleChan != -1)
    {
        //std::cout << "width " <<  getWidth() << " numchans  " << numChans << " height " << getChannelHeight() << std::endl;
        setSize(getWidth(),drawableChannels.size()*getChannelHeight());
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
    
    if (isDrawMethod)
    {
        plotter = supersampledPlotter;
    }
    else
    {
        plotter = perPixelPlotter;
    }
    
    resized();

}

int LfpDisplay::getChannelHeight()
{
//    return cachedDisplayChannelHeight;
    return drawableChannels[0].channel->getChannelHeight();
//    return channels[0]->getChannelHeight();
}

void LfpDisplay::cacheNewChannelHeight(int r)
{
    cachedDisplayChannelHeight = r;
}

bool LfpDisplay::getChannelsReversed()
{
    return channelsReversed;
}

void LfpDisplay::setChannelsReversed(bool state)
{
    if (state == channelsReversed) return; // bail early, in case bookkeeping error
    
    channelsReversed = state;
    
    if (getSingleChannelState()) return; // don't reverse if single channel
    
    // reverse channels that are currently in drawableChannels
    for (int i = 0, j = drawableChannels.size() - 1, len = drawableChannels.size()/2;
         i < len;
         i++, j--)
    {
        // remove channel and info components from front and back
        // moving toward middle
        removeChildComponent(drawableChannels[i].channel);
        removeChildComponent(drawableChannels[j].channel);
        removeChildComponent(drawableChannels[i].channelInfo);
        removeChildComponent(drawableChannels[j].channelInfo);
        
        // swap front and back, moving towards middle
        drawableChannels.swap(i, j);
        
        // also swap coords
        {
            const auto channelBoundsA = drawableChannels[i].channel->getBounds();
            const auto channelInfoBoundsA = drawableChannels[i].channelInfo->getBounds();
            
            drawableChannels[i].channel->setBounds(drawableChannels[j].channel->getBounds());
            drawableChannels[i].channelInfo->setBounds(drawableChannels[j].channelInfo->getBounds());
            drawableChannels[j].channel->setBounds(channelBoundsA);
            drawableChannels[j].channelInfo->setBounds(channelInfoBoundsA);
        }
    }
    
    // remove middle component if odd number of channels
    if (drawableChannels.size() % 2 != 0)
    {
        removeChildComponent(drawableChannels[drawableChannels.size()/2+1].channel);
        removeChildComponent(drawableChannels[drawableChannels.size()/2+1].channelInfo);
    }
    
    // add the channels and channel info again
    for (int i = 0, len = drawableChannels.size(); i < len; i++)
    {
        
        if (!drawableChannels[i].channel->getHidden())
        {
            addAndMakeVisible(drawableChannels[i].channel);
            addAndMakeVisible(drawableChannels[i].channelInfo);
        }
        
        // flag this to update the waveforms
        drawableChannels[i].channel->fullredraw = true;
    }
    
    // necessary to overwrite lfpChannelBitmap's display
    refresh();
}

int LfpDisplay::getChannelDisplaySkipAmount()
{
    return displaySkipAmt;
}

void LfpDisplay::setChannelDisplaySkipAmount(int skipAmt)
{
    displaySkipAmt = skipAmt;
    
    if (!getSingleChannelState())
        rebuildDrawableChannelsList();
    
    canvas->redraw();
}

bool LfpDisplay::getMedianOffsetPlotting()
{
    return m_MedianOffsetPlottingFlag;
}

void LfpDisplay::setMedianOffsetPlotting(bool isEnabled)
{
    m_MedianOffsetPlottingFlag = isEnabled;
}

bool LfpDisplay::getSpikeRasterPlotting()
{
    return m_SpikeRasterPlottingFlag;
}

void LfpDisplay::setSpikeRasterPlotting(bool isEnabled)
{
    m_SpikeRasterPlottingFlag = isEnabled;
}

float LfpDisplay::getSpikeRasterThreshold()
{
    return m_SpikeRasterThreshold;
}

void LfpDisplay::setSpikeRasterThreshold(float thresh)
{
    m_SpikeRasterThreshold = thresh;
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
        
        int newHeight = h+hdiff;
        
        // constrain the spread resizing to max and min values;
        if (newHeight < trackZoomInfo.minZoomHeight)
        {
            newHeight = trackZoomInfo.minZoomHeight;
            hdiff = 0;
        }
        else if (newHeight > trackZoomInfo.maxZoomHeight)
        {
            newHeight = trackZoomInfo.maxZoomHeight;
            hdiff = 0;
        }

        setChannelHeight(newHeight);
        int oldX=viewport->getViewPositionX();
        int oldY=viewport->getViewPositionY();

        setBounds(0,0,getWidth()-0, getChannelHeight()*drawableChannels.size()); // update height so that the scrollbar is correct

        int mouseY=e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
        int scrollBy = (mouseY/h)*hdiff*2;// compensate for motion of point under current mouse position
        viewport->setViewPosition(oldX,oldY+scrollBy); // set back to previous position plus offset

        options->setSpreadSelection(newHeight); // update combobox
        
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
    if (!getSingleChannelState())
    {
        
        std::cout << "Single channel on (" << chan << ")" << std::endl;
        singleChan = chan;
        
        int newHeight = viewport->getHeight();
        LfpChannelTrack lfpChannelTrack{drawableChannels[chan].channel, drawableChannels[chan].channelInfo};
        lfpChannelTrack.channelInfo->setEnabledState(true);
        lfpChannelTrack.channelInfo->setSingleChannelState(true);
        
        removeAllChildren();
        
        // disable unused channels
        for (int i = 0; i < getNumChannels(); i++)
        {
            if (i != chan)
            {
                drawableChannels[i].channel->setEnabledState(false);
            }
        }
        
        // update drawableChannels, give only the single channel to focus on
        drawableChannels.clearQuick();
        drawableChannels.add(lfpChannelTrack);
        
        addAndMakeVisible(lfpChannelTrack.channel);
        addAndMakeVisible(lfpChannelTrack.channelInfo);
        
        // set channel height and position (so that we allocate the smallest
        // necessary image size for drawing)
        setChannelHeight(newHeight, false);
        
        lfpChannelTrack.channel->setTopLeftPosition(canvas->leftmargin, 0);
        lfpChannelTrack.channelInfo->setTopLeftPosition(0, 0);
        setSize(getWidth(), getChannelHeight());
        
        viewport->setViewPosition(0, 0);

    }
//    else if (chan == singleChan || chan == -2)
    else
    {
        std::cout << "Single channel off" << std::endl;
        for (int n = 0; n < numChans; n++)
        {
            channelInfo[n]->setSingleChannelState(false);
        }
        
        setChannelHeight(cachedDisplayChannelHeight);

        reactivateChannels();
        rebuildDrawableChannelsList();
    }
}

void LfpDisplay::reactivateChannels()
{

    for (int n = 0; n < channels.size(); n++)
       setEnabledState(savedChannelState[n], n);

}

void LfpDisplay::rebuildDrawableChannelsList()
{
    
    if (displaySkipAmt != 0) removeAllChildren(); // start with clean slate
    
    Array<LfpChannelTrack> channelsToDraw;
    drawableChannels = Array<LfpDisplay::LfpChannelTrack>();
    
    // iterate over all channels and select drawable ones
    for (int i = 0, drawableChannelNum = 0; i < channels.size(); i++)
    {
//        std::cout << "\tchannel " << i << " has subprocessor index of "  << channelInfo[i]->getSubprocessorIdx() << std::endl;
        // if channel[i] is not sourced from the correct subprocessor, then hide it and continue
        //if (channelInfo[i]->getSubprocessorIdx() != getDisplayedSubprocessor())
        //{
        //    channels[i]->setHidden(true);
        //    channelInfo[i]->setHidden(true);
        //    continue;
        //}
        
		//std::cout << "Checking for hidden channels" << std::endl;
        if (displaySkipAmt == 0 || (i % displaySkipAmt == 0)) // no skips, add all channels
        {
            channels[i]->setHidden(false);
            channelInfo[i]->setHidden(false);
            
            channelInfo[i]->setDrawableChannelNumber(drawableChannelNum++);
            channelInfo[i]->resized(); // to update the conditional drawing of enableButton and channel num
            
            channelsToDraw.add(LfpDisplay::LfpChannelTrack{
                channels[i],
                channelInfo[i]
            });
            
            addAndMakeVisible(channels[i]);
            addAndMakeVisible(channelInfo[i]);
        }
        else // skip some channels
        {
//            if (i % (displaySkipAmt) == 0) // add these channels
//            {
//                channels[i]->setHidden(false);
//                channelInfo[i]->setHidden(false);
//                
//                channelsToDraw.add(LfpDisplay::LfpChannelTrack{
//                    channels[i],
//                    channelInfo[i]
//                });
//                
//                addAndMakeVisible(channels[i]);
//                addAndMakeVisible(channelInfo[i]);
//            }
//            else // but not these
//            {
                channels[i]->setHidden(true);
                channelInfo[i]->setHidden(true);
                
                removeChildComponent(channels[i]);
                removeChildComponent(channelInfo[i]);
//            }
        }
    }
    
    // check if channels should be added to drawableChannels in reverse
    if (getChannelsReversed())
    {
        for (int i = channelsToDraw.size() - 1; i >= 0; --i)
        {
            drawableChannels.add(channelsToDraw[i]);
        }
    }
    else
    {
        for (int i = 0; i < channelsToDraw.size(); ++i)
        {
            drawableChannels.add(channelsToDraw[i]);
        }
    }
    
    // this guards against an exception where the editor sets the drawable samplerate
    // before the lfpDisplay is fully initialized
    if (getHeight() > 0 && getWidth() > 0)
    {
        canvas->resizeToChannels();
    }
    
    setColors();
}

LfpBitmapPlotter * const LfpDisplay::getPlotterPtr() const
{
    return plotter;
}

bool LfpDisplay::getSingleChannelState()
{
    //if (singleChan < 0) return false;
    //else return true;
    return singleChan >= 0;
}

void LfpDisplay::mouseDown(const MouseEvent& event)
{
    if (drawableChannels.isEmpty())
    {
        return;
    }

    //int y = event.getMouseDownY(); //relative to each channel pos
    MouseEvent canvasevent = event.getEventRelativeTo(viewport);
    int y = canvasevent.getMouseDownY() + viewport->getViewPositionY(); // need to account for scrolling
    int x = canvasevent.getMouseDownX();

    int dist = 0;
    int mindist = 10000;
    int closest = 5;
    for (int n = 0; n < drawableChannels.size(); n++) // select closest instead of relying on eventComponent
    {
        drawableChannels[n].channel->deselect();

        int cpos = (drawableChannels[n].channel->getY() + (drawableChannels[n].channel->getHeight()/2));
        dist = int(abs(y - cpos));

//        std::cout << "Mouse down at " << y << " pos is "<< cpos << " n: " << n << "  dist " << dist << std::endl;

        if (dist < mindist)
        {
            mindist = dist-1;
            closest = n;
        }
    }

    drawableChannels[closest].channel->select();
    options->setSelectedType(drawableChannels[closest].channel->getType());

    if (event.mods.isRightButtonDown()) { // if right click
        PopupMenu channelMenu = channels[closest]->getOptions();
        const int result = channelMenu.show();
        drawableChannels[closest].channel->changeParameter(result);
    }
    else // if left click
    {
//    if (singleChan != -1)
        if (event.getNumberOfClicks() == 2) {
            toggleSingleChannel(closest);
        }
        
        if (getSingleChannelState())
        {
            
            //        std::cout << "singleChan = " << singleChan << " " << y << " " << drawableChannels[0].channel->getHeight() << " " << getRange() << std::endl;
            //channelInfo[singleChan]->updateXY(
            drawableChannels[0].channelInfo->updateXY(
                                                      float(x)/getWidth()*canvas->timebase,
                                                      (-(float(y)-viewport->getViewPositionY())/viewport->getViewHeight()*float(getRange()))+float(getRange()/2)
                                                      );
        }
    }

//    canvas->fullredraw = true;//issue full redraw

//    refresh();

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

