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
#ifndef __LFPTIMESCALE_H__
#define __LFPTIMESCALE_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
namespace LfpViewer {
#pragma  mark - LfpTimeScale -
//==============================================================================
/**
 
    Displays the timescale of the LfpDisplaySplitter in the viewport.
 
 */
class LfpTimescale : public Component
{
public:
    LfpTimescale(LfpDisplaySplitter*, LfpDisplay*);
    ~LfpTimescale();

    void paint(Graphics& g);
    
    void resized();
    
    /** Handles the drag to zoom feature on the timescale. The display must
        be paused to zoom */
    void mouseDrag(const MouseEvent &e) override;
    
    void mouseUp(const MouseEvent &e) override;

    void setTimebase(float t, float offset = 0.0f);

private:

    LfpDisplaySplitter* canvasSplit;
    LfpDisplay* lfpDisplay;

    float timebase;
    float offset;
    float labelIncrement;
    float numIncrements;

    Font font;

    StringArray labels;
    Array<bool> isMajor;
    Array<float> fractionWidth;

};

}; // namespace
#endif
