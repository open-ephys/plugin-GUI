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
    
    firstMessageReceived = false;

    incomingMessageDisplayArea = new MessageLabel("Message Display Area","No new messages");
    editableMessageDisplayArea = new MessageLabel("Message Display Area","Type a new message here.");
    editableMessageDisplayArea->setEditable(true);
    
    incomingMessageViewport = new Viewport("Incoming message viewport.");
    outgoingMessageViewport = new Viewport("Outgoing message viewport.");
    
    incomingMessageLog = new MessageLog(incomingMessageViewport);
    outgoingMessageLog = new MessageLog(outgoingMessageViewport);
    
    incomingMessageViewport->setViewedComponent(incomingMessageLog, false);
    outgoingMessageViewport->setViewedComponent(outgoingMessageLog, false);
    incomingMessageViewport->setScrollBarsShown(true, false);
    outgoingMessageViewport->setScrollBarsShown(true, false);

    addAndMakeVisible(incomingMessageDisplayArea);
    addAndMakeVisible(editableMessageDisplayArea);
    
    addAndMakeVisible(incomingMessageViewport);
    addAndMakeVisible(outgoingMessageViewport);
    
    //addAndMakeVisible(incomingMessageLog);
    //addAndMakeVisible(outgoingMessageLog);

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
    return editableMessageDisplayArea->getText();
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
        String msg ="Cannot save messages when recording is not active.";
        
        if (firstMessageReceived)
            incomingMessageLog->addMessage(new MessageLabel("message", incomingMessageDisplayArea->getText()));
        else
            firstMessageReceived = true;
        
        String timestring = Time::getCurrentTime().toString(false, true, true, true);
        incomingMessageDisplayArea->setText(timestring + " - " + msg, sendNotification);
        
        incomingBackground = Colours::red;
        
        resized();
        
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

    g.setColour(Colours::black.withAlpha(0.5f)); // edge color (58,58,58)

    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 6.0f);

    g.setColour(Colours::black.withAlpha(0.5f)); // background color (100,100,100)

    g.fillRoundedRectangle(2, 2, getWidth()-4, getHeight()-4, 4.0f);

    g.setColour(incomingBackground); // incoming background

    g.fillRoundedRectangle(4,
                           getHeight()-25,
                           getWidth()/2-12,
                           20,
                           5.0f);

    g.setColour(outgoingBackground); // outgoing background

    g.fillRoundedRectangle(getWidth()/2+26,
                           getHeight()-25,
                           getWidth()/2-82,
                           20,
                           5.0f);

}

void MessageCenterEditor::resized()
{
    if (incomingMessageDisplayArea != 0)
        incomingMessageDisplayArea->setBounds(4,getHeight()-25,getWidth()/2-5,20);

    if (editableMessageDisplayArea != 0)
        editableMessageDisplayArea->setBounds(getWidth()/2+26,getHeight()-25,getWidth()/2-80, 20);
    

    
    if (incomingMessageLog != 0)
    {
        float h = incomingMessageLog->getDesiredHeight();
        
        if (h < 265)
        {
            incomingMessageViewport->setBounds(4,
                                            getHeight() - h - 35,
                                            getWidth()/2-10,
                                            h);
            
        } else {
            incomingMessageViewport->setBounds(4,
            25,
            getWidth()/2-10,
            getHeight()-60);
        }
        
        incomingMessageLog->setBounds(0,
          0, getWidth()/2-10,
        h);

    }
    
    if (outgoingMessageLog != 0)
    {
        float h = outgoingMessageLog->getDesiredHeight();
        
        if (h < 265)
        {
            outgoingMessageViewport->setBounds(getWidth()/2+2,
                                            getHeight() - h - 35,
                                            getWidth()/2-10,
                                            h);
            
        } else {
            outgoingMessageViewport->setBounds(getWidth()/2+2,
            25,
            getWidth()/2-10,
            getHeight()-60);
        }
        
        outgoingMessageLog->setBounds(0,
          0, getWidth()/2-10,
        h);

    }
    
    if (sendMessageButton != 0)
        sendMessageButton->setBounds(getWidth()-50, getHeight()-25, 45, 20);
    
    incomingMessageViewport->setViewPositionProportionately(0, 1);
    outgoingMessageViewport->setViewPositionProportionately(0, 1);
}

void MessageCenterEditor::actionListenerCallback(const String& message)
{
    
    if (firstMessageReceived)
        incomingMessageLog->addMessage(new MessageLabel("message",
                                                    incomingMessageDisplayArea->getText()));
    else
        firstMessageReceived = true;

    String timestring = Time::getCurrentTime().toString(false, true, true, true);
    
    incomingMessageDisplayArea->setText(timestring + " - " + message, sendNotification);

    incomingBackground = Colours::orange;
    startTimer(75);
    resized();

}

// #################################################################


MessageLabel::MessageLabel(const String& componentName, const String& labelText)
    : Label(componentName,labelText)
{
    setJustificationType(Justification::bottomLeft);
    setColour(Label::textColourId, Colours::orange);
}


MessageLog::MessageLog(Viewport* vp) : viewport(vp), messageHeight(20)
{

}
 
 void MessageLog::addMessage(MessageLabel* message)
{
    messages.add(message);
    addAndMakeVisible(message);
    
}

int MessageLog::getDesiredHeight()
{

    return messages.size() * messageHeight + messageHeight;

}

void MessageLog::resized()
{
    int index = 0;
    for (auto & message: messages)
    {
        message->setBounds(0, messageHeight * ++index, getWidth(), messageHeight);
    }
}

void MessageLog::paint(Graphics& g)
{
    //g.setColour(Colours::white);
    //g.fillAll();
}

