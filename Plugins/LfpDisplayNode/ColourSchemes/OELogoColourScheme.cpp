/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2021 Open Ephys

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

#include "OELogoColourScheme.h"
#include "../LfpDisplayCanvas.h"
#include "../LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - OELogoColourScheme -

Array<Colour> OELogoColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(95, 180, 219));
    colours.add(Colour(153, 80, 53));
    colours.add(Colour(81, 144, 65));
    colours.add(Colour(205, 171, 201));
    colours.add(Colour(60, 128, 189));
    colours.add(Colour(180, 180, 181));
    colours.add(Colour(243, 207, 80));
    colours.add(Colour(217, 113, 74));

    return colours;
}();

OELogoColourScheme::OELogoColourScheme(LfpDisplay* display, LfpDisplaySplitter* canvas)
    : LfpViewer::ChannelColourScheme(OELogoColourScheme::colourList.size(), display, canvas)
{
    setName("Open Ephys Logo");
}

void OELogoColourScheme::paint(Graphics& g)
{

}

void OELogoColourScheme::resized()
{

}

const Colour OELogoColourScheme::getBackgroundColour() const
{
    return Colour(3, 3, 3);
}

const Colour OELogoColourScheme::getColourForIndex(int index) const
{
    return colourList[(int(index / colourGrouping)) % colourList.size()];
}

