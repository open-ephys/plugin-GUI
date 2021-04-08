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

#include "MonochromeGrayColourScheme.h"
#include "../LfpDisplayCanvas.h"
#include "../LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - MonochromeGrayColourScheme -

Array<Colour> MonochromeGrayColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;


    colours.add(Colour(216, 221, 215));
    colours.add(Colour(178, 180, 188));
    colours.add(Colour(129, 120, 117));
    colours.add(Colour(206, 201, 205));
    colours.add(Colour(157, 148, 141));
    colours.add(Colour(199, 194, 209));
    colours.add(Colour(139, 144, 144));
    colours.add(Colour(170, 174, 176));
    colours.add(Colour(228, 226, 220));
    colours.add(Colour(135, 118, 136));
    colours.add(Colour(179, 176, 179));
    colours.add(Colour(140, 133, 144));
    colours.add(Colour(165, 157, 165));
    colours.add(Colour(212, 214, 224));
    colours.add(Colour(156, 166, 166));
    colours.add(Colour(191, 186, 184));
    return colours;
}();

MonochromeGrayColourScheme::MonochromeGrayColourScheme(LfpDisplay* display, LfpDisplaySplitter* canvas)
    : LfpViewer::ChannelColourScheme(MonochromeGrayColourScheme::colourList.size(), display, canvas)
{
    setName("Monochrome Gray");
}

void MonochromeGrayColourScheme::paint(Graphics& g)
{

}

void MonochromeGrayColourScheme::resized()
{

}

const Colour MonochromeGrayColourScheme::getBackgroundColour() const
{
    return Colour(15, 15, 15);
}

const Colour MonochromeGrayColourScheme::getColourForIndex(int index) const
{
    return colourList[(int(index / colourGrouping)) % colourList.size()];
}


