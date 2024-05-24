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

#include "LightBackgroundColourScheme.h"
#include "../LfpDisplay.h"
#include "../LfpDisplayCanvas.h"

#include <math.h>

using namespace LfpViewer;

#pragma mark - LightBackgroundColourScheme -

Array<Colour> LightBackgroundColourScheme::colourList = []() -> Array<Colour>
{
    Array<Colour> colours;
    colours.add (Colour (55, 55, 55));

    return colours;
}();

LightBackgroundColourScheme::LightBackgroundColourScheme() : ChannelColourScheme ("Light Background", 1)
{
}

const Colour LightBackgroundColourScheme::getBackgroundColour() const
{
    return Colour (191, 230, 255);
}

const Colour LightBackgroundColourScheme::getColourForIndex (int index) const
{
    return colourList[0];
}
