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
    incomingBackground(Colours::black.withAlpha(0.0f)),
    outgoingBackground(Colours::black.withAlpha(0.0f)),
    backgroundColor(Colours::grey.withAlpha(0.0f))
{
    
    isExpanded = false;
    
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
    float outgoingAlpha = outgoingBackground.getFloatAlpha();
    float backgroundAlpha = backgroundColor.getFloatAlpha();

    if (incomingAlpha > 0)
    {
        incomingAlpha -= 0.05;

        if (incomingAlpha < 0)
            incomingAlpha = 0;

        incomingBackground = incomingBackground.withAlpha(incomingAlpha);

        shouldRepaint = true;
    }
    
    if (outgoingAlpha > 0)
    {
        outgoingAlpha -= 0.05;

        if (outgoingAlpha < 0)
            outgoingAlpha = 0;

        outgoingBackground = outgoingBackground.withAlpha(outgoingAlpha);

        shouldRepaint = true;
    }
    
    if (backgroundAlpha > 0)
    {
        backgroundAlpha -= 0.05;

        if (backgroundAlpha < 0)
            backgroundAlpha = 0;

        backgroundColor = backgroundColor.withAlpha(backgroundAlpha);

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

String MessageCenterEditor::getOutgoingMessage()
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
    if (!isExpanded)
        
    {
        isExpanded = true;
        
        backgroundColor = Colour(100,100,100);
        
        resized();
        
        startTimer(15);
    }

}


void MessageCenterEditor::collapse()
{
    if (isExpanded)
    {
        isExpanded = false;
        
        backgroundColor = Colours::lightgrey.withAlpha(0.30f);
                        
        resized();
        
        startTimer(40);
    }
    
}


void MessageCenterEditor::messageReceived(bool isRecording)
{
    if (!isRecording)
    {
        String msg = "Cannot save messages when recording is not active.";
        
        if (firstMessageReceived)
            incomingMessageLog->addMessage(new MessageLabel("message", incomingMessageDisplayArea->getText()));
        else
            firstMessageReceived = true;

        incomingMessageDisplayArea->setText(msg, sendNotification);
        
        incomingBackground = Colour(218, 112, 74);
        
        resized();
        
    }
    else
    {
        outgoingMessageLog->addMessage(new MessageLabel("message", editableMessageDisplayArea->getText()));
        outgoingBackground = Colour(244, 208, 80);
        
        resized();
    }

    startTimer(75);
}

void MessageCenterEditor::paint(Graphics& g)
{

    g.setColour(Colours::lightgrey.withAlpha(0.5f)); // edge color (58,58,58)

    g.fillRoundedRectangle(1, 1, getWidth()-2, getHeight()-2, 8.0f);

    if (isExpanded)
        g.setColour(Colours::lightgrey.withAlpha(0.8f));
    else
        g.setColour(Colour(100,100,100));
    
    g.fillRoundedRectangle(3, 3, getWidth()-6, getHeight()-6, 6.0f);
    
    g.setColour(backgroundColor);
    g.fillRoundedRectangle(3, 3, getWidth()-6, getHeight()-6, 6.0f);
    
    g.setColour(Colours::white.withAlpha(0.2f)); // background color (100,100,100)
    
    if (isExpanded)
        g.drawLine(getWidth()/2+9, 10, getWidth()/2+9, getHeight() - 30);

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
    
    if (isExpanded)
    {
        g.setFont (Font("Default Bold", 80, Font::plain));
        g.setColour(Colours::white.withAlpha(0.1f));
        g.drawText("INCOMING", 4,
        25,
        getWidth()/2-11,
        getHeight()-60, Justification::centred);
        g.drawText("OUTGOING", getWidth()/2+26,
        25,
        getWidth()/2-34,
        getHeight()-60, Justification::centred);
    }

    g.setColour(Colours::grey);
    g.drawRect(getWidth()/2+26,getHeight()-25,getWidth()/2-80, 20);

}

void MessageCenterEditor::resized()
{
    if (incomingMessageDisplayArea != 0)
        incomingMessageDisplayArea->setBounds(4,getHeight()-25,getWidth()/2-11,20);

    if (editableMessageDisplayArea != 0)
        editableMessageDisplayArea->setBounds(getWidth()/2+26,getHeight()-25,getWidth()/2-80, 20);
    
    if (incomingMessageLog != 0)
    {
        float h = incomingMessageLog->getDesiredHeight();
        
        if (h < 265)
        {
            incomingMessageViewport->setBounds(4,
                                            getHeight() - h - 35,
                                            getWidth()/2-11,
                                            h);
            
        } else {
            incomingMessageViewport->setBounds(4,
            25,
            getWidth()/2-11,
            getHeight()-60);
        }
        
        incomingMessageLog->setBounds(0,
          0, getWidth()/2-11,
        h);

    }
    
    if (outgoingMessageLog != 0)
    {
        float h = outgoingMessageLog->getDesiredHeight();
        
        if (h < 265)
        {
            outgoingMessageViewport->setBounds(getWidth()/2+26,
                                            getHeight() - h - 35,
                                            getWidth()/2-34,
                                            h);
            
        } else {
            outgoingMessageViewport->setBounds(getWidth()/2+26,
            25,
            getWidth()/2-34,
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

    incomingMessageDisplayArea->setText(message, sendNotification);

    incomingBackground = Colour(206, 172, 202);
    startTimer(75);
    resized();

}

// #################################################################


MessageLabel::MessageLabel(const String& componentName, const String& labelText)
    : Label(componentName,labelText)
{
    setJustificationType(Justification::bottomLeft);
    setColour(Label::textColourId, Colours::black);
    setColour(Label::textWhenEditingColourId, Colours::black);
    setColour(Label::outlineWhenEditingColourId, Colours::darkgrey);
    setFont(Font("Default", 15, Font::plain));
    setBorderSize(BorderSize<int>(0, 7, 2, 0));
    setMinimumHorizontalScale (1.0f);
    
    timestring = Time::getCurrentTime().toString(false, true, true, true);
}

String MessageLabel::getTooltip()
{
    return "Time of message: " + timestring;
}

void MessageLabel::prependText(String text)
{
    setText(text + getText(), dontSendNotification);
}


MessageLog::MessageLog(Viewport* vp) : viewport(vp), messageHeight(20)
{

}
 
 void MessageLog::addMessage(MessageLabel* message)
{
    messages.add(message);
    message->prependText(getMessageNumberAsString() + ": ");
    addAndMakeVisible(message);
    
}

String MessageLog::getMessageNumberAsString()
{
    int num_messages = messages.size();
    
    if (num_messages < 10)
    {
        return "000" + String(num_messages);
    } else if (num_messages >= 10 && num_messages < 100)
    {
        return "00" + String(num_messages);
    } else if (num_messages >= 100 && num_messages < 1000) {
        return "0" + String(num_messages);
    } else {
        return String(num_messages);
    }
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

void MessageLog::copyText()
{
    String text = "";
    
    for (auto & message: messages)
    {
        text += message->getText() + "\n";
    }
    
    SystemClipboard::copyTextToClipboard(text);
}
