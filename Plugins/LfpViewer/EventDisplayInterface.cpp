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

#include "EventDisplayInterface.h"

#include "LfpDisplay.h"
#include "LfpDisplayCanvas.h"

#include <math.h>

using namespace LfpViewer;

EventDisplayInterface::EventDisplayInterface (LfpDisplay* display_, LfpDisplaySplitter* split, int chNum) : isEnabled (true), display (display_), canvasSplit (split)
{
    channelNumber = chNum;

    chButton = std::make_unique<UtilityButton> (String (channelNumber + 1));
    chButton->setRadius (5.0f);
    chButton->addListener (this);
    addAndMakeVisible (chButton.get());

    checkEnabledState();
}

EventDisplayInterface::~EventDisplayInterface()
{
}

void EventDisplayInterface::checkEnabledState()
{
    isEnabled = display->getEventDisplayState (channelNumber);
}

void EventDisplayInterface::buttonClicked (Button* button)
{
    checkEnabledState();

    if (isEnabled)
    {
        display->setEventDisplayState (channelNumber, false);
    }
    else
    {
        display->setEventDisplayState (channelNumber, true);
    }

    isEnabled = ! isEnabled;

    repaint();
}

void EventDisplayInterface::paintOverChildren (Graphics& g)
{
    checkEnabledState();

    if (isEnabled)
    {
        g.setColour (display->channelColours[channelNumber * 2]);
        g.drawRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 5.0f, 3.0f);
    }
}

void EventDisplayInterface::resized()
{
    chButton->setBounds (0, 0, getWidth(), getHeight());
}
