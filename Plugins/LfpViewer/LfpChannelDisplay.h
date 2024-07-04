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

#ifndef __LFPCHANNELDISPLAY_H__
#define __LFPCHANNELDISPLAY_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"

namespace LfpViewer
{

/**
    Displays the information pertaining to a single data channel.
 */
class LfpChannelDisplay : public Component
{
public:
    /** Constructor */
    LfpChannelDisplay (LfpDisplaySplitter*, LfpDisplay*, LfpDisplayOptions*, int channelNumber);

    /** Destructor */
    ~LfpChannelDisplay();

    /** Sets component boundaries */
    void resized();

    /** Renders the LfpChannelDisplay*/
    void paint (Graphics& g);

    /** Similar to paint(), but just populates the lfpChannelBitmap
    
        needs to avoid a paint(Graphics& g) mechanism here becauswe we need to clear the screen in the lfpDisplay repaint(),
        because otherwise we cant deal with the channel overlap (need to clear a vertical section first, _then_ all channels are
        drawn, so cant do it per channel)

    */
    void pxPaint();

    /** Populates the lfpChannelBitmap while scrolling back in time

        needs to avoid a paint(Graphics& g) mechanism here becauswe we need to clear the screen in the lfpDisplay repaint(),
        because otherwise we cant deal with the channel overlap (need to clear a vertical section first, _then_ all channels are
        drawn, so cant do it per channel)

    */
    void pxPaintHistory (int playhead, int rightEdge, int maxScreenBufferIndex);

    /** Selects this channel*/
    void select();

    /** Deselects this channel */
    void deselect();

    /** Returns true if this channel is selected */
    bool getSelected();

    /** Sets the channel name */
    void setName (String);

    /** Sets the channel group */
    void setGroup (int);

    /** Sets the channel depth*/
    void setDepth (float);

    /** Sets whether or not the channel is recorded by an upstream Record Node*/
    void setRecorded (bool);

    /** Sets the colour for this channel's trace */
    void setColour (Colour c);

    /** Sets the height for this channel */
    void setChannelHeight (int);

    /** Returns the height for this channel*/
    int getChannelHeight();

    /** Sets the amount of pixel overlap with adjacent channels */
    void setChannelOverlap (int);

    /** Returns the amount of pixel overlap with adjacent channels*/
    int getChannelOverlap();

    /** Return the assigned channel number */
    int getChannelNumber();

    /** Return the assigned channel name */
    String getName();

    /** Returns the assigned channel number for this display, relative
        to the subset of channels being drawn to the canvas */
    int getDrawableChannelNumber();

    /** Set the channel number of this channel relative to the subset of
        channels being drawn to the canvas */
    void setDrawableChannelNumber (int channelId);

    /** Sets the voltage range for this channel */
    void setRange (float range);

    /** Returns the voltage range index for this channel*/
    float getRange();

    /** Sets whether this channel display should be inverted */
    void setInputInverted (bool);

    /** Returns whether this channel display is inverted */
    bool getInputInverted();

    /** Sets whether this channel display can be inverted */
    void setCanBeInverted (bool);

    /** Switches between pixel-wise and histogram drawing methods */
    void setDrawMethod (bool);

    /** Returns the available options for this channel */
    PopupMenu getOptions();

    /** Changes a parameter based on ID*/
    void changeParameter (const int id);

    /** Sets whether this channel is enabled */
    void setEnabledState (bool);

    /** Returns the enabled state for this channel*/
    bool getEnabledState() { return isEnabled; }

    /** Set the isHidden flag, indicates whether this channel display
        should render to screen or not */
    void setHidden (bool);

    /** Return a bool flag describing whether this channel display is
        hidden from the canvas */
    bool getHidden()
    {
        return isHidden;
    }

    /** Reut*/
    ContinuousChannel::Type getType();
    virtual void updateType (ContinuousChannel::Type);

    void setType (ContinuousChannel::Type);

    float getDepth() { return depth; }
    int getGroup() { return group; }

    int ifrom, ito, ito_local, ifrom_local;

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:
    void drawEventOverlay (int x, int yfrom, int yto, Image::BitmapData* image);

    LfpDisplaySplitter* canvasSplit;
    LfpDisplay* display;
    LfpDisplayOptions* options;

    bool isSelected;
    bool isHidden;

    int chan;
    int drawableChan;

    String name;
    int group;
    float depth;
    bool isRecorded;

    FontOptions channelFont;

    Colour lineColour;

    int channelOverlap;
    int channelHeight;
    float channelHeightFloat;

    float range;

    bool isEnabled;
    bool inputInverted;
    bool canBeInverted;
    bool drawMethod;
    bool recordingIsActive;

    ContinuousChannel::Type type;
    String typeStr;
};

}; // namespace LfpViewer
#endif
