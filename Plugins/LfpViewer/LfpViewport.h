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

#ifndef __LFPVIEWPORT_H__
#define __LFPVIEWPORT_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"

namespace LfpViewer
{

/**
    Encapsulates the logic for the LfpDisplaySplitter's viewable area and user inter-
    action (scrolling) when drawn in the environment.
 
    Not much is overridden here, it uses mostly JUCE's Viewport functionality
    by inheriting Viewport but stores a reference to the LfpDisplayCanvas.
 
    @see Viewport, LfpDisplaySplitter
 */
class LfpViewport : public Viewport
{
public:
    /** Constructor */
    LfpViewport (LfpDisplaySplitter* canvasSplit);

    /** Called when the viewport is scrolled*/
    void visibleAreaChanged (const Rectangle<int>& newVisibleArea);

private:
    LfpDisplaySplitter* canvasSplit;
};

}; // namespace LfpViewer
#endif
