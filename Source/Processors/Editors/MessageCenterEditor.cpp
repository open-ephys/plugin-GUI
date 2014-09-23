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

#include "MessageCenterEditor.h"

MessageCenterEditor::MessageCenterEditor(MessageCenter* owner) :
	AudioProcessorEditor(owner),
	messageCenter(owner),
    incomingBackground(Colours::grey.withAlpha(0.5f)),
    outgoingBackground(Colours::grey.withAlpha(0.5f))
{

    incomingMessageDisplayArea = new Label("Message Display Area","No new messages.");
    outgoingMessageDisplayArea = new Label("Message Display Area","Type a new message here.");
    outgoingMessageDisplayArea->setEditable(true);

    addAndMakeVisible(incomingMessageDisplayArea);
    addAndMakeVisible(outgoingMessageDisplayArea);

    sendMessageButton = new UtilityButton("Save", Font("Small Text", 0, Font::plain));
    sendMessageButton->addListener(this);
    sendMessageButton->setTooltip("Send a message to be saved by the record node");
    addAndMakeVisible(sendMessageButton);

}

MessageCenterEditor::~MessageCenterEditor()
{

}


void MessageCenterEditor::buttonClicked(Button* button)
{
	messageCenter->setParameter(1,1);
}

void MessageCenterEditor::labelTextChanged(Label* label)
{

}

void MessageCenterEditor::timerCallback()
{

}

bool MessageCenterEditor::keyPressed(const KeyPress& key)
{

}

String MessageCenterEditor::getLabelString()
{
	return outgoingMessageDisplayArea->getText();
}


void MessageCenterEditor::enable()
{
	//sendMessageButton->setVisible(true);
}

void MessageCenterEditor::disable()
{
	//sendMessageButton->setVisible(false);
}

void MessageCenterEditor::messageReceived(bool state)
{
	if (!state)
	{
		incomingMessageDisplayArea->setText("FAIL.", dontSendNotification);
	} else {
		incomingMessageDisplayArea->setText("SUCCESS!", dontSendNotification);
	}
}

void MessageCenterEditor::paint(Graphics& g)
{

    g.setColour(Colour(58,58,58));

    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(incomingBackground);

    g.fillRect(5, 5, getWidth()/2-5, getHeight()-10);

    g.setColour(outgoingBackground);

    g.fillRect(getWidth()/2+5, 5, getWidth()/2-60, getHeight()-10);

}

void MessageCenterEditor::resized()
{
    if (incomingMessageDisplayArea != 0)
        incomingMessageDisplayArea->setBounds(5,0,getWidth()/2-5,getHeight());

    if (outgoingMessageDisplayArea != 0)
        outgoingMessageDisplayArea->setBounds(getWidth()/2+5,0,getWidth()/2-60, getHeight());

    if (sendMessageButton != 0)
        sendMessageButton->setBounds(getWidth()-50, 5, 45, getHeight()-10);
}

void MessageCenterEditor::actionListenerCallback(const String& message)
{

    incomingMessageDisplayArea->setText(message, dontSendNotification);

    incomingBackground = Colours::orange;

    repaint();

}

void MessageCenterEditor::saveStateToXml(XmlElement* xml)
{

}

void MessageCenterEditor::loadStateFromXml(XmlElement* xml)
{

}
