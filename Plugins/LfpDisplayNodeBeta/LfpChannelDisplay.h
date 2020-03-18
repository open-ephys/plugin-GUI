// LfpChannelDisplay.h

#ifndef LFPCHANNELDISPLAY_H

#define LFPCHANNELDISPLAY_H

#include "LfpDisplayCanvas.h"

namespace LfpDisplayNodeBeta { 

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



};
#endif
