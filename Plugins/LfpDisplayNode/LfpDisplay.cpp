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

#include "ColourSchemes/DefaultColourScheme.h"
#include "ColourSchemes/MonochromeGrayColourScheme.h"
#include "ColourSchemes/MonochromeYellowColourScheme.h"
#include "ColourSchemes/MonochromePurpleColourScheme.h"
#include "ColourSchemes/MonochromeGreenColourScheme.h"
#include "ColourSchemes/OELogoColourScheme.h"
#include "ColourSchemes/TropicalColourScheme.h"


#include <math.h>
#include <numeric>

using namespace LfpViewer;

#pragma  mark - LfpDisplay -
// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplaySplitter* c, Viewport* v)
    : singleChan(-1)
    , canvasSplit(c)
    , viewport(v)
    , channelsReversed(false)
    , channelsOrderedByDepth(false)
    , displaySkipAmt(0)
    , m_SpikeRasterPlottingFlag(false)
{
    perPixelPlotter = new PerPixelBitmapPlotter(this);
    supersampledPlotter = new SupersampledBitmapPlotter(this);
    
    colourSchemeList.add(new DefaultColourScheme(this, canvasSplit));
    colourSchemeList.add(new MonochromeGrayColourScheme(this, canvasSplit));
    colourSchemeList.add(new MonochromeYellowColourScheme(this, canvasSplit));
    colourSchemeList.add(new MonochromePurpleColourScheme(this, canvasSplit));
    colourSchemeList.add(new MonochromeGreenColourScheme(this, canvasSplit));
    colourSchemeList.add(new OELogoColourScheme(this, canvasSplit));
    colourSchemeList.add(new TropicalColourScheme(this, canvasSplit));
    
    plotter = perPixelPlotter;
    m_MedianOffsetPlottingFlag = false;
    
    activeColourScheme = 0;
    totalHeight = 0;
    colorGrouping = 1;

    //hand-built palette (used for event channels)
    channelColours.add(Colour(224, 185, 36));
    channelColours.add(Colour(214, 210, 182));
    channelColours.add(Colour(243, 119, 33));
    channelColours.add(Colour(186, 157, 168));
    channelColours.add(Colour(237, 37, 36));
    channelColours.add(Colour(179, 122, 79));
    channelColours.add(Colour(217, 46, 171));
    channelColours.add(Colour(217, 139, 196));
    channelColours.add(Colour(101, 31, 255));
    channelColours.add(Colour(141, 111, 181));
    channelColours.add(Colour(48, 117, 255));
    channelColours.add(Colour(184, 198, 224));
    channelColours.add(Colour(116, 227, 156));
    channelColours.add(Colour(150, 158, 155));
    channelColours.add(Colour(82, 173, 0));
    channelColours.add(Colour(125, 99, 32));

    range[0] = 1000; // headstage channels
    range[1] = 500;  // aux channels
    range[2] = 500000; // adc channels

    scrollX = 0;
    scrollY = 0;

    addMouseListener(this, true);

    for (int i = 0; i < 8; i++)
    {
        eventDisplayEnabled[i] = true;
    }

    isPaused = false;

    savedChannelState.insertMultiple(0, true, 10000); // max 10k channels

    numChans = 0;


}

LfpDisplay::~LfpDisplay()
{

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

ChannelColourScheme * LfpDisplay::getColourSchemePtr()
{
    return colourSchemeList[activeColourScheme];
}

void LfpDisplay::setNumChannels(int newChannelCount)
{

   // std::cout << "setting num channels to " << newChannelCount << std::endl;
    
    if (numChans > newChannelCount)
    {

       // std::cout << "   removing " << numChans - newChannelCount << " extra channels." << std::endl;

        for (int i = newChannelCount; i < numChans; i++)
        {
            removeChildComponent(channels[i]);
            removeChildComponent(channelInfo[i]);

        }

        channels.removeLast(numChans - newChannelCount);
        channelInfo.removeLast(numChans - newChannelCount);
    }

    totalHeight = 0;

    cachedDisplayChannelHeight = canvasSplit->getChannelHeight();

	if (newChannelCount > 0)
	{
		for (int i = 0; i < newChannelCount; i++)
		{

            LfpChannelDisplay* lfpChan;
            LfpChannelDisplayInfo* lfpInfo;

            if (i >= numChans)
            {
               // std::cout << "ADDING NEW CHANNEL " << i << std::endl;

                lfpChan = new LfpChannelDisplay(canvasSplit, this, options, i);
                channels.add(lfpChan);

                lfpInfo = new LfpChannelDisplayInfo(canvasSplit, this, options, i);
                channelInfo.add(lfpInfo);
            }
            else {
                
                lfpChan = channels[i];
                lfpInfo = channelInfo[i];
            }
            
			lfpChan->setRange(range[options->getChannelType(i)]);
			lfpChan->setChannelHeight(canvasSplit->getChannelHeight());

			lfpInfo->setRange(range[options->getChannelType(i)]);
			lfpInfo->setChannelHeight(canvasSplit->getChannelHeight());
			lfpInfo->setSubprocessorIdx(canvasSplit->getChannelSubprocessorIdx(i));

            if (!getSingleChannelState())
            {
                lfpChan->setEnabledState(savedChannelState[i]);
                lfpInfo->setEnabledState(savedChannelState[i]);
            }
                

			totalHeight += lfpChan->getChannelHeight();

		}
	}

    numChans = newChannelCount;

   // std::cout << "Done setting channels." << std::endl;

}

void LfpDisplay::setColors()
{

    if (!getSingleChannelState())
    {
        for (int i = 0; i < drawableChannels.size(); i++)
        {
            drawableChannels[i].channel->setColour(getColourSchemePtr()->getColourForIndex(i));
            drawableChannels[i].channelInfo->setColour(getColourSchemePtr()->getColourForIndex(i));
        }
    }
    else {
        drawableChannels[0].channel->setColour(getColourSchemePtr()->getColourForIndex(singleChan));
        drawableChannels[0].channelInfo->setColour(getColourSchemePtr()->getColourForIndex(singleChan));
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

void LfpDisplay::restoreViewPosition()
{
    if (!getSingleChannelState())
      viewport->setViewPosition(scrollX, scrollY);
}

void LfpDisplay::resized()
{
    int totalHeight = 0;

   // std::cout << "LFP display width: " << getWidth() << std::endl;


    //std::cout << "Resizing channels" << std::endl;
    
    for (int i = 0; i < drawableChannels.size(); i++)
    {
        
        LfpChannelDisplay* disp = drawableChannels[i].channel;
        
        if (disp->getHidden()) continue;
        
        disp->setBounds(canvasSplit->leftmargin,
                        totalHeight-(disp->getChannelOverlap()*canvasSplit->channelOverlapFactor)/2,
                        getWidth()- canvasSplit->leftmargin,
                        disp->getChannelHeight()+(disp->getChannelOverlap()*canvasSplit->channelOverlapFactor));
        
       // disp-> resized();
        
        LfpChannelDisplayInfo* info = drawableChannels[i].channelInfo;

       // std::cout << info->getDepth() << std::endl;
        
        info->setBounds(2,
                        totalHeight-disp->getChannelHeight() + (disp->getChannelOverlap()*canvasSplit->channelOverlapFactor)/4.0,
                        canvasSplit->leftmargin -2,
                        disp->getChannelHeight());
        
        totalHeight += disp->getChannelHeight();
        
    }

    canvasSplit->fullredraw = true; //issue full redraw
    
    if (!getSingleChannelState())
    {
        viewport->setViewPosition(scrollX, scrollY);
        //std::cout << "Setting view position to " << scrollY << std::endl;
    }
    else {
        //std::cout << "Setting view position for single channel " << std::endl;
        viewport->setViewPosition(0, 0);
    }
       
    if (getWidth() > 0 && getHeight() > 0)
        lfpChannelBitmap = Image(Image::ARGB, getWidth() - canvasSplit->leftmargin, getHeight(), false);
    else
        lfpChannelBitmap = Image(Image::ARGB, 10, 10, false);
    
  //  std::cout << "Bitmap width: " << lfpChannelBitmap.getWidth() << std::endl;


    //inititalize background
    Graphics gLfpChannelBitmap(lfpChannelBitmap);
    gLfpChannelBitmap.setColour(getColourSchemePtr()->getBackgroundColour()); //background color
    gLfpChannelBitmap.fillRect(0,0, lfpChannelBitmap.getWidth(), lfpChannelBitmap.getHeight());

    canvasSplit->fullredraw = true;
    
    refresh();
    
    //std::cout << "Total height: " << totalHeight << std::endl;

}

void LfpDisplay::paint(Graphics& g)
{
    
    g.drawImageAt(lfpChannelBitmap, canvasSplit->leftmargin, 0);
    
}

void LfpDisplay::refresh()
{

    if (numChans == 0)
        return;

    // Ensure the lfpChannelBitmap has been initialized
    if (lfpChannelBitmap.isNull() || lfpChannelBitmap.getWidth() < getWidth() - canvasSplit->leftmargin)
    {
        resized();
    }

    // X-bounds of this update
    int fillfrom = canvasSplit->lastScreenBufferIndex[0];
    int fillto = (canvasSplit->screenBufferIndex[0]);
    
    ///if (fillfrom<0){fillfrom=0;};
    //if (fillto>lfpChannelBitmap.getWidth()){fillto=lfpChannelBitmap.getWidth();};
    
    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    // clear appropriate section of the bitmap --
    // we need to do this before each channel draws its new section of data into lfpChannelBitmap
    Graphics gLfpChannelBitmap(lfpChannelBitmap);
    gLfpChannelBitmap.setColour(getColourSchemePtr()->getBackgroundColour()); //background color

    if (canvasSplit->fullredraw)
    {
        gLfpChannelBitmap.fillRect(0, 0, lfpChannelBitmap.getWidth(), lfpChannelBitmap.getHeight());
        
    }
    else {

        if (fillfrom < fillto)
        {
            gLfpChannelBitmap.fillRect(fillfrom, 0, (fillto - fillfrom) + 2, lfpChannelBitmap.getHeight()); // just clear one section
            //std::cout << "Clearing " << fillfrom << " to " << fillto << std::endl;
        }
        else if (fillfrom > fillto) {

            gLfpChannelBitmap.fillRect(fillfrom, 0, lfpChannelBitmap.getWidth() - fillfrom + 2, lfpChannelBitmap.getHeight()); // first segment
            gLfpChannelBitmap.fillRect(0, 0, fillto + 2, lfpChannelBitmap.getHeight()); // second segment

            //std::cout << "Clearing " << fillfrom << " to " << lfpChannelBitmap.getWidth() << std::endl;
            //std::cout << "Clearing " << 0 << " to " << fillto << std::endl;
        }
        else {
            ; // no change, do nothing
        }
        
    }


    
    for (int i = 0; i < numChans; i++)
    {

        int componentTop = channels[i]->getY();
        int componentBottom = channels[i]->getHeight() + componentTop;

        if ((topBorder <= componentBottom && bottomBorder >= componentTop)) // only draw things that are visible
        {
            if (canvasSplit->fullredraw)
            {
                channels[i]->fullredraw = true;
                channels[i]->pxPaint();
                channelInfo[i]->repaint();
            }
            else
            {
                

                 channels[i]->pxPaint(); // draws to lfpChannelBitmap
                
                 // it's not clear why, but apparently because the pxPaint() is in a child component of LfpDisplay, 
                 // we also need to issue repaint() calls for each channel, even though there's nothing 
                 // to repaint there. Otherwise, the repaint call in LfpDisplay::refresh(), a few lines down, 
                 // lags behind the update line by ~60 px. This could have something to do with the repaint 
                 // message passing in juce. In any case, this seemingly redundant repaint here seems to fix the issue.
                
                 // we redraw from 0 to +2 (px) relative to the real redraw window, the +1 draws the vertical update line
                 if (fillfrom < fillto)
                 {
                     channels[i]->repaint(fillfrom, 0, (fillto - fillfrom) + 1, channels[i]->getHeight());
                 }
                    
                 else
                 {
                     channels[i]->repaint(fillfrom, 0, lfpChannelBitmap.getWidth() - fillfrom + 1, channels[i]->getHeight());
                     channels[i]->repaint(0, 0, fillto + 1, channels[i]->getHeight());
                 }
                
            }
            //std::cout << i << std::endl;
        }

    }

    if (fillfrom == 0 && singleChan != -1)
    {
        channelInfo[singleChan]->repaint();
    }
    
    if (canvasSplit->fullredraw)
    {
        repaint(0,topBorder,getWidth(),bottomBorder-topBorder);
    } else {
        //repaint(fillfrom, topBorder, (fillto-fillfrom)+1, bottomBorder-topBorder); // doesn't seem to be needed and results in duplicate repaint calls
    }
    
    canvasSplit->fullredraw = false;

    //std::cout << "REFRESH" << std::endl;
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
        canvasSplit->fullredraw = true; //issue full redraw
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


    /*if (resetSingle && singleChan != -1)
    {
        
        setSize(getWidth(), getChannelHeight());
        viewport->setScrollBarsShown(true,false);
        viewport->setViewPosition(juce::Point<int>(0,singleChan*r));
        singleChan = -1;
        for (int n = 0; n < numChans; n++)
        {
			channelInfo[n]->setEnabledState(savedChannelState[n]);
        }
    }
    else*/
    if (singleChan == -1) {

        int overallHeight = drawableChannels.size() * getChannelHeight();
        
        setSize(getWidth(), overallHeight);
        //std::cout << "LFP DISPLAY width: " << getWidth() << "; height: " << overallHeight << std::endl;

    }

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

    if (drawableChannels.size() > 0)
        return drawableChannels[0].channel->getChannelHeight();
    else
        return 0;
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

void LfpDisplay::orderChannelsByDepth(bool state)
{
    if (state == channelsOrderedByDepth) return; // bail early, in case bookkeeping error

    channelsOrderedByDepth = state;

    if (getSingleChannelState()) return; // don't reverse if single channel

    const int numChannels = drawableChannels.size();

    std::vector<float> depths(numChannels);

    bool allSame = true;
    float last = drawableChannels[0].channelInfo->getDepth();

    if (channelsOrderedByDepth)
    {

        for (int i = 0; i < drawableChannels.size(); i++)
        {
            float d = drawableChannels[i].channelInfo->getDepth();

            if (d != last)
                allSame = false;

            //std::cout << d << std::endl;

            depths[i] = d;

            last = d;
        }

        if (allSame)
            return;
            
    }
    else {
        for (int i = 0; i < drawableChannels.size(); i++)
        {
            int ch = drawableChannels[i].channel->getChannelNumber();
            depths[i] = float(ch);

            //std::cout << ch << std::endl;
        }
            
    }
    
    std::vector<int> V(numChannels);

    std::iota(V.begin(), V.end(), 0); //Initializing
    sort(V.begin(), V.end(), [&](int i, int j) {return depths[i] <= depths[j]; });

    // reverse channels that are currently in drawableChannels
   // std::cout << "New order: " << std::endl;
    juce::Array<LfpChannelTrack> orderedDrawableChannels;
    
    for (int i = 0; i < drawableChannels.size(); i++)
    {
        //std::cout << V[i] << std::endl;
        // re-order by depth
        removeChildComponent(drawableChannels[V[i]].channel);
        // drawableChannels.move(V[i]-i, drawableChannels.size()); // not working
        orderedDrawableChannels.add(drawableChannels[V[i]]);
    }
    
    drawableChannels = orderedDrawableChannels;

    for (int i = 0; i < drawableChannels.size(); i++)
    {
        if (!drawableChannels[i].channel->getHidden())
        {
            addAndMakeVisible(drawableChannels[i].channel);
        }
        drawableChannels[i].channel->fullredraw = true;
    }

    // necessary to overwrite lfpChannelBitmap's display
    resized();
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
    
    canvasSplit->redraw();
}

void LfpDisplay::setScrollPosition(int x, int y)
{
    scrollX = x;
    scrollY = y;
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

    
    

   // std::cout << "Y: " << scrollY << std::endl;
    
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
        
        canvasSplit->fullredraw = true;//issue full redraw - scrolling without modifier doesnt require a full redraw
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
            canvasSplit->fullredraw = true; //issue full redraw - scrolling without modifier doesnt require a full redraw
            
        }
        else    // just scroll
        {
            //  passes the event up to the viewport so the screen scrolls
            if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
            {
                viewport->mouseWheelMove(e.getEventRelativeTo(canvasSplit), wheel);

                //canvasSplit->syncDisplayBuffer();
            }
                

        }
      
    }

    if (!getSingleChannelState())
    {
        scrollX = viewport->getViewPositionX();
        scrollY = viewport->getViewPositionY();
    }
    
   // refresh(); // doesn't seem to be needed now that channels draw to bitmap

}

void LfpDisplay::toggleSingleChannel(LfpChannelTrack drawableChannel)
{
    if (!getSingleChannelState())
    {
        singleChan = drawableChannel.channel->getChannelNumber();

        rebuildDrawableChannelsList();

    }
    else
    {
        std::cout << "Single channel off" << std::endl;

        drawableChannels[0].channelInfo->setSingleChannelState(false);

        singleChan = -1;
        
        setChannelHeight(cachedDisplayChannelHeight);

        reactivateChannels();
        rebuildDrawableChannelsList();
    }
}

void LfpDisplay::reactivateChannels()
{

    for (int n = 0; n < channels.size(); n++)
       setEnabledState(savedChannelState[n], n, true);

}

void LfpDisplay::rebuildDrawableChannelsList()
{

    if (getSingleChannelState())
    {
        //std::cout << "Single channel on (" << singleChan << ")" << std::endl;

        int newHeight = viewport->getHeight();

        int channelIndex = -1;

        for (int i = 0; i < channels.size(); i++)
        {
            if (channels[i]->getChannelNumber() == singleChan)
            {
                channelIndex = i;
                break;
            }
        }
            
        if (channelIndex > -1)
        {

            if (drawableChannels.size() != 1) // if we haven't already gone through this ordeal
            {
                LfpChannelTrack lfpChannelTrack{ channels[channelIndex], channelInfo[channelIndex] };

                removeAllChildren();

                // disable unused channels
                for (int i = 0; i < drawableChannels.size(); i++)
                {
                    if (drawableChannels[i].channel != lfpChannelTrack.channel)
                        drawableChannels[i].channel->setEnabledState(false);
                }

                // update drawableChannels, give only the single channel to focus on
                drawableChannels = Array<LfpDisplay::LfpChannelTrack>();
                drawableChannels.add(lfpChannelTrack);

                lfpChannelTrack.channel->setEnabledState(true);
                lfpChannelTrack.channelInfo->setEnabledState(true);
                lfpChannelTrack.channelInfo->setSingleChannelState(true);

                addAndMakeVisible(lfpChannelTrack.channel);
                addAndMakeVisible(lfpChannelTrack.channelInfo);

                // set channel height and position (so that we allocate the smallest
                // necessary image size for drawing)
                setChannelHeight(newHeight, false);

                lfpChannelTrack.channel->setTopLeftPosition(canvasSplit->leftmargin, 0);
                lfpChannelTrack.channelInfo->setTopLeftPosition(0, 0);
                setSize(getWidth(), getChannelHeight());

                viewport->setViewPosition(0, 0);

                setColors();

                // this guards against an exception where the editor sets the drawable samplerate
                // before the lfpDisplay is fully initialized
                if (getHeight() > 0 && getWidth() > 0)
                {
                    canvasSplit->resizeToChannels();
                }

            }
           
            //std::cout << "Finished single channel rebuild" << std::endl;

            return;
        }
        else {

            reactivateChannels();

            singleChan = -1; // our channel no longer exists
        }

        //std::cout << "Single channel not found." << std::endl;

    }

   // std::cout << "Standard channel rebuild" << std::endl;

    removeAllChildren(); // start with clean slate
    
    Array<LfpChannelTrack> channelsToDraw;
    drawableChannels = Array<LfpDisplay::LfpChannelTrack>();
    
    // iterate over all channels and select drawable ones
    for (int i = 0, drawableChannelNum = 0; i < channels.size(); i++)
    {
        
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
            channels[i]->setHidden(true);
            channelInfo[i]->setHidden(true);
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
    else if(channelsOrderedByDepth)
    {
        std::vector<float> depths(channelsToDraw.size());

        bool allSame = true;
        float last = channelsToDraw[0].channel->getDepth();

        for (int i = 0; i < channelsToDraw.size(); i++)
        {
            float d = channelsToDraw[i].channelInfo->getDepth();

            if (d != last)
                allSame = false;

            depths[i] = d;
            last = d;
        }

        if (!allSame)
        {
            std::vector<int> V(channelsToDraw.size());

            std::iota(V.begin(), V.end(), 0); //Initializing
            sort(V.begin(), V.end(), [&](int i, int j) {return depths[i] <= depths[j]; });
            
            for (int i = 0; i < channelsToDraw.size(); i++)
            {
                drawableChannels.add(channelsToDraw[V[i]]);
            }
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
        canvasSplit->resizeToChannels();
    }
    
    setColors();

   // std::cout << "Finished standard channel rebuild" << std::endl;
}

LfpBitmapPlotter * const LfpDisplay::getPlotterPtr() const
{
    return plotter;
}

bool LfpDisplay::getSingleChannelState()
{
    return singleChan >= 0;
}

int LfpDisplay::getSingleChannelShown()
{
    return singleChan;
}

void LfpDisplay::setSingleChannelView(int chan)
{
    singleChan = chan;
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

       // std::cout << "Mouse down at " << y << " pos is "<< cpos << " n: " << n << "  dist " << dist << std::endl;

        if (dist < mindist)
        {
            mindist = dist-1;
            closest = n;
        }
    }

    //std::cout << "Closest channel" << closest << std::endl;

    drawableChannels[closest].channel->select();
    options->setSelectedType(drawableChannels[closest].channel->getType());

    if (event.mods.isRightButtonDown()) { // if right click
        PopupMenu channelMenu = channels[closest]->getOptions();
        const int result = channelMenu.show();
        drawableChannels[closest].channel->changeParameter(result);
    }
    else // if left click
    {
        if (event.getNumberOfClicks() == 2) {
            toggleSingleChannel(drawableChannels[closest]);
        }
        
        if (getSingleChannelState()) // show info for point that was selected
        {
            drawableChannels[0].channelInfo->updateXY(
                                                      float(x)/getWidth()*canvasSplit->timebase,
                                                      (-(float(y)-viewport->getViewPositionY())/viewport->getViewHeight()*float(getRange()))+float(getRange()/2)
                                                      );
        }
    }

    canvasSplit->select();

    canvasSplit->fullredraw = true; //issue full redraw

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

       // if (!state)
        //    std::cout << "UPDATING CANVAS VALUE FOR CHANNEL " << chan <<  std::endl;

        //canvasSplit->isChannelEnabled.set(chan, state);

       // if (!state)
        //    std::cout << "CANVAS VALUE: " << canvasSplit->isChannelEnabled[chan] << std::endl;
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

