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

#include "PerPixelBitmapPlotter.h"
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
#include "SupersampledBitmapPlotter.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - PerPixelBitmapPlotter -

PerPixelBitmapPlotter::PerPixelBitmapPlotter(LfpDisplay * lfpDisplay)
    : LfpBitmapPlotter(lfpDisplay)
{ }

void PerPixelBitmapPlotter::plot(Image::BitmapData &bitmapData, LfpBitmapPlotterInfo &pInfo)
{
    int jfrom = pInfo.from + pInfo.y;
    int jto = pInfo.to + pInfo.y;
    
    //if (yofs<0) {yofs=0;};
    
    if (pInfo.samp < 0) {pInfo.samp = 0;};
    if (pInfo.samp >= display->lfpChannelBitmap.getWidth()) {pInfo.samp = display->lfpChannelBitmap.getWidth()-1;}; // this shouldnt happen, there must be some bug above - to replicate, run at max refresh rate where draws overlap the right margin by a lot
    
    if (jfrom<0) {jfrom=0;};
    if (jto >= display->lfpChannelBitmap.getHeight()) {jto=display->lfpChannelBitmap.getHeight()-1;};
    
    for (int j = jfrom; j <= jto; j += 1)
    {
        
        //uint8* const pu8Pixel = bdSharedLfpDisplay.getPixelPointer(	(int)(i),(int)(j));
        //*(pu8Pixel)		= 200;
        //*(pu8Pixel+1)	= 200;
        //*(pu8Pixel+2)	= 200;
        
        bitmapData.setPixelColour(pInfo.samp,j,pInfo.lineColour);
        
    }
}

