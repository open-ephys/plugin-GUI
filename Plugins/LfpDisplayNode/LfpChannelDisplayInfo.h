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
#ifndef __LFPCHANNELDISPLAYINFO_H__
#define __LFPCHANNELDISPLAYINFO_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
#include "LfpChannelDisplay.h"

namespace LfpViewer {

/**
    Displays meta data pertaining to an associated channel, such as channel number.
 
    The enableButton displays the channel number and toggles the drawing of the
    associated LfpChannelDisplay waveform on or off.
 */
class LfpChannelDisplayInfo : public LfpChannelDisplay,
    public Button::Listener,
    public TooltipClient
{
    friend class LfpDisplay;
public:

    /** Constructor */
    LfpChannelDisplayInfo(LfpDisplaySplitter*, LfpDisplay*, LfpDisplayOptions*, int channelNumber);

    /** Draws this info for one channel */
    void paint(Graphics& g) override;

    /** Responds to button clicks (toggles channel on and off)*/
    void buttonClicked(Button* button) override;

    /** Sets component layout */
    void resized() override;

    /** Changes the color of the display to show recorded channels*/
    void recordingStarted();

    /** Reverts the color changes from start of recording */
    void recordingStopped();

    /** Sets whether this channel is enabled */
    void setEnabledState(bool);

    /** Updates the channel type (DATA, ADC, AUX) */
    void updateType(ContinuousChannel::Type) override;

    /** Updates the position of this component*/
    void updateXY(float, float);

    /** Sets whether this channel is in single-channel mode */
    void setSingleChannelState(bool);
    
    /** Returns the sample rate associated with this channel */
    int getChannelSampleRate();

    /** Sets the sample rate associated with this channel */
    void setChannelSampleRate(int samplerate);
    
    /** Updates the parent LfpDisplay that the track vertical zoom should update */
    virtual void mouseDrag(const MouseEvent &event) override;
    
    /** Disengages the mouse drag to resize track height */
    virtual void mouseUp(const MouseEvent &event) override;


    
private:

    bool isSingleChannel;
    float x, y;
    
    int samplerate;
    int subProcessorIdx;
    
    std::unique_ptr<UtilityButton> enableButton;
    
    bool channelTypeStringIsVisible;
    bool channelNumberHidden;
    
    /** Get/set whether enabled button is visible*/
    void setEnabledButtonVisibility(bool shouldBeVisible);
    bool getEnabledButtonVisibility();

    /** Get/set whether channel type string is visible*/
    void setChannelTypeStringVisibility(bool shouldBeVisible);
    bool getChannelTypeStringVisibility();
    
    /** Get/set whether channel number is hidden */
    void setChannelNumberIsHidden(bool shouldBeHidden);
    bool isChannelNumberHidden();

    // Inherited via TooltipClient
    String getTooltip() override;
};

}; // namespace
#endif
