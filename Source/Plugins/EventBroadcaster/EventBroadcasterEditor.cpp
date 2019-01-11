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

    EventBroadcaster* p = (EventBroadcaster*)getProcessor();

    restartConnection = new UtilityButton("Restart Connection",Font("Default", 15, Font::plain));
    restartConnection->setBounds(10,35,150,18);
    restartConnection->addListener(this);
    addAndMakeVisible(restartConnection);

    urlLabel = new Label("Port", "Port:");
    urlLabel->setBounds(20, 60, 140, 25);
    addAndMakeVisible(urlLabel);

    portLabel = new Label("Port", String(p->getListeningPort()));
    portLabel->setBounds(70,65,80,18);
    portLabel->setFont(Font("Default", 15, Font::plain));
    portLabel->setColour(Label::textColourId, Colours::white);
    portLabel->setColour(Label::backgroundColourId, Colours::grey);
    portLabel->setEditable(true);
    portLabel->addListener(this);
    addAndMakeVisible(portLabel);

    formatLabel = new Label("Format", "Format:");
    formatLabel->setBounds(5, 100, 60, 25);
    addAndMakeVisible(formatLabel);

    formatBox = new ComboBox("FormatBox");
    formatBox->setBounds(65, 100, 100, 20);
    formatBox->addItem("Raw binary", EventBroadcaster::Format::RAW_BINARY);
    formatBox->addItem("Header only", EventBroadcaster::Format::HEADER_ONLY);
    formatBox->addItem("Header/JSON", EventBroadcaster::Format::HEADER_AND_JSON);
    formatBox->setSelectedId(p->getOutputFormat());
    formatBox->addListener(this);
    addAndMakeVisible(formatBox);

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


void EventBroadcasterEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == formatBox)
    {
        auto p = static_cast<EventBroadcaster*>(getProcessor());
        p->setOutputFormat(comboBoxThatHasChanged->getSelectedId());
    }
}


void EventBroadcasterEditor::setDisplayedPort(int port)
{
    portLabel->setText(String(port), dontSendNotification);
}


void EventBroadcasterEditor::setDisplayedFormat(int format)
{
    formatBox->setSelectedId(format, dontSendNotification);
}