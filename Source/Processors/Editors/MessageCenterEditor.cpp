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
    incomingBackground(100, 100, 100),
    outgoingBackground(100, 100, 100)
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

	if (label == incomingMessageDisplayArea)
	{
		incomingBackground = Colours::orange;
		startTimer(20);
	}
}

void MessageCenterEditor::timerCallback()
{

	uint8 defaultValue = 100;

	uint8 inRed = incomingBackground.getRed();
	uint8 inGreen = incomingBackground.getGreen();
	uint8 inBlue = incomingBackground.getBlue();

	uint8 outRed = outgoingBackground.getRed();
	uint8 outGreen = outgoingBackground.getGreen();
	uint8 outBlue = outgoingBackground.getBlue();

	bool shouldRepaint = false;

	if (inGreen > defaultValue)
	{
		inGreen -= 1; shouldRepaint = true;
	}
	else if (inGreen < defaultValue)
	{
		inGreen += 1; shouldRepaint = true;
	}

	if (inRed > defaultValue)
	{
		inRed -= 1; shouldRepaint = true;
	}
	else if (inRed < defaultValue)
	{
		inRed += 1; shouldRepaint = true;
	}

	if (inBlue > defaultValue)
	{
		inBlue -= 1; shouldRepaint = true;
	}
	else if (inBlue < defaultValue)
	{
		inBlue += 1; shouldRepaint = true;
	}

	if (outGreen > defaultValue)
	{
		outGreen -= 1; shouldRepaint = true;
	}
	else if (outGreen < defaultValue)
	{
		outGreen += 1; shouldRepaint = true;
	}

	if (outRed > defaultValue)
	{
		outRed -= 1; shouldRepaint = true;
	}
	else if (outRed < defaultValue)
	{
		outRed += 1; shouldRepaint = true;
	}

	if (outBlue > defaultValue)
	{
		outBlue -= 1; shouldRepaint = true;
	}
	else if (outBlue < defaultValue)
	{
		outBlue += 1; shouldRepaint = true;
	}

	incomingBackground = Colour(inRed, inGreen, inBlue);
	outgoingBackground = Colour(outRed, outGreen, outBlue);

	if (shouldRepaint)
		repaint();
	else
		stopTimer();

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
		incomingMessageDisplayArea->setText("Cannot save messages when recording is not active.", sendNotification);
		incomingBackground = Colours::red;
	} else {
		incomingMessageDisplayArea->setText("Message sent.", sendNotification);
		incomingBackground = Colours::green;
	}

	startTimer(20);
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

    incomingMessageDisplayArea->setText(message, sendNotification);

    incomingBackground = Colours::orange;
    startTimer(20);

}

void MessageCenterEditor::saveStateToXml(XmlElement* xml)
{

}

void MessageCenterEditor::loadStateFromXml(XmlElement* xml)
{

}
