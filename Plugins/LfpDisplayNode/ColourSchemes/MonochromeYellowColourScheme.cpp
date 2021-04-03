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

#include "MonochromeYellowColourScheme.h"
#include "../LfpDisplayCanvas.h"
#include "../LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - MonochromeYellowColourScheme -

Array<Colour> MonochromeYellowColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(247, 212, 86));
    colours.add(Colour(204, 172, 119));
    colours.add(Colour(162, 118, 147));
    colours.add(Colour(231, 196, 109));
    colours.add(Colour(173, 145, 127));
    colours.add(Colour(222, 192, 108));
    colours.add(Colour(163, 138, 130));
    colours.add(Colour(197, 162, 128));
    colours.add(Colour(254, 216, 98));
    colours.add(Colour(154, 131, 134));
    colours.add(Colour(213, 186, 109));
    colours.add(Colour(165, 150, 132));
    colours.add(Colour(197, 166, 117));
    colours.add(Colour(247, 203, 104));
    colours.add(Colour(192, 164, 135));
    colours.add(Colour(212, 176, 104));
    return colours;
}();


MonochromeYellowColourScheme::MonochromeYellowColourScheme(LfpDisplay* display, LfpDisplaySplitter* canvas)
    : LfpViewer::ChannelColourScheme(MonochromeYellowColourScheme::colourList.size(), display, canvas)
{
    setName("Monochrome Yellow");
}

void MonochromeYellowColourScheme::paint(Graphics& g)
{

}

void MonochromeYellowColourScheme::resized()
{

}

const Colour MonochromeYellowColourScheme::getBackgroundColour() const
{
    return Colour(15, 15, 25);
}

const Colour MonochromeYellowColourScheme::getColourForIndex(int index) const
{
    return colourList[(int(index / colourGrouping)) % colourList.size()];
}

