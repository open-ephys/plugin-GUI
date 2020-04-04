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

#include "LfpChannelColourScheme.h"
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

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpChannelColourScheme -

int LfpChannelColourScheme::colourGrouping = 1;

void LfpChannelColourScheme::setColourGrouping(int grouping)
{
    colourGrouping = grouping;
}

int LfpChannelColourScheme::getColourGrouping()
{
    return colourGrouping;
}

