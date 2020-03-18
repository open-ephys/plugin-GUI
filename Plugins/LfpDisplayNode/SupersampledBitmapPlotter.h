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
#ifndef __SUPERSAMPLEDBITMAPPLOTTER_H__
#define __SUPERSAMPLEDBITMAPPLOTTER_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
#include "LfpBitmapPlotter.h"
namespace LfpViewer {
#pragma  mark - SupersampledBitmapPlotter -
//==============================================================================
/**
 Abstraction of the supersampled line-based plotting method.
 */
class SupersampledBitmapPlotter : public LfpBitmapPlotter
{
public:
    SupersampledBitmapPlotter(LfpDisplay * lfpDisplay);
    virtual ~SupersampledBitmapPlotter() {}
    
    /** Plots one subsample of data from a single channel to the bitmap provided */
    virtual void plot(Image::BitmapData &bitmapData, LfpBitmapPlotterInfo &plotterInfo) override;
};
   
}; // namespace
#endif
