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

#ifndef __LFPDISPLAY_H__
#define __LFPDISPLAY_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"

namespace LfpViewer
{
#pragma mark - LfpDisplay -
//==============================================================================
/**
 
    Holds and draws all of the LfpDisplayChannel and lfpDisplayChannelInfo 
    instances.
 
    All of the channels and channelInfos are drawn here to a "master" bitmap
    lfpChannelBitmap with height equal to the sum of all channel heights. This
    bitmap is drawn by the LfpViewport using Viewport::setViewedComponent.
 
 */
class LfpDisplay : public Component,
                   public Timer
{
public:
    /** Constructor */
    LfpDisplay (LfpDisplaySplitter*, Viewport*);

    /** Destructor*/
    ~LfpDisplay();

    /** Used to plot the channel data */
    Image lfpChannelBitmap;

    /** Draws the full channel image */
    void paint (Graphics& g) override;

    /** Updates the channel image from the screen buffer*/
    void refresh();

    /** Updates the size and location of individual channels*/
    void resized() override;

    /** Updates the number of displayed channels */
    void setNumChannels (int numChannels);

    /** Returns the number of display channels*/
    int getNumChannels();

    /** Returns the overall display height (for scrolling purposes) */
    int getTotalHeight();

    /** Sets the scroll amount to the previously stored value*/
    void restoreViewPosition();

    /** Reactivates channels after switching away from single channel mode*/
    void reactivateChannels();

    /** Selects channels / switches to single channel mode on double click*/
    void mouseDown (const MouseEvent& event) override;

    /** Scrolls the display*/
    void mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel) override;

    /** Sets the display range for a particular channel type*/
    void setRange (float range, ContinuousChannel::Type type);

    /** Returns the display range for the current channel type*/
    int getRange();

    /** Returns the display range for the specified channel type */
    int getRange (ContinuousChannel::Type type);

    /** Sets the channel height in pixels */
    void setChannelHeight (int r, bool resetSingle = true);

    /** Returns the channel height in pixels */
    int getChannelHeight();

    /** Caches a new channel height without updating the channels */
    void cacheNewChannelHeight (int r);

    /** Gets a pointer to the current colour scheme */
    ChannelColourScheme* getColourSchemePtr();

    /** Sets whether the input should be inverted */
    void setInputInverted (bool);

    /** Returns whether the input should be inverted across all channels */
    Array<bool> getInputInverted();

    /** Changes between super-sampled and per-pixel plotter */
    void setDrawMethod (bool);

    /** Returns a bool indicating if the channels are displayed in reverse order (true) */
    bool getChannelsReversed();

    /** Reorders the displayed channels, reversed if state == true and normal if false */
    void setChannelsReversed (bool state);

    /** Reorders the displayed channels by depth if state == true and normal if false */
    void orderChannelsByDepth (bool state);

    /** Returns true if channels are ordered by depth */
    bool shouldOrderChannelsByDepth();

    /** Returns a factor of 2 by which the displayed channels should skip */
    int getChannelDisplaySkipAmount();

    /** Set the amount of channels to skip (hide) between each that is displayed */
    void setChannelDisplaySkipAmount (int skipAmt);

    /** Updates colours across channels */
    void setColours();

    /** Sets the index of the selected colour scheme */
    void setActiveColourSchemeIdx (int index);

    /** Gets the index of the selected colour scheme */
    int getActiveColourSchemeIdx();

    /** Returns the number of available colour schemes*/
    int getNumColourSchemes();

    /** Returns the names of the available colour schemes*/
    Array<String> getColourSchemeNameArray();

    /** Sets whether events are displayed for a particular ttl line*/
    bool setEventDisplayState (int ttlLine, bool state);

    /** Returns whether events are displayed for a particular ttl line */
    bool getEventDisplayState (int ttlLine);

    /** Returns the number of adjacent channels of each colour */
    int getColourGrouping();

    /** Sets the number of adjacent channels of each colour */
    void setColourGrouping (int i);

    /** Sets whether a particular channel is enabled */
    void setEnabledState (bool state, int chan, bool updateSavedChans = true);

    /** Returns whether a particular channel is enabled */
    bool getEnabledState (int chan);

    /** Sets the scroll offset for this display*/
    void setScrollPosition (int x, int y);

    /** Returns true if the median offset is enabled for plotting, else false */
    bool getMedianOffsetPlotting();

    /** Sets the state for the median offset plotting function */
    void setMedianOffsetPlotting (bool isEnabled);

    /** Returns true if spike raster is enabled for plotting, else false */
    bool getSpikeRasterPlotting();

    /** Sets the state for the spike raster plotting function */
    void setSpikeRasterPlotting (bool isEnabled);

    /** Returns the value at which the spike raster will detect and draw spikes */
    float getSpikeRasterThreshold();

    /** Set the threshold value for the spike raster plotting function */
    void setSpikeRasterThreshold (float thresh);

    /** Returns true if a single channel is focused in viewport */
    bool getSingleChannelState();

    /** Returns the index of the channel that is focused in viewport */
    int getSingleChannelShown();

    /** Sets the view to a single channel */
    void setSingleChannelView (int channel);

    /** Sets paused state of the display */
    void pause (bool shouldPause);

    /** Returns true if the display is paused */
    bool isPaused();

    /** Sets the time offset for the display */
    void setTimeOffset (float offset);

    /** Sets playhead back to left edge*/
    void sync();

    /** Convenience struct for holding a channel and its info in drawableChannels */
    struct LfpChannelTrack
    {
        LfpChannelDisplay* channel;
        LfpChannelDisplayInfo* channelInfo;
    };

    /** Holds the channels that are being drawn */
    Array<LfpChannelTrack> drawableChannels;

    /** Set the viewport's channel focus behavior.
     
        When a single channel is selected, it fills the entire viewport and
        all other channels are hidden. Double clicking a channel's info/event
        display toggles this setting.
     
        @param chan     If chan is < 0, no channel will be selected for singular
                        focus. Giving a value of 0 or greater hides all channels
                        except for the one at that index in drawableChannels[].
                        Note: this parameter is NOT the index in channel[], but
                        the index of the channel in drawableChannels[].
     */
    void toggleSingleChannel (LfpChannelTrack drawableChannel);

    /** Reconstructs the list of drawableChannels based on ordering and filterning parameters */
    void rebuildDrawableChannelsList();

    /** Updates the channel range, after the channel type has been set*/
    void updateRange (int i);

    /** Returns a const pointer to the internally managed plotter method class */
    LfpBitmapPlotter* const getPlotterPtr() const;

    /** Current background colour (based on the selected colour scheme)*/
    Colour backgroundColour;

    /** Array of channel colours (based on the selected colour scheme*/
    Array<Colour> channelColours;

    /** All available channels (even ones that are not drawn) */
    OwnedArray<LfpChannelDisplay> channels;

    /** All available display info objects (even ones that are not drawn) */
    OwnedArray<LfpChannelDisplayInfo> channelInfo;

    /** Holds state of event display for first 8 ttl lines */
    bool eventDisplayEnabled[8];

    /** Enables simple pause function by skipping screen buffer updates */
    bool displayIsPaused = false;

    /** Pointer to display options*/
    LfpDisplayOptions* options;

    /** Convenience struct to store all variables particular to zooming mechanics */
    struct TrackZoomInfo_Struct
    {
        const int minZoomHeight = 10;
        const int maxZoomHeight = 150;
        int currentZoomHeight; // the current zoom height for the drawableChannels (not
            // currently in use)

        bool isScrollingX = false;
        bool isScrollingY = false;
        int componentStartHeight; // a cache for the dimensions of a component during drag events
        float timescaleStartScale; // a cache for the timescale size during drag events
        float zoomPivotRatioX; // a cache for calculating the anchor point when adjusting viewport
        float zoomPivotRatioY;
        juce::Point<int> zoomPivotViewportOffset; // similar to above, but pixel-wise offset
        bool unpauseOnScrollEnd;
    };

    /** Instance of trackZoomInfo struct */
    TrackZoomInfo_Struct trackZoomInfo;

    /** Stores whether or not channels are enabled */
    Array<bool> savedChannelState;

    /** x-index of display bitmap updated on previous refresh */
    int lastBitmapIndex;

    /** total number of pixels filled in the display bitmap 
     * (used to update RMS and Mean values in single channel display mode) */
    int totalPixelsFilled;

private:
    /** Used to throttle refresh speed when scrolling backwards */
    void timerCallback() override;

    int singleChan;

    int pausePoint;
    int lastFillFrom;
    bool canRefresh;
    bool colourSchemeChanged = false;

    float timeOffset = 0.0f;
    bool timeOffsetChanged;

    int numChans;
    int displaySkipAmt;

    /** Holds a channel height if reset during single channel focus */
    int cachedDisplayChannelHeight;

    float drawableSampleRate;
    uint32 drawableSubprocessor;

    int scrollX;
    int scrollY;

    int totalHeight;

    int colourGrouping;

    bool channelsReversed;
    bool channelsOrderedByDepth;
    bool m_MedianOffsetPlottingFlag;
    bool m_SpikeRasterPlottingFlag;
    float m_SpikeRasterThreshold;

    LfpDisplaySplitter* canvasSplit;
    Viewport* viewport;

    float range[3];

    LfpBitmapPlotter* plotter;

    std::unique_ptr<PerPixelBitmapPlotter> perPixelPlotter;
    std::unique_ptr<SupersampledBitmapPlotter> supersampledPlotter;

    uint8 activeColourScheme;
    OwnedArray<ChannelColourScheme> colourSchemeList;
};

}; // namespace LfpViewer
#endif
