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

#include "SupersampledBitmapPlotter.h"
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

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpSupersampledBitmapPlotter -

SupersampledBitmapPlotter::SupersampledBitmapPlotter(LfpDisplay * lfpDisplay)
    : LfpBitmapPlotter(lfpDisplay)
{ }

void SupersampledBitmapPlotter::plot(Image::BitmapData &bdLfpChannelBitmap, LfpBitmapPlotterInfo &pInfo)
{
    std::array<float, MAX_N_SAMP_PER_PIXEL> samplesThisPixel = pInfo.samplesPerPixel;
    int sampleCountThisPixel = pInfo.sampleCountPerPixel;
    
    if (pInfo.samplerange>0 && sampleCountThisPixel>0)
    {
        
        //float localHist[samplerange]; // simple histogram
        Array<float> rangeHist; // [samplerange]; // paired range histogram, same as plotting at higher res. and subsampling
        
        for (int k = 0; k <= pInfo.samplerange; k++)
            rangeHist.add(0);
        
        for (int k = 0; k <= sampleCountThisPixel; k++) // add up paired-range histogram per pixel - for each pair fill intermediate with uniform distr.
        {
            int cs_this = (((samplesThisPixel[k]/pInfo.range*pInfo.channelHeightFloat)+pInfo.height/2)-pInfo.from); // sample values -> pixel coordinates relative to from
            int cs_next = (((samplesThisPixel[k+1]/pInfo.range*pInfo.channelHeightFloat)+pInfo.height/2)-pInfo.from);
            
            if (cs_this<0) {cs_this=0;};                        //here we could clip the diaplay to the max/min, or ignore out of bound values, not sure which one is better
            if (cs_this>pInfo.samplerange) {cs_this=pInfo.samplerange;};
            if (cs_next<0) {cs_next=0;};
            if (cs_next>pInfo.samplerange) {cs_next=pInfo.samplerange;};
            
            int hfrom=0;
            int hto=0;
            
            if (cs_this<cs_next)
            {
                hfrom = (cs_this);  hto = (cs_next);
            }
            else
            {
                hfrom = (cs_next);  hto = (cs_this);
            }
            //float hrange=hto-hfrom;
            float ha=1;
            for (int l=hfrom; l<hto; l++)
            {
                rangeHist.set(l, rangeHist[l] + ha); //this emphasizes fast Y components
                
                //rangeHist[l]+=1/hrange; // this is like an oscilloscope, same energy depositetd per dx, not dy
            }
        }
        
        for (int s = 0; s <= pInfo.samplerange; s ++)  // plot histogram one pixel per bin
        {
            float a=15*((rangeHist[s])/(sampleCountThisPixel)) * (2*(0.2+pInfo.histogramParameterA));
            if (a>1.0f) {a=1.0f;};
            if (a<0.0f) {a=0.0f;};
            
            //Colour gradedColor = lineColour.withMultipliedBrightness(2.0f).interpolatedWith(lineColour.withMultipliedSaturation(0.6f).withMultipliedBrightness(0.3f),1-a) ;
            Colour gradedColor =  pInfo.lineColourBright.interpolatedWith(pInfo.lineColourDark,1-a);
            //Colour gradedColor =  Colour(0,255,0);
            
            int ploty = pInfo.from + s + pInfo.y;
            if(ploty>0 && ploty < display->lfpChannelBitmap.getHeight()) {
                bdLfpChannelBitmap.setPixelColour(pInfo.samp, pInfo.from + s + pInfo.y, gradedColor);
            }
        }
        
    } else {
        
        int ploty = pInfo.from + pInfo.y;
        if(ploty>0 && ploty < display->lfpChannelBitmap.getHeight()) {
            bdLfpChannelBitmap.setPixelColour(pInfo.samp, ploty, pInfo.lineColour);
        }
    }
}

