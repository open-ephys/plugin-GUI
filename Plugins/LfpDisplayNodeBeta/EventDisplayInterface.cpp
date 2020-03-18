// EventDisplayInterface.cpp

#include "EventDisplayInterface.h"
#include "LfpDisplay.h"

using namespace LfpDisplayNodeBeta;

// Event display Options --------------------------------------------------------------------

EventDisplayInterface::EventDisplayInterface(LfpDisplay* display_, LfpDisplayCanvas* canvas_, int chNum):
    isEnabled(true), display(display_), canvas(canvas_)
{

    channelNumber = chNum;

    chButton = new UtilityButton(String(channelNumber+1), Font("Small Text", 13, Font::plain));
    chButton->setRadius(5.0f);
    chButton->setBounds(4,4,14,14);
    chButton->setEnabledState(true);
    chButton->setCorners(true, false, true, false);
    //chButton.color = display->channelColours[channelNumber*2];
    chButton->addListener(this);
    addAndMakeVisible(chButton);


    checkEnabledState();

}

EventDisplayInterface::~EventDisplayInterface()
{

}

void EventDisplayInterface::checkEnabledState()
{
    isEnabled = display->getEventDisplayState(channelNumber);

    //repaint();
}

void EventDisplayInterface::buttonClicked(Button* button)
{
    checkEnabledState();
    if (isEnabled)
    {
        display->setEventDisplayState(channelNumber, false);
    }
    else
    {
        display->setEventDisplayState(channelNumber, true);
    }

    repaint();

}


void EventDisplayInterface::paint(Graphics& g)
{

    checkEnabledState();

    if (isEnabled)
    {
        g.setColour(display->channelColours[channelNumber*2]);
        g.fillRoundedRectangle(2,2,18,18,6.0f);
    }


    //g.drawText(String(channelNumber), 8, 2, 200, 15, Justification::left, false);

}
