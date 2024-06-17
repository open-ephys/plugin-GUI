/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "LfpDisplay.h"
#include "LfpDisplayCanvas.h"

#include <math.h>

using namespace LfpViewer;

#pragma mark - LfpTimescale -
// -------------------------------------------------------------

LfpTimescale::LfpTimescale (LfpDisplaySplitter* c, LfpDisplay* lfpDisplay)
    : canvasSplit (c), lfpDisplay (lfpDisplay), offset (0.0f), isPaused (false)
{
    font = FontOptions ("Fira Code", "Medium", 16.0f);

    setWantsKeyboardFocus (true);
}

void LfpTimescale::paint (Graphics& g)
{
    //std::cout << "Repainting timescale with offset of " << timeOffset << std::endl;

    g.setFont (font);

    if (isPaused)
    {
        if (canvasSplit->getSelectedState())
            g.setColour (Colour (25, 25, 25));
        else
            g.setColour (findColour (ThemeColours::defaultText));
    }
    else
    {
        if (canvasSplit->getSelectedState())
            g.setColour (Colour (25, 25, 25).withAlpha (0.5f));
        else
            g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
    }

    const String timeScaleUnitLabel = (timebase >= 2) ? ("s") : ("ms");

    int startIndex;

    if (offset > 0)
        startIndex = 0;
    else
        startIndex = 1;

    const int timescaleHeight = 30;

    for (int i = startIndex; i < labels.size(); i++)
    {
        float xLoc = getWidth() * fractionWidth[i] + float (timeOffset);

        if (xLoc > 0)
        {
            float lineHeight;

            g.fillRect (xLoc,
                        0.0f,
                        2.0f,
                        (float) timescaleHeight);

            g.drawText (labels[i] + " " + timeScaleUnitLabel,
                        xLoc + 10,
                        0,
                        100,
                        timescaleHeight,
                        Justification::left,
                        false);
        }
    }
}

void LfpTimescale::mouseUp (const MouseEvent& e)
{
    //if (e.mods.isLeftButtonDown())
    //{
    //    lfpDisplay->trackZoomInfo.isScrollingX = false;
    //}

    // Update curent time offset after dragging is over
    currentTimeOffset = timeOffset;
}

void LfpTimescale::setPausedState (bool isPaused_)
{
    if (! isPaused_)
    {
        lfpDisplay->pause (false);
        timeOffset = 0;
        currentTimeOffset = timeOffset;
        isPaused = false;
        stopTimer();
    }
    else
    {
        lfpDisplay->pause (true);
        isPaused = true;
        startTimer (20);
    }

    repaint();
}

void LfpTimescale::resized()
{
    setTimebase (timebase);
}

void LfpTimescale::mouseDown (const juce::MouseEvent& e)
{
    canvasSplit->select();

    if (e.getNumberOfClicks() == 2 && CoreServices::getAcquisitionStatus())
    {
        setPausedState (! isPaused);
    }
}

void LfpTimescale::timerCallback()
{
    if (isPaused && timeOffsetChanged)
    {
        lfpDisplay->setTimeOffset (timeOffset);

        repaint();

        timeOffsetChanged = false;
    }
}

void LfpTimescale::mouseDrag (const juce::MouseEvent& e)
{
    int dragDeltaX = (e.getScreenPosition().getX() - e.getMouseDownScreenX()); // invert so drag up -> scale up

    scrollTimescale (dragDeltaX);
}

void LfpTimescale::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& w)
{
    int scrollDeltaX = (w.deltaY * 100); // amplify mouse wheel move delta value

    if (scrollTimescale (scrollDeltaX))
        currentTimeOffset = timeOffset;
}

bool LfpTimescale::keyPressed (const KeyPress& key)
{
    if (canvasSplit->isInTriggeredMode() || ! isPaused)
        return false;

    int keyDeltaX = 25; // constant delta value

    if (key == KeyPress (KeyPress::leftKey, ModifierKeys::ctrlModifier, 0))
    {
        if (scrollTimescale (keyDeltaX))
        {
            currentTimeOffset = timeOffset;
            return true;
        }
    }
    else if (key == KeyPress (KeyPress::rightKey, ModifierKeys::ctrlModifier, 0))
    {
        if (scrollTimescale (-keyDeltaX))
        {
            currentTimeOffset = timeOffset;
            return true;
        }
    }

    return false;
}

bool LfpTimescale::scrollTimescale (int deltaX)
{
    if (canvasSplit->isInTriggeredMode() || ! isPaused || ! hasKeyboardFocus (false))
        return false;

    timeOffset = currentTimeOffset + deltaX;

    if (timeOffset < 0)
        timeOffset = 0;

    if (timeOffset > getWidth() * 3)
        timeOffset = getWidth() * 3;

    if (currentTimeOffset != timeOffset)
    {
        timeOffsetChanged = true;
        return true;
    }

    return false;
}

void LfpTimescale::setTimebase (float timebase_, float offset_)
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
        String labelString = String (time * ((timebase >= 2) ? (1) : (1000.0f)));
        labels.add (labelString.substring (0, 6));

        fractionWidth.add ((time + offset) / timebase);

        if (index % 2 == 0)
            isMajor.add (true);
        else
            isMajor.add (false);

        time += stepSize;
        index++;
    }

    repaint();
}
