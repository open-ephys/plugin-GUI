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
#pragma  mark - LfpChannelDisplayInfo -
//==============================================================================
/**
    Displays meta data pertaining to an associated channel, such as channel number.
 
    The enableButton displays the channel number and toggles the drawing of the
    associated LfpChannelDisplay waveform on or off.
 */
class LfpChannelDisplayInfo : public LfpChannelDisplay,
    public Button::Listener
{
    friend class LfpDisplay;
public:
    LfpChannelDisplayInfo(LfpDisplayCanvas*, LfpDisplay*, LfpDisplayOptions*, int channelNumber);

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void resized();

    void setEnabledState(bool);
    void updateType();

    void updateXY(float, float);

    void setSingleChannelState(bool);
    
    /** Returns the sample rate associated with this channel */
    int getChannelSampleRate();
    /** Sets the sample rate associated with this channel */
    void setChannelSampleRate(int samplerate);
    
    int getSubprocessorIdx() { return subProcessorIdx; }
    
    void setSubprocessorIdx(int subProcessorIdx_) { subProcessorIdx = subProcessorIdx_; }
    
    /** Updates the parent LfpDisplay that the track vertical zoom should update */
    virtual void mouseDrag(const MouseEvent &event) override;
    
    /** Disengages the mouse drag to resize track height */
    virtual void mouseUp(const MouseEvent &event) override;
    
private:

    bool isSingleChannel;
    float x, y;
    
    int samplerate;
    int subProcessorIdx;
    
    ScopedPointer<UtilityButton> enableButton;
    
    bool channelTypeStringIsVisible;
    bool channelNumberHidden;
    
    void setEnabledButtonVisibility(bool shouldBeVisible);
    bool getEnabledButtonVisibility();

    void setChannelTypeStringVisibility(bool shouldBeVisible);
    bool getChannelTypeStringVisibility();
    
    void setChannelNumberIsHidden(bool shouldBeHidden);
    bool isChannelNumberHidden();
};

}; // namespace
#endif
