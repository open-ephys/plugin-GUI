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
#ifndef __LFPDISPLAY_H__
#define __LFPDISPLAY_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
namespace LfpViewer {
#pragma  mark - LfpDisplay -
//==============================================================================
/**
 
    Holds and draws all of the LfpDisplayChannel and lfpDisplayChannelInfo 
    instances.
 
    All of the channels and channelInfos are drawn here to a "master" bitmap
    lfpChannelBitmap with height equal to the sum of all channel heights. This
    bitmap is drawn by the LfpViewport using Viewport::setViewedComponent.
 
 */
class LfpDisplay : public Component
{
public:
    LfpDisplay(LfpDisplayCanvas*, Viewport*);
    ~LfpDisplay();
    
    Image lfpChannelBitmap; // plot as bitmap instead of separately setting pixels
    // this is done purely for the preformance improvement

    void setNumChannels(int numChannels);
    int getNumChannels();

    int getTotalHeight();

    void paint(Graphics& g);

    void refresh();

    void resized();

    void reactivateChannels();

    void mouseDown(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;

	void setRange(float range, DataChannel::DataChannelTypes type);
    
    //Withouth parameters returns selected type
    int getRange();
	int getRange(DataChannel::DataChannelTypes type);

    void setChannelHeight(int r, bool resetSingle = true);
    int getChannelHeight();
    
    LfpChannelColourScheme * getColourSchemePtr();
        
    /** Caches a new channel height without updating the channels */
    void cacheNewChannelHeight(int r);
    
    void setInputInverted(bool);
    void setDrawMethod(bool);
    
    /** Returns a bool indicating if the channels are displayed in reverse order (true) */
    bool getChannelsReversed();
    
    /** Reorders the displayed channels, reversed if state == true and normal if false */
    void setChannelsReversed(bool state);
    
    /** Returns a factor of 2 by which the displayed channels should skip */
    int getChannelDisplaySkipAmount();
    
    /** Set the amount of channels to skip (hide) between each that is displayed */
    void setChannelDisplaySkipAmount(int skipAmt);

    void setColors();
    
    void setActiveColourSchemeIdx(int index);
    int getActiveColourSchemeIdx();
    
    int getNumColourSchemes();
    StringArray getColourSchemeNameArray();

    bool setEventDisplayState(int ch, bool state);
    bool getEventDisplayState(int ch);

    int getColorGrouping();
    void setColorGrouping(int i);

    void setEnabledState(bool state, int chan, bool updateSavedChans = true);
    bool getEnabledState(int);
    
    /** Returns true if the median offset is enabled for plotting, else false */
    bool getMedianOffsetPlotting();
    
    /** Sets the state for the median offset plotting function */
    void setMedianOffsetPlotting(bool isEnabled);
    
    /** Returns true if spike raster is enabled for plotting, else false */
    bool getSpikeRasterPlotting();
    
    /** Sets the state for the spike raster plotting function */
    void setSpikeRasterPlotting(bool isEnabled);
    
    /** Returns the value at which the spike raster will detect and draw spikes */
    float getSpikeRasterThreshold();
    
    /** Set the threshold value for the spike raster plotting function */
    void setSpikeRasterThreshold(float thresh);

    /** Returns true if a single channel is focused in viewport */
    bool getSingleChannelState();
    
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
    void toggleSingleChannel(int chan = -2);
    
    /** Reconstructs the list of drawableChannels based on ordering and filterning parameters */
    void rebuildDrawableChannelsList();
    
    /** Returns a const pointer to the internally managed plotter method class */
    LfpBitmapPlotter * const getPlotterPtr() const;

    Colour backgroundColour;
    
    Array<Colour> channelColours;

    OwnedArray<LfpChannelDisplay> channels;             // all channels
    OwnedArray<LfpChannelDisplayInfo> channelInfo;      // all channelInfos
    
    /** Convenience struct for holding a channel and its info in drawableChannels */
    struct LfpChannelTrack
    {
        LfpChannelDisplay * channel;
        LfpChannelDisplayInfo * channelInfo;
    };
    Array<LfpChannelTrack> drawableChannels;        // holds the channels and info that are
                                                    // drawable to the screen

    bool eventDisplayEnabled[8];
    bool isPaused; // simple pause function, skips screen buffer updates

    LfpDisplayOptions* options;
    
    /** Convenience struct to store all variables particular to zooming mechanics */
    struct TrackZoomInfo_Struct
    {
        const int minZoomHeight = 10;
        const int maxZoomHeight = 150;
        int currentZoomHeight;          // the current zoom height for the drawableChannels (not
                                        // currently in use)
        
        bool isScrollingX = false;
        bool isScrollingY = false;
        int componentStartHeight;       // a cache for the dimensions of a component during drag events
        float timescaleStartScale;        // a cache for the timescale size during drag events
        float zoomPivotRatioX;          // a cache for calculating the anchor point when adjusting viewport
        float zoomPivotRatioY;
        juce::Point<int> zoomPivotViewportOffset;                     // similar to above, but pixel-wise offset
        bool unpauseOnScrollEnd;
    };
    
    TrackZoomInfo_Struct trackZoomInfo; // and create an instance here

private:
    
    int singleChan;
	Array<bool> savedChannelState;

    int numChans;
    int displaySkipAmt;
    int cachedDisplayChannelHeight;     // holds a channel height if reset during single channel focus
    float drawableSampleRate;
    uint32 drawableSubprocessor;

    int totalHeight;

    int colorGrouping;
    
    bool channelsReversed;
    bool m_MedianOffsetPlottingFlag;
    bool m_SpikeRasterPlottingFlag;
    float m_SpikeRasterThreshold;

    LfpDisplayCanvas* canvas;
    Viewport* viewport;

    float range[3];
    
    LfpBitmapPlotter * plotter;
    
    ScopedPointer<PerPixelBitmapPlotter> perPixelPlotter;
    ScopedPointer<SupersampledBitmapPlotter> supersampledPlotter;

    // TODO: (kelly) add reference to a color scheme
//    LfpChannelColourScheme * colourScheme;
    uint8 activeColourScheme;
    OwnedArray<LfpChannelColourScheme> colourSchemeList;
};
  
}; // namespace
#endif
