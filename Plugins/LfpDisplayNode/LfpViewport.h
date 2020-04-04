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
#ifndef __LFPVIEWPORT_H__
#define __LFPVIEWPORT_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
namespace LfpViewer {
#pragma  mark - LfpViewport -
//==============================================================================
/**
    Encapsulates the logic for the LfpDisplayCanvas's viewable area and user inter-
    action (scrolling) when drawn in the environment.
 
    Not much is overridden here, it uses mostly JUCE's Viewport functionality
    by inheriting Viewport but stores a reference to the LfpDisplayCanvas.
 
    @see Viewport, LfpDisplayCanvas
 */
class LfpViewport : public Viewport
{
public:
    LfpViewport(LfpDisplayCanvas* canvas);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);

private:
    LfpDisplayCanvas* canvas;
};

}; // namespace
#endif
