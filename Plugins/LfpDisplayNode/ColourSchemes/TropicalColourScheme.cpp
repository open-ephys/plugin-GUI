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

#include "TropicalColourScheme.h"
#include "../LfpDisplayCanvas.h"
#include "../LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - TropicalColourScheme -

Array<Colour> TropicalColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(255, 50, 177));
    colours.add(Colour(255, 248, 50));
    colours.add(Colour(118, 255, 134));
    colours.add(Colour(159, 248, 255));
    colours.add(Colour(247, 210, 242));
    colours.add(Colour(255, 228, 159));
    colours.add(Colour(255, 167, 102));
    colours.add(Colour(255, 255, 255));

    return colours;
}();

TropicalColourScheme::TropicalColourScheme(LfpDisplay* display, LfpDisplaySplitter* canvas)
    : LfpViewer::ChannelColourScheme(TropicalColourScheme::colourList.size(), display, canvas)
{
    setName("Tropical");
}

void TropicalColourScheme::paint(Graphics& g)
{

}

void TropicalColourScheme::resized()
{

}

const Colour TropicalColourScheme::getBackgroundColour() const
{
    return Colour(0, 40, 70);
}

const Colour TropicalColourScheme::getColourForIndex(int index) const
{
    return colourList[(int(index / colourGrouping)) % colourList.size()];
}

