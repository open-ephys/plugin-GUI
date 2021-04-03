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

#include "MonochromeGreenColourScheme.h"
#include "../LfpDisplayCanvas.h"
#include "../LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - MonochromeGreenColourScheme -

Array<Colour> MonochromeGreenColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(140, 219, 142));
    colours.add(Colour(117, 182, 116));
    colours.add(Colour(68, 143, 79));
    colours.add(Colour(126, 212, 126));
    colours.add(Colour(86, 158, 86));
    colours.add(Colour(125, 213, 119));
    colours.add(Colour(91, 156, 94));
    colours.add(Colour(99, 193, 99));
    colours.add(Colour(132, 228, 134));
    colours.add(Colour(83, 136, 88));
    colours.add(Colour(108, 201, 122));
    colours.add(Colour(84, 160, 87));
    colours.add(Colour(101, 174, 106));
    colours.add(Colour(137, 214, 130));
    colours.add(Colour(98, 178, 106));
    colours.add(Colour(126, 207, 119));
    return colours;
}();


MonochromeGreenColourScheme::MonochromeGreenColourScheme(LfpDisplay* display, LfpDisplaySplitter* canvas)
    : LfpViewer::ChannelColourScheme(MonochromeGreenColourScheme::colourList.size(), display, canvas)
{
    setName("Monochrome Green");
}

void MonochromeGreenColourScheme::paint(Graphics& g)
{

}

void MonochromeGreenColourScheme::resized()
{

}

const Colour MonochromeGreenColourScheme::getBackgroundColour() const
{
    return Colour(25, 4, 25);
}

const Colour MonochromeGreenColourScheme::getColourForIndex(int index) const
{
    return colourList[(int(index / colourGrouping)) % colourList.size()];
}

