/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include <stdio.h>

class MessageCenter;

/**
    Displays information about a particular message
 */
class MessageLabel : public Label
{
public:
    /** Constructor */
    MessageLabel (const String& componentName = String(), const String& labelText = String());

    /** Returns tooltip */
    String getTooltip();

    /** Add text at the beginning of a string */
    void prependText (String text);

private:
    /** Holds info about the time the message was sent or received */
    String timestring;
};

/**
    Holds a list of incoming or outgoing messages
 */
class MessageLog : public Component
{
public:
    /** Constructor */
    MessageLog (Viewport* vp);

    /** Adds a new message to the log*/
    void addMessage (MessageLabel* message);

    /** Gets desired height (based on number of messages) */
    int getDesiredHeight();

    /** Called when log changes size */
    void resized();

    /** Copies the contents of the log to the clipboard */
    void copyText();

private:
    /** Array of message labels */
    OwnedArray<MessageLabel> messages;

    /** Allows scrolling through message history */
    Viewport* viewport;

    /** Holds total height of messages */
    int messageHeight;

    /** Returns a string for each message number */
    String getMessageNumberAsString();
};

/**
	Holds the interface for adding events to the message queue

	@see MessageCenter

*/

class MessageCenterEditor : public AudioProcessorEditor,
                            public Button::Listener,
                            public Timer

{
public:
    /** Constructor */
    MessageCenterEditor (MessageCenter* owner);

    /** Destructor */
    ~MessageCenterEditor();

    /** Renders the editor */
    void paint (Graphics& g);

    /** Called when UI changes size */
    void resized();

    /** Expands the editor */
    void expand();

    /** Collapses the editor */
    void collapse();

    /** Adds a message to the editor */
    void addMessage (const String& message);

    /** Gets the outgoing message text */
    String getOutgoingMessage();

    /** Pointer to the MessageCenter */
    MessageCenter* messageCenter;

private:
    /** Button callback */
    void buttonClicked (Button* button) override;

    /** Timer callback*/
    void timerCallback() override;

    /** A JUCE label used to display message text. */
    std::unique_ptr<MessageLabel> incomingMessageDisplayArea;

    /** A JUCE label used to input message text. */
    std::unique_ptr<MessageLabel> editableMessageDisplayArea;

    /** A JUCE button used to send messages. */
    std::unique_ptr<Button> sendMessageButton;

    /** A log of all incoming messages. */
    std::unique_ptr<MessageLog> incomingMessageLog;

    /** A log of all outgoing messages. */
    std::unique_ptr<MessageLog> outgoingMessageLog;

    /** A viewport to hold the log all incoming messages. */
    std::unique_ptr<Viewport> incomingMessageViewport;

    /** A viewport to hold the log of all outgoing messages. */
    std::unique_ptr<Viewport> outgoingMessageViewport;

    Colour incomingBackground;
    Colour outgoingBackground;
    Colour backgroundColor;

    bool firstMessageReceived = false;
    bool isExpanded = false;

    String outgoingMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageCenterEditor);
};

#endif // MESSAGECENTEREDITOR_H_INCLUDED
