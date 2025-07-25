/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef __LFPBITMAPPLOTTERINFO_H__
#define __LFPBITMAPPLOTTERINFO_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
namespace LfpViewer
{

/**
    Information struct for plotting method encapsulation classes.
 */
struct LfpBitmapPlotterInfo
{
    int channelID;
    int samp;
    int to;
    int from;
    int x;
    int y;
    int height;
    int width;
    float channelHeightFloat;
    std::array<float, MAX_N_SAMP_PER_PIXEL> samplesPerPixel;
    int sampleCountPerPixel;
    float range;
    int samplerange;
    float histogramParameterA;
    Colour lineColour;
    Colour lineColourBright;
    Colour lineColourDark;
};

}; // namespace LfpViewer
#endif
