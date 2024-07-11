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

#ifndef __LFPTIMESCALE_H__
#define __LFPTIMESCALE_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"

namespace LfpViewer
{

/**
 
    Displays the timescale of the LfpDisplaySplitter in the viewport.
 
 */
class LfpTimescale : public Component,
                     public Timer
{
public:
    /** Constructor */
    LfpTimescale (LfpDisplaySplitter*, LfpDisplay*);

    /** Destructor */
    ~LfpTimescale() {}

    /** Renders timescale*/
    void paint (Graphics& g) override;

    /** Updates time markers to fit display width*/
    void resized() override;

    /** Handles scrolling back in time */
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseDown (const MouseEvent& e) override;
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& w) override;
    bool keyPressed (const KeyPress& key) override;

    /** Changes the time interval*/
    void setTimebase (float t, float offset = 0.0f);

    /** Set paused state (called by options interface */
    void setPausedState (bool isPaused);

    /** Timer callback -- used to throttle scrolling */
    void timerCallback() override;

private:
    LfpDisplaySplitter* canvasSplit;
    LfpDisplay* lfpDisplay;

    float timebase;
    float offset;
    float labelIncrement;
    float numIncrements;

    int timeOffset = 0;
    int currentTimeOffset;
    bool timeOffsetChanged = false;

    bool isPaused;

    FontOptions font;

    Array<String> labels;
    Array<bool> isMajor;
    Array<float> fractionWidth;

    /** Updates timer offeset according to delta value */
    bool scrollTimescale (int delta);
};

}; // namespace LfpViewer
#endif
