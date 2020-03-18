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

#include "ShowHideOptionsButton.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"
#include "LfpChannelColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - ShowHideOptionsButton -
// =============================================================

ShowHideOptionsButton::ShowHideOptionsButton(LfpDisplayOptions* options) : Button("Button")
{
    setClickingTogglesState(true);
}
ShowHideOptionsButton::~ShowHideOptionsButton()
{

}

void ShowHideOptionsButton::paintButton(Graphics& g, bool, bool) 
{   
    g.setColour(Colours::white);

    Path p;

    float h = getHeight();
    float w = getWidth();

    if (getToggleState())
    {
        p.addTriangle(0.5f*w, 0.2f*h,
                      0.2f*w, 0.8f*h,
                      0.8f*w, 0.8f*h);
    }
    else
    {
        p.addTriangle(0.8f*w, 0.8f*h,
                      0.2f*w, 0.5f*h,
                      0.8f*w, 0.2f*h);
    }

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    g.strokePath(p, pst);
}

