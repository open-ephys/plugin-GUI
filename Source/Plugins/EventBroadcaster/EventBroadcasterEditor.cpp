/*
  ==============================================================================

    EventBroadcasterEditor.cpp
    Created: 22 May 2015 3:34:30pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#include "EventBroadcasterEditor.h"
#include "EventBroadcaster.h"


EventBroadcasterEditor::EventBroadcasterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 180;

    urlLabel = new Label("Port", "Port:");
    urlLabel->setBounds(20,80,140,25);
    addAndMakeVisible(urlLabel);
    EventBroadcaster* p = (EventBroadcaster*)getProcessor();

    restartConnection = new UtilityButton("Restart Connection",Font("Default", 15, Font::plain));
    restartConnection->setBounds(20,45,150,18);
    restartConnection->addListener(this);
    addAndMakeVisible(restartConnection);

    portLabel = new Label("Port", String(p->getListeningPort()));
    portLabel->setBounds(70,85,80,18);
    portLabel->setFont(Font("Default", 15, Font::plain));
    portLabel->setColour(Label::textColourId, Colours::white);
    portLabel->setColour(Label::backgroundColourId, Colours::grey);
    portLabel->setEditable(true);
    portLabel->addListener(this);
    addAndMakeVisible(portLabel);

    setEnabledState(false);
}


void EventBroadcasterEditor::buttonEvent(Button* button)
{
    if (button == restartConnection)
    {
        EventBroadcaster* p = (EventBroadcaster*)getProcessor();
        int status = p->setListeningPort(p->getListeningPort(), true);

#ifdef ZEROMQ
        if (status != 0)
        {
            CoreServices::sendStatusMessage(String("Restart failed: ") + zmq_strerror(status));
        }
#endif
    }
}


void EventBroadcasterEditor::labelTextChanged(juce::Label* label)
{
    if (label == portLabel)
    {
        Value val = label->getTextValue();

        EventBroadcaster* p = (EventBroadcaster*)getProcessor();
        int status = p->setListeningPort(val.getValue());

#ifdef ZEROMQ
        if (status != 0)
        {
            CoreServices::sendStatusMessage(String("Port change failed: ") + zmq_strerror(status));
        }
#endif
    }
}

void EventBroadcasterEditor::setDisplayedPort(int port)
{
    portLabel->setText(String(port), dontSendNotification);
}
