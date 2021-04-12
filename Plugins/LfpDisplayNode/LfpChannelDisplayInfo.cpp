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

#include "LfpChannelDisplayInfo.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpChannelDisplayInfo -
// -------------------------------

LfpChannelDisplayInfo::LfpChannelDisplayInfo(LfpDisplaySplitter* canvas_, LfpDisplay* display_, LfpDisplayOptions* options_, int ch)
    : LfpChannelDisplay(canvas_, display_, options_, ch)
{

    chan = ch;
    x = -1.0f;
    y = -1.0f;

//    enableButton = new UtilityButton(String(ch+1), Font("Small Text", 13, Font::plain));
    enableButton = new UtilityButton("", Font("Small Text", 13, Font::plain));
    enableButton->setRadius(5.0f);

    enableButton->setEnabledState(true);
    enableButton->setCorners(true, true, true, true);
    enableButton->addListener(this);
    enableButton->setClickingTogglesState(true);
    enableButton->setToggleState(true, dontSendNotification);
    
    isSingleChannel = false;

    addAndMakeVisible(enableButton);

}

void LfpChannelDisplayInfo::updateType()
{
    type = options->getChannelType(chan);
    typeStr = options->getTypeName(type);
    repaint();
}

void LfpChannelDisplayInfo::buttonClicked(Button* button)
{

    bool state = button->getToggleState();

    display->setEnabledState(state, chan, true);

    //UtilityButton* b = (UtilityButton*) button;

    // if (state)
    // {
    //  b->setLabel("ON");
    // } else {
    //  b->setLabel("OFF");
    // }

    //std::cout << "Turn channel " << chan << " to " << button->getToggleState() << std::endl;

}

void LfpChannelDisplayInfo::setEnabledState(bool state)
{
    enableButton->setToggleState(state, dontSendNotification);
}

void LfpChannelDisplayInfo::setSingleChannelState(bool state)
{
    isSingleChannel = state;
}

int LfpChannelDisplayInfo::getChannelSampleRate()
{
    return samplerate;
}

void LfpChannelDisplayInfo::setChannelSampleRate(int samplerate_)
{
    samplerate = samplerate_;
}

void LfpChannelDisplayInfo::mouseDrag(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown()) // double check that we initiate only for left click and hold
    {
        if (e.mods.isCommandDown() && !display->getSingleChannelState())  // CTRL + drag -> change channel spacing
        {
            
            // init state in our track zooming info struct
            if (!display->trackZoomInfo.isScrollingY)
            {
                auto & zoomInfo = display->trackZoomInfo;
                
                zoomInfo.isScrollingY = true;
                zoomInfo.componentStartHeight = getChannelHeight();
                zoomInfo.zoomPivotRatioY = (getY() + e.getMouseDownY())/(float)display->getHeight();
                zoomInfo.zoomPivotRatioX = (getX() + e.getMouseDownX())/(float)display->getWidth();
                zoomInfo.zoomPivotViewportOffset = getPosition() + e.getMouseDownPosition() - canvasSplit->viewport->getViewPosition();
                
                zoomInfo.unpauseOnScrollEnd = !display->isPaused;
                if (!display->isPaused) display->options->togglePauseButton(true);
            }
            
            int h = display->trackZoomInfo.componentStartHeight;
            int hdiff=0;
            int dragDeltaY = -0.1 * (e.getScreenPosition().getY() - e.getMouseDownScreenY()); // invert so drag up -> scale up
            
//             std::cout << dragDeltaY << std::endl;
            if (dragDeltaY > 0)
            {
                hdiff = 2 * dragDeltaY;
            }
            else
            {
                if (h > 5)
                    hdiff = 2 * dragDeltaY;
            }
            
            if (abs(h) > 100) // accelerate scrolling for large ranges
                hdiff *= 3;
            
            int newHeight = h+hdiff;
            
            // constrain the spread resizing to max and min values;
            if (newHeight < display->trackZoomInfo.minZoomHeight)
            {
                newHeight = display->trackZoomInfo.minZoomHeight;
            }
            else if (newHeight > display->trackZoomInfo.maxZoomHeight)
            {
                newHeight = display->trackZoomInfo.maxZoomHeight;
            }
            
            // return early if there is nothing to update
            if (newHeight == getChannelHeight())
            {
                return;
            }
            
            // set channel heights for all channel
//            display->setChannelHeight(newHeight);
            for (int i = 0; i < display->getNumChannels(); ++i)
            {
                display->channels[i]->setChannelHeight(newHeight);
                display->channelInfo[i]->setChannelHeight(newHeight);
            }
            
            options->setSpreadSelection(newHeight, false, true); // update combobox
            
            canvasSplit->fullredraw = true;//issue full redraw - scrolling without modifier doesnt require a full redraw
            
            display->setBounds(0,0,display->getWidth()-0, display->getChannelHeight()*display->drawableChannels.size()); // update height so that the scrollbar is correct
            
            int newViewportY = display->trackZoomInfo.zoomPivotRatioY * display->getHeight() - display->trackZoomInfo.zoomPivotViewportOffset.getY();
            if (newViewportY < 0) newViewportY = 0; // make sure we don't adjust beyond the edge of the actual view
            
            canvasSplit->viewport->setViewPosition(0, newViewportY);
        }
    }
}

void LfpChannelDisplayInfo::mouseUp(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown() && display->trackZoomInfo.isScrollingY)
    {
        display->trackZoomInfo.isScrollingY = false;
        if (display->trackZoomInfo.unpauseOnScrollEnd)
        {
            display->isPaused = false;
            display->options->togglePauseButton(false);
        }
    }
}

void LfpChannelDisplayInfo::paint(Graphics& g)
{
    int center = getHeight()/2 - (isSingleChannel?(75):(0));
	const bool showChannelNumbers = options->getChannelNameState();

    // Draw the channel numbers
    g.setColour(Colours::grey);
    const String channelString = (isChannelNumberHidden() ? ("--") :
		showChannelNumbers ? String(getChannelNumber() + 1) : getName());
    bool isCentered = !getEnabledButtonVisibility();
    
    g.drawText(channelString,
               showChannelNumbers ? 6 : 2,
               center-4,
               getWidth(),
               10,
               isCentered ? Justification::centred : Justification::centredLeft,
               false);
    
    g.setColour(lineColour);
    g.fillRect(0, 0, 2, getHeight());
    
    if (getChannelTypeStringVisibility())
    {
        g.setFont(Font("Small Text", 13, Font::plain));
        g.drawText(typeStr,5,center+10,41,10,Justification::centred,false);
    }
    // g.setFont(channelHeightFloat*0.3);
    g.setFont(Font("Small Text", 11, Font::plain));

    if (isSingleChannel)
    {
        g.setColour(Colours::darkgrey);
        g.drawText("STD:", 5, center+90,41,10,Justification::centred,false);
        g.drawText("MEAN:", 5, center+40,41,10,Justification::centred,false);
        
        if (x > 0)
        {
            g.drawText("uV:", 5, center+140,41,10,Justification::centred,false);
        }
        //g.drawText("Y:", 5, center+200,41,10,Justification::centred,false);

        g.setColour(Colours::grey);
        g.drawText(String(canvasSplit->getStd(chan)), 5, center+110,41,10,Justification::centred,false);
        g.drawText(String(canvasSplit->getMean(chan)), 5, center+60,41,10,Justification::centred,false);
        if (x > 0)
        {
            //g.drawText(String(x), 5, center+150,41,10,Justification::centred,false);
            g.drawText(String(y), 5, center+160,41,10,Justification::centred,false);
        }
        
    }

    //  g.drawText(name, 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);

}

void LfpChannelDisplayInfo::updateXY(float x_, float y_)
{
    x = x_;
    y = y_;
}

void LfpChannelDisplayInfo::resized()
{

   // std::cout << "Resizing info" << std::endl;

    int center = getHeight()/2 - (isSingleChannel?(75):(0));
    setEnabledButtonVisibility(getHeight() >= 16);
    
    if (getEnabledButtonVisibility())
    {
        enableButton->setBounds(getWidth() - 13, center - 5, 10, 10);
    }
    
    setChannelNumberIsHidden(getHeight() < 16 && (getDrawableChannelNumber() + 1) % 10 != 0);
    
    setChannelTypeStringVisibility(getHeight() > 34);
}

void LfpChannelDisplayInfo::setEnabledButtonVisibility(bool shouldBeVisible)
{
    if (shouldBeVisible)
    {
        addAndMakeVisible(enableButton);
    }
    else if (enableButton->isVisible())
    {
        removeChildComponent(enableButton);
        enableButton->setVisible(false);
    }
    
}

bool LfpChannelDisplayInfo::getEnabledButtonVisibility()
{
    return enableButton->isVisible();
}

void LfpChannelDisplayInfo::setChannelTypeStringVisibility(bool shouldBeVisible)
{
    channelTypeStringIsVisible = shouldBeVisible;
}

bool LfpChannelDisplayInfo::getChannelTypeStringVisibility()
{
    return channelTypeStringIsVisible || isSingleChannel;
}

void LfpChannelDisplayInfo::setChannelNumberIsHidden(bool shouldBeHidden)
{
    channelNumberHidden = shouldBeHidden;
}

bool LfpChannelDisplayInfo::isChannelNumberHidden()
{
    return channelNumberHidden;
}

