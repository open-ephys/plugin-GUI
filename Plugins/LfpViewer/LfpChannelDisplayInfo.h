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

#ifndef __LFPCHANNELDISPLAYINFO_H__
#define __LFPCHANNELDISPLAYINFO_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpChannelDisplay.h"
#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"

namespace LfpViewer
{

/**
    Displays channel labels and controls for all visible channels.
 */
class LfpChannelDisplayInfo : public Component,
                              public Button::Listener,
                              public TooltipClient
{
    friend class LfpDisplay;

public:
    /** Constructor */
    LfpChannelDisplayInfo (LfpDisplaySplitter*, LfpDisplay*);

    /** Draws channel info for all visible channels */
    void paint (Graphics& g) override;

    /** Responds to button clicks (toggles channel on and off)*/
    void buttonClicked (Button* button) override;

    /** Sets component layout */
    void resized() override;

    /** Changes the colour of the display to show recorded channels*/
    void recordingStarted();

    /** Reverts the colour changes from start of recording */
    void recordingStopped();

    /** Updates the position of the selected point in single-channel mode */
    void updateXY (float, float);

    /** Updates the mean and RMS values of the channel */
    void updateMeanAndRMS();

    /** Sets whether this channel is in single-channel mode */
    void setSingleChannelState (bool);

    /** Returns the sample rate associated with this channel */
    int getChannelSampleRate();

    /** Sets the sample rate associated with this channel */
    void setChannelSampleRate (int samplerate);

    /** Updates the parent LfpDisplay that the track vertical zoom should update */
    virtual void mouseDrag (const MouseEvent& event) override;

    /** Disengages the mouse drag to resize track height */
    virtual void mouseUp (const MouseEvent& event) override;

private:
    int getTrackCenterY (const LfpChannelDisplay& channel) const;
    int getClosestDrawableTrackIndex (int y) const;
    void syncEnableButtons();

    LfpDisplaySplitter* canvasSplit;
    LfpDisplay* display;

    bool recordingIsActive;
    bool isSingleChannel;
    float x, y;

    float rms;
    float mean;

    int samplerate;
    OwnedArray<UtilityButton> enableButtons;

    Path pointerPath;

    String getTooltip() override;
};

}; // namespace LfpViewer
#endif
