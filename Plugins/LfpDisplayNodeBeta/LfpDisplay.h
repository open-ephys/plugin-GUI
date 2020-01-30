// LfpDisplay.h

#ifndef LFPDISPLAY_H

#define LFPDISPLAY_H

#include "LfpDisplayCanvas.h"

namespace LfpDisplayNodeBeta {
  
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

};

#endif
