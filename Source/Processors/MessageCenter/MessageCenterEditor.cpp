/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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
    acquisitionIsActive(false),
    messageCenter(owner),
    incomingBackground(250, 200, 200), // (100,100,100)
    outgoingBackground(260, 99, 240)    // (100,100,100)
{

    incomingMessageDisplayArea = new MessageLabel("Message Display Area","No new messages.");
    outgoingMessageDisplayArea = new MessageLabel("Message Display Area","TEST");
    editableMessageDisplayArea = new MessageLabel("Message Display Area","Type a new message here.");
    editableMessageDisplayArea->setEditable(true);

    addAndMakeVisible(incomingMessageDisplayArea);
    addAndMakeVisible(outgoingMessageDisplayArea);
    addAndMakeVisible(editableMessageDisplayArea);

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

    bool shouldRepaint = false;

    float incomingAlpha = incomingBackground.getFloatAlpha();

    if (incomingAlpha > 0)
    {
        incomingAlpha -= 0.05;

        if (incomingAlpha < 0)
            incomingAlpha = 0;

        incomingBackground = incomingBackground.withAlpha(incomingAlpha);

        shouldRepaint = true;
    }

    if (shouldRepaint)
        repaint();
    else
        stopTimer();

}

bool MessageCenterEditor::keyPressed(const KeyPress& key)
{
    return false;
}

String MessageCenterEditor::getLabelString()
{
    return outgoingMessageDisplayArea->getText();
}

void MessageCenterEditor::startAcquisition()
{
    acquisitionIsActive = true;
}

void MessageCenterEditor::stopAcquisition()
{
    acquisitionIsActive = false;
}

void MessageCenterEditor::enable()
{
    //sendMessageButton->setVisible(true);
}

void MessageCenterEditor::disable()
{
    //sendMessageButton->setVisible(false);
}

void MessageCenterEditor::expand()
{
    isExpanded = true;
    resized();
}


void MessageCenterEditor::collapse()
{
    isExpanded = false;
    resized();
}


void MessageCenterEditor::messageReceived(bool state)
{
    if (!state)
    {
        incomingMessageDisplayArea->setText("Cannot save messages when recording is not active.", sendNotification);
        incomingBackground = Colours::red;
    }
    else
    {
        incomingMessageDisplayArea->setText("Message sent.", sendNotification);
        incomingBackground = Colours::green;
    }

    startTimer(75);
}

void MessageCenterEditor::paint(Graphics& g)
{

    if (isExpanded)
    {
        g.setColour(Colours::black.withAlpha(0.5f)); // edge color (58,58,58)
    } else {
        g.setColour(Colours::black); // edge color (58,58,58)
    }
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 6.0f);

    g.setColour(Colour(200,220,230)); // background color (100,100,100)

    g.fillRoundedRectangle(2, 2, getWidth()-4, getHeight()-4, 4.0f);

    g.setColour(incomingBackground); // incoming background

    g.fillRect(36, 5, getWidth()/2-5, getHeight()-10);

    g.setColour(outgoingBackground); // outgoing background

    g.fillRect(getWidth()/2+5, 5, getWidth()/2-60, getHeight()-10);

}

void MessageCenterEditor::resized()
{
    if (incomingMessageDisplayArea != 0)
        incomingMessageDisplayArea->setBounds(36,5,getWidth()/2-5,getHeight()-12);

    if (outgoingMessageDisplayArea != 0 && isExpanded)
        outgoingMessageDisplayArea->setBounds(getWidth()/2+5,5,getWidth()/2-60, getHeight()-30);
    
    if (editableMessageDisplayArea != 0)
        editableMessageDisplayArea->setBounds(getWidth()/2+5,getHeight()-24,getWidth()/2-60, 18);

    if (sendMessageButton != 0)
        sendMessageButton->setBounds(getWidth()-50, getHeight()-25, 45, 20);
}

void MessageCenterEditor::actionListenerCallback(const String& message)
{

    incomingMessageDisplayArea->setText(message, sendNotification);

    incomingBackground = Colours::orange;
    startTimer(75);

}


MessageLabel::MessageLabel(const String& componentName, const String& labelText)
    : Label(componentName,labelText)
{
    setJustificationType(Justification::bottomLeft);
}


