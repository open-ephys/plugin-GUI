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

#include "LfpTimescale.h"

#include "LfpDisplayCanvas.h"
#include "LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpTimescale -
// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplaySplitter* c, LfpDisplay* lfpDisplay)
    : canvasSplit(c)
    , lfpDisplay(lfpDisplay)
    , offset(0.0f), isPaused(false)
{

    font = Font("Default", 16, Font::plain);
}


void LfpTimescale::paint(Graphics& g)
{

    //std::cout << "Repainting timescale with offset of " << timeOffset << std::endl;

    g.setFont(font);

    if (isPaused)
    {
        if (canvasSplit->getSelectedState())
            g.setColour(Colour(25, 25, 25));
        else
            g.setColour(Colour(200, 200, 200));
            
    }
    else
        g.setColour(Colour(100, 100, 100));

    const String timeScaleUnitLabel = (timebase >= 2) ? ("s") : ("ms");

    int startIndex; 

    if (offset > 0)
        startIndex = 0;
    else
        startIndex = 1;

    const int timescaleHeight = 30;

    for (int i = startIndex; i < labels.size(); i++)
    {

        float xLoc = getWidth() * fractionWidth[i] + float(timeOffset);

        if (xLoc > 0)
        {
            float lineHeight;

            g.drawLine(xLoc,
                0,
                xLoc,
                timescaleHeight,
                2.0f);

            g.drawText(labels[i] + " " + timeScaleUnitLabel,
                xLoc + 10,
                0,
                100,
                timescaleHeight,
                Justification::left, false);
        }
       
    }

}

void LfpTimescale::mouseUp(const MouseEvent &e)
{
    //if (e.mods.isLeftButtonDown())
    //{
    //    lfpDisplay->trackZoomInfo.isScrollingX = false;
    //}

    
    
}

void LfpTimescale::setPausedState(bool isPaused_)
{
    if (!isPaused_)
    {
        lfpDisplay->pause(false);
        timeOffset = 0;
        isPaused = false;
        stopTimer();
    }
    else {
        lfpDisplay->pause(true);
        isPaused = true;
        startTimer(50);
    }

    currentTimeOffset = timeOffset;

    repaint();
}

void LfpTimescale::resized()
{
    setTimebase(timebase);
}

void LfpTimescale::mouseDown(const juce::MouseEvent& e)
{

    canvasSplit->select();
    
    // TODO: only allow pausing while acquisition is active 
    if (e.getNumberOfClicks() == 2)
    {
        setPausedState(false);
    }
    else
    {
        setPausedState(true);
    }
		
    

}

void LfpTimescale::timerCallback()
{
	if (isPaused && timeOffsetChanged)
	{
        lfpDisplay->setTimeOffset(timeOffset);

        repaint();

        timeOffsetChanged = false;
	}
}

void LfpTimescale::mouseDrag(const juce::MouseEvent &e)
{
    
    if (canvasSplit->isInTriggeredMode())
        return;
    
    int dragDeltaX = (e.getScreenPosition().getX() - e.getMouseDownScreenX()); // invert so drag up -> scale up

    timeOffset = currentTimeOffset + int(dragDeltaX);

    if (timeOffset < 0)
        timeOffset = 0;

    if (timeOffset > getWidth() * 3)
        timeOffset = getWidth() * 3;

    if (currentTimeOffset != timeOffset)
        timeOffsetChanged = true;


    /*if (e.mods.isLeftButtonDown()) // double check that we initiate only for left click and hold
    {
        if (e.mods.isCommandDown())  // CTRL + drag -> change channel spacing
        {
            // init state in our track zooming info struct
            if (!lfpDisplay->trackZoomInfo.isScrollingX)
            {
                lfpDisplay->trackZoomInfo.isScrollingX = true;
                lfpDisplay->trackZoomInfo.timescaleStartScale = timebase;
            }

            float timescale = lfpDisplay->trackZoomInfo.timescaleStartScale;
            float dTimescale=0;
            int dragDeltaX = (e.getScreenPosition().getX() - e.getMouseDownScreenX()); // invert so drag up -> scale up

//            std::cout << dragDeltaX << std::endl;
            
            else
            {
                // TODO: (kelly) change this to scale appropriately for -dragDeltaX
                if (timescale > 0.25)
                    dTimescale = 0.01 * dragDeltaX;
            }
            
            if (timescale >= 1) // accelerate scrolling for large ranges
                dTimescale *= 4;
            
            if (timescale >= 5)
                dTimescale *= 4;
            
            if (timescale >= 10)
                dTimescale *= 4;
            
            // round dTimescale to the nearest 0.005 sec
            dTimescale = ((dTimescale + (0.005/2)) / 0.005) * 0.005;
            
            float newTimescale = timescale+dTimescale;
            
            if (newTimescale < 0.25) newTimescale = 0.250;
            if (newTimescale > 20) newTimescale = 20;
            
            // don't bother updating if the new timebase is the same as the old (if clipped, for example)
            if (timescale != newTimescale)
            {
                lfpDisplay->options->setTimebaseAndSelectionText(newTimescale);
                setTimebase(canvasSplit->timebase);
            }
        }
    }*/
}

void LfpTimescale::setTimebase(float timebase_, float offset_)
{
    timebase = timebase_;
    offset = offset_;

    labels.clear();
    isMajor.clear();
    fractionWidth.clear();

    float stepSize;

    if (timebase <= 0.01)
        stepSize = 0.002;
    else if (timebase > 0.01 && timebase <= 0.025)
        stepSize = 0.005;
    else if (timebase > 0.025 && timebase <= 0.1)
        stepSize = 0.01;
    else if (timebase > 0.1 && timebase <= 0.25)
        stepSize = 0.025;
    else if (timebase > 0.25 && timebase <= 0.5)
        stepSize = 0.1;
    else if (timebase > 0.5 && timebase < 2)
        stepSize = 0.25;
    else if (timebase >= 2 && timebase < 5)
        stepSize = 0.5;
    else if (timebase >= 5 && timebase < 15)
        stepSize = 1.0;
    else
        stepSize = 2.0;

    float time = -timebase * 4;
    int index = 0;

    while ((time + offset) < timebase)
    {
        String labelString = String(time * ((timebase >= 2) ? (1) : (1000.0f)));
        labels.add(labelString.substring(0, 6));

        fractionWidth.add((time + offset) / timebase);
        
        if (index % 2 == 0)
            isMajor.add(true);
        else
            isMajor.add(false);

        time += stepSize;
        index++;
    }

    repaint();

}

