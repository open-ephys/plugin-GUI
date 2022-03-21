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

#ifndef MESSAGECENTEREDITOR_H_INCLUDED
#define MESSAGECENTEREDITOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "MessageCenter.h"
#include <stdio.h>

class MessageLabel;
class MessageLog;

/**
	Holds the interface for adding events to the message queue

	@see MessageCenter

*/

class MessageCenterEditor : public AudioProcessorEditor,
    public Button::Listener,
    public Timer,
    public ActionListener

{
public:

    /** Constructor */
    MessageCenterEditor(MessageCenter* owner);

    /** Destructor */
    ~MessageCenterEditor() { }

    /** Renders the editor */
    void paint(Graphics& g);

    bool keyPressed(const KeyPress& key);

    void resized();
    
    void expand();
    void collapse();

    void startAcquisition();
    void stopAcquisition();

    void messageReceived(bool state);

    void broadcastMessage(String msg);

    String getOutgoingMessage();

    MessageCenter* messageCenter;

private:

    void buttonClicked(Button* button);
    void timerCallback();
    bool acquisitionIsActive;

    bool isEnabled;
    bool isExpanded;

    String outgoingMessage;

    /** Called when a new message is received. */
    void actionListenerCallback(const String& message);

    /** A JUCE label used to display message text. */
    ScopedPointer<MessageLabel> incomingMessageDisplayArea;

    /** A JUCE label used to input message text. */
    ScopedPointer<MessageLabel> editableMessageDisplayArea;
    
    /** A JUCE button used to send messages. */
    ScopedPointer<Button> sendMessageButton;

    /** A log of all incoming messages. */
    ScopedPointer<MessageLog> incomingMessageLog;
    
    /** A log of all outgoing messages. */
    ScopedPointer<MessageLog> outgoingMessageLog;
    
    /** A viewport to hold the log all incoming messages. */
    ScopedPointer<Viewport> incomingMessageViewport;
    
    /** A viewport to hold the log of all outgoing messages. */
    ScopedPointer<Viewport> outgoingMessageViewport;
    
    Colour incomingBackground;
    Colour outgoingBackground;
    Colour backgroundColor;

    bool firstMessageReceived;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageCenterEditor);

};

class MessageLog : public Component
{
public:
    MessageLog(Viewport * vp);
    
    void addMessage(MessageLabel* message);

    int getDesiredHeight();
    
    void resized();
    
    void copyText();
    
private:
    OwnedArray<MessageLabel> messages;
    Viewport* viewport;
    int messageHeight;
    
    String getMessageNumberAsString();
};


class MessageLabel : public Label
{
public:
    MessageLabel(const String& componentName=String(), const String& labelText=String());
    
    String getTooltip();
    
    void prependText(String text);
    
private:
    String timestring;

};

#endif  // MESSAGECENTEREDITOR_H_INCLUDED
