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
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpTimescale -
// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplaySplitter* c, LfpDisplay* lfpDisplay)
    : canvasSplit(c)
    , lfpDisplay(lfpDisplay)
    , offset(0.0f)
{

    font = Font("Default", 16, Font::plain);
}

LfpTimescale::~LfpTimescale()
{

}

void LfpTimescale::paint(Graphics& g)
{

    g.setFont(font);

    g.setColour(Colour(100,100,100));

    const String timeScaleUnitLabel = (timebase >= 2) ? ("s") : ("ms");

    int startIndex; 

    if (offset > 0)
        startIndex = 0;
    else
        startIndex = 1;

    for (int i = startIndex; i < labels.size(); i++)
    {

        float lineHeight;

        g.drawLine(getWidth() * fractionWidth[i],
            0,
            getWidth() * fractionWidth[i],
            getHeight(),
            2.0f);

        g.drawText(labels[i] + " " + timeScaleUnitLabel,
                   getWidth()*fractionWidth[i]+10,
                   0,
                   100,
                   getHeight(),
                   Justification::left, false);
        
    }

}

void LfpTimescale::mouseUp(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown())
    {
        lfpDisplay->trackZoomInfo.isScrollingX = false;
    }

    canvasSplit->select();
    
}

void LfpTimescale::resized()
{
    setTimebase(timebase);
}

void LfpTimescale::mouseDrag(const juce::MouseEvent &e)
{
    if (e.mods.isLeftButtonDown()) // double check that we initiate only for left click and hold
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
            if (dragDeltaX > 0)
            {
                dTimescale = 0.01 * dragDeltaX;
            }
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
    }
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

    float time = 0;
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

