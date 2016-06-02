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
#ifndef __LFPDISPLAYCANVAS_H_B711873A__
#define __LFPDISPLAYCANVAS_H_B711873A__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "LfpDisplayNode.h"
#include "../../Processors/Visualization/Visualizer.h"
#define CHANNEL_TYPES 3

namespace LfpDisplayNodeBeta { 

class LfpDisplayNode;

class LfpTimescale;
class LfpDisplay;
class LfpChannelDisplay;
class LfpChannelDisplayInfo;
class EventDisplayInterface;
class LfpViewport;

/**

  Displays multiple channels of continuous data.

  @see LfpDisplayNode, LfpDisplayEditor

*/

class LfpDisplayCanvas : public Visualizer,
    public Slider::Listener,
    public ComboBox::Listener,
    public Button::Listener,
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

    void setRangeSelection(float range, bool canvasMustUpdate = false); // set range selection combo box to correct value if it has been changed by scolling etc.
    void setSpreadSelection(int spread, bool canvasMustUpdate = false); // set spread selection combo box to correct value if it has been changed by scolling etc.

    void paint(Graphics& g);

    void refresh();

    void resized();

    int getChannelHeight();
    
    float channelOverlapFactor;

    float histogramParameterA;
    float histogramParameterB;

    int getNumChannels();
    bool getInputInvertedState();
    bool getDrawMethodState();

    const float getXCoord(int chan, int samp);
    const float getYCoord(int chan, int samp);
    
    const float *getSamplesPerPixel(int chan, int px);
    const int getSampleCountPerPixel(int px);
    
    const float getYCoordMin(int chan, int samp);
    const float getYCoordMean(int chan, int samp);
    const float getYCoordMax(int chan, int samp);

    Array<int> screenBufferIndex;
    Array<int> lastScreenBufferIndex;

    void comboBoxChanged(ComboBox* cb);
    void buttonClicked(Button* button);
    
    /** Handles slider events for all editors. */
    void sliderValueChanged(Slider* sl);
    
    /** Called by sliderValueChanged(). Deals with clicks on custom sliders. Subclasses
     of GenericEditor should modify this method only.*/
    void sliderEvent(Slider* sl);
    
    void saveVisualizerParameters(XmlElement* xml);
    void loadVisualizerParameters(XmlElement* xml);

    bool keyPressed(const KeyPress& key);
    bool keyPressed(const KeyPress& key, Component* orig);

    ChannelType getChannelType(int n);
    ChannelType getSelectedType();
    String getTypeName(ChannelType type);
    int getRangeStep(ChannelType type);

    void setSelectedType(ChannelType type, bool toggleButton = true);

    //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for each display;
    static const int leftmargin=50; // left margin for lfp plots (so the ch number text doesnt overlap)

    Array<bool> isChannelEnabled;
    
    bool  drawClipWarning; // optinally draw (subtle) warning if data is clipped in display
    bool  drawSaturationWarning; // optionally raise hell if the actual data is saturating
    
    float selectedSaturationValueFloat; // TODO: this is way ugly - we should refactor all these parameters soon and get them into a nicer format- probably when we do the genreal plugin parameter overhaul.

    
    int nChans;

private:
    
    Array<float> sampleRate;
    float timebase;
    float displayGain;
    float timeOffset;
    //int spread ; // vertical spacing between channels


    static const int MAX_N_CHAN = 2048;  // maximum number of channels
    static const int MAX_N_SAMP = 5000; // maximum display size in pixels
    static const int MAX_N_SAMP_PER_PIXEL = 1000; // maximum samples considered for drawing each pixel
    //float waves[MAX_N_CHAN][MAX_N_SAMP*2]; // we need an x and y point for each sample

    LfpDisplayNode* processor;
    AudioSampleBuffer* displayBuffer; // sample wise data buffer for display
    AudioSampleBuffer* screenBuffer; // subsampled buffer- one int per pixel

    //'define 3 buffers for min mean and max for better plotting of spikes
    // not pretty, but 'AudioSampleBuffer works only for channels X samples
    AudioSampleBuffer* screenBufferMin; // like screenBuffer but holds min/mean/max values per pixel
    AudioSampleBuffer* screenBufferMean; // like screenBuffer but holds min/mean/max values per pixel
    AudioSampleBuffer* screenBufferMax; // like screenBuffer but holds min/mean/max values per pixel

    MidiBuffer* eventBuffer;

    ScopedPointer<LfpTimescale> timescale;
    ScopedPointer<LfpDisplay> lfpDisplay;
    ScopedPointer<LfpViewport> viewport;

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

    StringArray voltageRanges[CHANNEL_TYPES];
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels
    StringArray colorGroupings; // option for coloring every N channels the same
    StringArray overlaps; //
    StringArray saturationThresholds; //default values for when different amplifiers saturate

    
    ChannelType selectedChannelType;
    int selectedVoltageRange[CHANNEL_TYPES];
    String selectedVoltageRangeValues[CHANNEL_TYPES];
    float rangeGain[CHANNEL_TYPES];
    StringArray rangeUnits;
    StringArray typeNames;
    int rangeSteps[CHANNEL_TYPES];

    int selectedSpread;
    String selectedSpreadValue;

    int selectedTimebase;
    String selectedTimebaseValue;

    int selectedOverlap;
    String selectedOverlapValue;
    
    int selectedSaturation; // for saturation warning
    String selectedSaturationValue;

    
    OwnedArray<EventDisplayInterface> eventDisplayInterfaces;

    void refreshScreenBuffer();
    void updateScreenBuffer();

    Array<int> displayBufferIndex;
    int displayBufferSize;

    int scrollBarThickness;
    
    //float samplesPerPixel[MAX_N_SAMP][MAX_N_SAMP_PER_PIXEL];
    float*** samplesPerPixel;
    int sampleCountPerPixel[MAX_N_SAMP];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayCanvas);

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

    void mouseDown(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;


    void setRange(float range, ChannelType type);
    
    //Withouth parameters returns selected type
    int getRange();
    int getRange(ChannelType type);

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

    void reactivateChannels();
    
private:
    
    
    void toggleSingleChannel(int chan);
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
    LfpChannelDisplay(LfpDisplayCanvas*, LfpDisplay*, int channelNumber);
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

    ChannelType getType();
    void updateType();

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:

    
    LfpDisplayCanvas* canvas;
    LfpDisplay* display;

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

    ChannelType type;
    String typeStr;
    
    

};

class LfpChannelDisplayInfo : public LfpChannelDisplay,
    public Button::Listener
{
public:
    LfpChannelDisplayInfo(LfpDisplayCanvas*, LfpDisplay*, int channelNumber);

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void resized();

    void setEnabledState(bool);
    void updateType();

private:

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

#endif  // __LFPDISPLAYCANVAS_H_B711873A__
