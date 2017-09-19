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
#ifndef __LFPDISPLAYCANVAS_H_Alpha__
#define __LFPDISPLAYCANVAS_H_Alpha__

#include <VisualizerWindowHeaders.h>
#include "LfpDisplayNode.h"

#include <vector>
#include <array>

#define CHANNEL_TYPES 3
#define MAX_N_CHAN 2048
#define MAX_N_SAMP 5000
#define MAX_N_SAMP_PER_PIXEL 100

namespace LfpDisplayNodeAlpha { 

class LfpDisplayNode;

class LfpTimescale;
class LfpDisplay;
class LfpChannelDisplay;
class LfpChannelDisplayInfo;
class EventDisplayInterface;
class LfpViewport;
class LfpDisplayOptions;
class LfpBitmapPlotter;
class PerPixelBitmapPlotter;
class SupersampledBitmapPlotter;
class LfpChannelColourScheme;

    
    
#pragma mark - LfpDisplayCanvas -
//==============================================================================
/**

  Displays multiple channels of continuous data.

  @see LfpDisplayNode, LfpDisplayEditor

*/
    
class LfpDisplayCanvas : public Visualizer,
    public KeyListener
{
public:
    LfpDisplayCanvas(LfpDisplayNode* n);
    ~LfpDisplayCanvas();

    void beginAnimation();
    void endAnimation();

    void refreshState();
    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float) {}

    void paint(Graphics& g);

    void refresh();
    void resized();
    
    /** Resizes the LfpDisplay to the size required to fit all channels that are being
        drawn to the screen.
        
        @param respectViewportPosition  (optional) if true, viewport automatically
                                        scrolls to maintain view prior to resizing
     */
    void resizeToChannels(bool respectViewportPosition = false);

    void toggleOptionsDrawer(bool);

    int getChannelHeight();
    
    float channelOverlapFactor;

    float histogramParameterA;
    float histogramParameterB;

    /** Returns the number of both continuous and event channels tracked by the canvas */
    int getNumChannels();
    /** Returns the number of channels NOT hidden for display */
    int getNumChannelsVisible();
    bool getInputInvertedState();
    
    /** Returns a bool describing whether the spike raster functionality is enabled */
    bool getDisplaySpikeRasterizerState();
    
    bool getDrawMethodState();
    
    int getChannelSampleRate(int channel);
    
    /** Delegates a samplerate for drawing to the LfpDisplay referenced by this canvas */
    void setDrawableSampleRate(float samplerate);
    
    /** Returns the subprocessor index of the given channel */
    int getChannelSubprocessorIdx(int channel);
    
    /** Delegates a subprocessor index for drawing to the LfpDisplay referenced by this
        this canvas */
    void setDrawableSubprocessor(int idx);

    const float getXCoord(int chan, int samp);
    const float getYCoord(int chan, int samp);
    
    std::array<float, MAX_N_SAMP_PER_PIXEL> getSamplesPerPixel(int chan, int px);
    const int getSampleCountPerPixel(int px);
    
    const float getYCoordMin(int chan, int samp);
    const float getYCoordMean(int chan, int samp);
    const float getYCoordMax(int chan, int samp);

    float getMean(int chan);
    float getStd(int chan);

    Array<int> screenBufferIndex;
    Array<int> lastScreenBufferIndex;

    void saveVisualizerParameters(XmlElement* xml);
    void loadVisualizerParameters(XmlElement* xml);

    bool keyPressed(const KeyPress& key);
    bool keyPressed(const KeyPress& key, Component* orig);

    //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for each display;
    static const int leftmargin=50; // left margin for lfp plots (so the ch number text doesnt overlap)

    Array<bool> isChannelEnabled;
    
    bool  drawClipWarning; // optinally draw (subtle) warning if data is clipped in display
    bool  drawSaturationWarning; // optionally raise hell if the actual data is saturating
    
    int nChans;
    //int nChansVisible; // the number of channels NOT hidden for display

    float timebase;

    void redraw();

	DataChannel::DataChannelTypes selectedChannelType;

    ScopedPointer<LfpViewport> viewport;

private:
    
    Array<float> sampleRate;

    bool optionsDrawerIsOpen;
    
    float displayGain;
    float timeOffset;
    //int spread ; // vertical spacing between channels

    
    //float waves[MAX_N_CHAN][MAX_N_SAMP*2]; // we need an x and y point for each sample

    LfpDisplayNode* processor;
    AudioSampleBuffer* displayBuffer; // sample wise data buffer for display
    ScopedPointer<AudioSampleBuffer> screenBuffer; // subsampled buffer- one int per pixel

    //'define 3 buffers for min mean and max for better plotting of spikes
    // not pretty, but 'AudioSampleBuffer works only for channels X samples
    ScopedPointer<AudioSampleBuffer> screenBufferMin; // like screenBuffer but holds min/mean/max values per pixel
    ScopedPointer<AudioSampleBuffer> screenBufferMean; // like screenBuffer but holds min/mean/max values per pixel
    ScopedPointer<AudioSampleBuffer> screenBufferMax; // like screenBuffer but holds min/mean/max values per pixel

    MidiBuffer* eventBuffer;

    ScopedPointer<LfpTimescale> timescale;
    ScopedPointer<LfpDisplay> lfpDisplay;
    
    ScopedPointer<LfpDisplayOptions> options;

    void refreshScreenBuffer();
    void updateScreenBuffer();

    Array<int> displayBufferIndex;
    int displayBufferSize;

    int scrollBarThickness;
    
    //float samplesPerPixel[MAX_N_SAMP][MAX_N_SAMP_PER_PIXEL];
    //float*** samplesPerPixel;

	void resizeSamplesPerPixelBuffer(int numChannels);
    std::vector<std::array<std::array<float, MAX_N_SAMP_PER_PIXEL>, MAX_N_SAMP>> samplesPerPixel;

    int sampleCountPerPixel[MAX_N_SAMP];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayCanvas);

};

    
    
#pragma mark - ShowHideOptionsButton -
//==============================================================================
/**
 
 Toggles view options drawer for LfpDisplayCanvas.
 
 */
class ShowHideOptionsButton : public Button
{
public:
    ShowHideOptionsButton(LfpDisplayOptions*);
    virtual ~ShowHideOptionsButton();
    void paintButton(Graphics& g, bool, bool);
    LfpDisplayOptions* options;
};
    
    
    
#pragma mark - LfpDisplayOptions -
//==============================================================================
/**
 
    Holds the LfpDisplay UI controls
 
 */
class LfpDisplayOptions : public Component,
    public Slider::Listener,
    public ComboBox::Listener,
    public Button::Listener
{
public:
    LfpDisplayOptions(LfpDisplayCanvas*, LfpTimescale*, LfpDisplay*, LfpDisplayNode*);
    ~LfpDisplayOptions();

    void paint(Graphics& g);
    void resized();

    void setRangeSelection(float range, bool canvasMustUpdate = false); // set range selection combo box to correct value if it has been changed by scolling etc.
    void setSpreadSelection(int spread, bool canvasMustUpdate = false, bool deferDisplayRefresh = false); // set spread selection combo box to correct value if it has been changed by scolling etc.

    void comboBoxChanged(ComboBox* cb);
    void buttonClicked(Button* button);
    
    /** Changes the timebase value used by LfpTimescale and LfpDisplayCanvas. */
    void setTimebaseAndSelectionText(float timebase);
    
    /** Handles slider events for all editors. */
    void sliderValueChanged(Slider* sl);
    
    /** Called by sliderValueChanged(). Deals with clicks on custom sliders. Subclasses
     of GenericEditor should modify this method only.*/
    void sliderEvent(Slider* sl);

    int getChannelHeight();
    bool getDrawMethodState();
    bool getInputInvertedState();
    
    /** Return a bool describing whether the spike raster functionality is enabled */
    bool getDisplaySpikeRasterizerState();
    
    /** Sets the state of the spike raster functionality on/off */
    void setDisplaySpikeRasterizerState(bool isEnabled);

    //void setRangeSelection(float range, bool canvasMustUpdate);
    void setSpreadSelection();

    void togglePauseButton(bool sendUpdate = true);

    void saveParameters(XmlElement* xml);
    void loadParameters(XmlElement* xml);

	DataChannel::DataChannelTypes getChannelType(int n);
	DataChannel::DataChannelTypes getSelectedType();
    String getTypeName(DataChannel::DataChannelTypes type);
	int getRangeStep(DataChannel::DataChannelTypes type);

	void setSelectedType(DataChannel::DataChannelTypes type, bool toggleButton = true);

    int selectedSpread;
    String selectedSpreadValue;

    int selectedTimebase;
    String selectedTimebaseValue;

    int selectedOverlap;
    String selectedOverlapValue;
    
    int selectedChannelDisplaySkip;
    String selectedChannelDisplaySkipValue;
    
    int selectedSpikeRasterThreshold;
    String selectedSpikeRasterThresholdValue;
    
    // this enum is a candidate option for refactoring, not used yet
    enum ChannelDisplaySkipValue {
        None = 0,
        One,
        Two,
        Four,
        Eight,
        Sixteen,
        ThirtyTwo
    } enum_selectedChannelDisplaySkipValue = None;
    
    int selectedSaturation; // for saturation warning
    String selectedSaturationValue;
    float selectedSaturationValueFloat; // TODO: this is way ugly - we should refactor all these parameters soon and get them into a nicer format- probably when we do the general plugin parameter overhaul.

private:

    LfpDisplayCanvas* canvas;
    LfpDisplay* lfpDisplay;
    LfpTimescale* timescale;
    LfpDisplayNode* processor;
    
    Font labelFont;
    Colour labelColour;

    ScopedPointer<ComboBox> timebaseSelection;
    ScopedPointer<ComboBox> rangeSelection;
    ScopedPointer<ComboBox> spreadSelection;
    
    ScopedPointer<ComboBox> overlapSelection;
    ScopedPointer<UtilityButton> drawClipWarningButton; // optinally draw (subtle) warning if data is clipped in display
    
    ScopedPointer<ComboBox> saturationWarningSelection;
    ScopedPointer<UtilityButton> drawSaturateWarningButton; // optionally raise hell if the actual data is saturating
    
    ScopedPointer<ComboBox> colorGroupingSelection;
    ScopedPointer<UtilityButton> invertInputButton;
    ScopedPointer<UtilityButton> drawMethodButton;
    ScopedPointer<UtilityButton> pauseButton;
    OwnedArray<UtilityButton> typeButtons;
    
    // label and button for spike raster functionality
    ScopedPointer<Label> spikeRasterLabel;
    ScopedPointer<ComboBox> spikeRasterSelection;
    StringArray spikeRasterSelectionOptions;
    
    // label and button for reversing the order of displayed channels
    ScopedPointer<Label> reverseChannelsDisplayLabel;
    ScopedPointer<UtilityButton> reverseChannelsDisplayButton;
    
    // label and combobox for channel skipping in display
    ScopedPointer<Label> channelDisplaySkipLabel;
    ScopedPointer<ComboBox> channelDisplaySkipSelection;
    StringArray channelDisplaySkipOptions;
    
    // label and combobox for stream rate to be displayed (only show one or other)
    ScopedPointer<Label> streamRateDisplayedLabel;
    ScopedPointer<ComboBox> streamRateDisplayedSelection;
    StringArray streamRateDisplayedOptions;
    
    // label and toggle button for the median offset plotting feature
    ScopedPointer<Label> medianOffsetPlottingLabel;
    ScopedPointer<UtilityButton> medianOffsetPlottingButton;
    
    // label and combobox for color scheme options
    ScopedPointer<Label> colourSchemeOptionLabel;
    ScopedPointer<ComboBox> colourSchemeOptionSelection;
    
    ScopedPointer<Slider> brightnessSliderA;
    ScopedPointer<Slider> brightnessSliderB;
    
    ScopedPointer<Label> sliderALabel;
    ScopedPointer<Label> sliderBLabel;

    ScopedPointer<ShowHideOptionsButton> showHideOptionsButton;

    // TODO: (kelly) consider moving these into a config singleton (meyers?) to clean up
    //               the constructor and huge array inits currently in there
    StringArray voltageRanges[CHANNEL_TYPES];
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels
    StringArray colorGroupings; // option for coloring every N channels the same
    StringArray overlaps; //
    StringArray saturationThresholds; //default values for when different amplifiers saturate
    
	DataChannel::DataChannelTypes selectedChannelType;
    int selectedVoltageRange[CHANNEL_TYPES];
    String selectedVoltageRangeValues[CHANNEL_TYPES];
    float rangeGain[CHANNEL_TYPES];
    StringArray rangeUnits;
    StringArray typeNames;
    int rangeSteps[CHANNEL_TYPES];

    OwnedArray<EventDisplayInterface> eventDisplayInterfaces;

};
    
    
    
#pragma mark - LfpTimeScale -
//==============================================================================
/**
 
    Displays the timescale of the LfpDisplayCanvas in the viewport.
 
 */
class LfpTimescale : public Component
{
public:
    LfpTimescale(LfpDisplayCanvas*, LfpDisplay*);
    ~LfpTimescale();

    void paint(Graphics& g);
    
    void resized();
    
    /** Handles the drag to zoom feature on the timescale. The display must
        be paused to zoom */
    void mouseDrag(const MouseEvent &e) override;
    
    void mouseUp(const MouseEvent &e) override;

    void setTimebase(float t);

private:

    LfpDisplayCanvas* canvas;
    LfpDisplay* lfpDisplay;

    float timebase;
    float labelIncrement;
    float numIncrements;

    Font font;

    StringArray labels;

};

    
    
#pragma mark - LfpDisplay -
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
    
    /** Returns the sample rate that is currently filtering the drawable channels */
    float getDisplayedSampleRate();
    
    /** Sets the samplerate that displayed channels must be set to. No channels with
        differing samplerates will be drawn to screen.
     
        This function does not automatically repopulate the drawableChannels list, so
        rebuildDrawableChannelsList must be called before the screen is updated.
     
        @see LfpDisplayCanvas::setDrawableSampleRate, LfpDisplayNode::updateSettings
     */
    void setDisplayedSampleRate(float samplerate);
    
    int getDisplayedSubprocessor();
    
    void setDisplayedSubprocessor(int subProcessorIdx);
    
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
        Point<int> zoomPivotViewportOffset;                     // similar to above, but pixel-wise offset
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
    int drawableSubprocessorIdx;

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
  
    
    
#pragma mark - LfpChannelDisplay -
//==============================================================================
/**
    Displays the information pertaining to a single data channel.
 */
class LfpChannelDisplay : public Component
{
public:
    LfpChannelDisplay(LfpDisplayCanvas*, LfpDisplay*, LfpDisplayOptions*, int channelNumber);
    ~LfpChannelDisplay();

    void resized();
    
    void paint(Graphics& g);
    
    void pxPaint(); // like paint, but just populate lfpChannelBitmap
                    // needs to avoid a paint(Graphics& g) mechanism here becauswe we need to clear the screen in the lfpDisplay repaint(),
                    // because otherwise we cant deal with the channel overlap (need to clear a vertical section first, _then_ all channels are dawn, so cant do it per channel)
                

    void select();
    void deselect();

    bool getSelected();

    void setName(String);

    void setColour(Colour c);

    void setChannelHeight(int);
    int getChannelHeight();

    void setChannelOverlap(int);
    int getChannelOverlap();
    
    /** Return the assigned channel number for this display */
    int getChannelNumber();
    
    /** Returns the assigned channel number for this display, relative
        to the subset of channels being drawn to the canvas */
    int getDrawableChannelNumber();
    
    /** Set the channel number of this channel relative to the subset of
        channels being drawn to the canvas */
    void setDrawableChannelNumber(int channelId);

    void setRange(float range);
    int getRange();

    void setInputInverted(bool);
    void setCanBeInverted(bool);

    void setDrawMethod(bool);

    PopupMenu getOptions();
    void changeParameter(const int id);

    void setEnabledState(bool);
    bool getEnabledState()
    {
        return isEnabled;
    }
    
    /** Set the isHidden flag, indicates whether this channel display
        should render to screen or not */
    void setHidden(bool);
    
    /** Return a bool flag describing whether this channel display is
        hidden from the canvas */
    bool getHidden() {
        return isHidden;
    }

	DataChannel::DataChannelTypes getType();
    void updateType();

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:

    
    LfpDisplayCanvas* canvas;
    LfpDisplay* display;
    LfpDisplayOptions* options;

    bool isSelected;
    bool isHidden;

    int chan;
    int drawableChan;

    String name;

    Font channelFont;

    Colour lineColour;

    int channelOverlap;
    int channelHeight;
    float channelHeightFloat;

    float range;

    bool isEnabled;
    bool inputInverted;
    bool canBeInverted;
    bool drawMethod;

	DataChannel::DataChannelTypes type;
    String typeStr;
    
    

};
   
    
    
#pragma mark - LfpChannelDisplayInfo -
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

    
    
#pragma mark - EventDisplayInterface -
//==============================================================================
/**
    Interface class for Event Display channels.
 */
class EventDisplayInterface : public Component,
    public Button::Listener
{
public:
    EventDisplayInterface(LfpDisplay*, LfpDisplayCanvas*, int chNum);
    ~EventDisplayInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void checkEnabledState();

    bool isEnabled;

private:

    int channelNumber;

    LfpDisplay* display;
    LfpDisplayCanvas* canvas;

    ScopedPointer<UtilityButton> chButton;

};
    
    
    
#pragma mark - LfpViewport -
//==============================================================================
/**
    Encapsulates the logic for the LfpDisplayCanvas's viewable area and user inter-
    action (scrolling) when drawn in the environment.
 
    Not much is overridden here, it uses mostly JUCE's Viewport functionality
    by inheriting Viewport but stores a reference to the LfpDisplayCanvas.
 
    @see Viewport, LfpDisplayCanvas
 */
class LfpViewport : public Viewport
{
public:
    LfpViewport(LfpDisplayCanvas* canvas);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);

private:
    LfpDisplayCanvas* canvas;
};

    
    
#pragma mark - LfpBitmapPlotterInfo -
//==============================================================================
/**
    Information struct for plotting method encapsulation classes.
 */
struct LfpBitmapPlotterInfo
{
    int channelID;
    int samp;
    int to;
    int from;
    int x;
    int y;
    int height;
    int width;
    float channelHeightFloat;
    std::array<float, MAX_N_SAMP_PER_PIXEL> samplesPerPixel;
    int sampleCountPerPixel;
    float range;
    int samplerange;
    float histogramParameterA;
    Colour lineColour;
    Colour lineColourBright;
    Colour lineColourDark;
};

    
    
#pragma mark - LfpBitmapPlotter -
//==============================================================================
/**
    Interface class for different plotting methods.
 */
class LfpBitmapPlotter
{
public:
    LfpBitmapPlotter(LfpDisplay * lfpDisplay)
        : display(lfpDisplay)
    {}
    virtual ~LfpBitmapPlotter() {}
    
    /** Plots one subsample of data from a single channel to the bitmap provided */
    virtual void plot(Image::BitmapData &bitmapData, LfpBitmapPlotterInfo &plotterInfo) = 0;
    
protected:
    LfpDisplay * display;
};

    
    
#pragma mark - PerPixelBitmapPlotter -
//==============================================================================
/**
    Abstraction of the per-pixel plotting method.
 */
class PerPixelBitmapPlotter : public LfpBitmapPlotter
{
public:
    PerPixelBitmapPlotter(LfpDisplay * lfpDisplay);
    virtual ~PerPixelBitmapPlotter() {}
    
    /** Plots one subsample of data from a single channel to the bitmap provided */
    virtual void plot(Image::BitmapData &bitmapData, LfpBitmapPlotterInfo &plotterInfo) override;
};
    
    
    
#pragma mark - SupersampledBitmapPlotter -
//==============================================================================
/**
 Abstraction of the supersampled line-based plotting method.
 */
class SupersampledBitmapPlotter : public LfpBitmapPlotter
{
public:
    SupersampledBitmapPlotter(LfpDisplay * lfpDisplay);
    virtual ~SupersampledBitmapPlotter() {}
    
    /** Plots one subsample of data from a single channel to the bitmap provided */
    virtual void plot(Image::BitmapData &bitmapData, LfpBitmapPlotterInfo &plotterInfo) override;
};
   
    
    
#pragma mark - LfpChannelColourScheme -
/**
 Interface for a color scheme object
 */
class LfpChannelColourScheme : public Component
{
public:
    LfpChannelColourScheme(int numColourChannels_, LfpDisplay* display, LfpDisplayCanvas* canvas)
    : lfpDisplay(display)
    , canvas(canvas)
    , numColourChannels(numColourChannels_)
    { }
    
    virtual ~LfpChannelColourScheme() {}
    
    void paint(Graphics &g) override {}
    void resized() override {}
    
    virtual const Colour getColourForIndex(int index) const = 0;
    
    /** Returns true if a color scheme has configurable UI elements that
        must be drawn to the options drawer. Subclasses should override this
        if they have drawable elements in the options drawer. */
    virtual bool hasConfigurableElements() { return false; }
    
    void setColourGrouping(int grouping);
    int getColourGrouping();
    
protected:
    LfpDisplay * lfpDisplay;
    LfpDisplayCanvas * canvas;
    
    int numColourChannels;
    static int colourGrouping;
};
    
    
    
#pragma mark - LfpDefaultColourScheme -
class LfpDefaultColourScheme : public LfpChannelColourScheme
{
public:
    LfpDefaultColourScheme(LfpDisplay*, LfpDisplayCanvas*);
    virtual ~LfpDefaultColourScheme() {}
    
    void paint(Graphics &g) override;
    void resized() override;
    
    virtual const Colour getColourForIndex(int index) const override;
    
private:
    static Array<Colour> colourList;
};
    
    
    
#pragma mark - LfpMonochromaticColourScheme -
enum ColourPattern
{
    DOWN = 0,
    UP,
    DOWN_UP,
    UP_DOWN
};
    
class LfpMonochromaticColourScheme : public LfpChannelColourScheme,
    public ComboBox::Listener,
    public Slider::Listener
{
public:
    LfpMonochromaticColourScheme(LfpDisplay*, LfpDisplayCanvas*);
    virtual ~LfpMonochromaticColourScheme() {}
    
    void paint(Graphics &g) override;
    void resized() override;
    
    virtual bool hasConfigurableElements() override { return true; };
    
    void sliderValueChanged(Slider* sl);
    void comboBoxChanged(ComboBox *cb);
    
    /** Catches mouseUp to determine whether the base hue has changed. */
    void mouseUp(const MouseEvent &e) override;
    
    void setBaseHue(Colour base);
    const Colour getBaseHue() const;
    
    void setColourPattern(ColourPattern newPattern) { colourPattern = newPattern; }
    ColourPattern getColourPattern() { return colourPattern; }
    
    void setNumColourSeriesSteps(int numSteps);
    int getNumColourSeriesSteps();
    
    virtual const Colour getColourForIndex(int index) const override;
    
protected:
    bool isBlackAndWhite; // Not used yet
    Colour baseHue;
    Colour swatchHue;
    Array<Colour> colourList;
    
    ColourPattern colourPattern;
    
    ScopedPointer<Label> numChannelsLabel;
    ScopedPointer<ComboBox> numChannelsSelection;
    ScopedPointer<Label> baseHueLabel;
    ScopedPointer<Slider> baseHueSlider;
    ScopedPointer<Label> colourPatternLabel;
    ScopedPointer<ComboBox> colourPatternSelection;
    
    Rectangle<int> colourSwatchRect;
    
    virtual void calculateColourSeriesFromBaseHue();
};
    
#pragma mark - LfpGradientColourScheme
    
class LfpGradientColourScheme : public LfpMonochromaticColourScheme
{
public:
    
    LfpGradientColourScheme(LfpDisplay*, LfpDisplayCanvas*);
    
    void paint(Graphics &) override;
    void resized() override;
    
    void sliderValueChanged(Slider *sl) override;
    void mouseUp(const MouseEvent &e) override;
    
    void setLerpToHue(Colour c);
    Colour getLerpToHue();
    
private:
    Colour baseHueB;
    Colour swatchHueB;
    Rectangle<int> colourSwatchRectB;
    
    ScopedPointer<Label> baseHueLabelB;
    ScopedPointer<Slider> baseHueSliderB;
    
    void calculateColourSeriesFromBaseHue() override;
};
    
};

#endif  // __LFPDISPLAYCANVAS_H_Alpha__
