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

#include "MonochromePurpleColourScheme.h"
#include "../LfpDisplayCanvas.h"
#include "../LfpDisplay.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - MonochromePurpleColourScheme -

Array<Colour> MonochromePurpleColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(177, 147, 220));
    colours.add(Colour(145, 112, 210));
    colours.add(Colour(99, 85, 178));
    colours.add(Colour(166, 129, 227));
    colours.add(Colour(107, 93, 195));
    colours.add(Colour(163, 122, 224));
    colours.add(Colour(105, 103, 183));
    colours.add(Colour(130, 108, 204));
    colours.add(Colour(177, 142, 232));
    colours.add(Colour(104, 91, 186));
    colours.add(Colour(138, 122, 211));
    colours.add(Colour(109, 92, 189));
    colours.add(Colour(131, 117, 199));
    colours.add(Colour(171, 140, 228));
    colours.add(Colour(130, 109, 188));
    colours.add(Colour(154, 120, 219));
    return colours;
}();


MonochromePurpleColourScheme::MonochromePurpleColourScheme(LfpDisplay* display, LfpDisplaySplitter* canvas)
    : LfpViewer::ChannelColourScheme(MonochromePurpleColourScheme::colourList.size(), display, canvas)
{
    setName("Monochrome Purple");
}

void MonochromePurpleColourScheme::paint(Graphics& g)
{

}

void MonochromePurpleColourScheme::resized()
{

}

const Colour MonochromePurpleColourScheme::getBackgroundColour() const
{
    return Colour(5, 25, 5);
}

const Colour MonochromePurpleColourScheme::getColourForIndex(int index) const
{
    return colourList[(int(index / colourGrouping)) % colourList.size()];
}

