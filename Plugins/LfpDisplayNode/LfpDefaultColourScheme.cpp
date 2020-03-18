/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

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

#include "LfpDefaultColourScheme.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"
#include "LfpChannelColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpDefaultColourScheme -

Array<Colour> LfpDefaultColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(224,185,36));
    colours.add(Colour(214,210,182));
    colours.add(Colour(243,119,33));
    colours.add(Colour(186,157,168));
    colours.add(Colour(237,37,36));
    colours.add(Colour(179,122,79));
    colours.add(Colour(217,46,171));
    colours.add(Colour(217, 139,196));
    colours.add(Colour(101,31,255));
    colours.add(Colour(141,111,181));
    colours.add(Colour(48,117,255));
    colours.add(Colour(184,198,224));
    colours.add(Colour(116,227,156));
    colours.add(Colour(150,158,155));
    colours.add(Colour(82,173,0));
    colours.add(Colour(125,99,32));
    return colours;
}();

LfpDefaultColourScheme::LfpDefaultColourScheme(LfpDisplay* display, LfpDisplayCanvas* canvas)
	: LfpViewer::LfpChannelColourScheme(LfpDefaultColourScheme::colourList.size(), display, canvas)
{
    setName("Default");
}

void LfpDefaultColourScheme::paint(Graphics &g)
{
    
}

void LfpDefaultColourScheme::resized()
{
    
}

const Colour LfpDefaultColourScheme::getColourForIndex(int index) const
{
//    return colourList[index % colourList.size()];
    return colourList[(int(index/colourGrouping)) % colourList.size()];
}

