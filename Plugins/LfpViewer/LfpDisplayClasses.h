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

#ifndef LFPDISPLAYCLASSES_H
#define LFPDISPLAYCLASSES_H

namespace LfpViewer
{
constexpr int MAX_N_CHAN = 16;
constexpr int MAX_N_SAMP_PER_PIXEL = 100;
constexpr int CHANNEL_TYPES = 3;
enum SplitLayouts
{
    SINGLE = 1,
    TWO_VERT,
    THREE_VERT,
    TWO_HORZ,
    THREE_HORZ
};

class LfpDisplayNode;
class LfpDisplayCanvas;
class LfpDisplaySplitter;
class ShowHideOptionsButton;
class LfpTimescale;
class LfpDisplay;
class LfpDisplayOptions;
class LfpChannelDisplay;
class LfpChannelDisplayInfo;
class EventDisplayInterface;
class LfpViewport;
class LfpBitmapPlotterInfo;
class LfpBitmapPlotter;
class PerPixelBitmapPlotter;
class SupersampledBitmapPlotter;
class ChannelColourScheme;
class DefaultColourScheme;
class MonochromaticColourScheme;
class GradientColourScheme;

}; // namespace LfpViewer
#endif
