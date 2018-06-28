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
#ifndef __LFPDISPLAYCANVAS_H_BETA__
#define __LFPDISPLAYCANVAS_H_BETA__

#include <VisualizerWindowHeaders.h>
#include "LfpDisplayNode.h"

#include <vector>
#include <array>

#define CHANNEL_TYPES 3
#define MAX_N_CHAN 2048
#define MAX_N_SAMP 5000
#define MAX_N_SAMP_PER_PIXEL 100

namespace LfpDisplayNodeBeta { 

class LfpDisplayNode;

class LfpTimescale;
class LfpDisplay;
class LfpChannelDisplay;
class LfpChannelDisplayInfo;
class EventDisplayInterface;
class LfpViewport;
class LfpDisplayOptions;

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

    void toggleOptionsDrawer(bool);

    int getChannelHeight();
    
    float channelOverlapFactor;

    float histogramParameterA;
    float histogramParameterB;

    int getNumChannels();
    bool getInputInvertedState();
    bool getDrawMethodState();

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

class ShowHideOptionsButton : public Button
{
public:
    ShowHideOptionsButton(LfpDisplayOptions*);
    virtual ~ShowHideOptionsButton();
    void paintButton(Graphics& g, bool, bool);
    LfpDisplayOptions* options;
};

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
    void setSpreadSelection(int spread, bool canvasMustUpdate = false); // set spread selection combo box to correct value if it has been changed by scolling etc.

    void comboBoxChanged(ComboBox* cb);
    void buttonClicked(Button* button);
    
    /** Handles slider events for all editors. */
    void sliderValueChanged(Slider* sl);
    
    /** Called by sliderValueChanged(). Deals with clicks on custom sliders. Subclasses
     of GenericEditor should modify this method only.*/
    void sliderEvent(Slider* sl);

    int getChannelHeight();
    bool getDrawMethodState();
    bool getInputInvertedState();

    //void setRangeSelection(float range, bool canvasMustUpdate);
    void setSpreadSelection();

    void togglePauseButton();

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
    
    int selectedSaturation; // for saturation warning
    String selectedSaturationValue;
    float selectedSaturationValueFloat; // TODO: this is way ugly - we should refactor all these parameters soon and get them into a nicer format- probably when we do the general plugin parameter overhaul.

private:

    LfpDisplayCanvas* canvas;
    LfpDisplay* lfpDisplay;
    LfpTimescale* timescale;
    LfpDisplayNode* processor;

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
    
    
    ScopedPointer<Slider> brightnessSliderA;
    ScopedPointer<Slider> brightnessSliderB;
    
    ScopedPointer<Label> sliderALabel;
    ScopedPointer<Label> sliderBLabel;

    ScopedPointer<ShowHideOptionsButton> showHideOptionsButton;

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



class LfpTimescale : public Component
{
public:
    LfpTimescale(LfpDisplayCanvas*);
    ~LfpTimescale();

    void paint(Graphics& g);

    void setTimebase(float t);

private:

    LfpDisplayCanvas* canvas;

    float timebase;

    Font font;

    StringArray labels;

};

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
    void setInputInverted(bool);
    void setDrawMethod(bool);

    void setColors();

    bool setEventDisplayState(int ch, bool state);
    bool getEventDisplayState(int ch);

    int getColorGrouping();
    void setColorGrouping(int i);

    void setEnabledState(bool state, int chan, bool updateSavedChans = true);
    bool getEnabledState(int);

    bool getSingleChannelState();

    Colour backgroundColour;
    
    Array<Colour> channelColours;

    Array<LfpChannelDisplay*> channels;
    Array<LfpChannelDisplayInfo*> channelInfo;

    bool eventDisplayEnabled[8];
    bool isPaused; // simple pause function, skips screen bufer updates
    void toggleSingleChannel(int chan = -2);

    LfpDisplayOptions* options;

    
private:
    
    int singleChan;
	Array<bool> savedChannelState;

    int numChans;

    int totalHeight;

    int colorGrouping;

    LfpDisplayCanvas* canvas;
    Viewport* viewport;

    float range[3];


};

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

	DataChannel::DataChannelTypes getType();
    void updateType();

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:

    
    LfpDisplayCanvas* canvas;
    LfpDisplay* display;
    LfpDisplayOptions* options;

    bool isSelected;

    int chan;

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

class LfpChannelDisplayInfo : public LfpChannelDisplay,
    public Button::Listener
{
public:
    LfpChannelDisplayInfo(LfpDisplayCanvas*, LfpDisplay*, LfpDisplayOptions*, int channelNumber);

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void resized();

    void setEnabledState(bool);
    void updateType();

    void updateXY(float, float);

    void setSingleChannelState(bool);

private:

    bool isSingleChannel;
    float x, y;
    ScopedPointer<UtilityButton> enableButton;

};

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

class LfpViewport : public Viewport
{
public:
    LfpViewport(LfpDisplayCanvas* canvas);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);

private:
    LfpDisplayCanvas* canvas;
};
};

#endif  // __LFPDISPLAYCANVAS_H_BETA__
