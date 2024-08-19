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

#ifndef __LFPDISPLAYCANVAS_H__
#define __LFPDISPLAYCANVAS_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"

#include "LfpDisplay.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpViewport.h"

namespace LfpViewer
{

class LfpDisplaySplitter;
class DisplayBuffer;

/**

  Displays multiple channels of continuous data across
  up to three split displays.

  @see LfpDisplayNode, LfpDisplayEditor

*/

class TESTABLE LfpDisplayCanvas : public Visualizer,
                                  public KeyListener
{
public:
    /** Constructor */
    LfpDisplayCanvas (LfpDisplayNode* n, SplitLayouts sl, bool isLoading);

    /** Destructor */
    ~LfpDisplayCanvas();

    /** Start rendering */
    void beginAnimation() override;

    /** Stop rendering */
    void endAnimation() override;

    /** Called when the tab becomes visible again*/
    void refreshState() override;

    /* Called when settings need to be updated*/
    void updateSettings() override;

    /** Not used -- only refresh split displays while animating */
    void refresh() override {}

    /** Re-sets all displays to the left edge of the screen */
    void syncDisplays();

    /* Called when the component changes size*/
    void resized() override;

    /** Sets one of 5 different split layout options*/
    void setLayout (SplitLayouts);

    /** Returns true if a split display needs to be squeezed vertically to accomodate options interface */
    bool makeRoomForOptions (int splitID);

    /** Returns true if a split display can be selected */
    bool canSelect (int splitID);

    /** Selects a particular split display */
    void select (LfpDisplaySplitter*);

    /** Shows/hides options draw */
    void toggleOptionsDrawer (bool);

    /** Gets the number of active split displays */
    int getTotalSplits();

    /** Saves parameters */
    void saveCustomParametersToXml (XmlElement* xml) override;

    /** Loads parameters */
    void loadCustomParametersFromXml (XmlElement* xml) override;

    /** Responds to space bar presses */
    bool keyPressed (const KeyPress& key) override;

    /** Responds to space bar presses in sub-components */
    bool keyPressed (const KeyPress& key, Component* orig) override;

    /** Mouse listeners */
    void mouseMove (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;
    void mouseUp (const MouseEvent&) override;

    /** Sets a buffer to nullptr when it's no longer needed */
    void removeBufferForDisplay (int);

#if BUILD_TESTS
    bool getChannelBitmapBounds (int splitIndex, int& x, int& y, int& width, int& height);

    bool getChannelColours (int splitIndex, Array<Colour>& channelColours, Colour& backgroundColour);

    bool setChannelHeight (int splitIndex, int height);

    bool setChannelRange (int splitIndex, int range, ContinuousChannel::Type type);
#endif

    bool isLoading;

    bool optionsDrawerIsOpen;

private:
    LfpDisplayNode* processor;

    OwnedArray<LfpDisplaySplitter> displaySplits;

    std::unique_ptr<ComboBox> displaySelection;
    std::unique_ptr<Label> displayLabel;

    SplitLayouts selectedLayout;

    float doubleVerticalSplitRatio;
    Array<float> tripleVerticalSplitRatio;
    float doubleHorizontalSplitRatio;
    Array<float> tripleHorizontalSplitRatio;

    int borderToDrag;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayCanvas);
};

/** 

    Displays continuous data and events for one DataStream.

    Each LfpDisplayCanvas can have up to 3 LfpDisplaySplitters
    shown at a given time.

*/
class LfpDisplaySplitter : public Component,
                           public ComboBox::Listener,
                           public Timer
{
public:
    /** Constructor */
    LfpDisplaySplitter (LfpDisplayNode* node, LfpDisplayCanvas* canvas, DisplayBuffer* displayBuffer, int id);

    /** Destructor */
    ~LfpDisplaySplitter() {}

    /** Fills background and draws border */
    void paint (Graphics& g);

    /** Starts rendering callbacks */
    void beginAnimation();

    /** Ends rendering callbacks */
    void endAnimation();

    /** Synchronizes displays when tab becomes visible again */
    void refreshSplitterState();

    /** Called when the signal chain is updated */
    void updateSettings();

    /** Called when recording started*/
    void recordingStarted();

    /** Called when recording started*/
    void recordingStopped();

    /** Changes component bounds */
    void resized();

    /** Updates the screen buffer and refreshes the LfpDisplay */
    void refresh();

    /** Redraws the entire split display */
    void redraw();

    /** Re-sets the vertical bar to the left-hand size */
    void syncDisplay();

    /** Re-sets the display buffer to the latest available index */
    void syncDisplayBuffer();

    /** Initiate full redraw after display is scrolled */
    void visibleAreaChanged();

    /** Selects this split display */
    void select();

    /** De-selects this split display */
    void deselect();

    /** Returns true if this split display is selected */
    bool getSelectedState() { return isSelected; }

    /** Toggles pause button for this split display */
    void handleSpaceKeyPauseEvent();

    /** Respond to user's subprocessor selection */
    void comboBoxChanged (ComboBox* cb);

    /** Re-set bounds of LfpDisplay when the number of channels has been updated */
    void resizeToChannels (bool respectViewportPosition = false);

    /** Returns the height for each channel */
    int getChannelHeight();

    /** Returns the number of both continuous and event channels tracked by the canvas */
    int getNumChannels();

    /** Returns the number of channels NOT hidden for display */
    int getNumChannelsVisible();

    /** Returns true if channel polarity is inverted*/
    bool getInputInvertedState();

    /** Delegates a samplerate for drawing to the LfpDisplay referenced by this canvas */
    void setDrawableSampleRate (float samplerate);

    /** Returns the subprocessor index of the given channel */
    uint16 getChannelStreamId (int channel);

    /** Delegates a DataStream for drawing to the LfpDisplay referenced by this
        this canvas */
    void setDrawableStream (uint16 streamId);

    /** Gets the X coordinate (in s) for a particular channel / sample combo */
    const float getXCoord (int chan, int samp);

    /** Gets the Y coordinate (in uV) for a particular channel / sample combo */
    const float getYCoord (int chan, int samp);

    /** Gets the minimum Y coordinate (in uV) for a particular channel / sample combo */
    const float getYCoordMin (int chan, int samp);

    /** Gets the mean Y coordinate (in uV) for a particular channel / sample combo */
    const float getYCoordMean (int chan, int samp);

    /** Gets the maximum Y coordinate (in uV) for a particular channel / sample combo */
    const float getYCoordMax (int chan, int samp);

    /** Gets the event channel state for a particular sample */
    const float getEventState (int samp);

    /** Returns the screen buffer's mean of a given channel */
    float getScreenBufferMean (int chan);

    /** Returns the display buffer's mean of a given channel */
    float getDisplayBufferMean (int chan);

    /** Returns the standard deviation of a given channnel*/
    float getRMS (int chan);

    /** Sets the channel to use for display triggering */
    void setTriggerChannel (int);

    /** Returns true if the display is in triggered mode */
    bool isInTriggeredMode() { return triggerChannel > -1; }

    /** Set whether triggered display should use online averaging */
    void setAveraging (bool);

    /** Reset trial count for online average */
    void resetTrials();

    /** Sets the timebase for this display (in s) */
    void setTimebase (float timebase);

    /** Stops/starts updating the screen buffer */
    void pause (bool shouldPause);

    Array<int> screenBufferIndex;
    Array<int> lastScreenBufferIndex;
    Array<float> leftOverSamples;

    float channelOverlapFactor;

    /** Used to indicate that a full redraw is required.
        Set false after each full redraw, there is a similar switch for each display.
     */
    bool fullredraw;

    /** Left margin for lfp plots (so the ch number text doesnt overlap) */
    static const int leftmargin = 60; //

    Array<bool> isChannelEnabled;

    bool isLoading;

    /** Optinally draw (subtle) warning if data is clipped in display */
    bool drawClipWarning;

    /** Optionally draw hi-vis (candy cane) warning if the actual data is saturating*/
    bool drawSaturationWarning;

    int nChans;

    int splitID;

    float timebase;

    int screenBufferWidth;

    ContinuousChannel::Type selectedChannelType;

    std::unique_ptr<ComboBox> streamSelection;

    std::unique_ptr<LfpViewport> viewport;
    std::unique_ptr<LfpTimescale> timescale;
    std::unique_ptr<LfpDisplay> lfpDisplay;
    std::unique_ptr<LfpDisplayOptions> options;

    /** Sample-wise data buffer for display*/
    DisplayBuffer* displayBuffer;

    /** Refreshes the display */
    void timerCallback();

    /** Send a message to audio monitor one channel */
    void monitorChannel (int channel);

    uint16 selectedStreamId = 0;

    void refreshScreenBuffer();

    bool shouldRebuildChannelList = false;

    void setFilteredChannels (Array<int> channels) { filteredChannels = channels; }
    Array<int> getFilteredChannels() { return filteredChannels; }

    String getStreamKey();

private:
    bool isSelected;
    bool isUpdating;

    LfpDisplayNode* processor;
    LfpDisplayCanvas* canvas;

    float sampleRate;
    float numTrials;

    bool trialAveraging;

    float displayGain;
    float timeOffset;

    int triggerChannel;
    bool reachedEnd;

    float displayedSampleRate;

    int samplesPerBufferPass;

    int eventState;

    std::unique_ptr<AudioBuffer<float>> eventDisplayBuffer; // buffer for event data

    /** Define buffers for min, mean, and max for better plotting of spikes */
    std::unique_ptr<AudioBuffer<float>> screenBufferMin; // like screenBuffer but holds minimum values per pixel
    std::unique_ptr<AudioBuffer<float>> screenBufferMean; // like screenBuffer but holds mean values per pixel
    std::unique_ptr<AudioBuffer<float>> screenBufferMax; // like screenBuffer but holds max values per pixel

    void updateScreenBuffer();

    Array<int> displayBufferIndex;
    int displayBufferSize;

    int scrollBarThickness;

    Array<int> filteredChannels = Array<int>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplaySplitter);
};

}; // namespace LfpViewer
#endif
