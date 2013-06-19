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
#ifndef __LFPTRIGAVGCAVCAS_H_B711873A__
#define __LFPTRIGAVGCAVCAS_H_B711873A__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../LfpTriggeredAverageNode.h"
#include "Visualizer.h"

class LfpTrigAvgNode;

class LfpTrigAvgTimescale;
class LfpTrigAvgDisplay;
class LfpTrigAvgChannelDisplay;
class LfpTrigAvgChannelDisplayInfo;
class LfpTrigAvgEventInterface;

/**

  Displays multiple channels of continuous data.

  @see LfpTrigAvgNode, LfpTrigAvgDisplayEditor

*/

class LfpTrigAvgCanvas : public Visualizer,
    public ComboBox::Listener

{
public:
    LfpTrigAvgCanvas(LfpTrigAvgNode* n);
    ~LfpTrigAvgCanvas();

    void beginAnimation();
    void endAnimation();

    void refreshState();

    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float) {}

    void paint(Graphics& g);

    void refresh();

    void resized();

    int getChannelHeight();

    int getNumChannels();

    float getXCoord(int chan, int samp);
    float getYCoord(int chan, int samp);

    int screenBufferIndex;
    int lastScreenBufferIndex;

    void comboBoxChanged(ComboBox* cb);

    void saveVisualizerParameters(XmlElement* xml);

    void loadVisualizerParameters(XmlElement* xml);

    //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for ach ch display;
    static const int leftmargin=50; // left margin for lfp plots (so the ch number text doesnt overlap)

private:

    float sampleRate;
    float timebase;
    float displayGain;
    float timeOffset;
    //int spread ; // vertical spacing between channels

    static const int MAX_N_CHAN = 256;  // maximum number of channels
    static const int MAX_N_SAMP = 5000; // maximum display size in pixels
    //float waves[MAX_N_CHAN][MAX_N_SAMP*2]; // we need an x and y point for each sample

    LfpTrigAvgNode* processor;
    AudioSampleBuffer* displayBuffer;
    AudioSampleBuffer* screenBuffer;
    MidiBuffer* eventBuffer;

    ScopedPointer<LfpTrigAvgTimescale> timescale;
    ScopedPointer<LfpTrigAvgDisplay> display;
    ScopedPointer<Viewport> viewport;

    ScopedPointer<ComboBox> timebaseSelection;
    ScopedPointer<ComboBox> rangeSelection;
    ScopedPointer<ComboBox> spreadSelection;

    StringArray voltageRanges;
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels

    OwnedArray<LfpTrigAvgEventInterface> LfpTrigAvgEventInterfaces;

    void refreshScreenBuffer();
    void updateScreenBuffer();

    int displayBufferIndex;
    int displayBufferSize;

    int scrollBarThickness;

    int nChans;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpTrigAvgCanvas);

};

class LfpTrigAvgTimescale : public Component
{
public:
    LfpTrigAvgTimescale(LfpTrigAvgCanvas*);
    ~LfpTrigAvgTimescale();

    void paint(Graphics& g);

    void setTimebase(float t);

private:

    LfpTrigAvgCanvas* canvas;

    float timebase;

    Font font;

    StringArray labels;

};

class LfpTrigAvgDisplay : public Component
{
public:
    LfpTrigAvgDisplay(LfpTrigAvgCanvas*, Viewport*);
    ~LfpTrigAvgDisplay();

    void setNumChannels(int numChannels);
    int getNumChannels();

    int getTotalHeight();

    void paint(Graphics& g);

    void refresh();

    void resized();

    void mouseDown(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;


    void setRange(float range);
    int getRange();

    void setChannelHeight(int r);
    int getChannelHeight();


    bool setEventDisplayState(int ch, bool state);
    bool getEventDisplayState(int ch);


    Array<Colour> channelColours;

    Array<LfpTrigAvgChannelDisplay*> channels;
    Array<LfpTrigAvgChannelDisplayInfo*> channelInfo;

    bool eventDisplayEnabled[8];

private:
    int numChans;

    int totalHeight;

    LfpTrigAvgCanvas* canvas;
    Viewport* viewport;    

    float range;

};

class LfpTrigAvgChannelDisplay : public Component
{
public:
    LfpTrigAvgChannelDisplay(LfpTrigAvgCanvas*, LfpTrigAvgDisplay*, int channelNumber);
    ~LfpTrigAvgChannelDisplay();

    void paint(Graphics& g);

    void select();
    void deselect();

    void setName(String);

    void setColour(Colour c);

    void setChannelHeight(int);
    int getChannelHeight();

    void setChannelOverlap(int);
    int getChannelOverlap();

    void setRange(float range);
    int getRange();

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:

    LfpTrigAvgCanvas* canvas;
    LfpTrigAvgDisplay* display;

    bool isSelected;

    int chan;

    String name;

    Font channelFont;

    Colour lineColour;

    int channelOverlap;
    int channelHeight;
    float channelHeightFloat;

    float range;

};

class LfpTrigAvgChannelDisplayInfo : public LfpTrigAvgChannelDisplay
{
public:
    LfpTrigAvgChannelDisplayInfo(LfpTrigAvgCanvas*, LfpTrigAvgDisplay*, int channelNumber);

    void paint(Graphics& g);

};

class LfpTrigAvgEventInterface : public Component,
    public Button::Listener
{
public:
    LfpTrigAvgEventInterface(LfpTrigAvgDisplay*, LfpTrigAvgCanvas*, int chNum);
    ~LfpTrigAvgEventInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void checkEnabledState();

    bool isEnabled;

private:

    int channelNumber;

    LfpTrigAvgDisplay* display;
    LfpTrigAvgCanvas* canvas;

    ScopedPointer<UtilityButton> chButton;

};


#endif  // __LFPTRIGAVGCAVCAS_H_B711873A__
