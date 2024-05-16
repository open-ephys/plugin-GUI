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

#include "EditorViewportButtons.h"

#include "LookAndFeel/CustomLookAndFeel.h"

SignalChainScrollButton::SignalChainScrollButton (int direction)
    : TextButton ("Signal Chain Scroll Button " + String (direction))
{
    if (direction == DOWN)
    {
        path.addTriangle (0.0f, 0.0f, 9.0f, 20.0f, 18.0f, 0.0f);
    }
    else
    {
        path.addTriangle (0.0f, 20.0f, 9.0f, 0.0f, 18.0f, 20.0f);
    }

    setClickingTogglesState (false);
}

void SignalChainScrollButton::setActive (bool state)
{
    isActive = state;
}

void SignalChainScrollButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    g.setColour (findColour (ThemeColors::defaultFill));
    path.scaleToFit (0, 0, getWidth(), getHeight(), true);

    g.strokePath (path, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));
}