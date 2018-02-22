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

/**
	Holds the interface for adding events to the message queue

	@see MessageCenter

*/

class MessageCenterEditor : public AudioProcessorEditor,
    public Button::Listener,
    public Label::Listener,
    public Timer,
    public ActionListener

{
public:
    MessageCenterEditor(MessageCenter* owner);
    ~MessageCenterEditor();

    void paint(Graphics& g);

    bool keyPressed(const KeyPress& key);

    void resized();

    void enable();
    void disable();

    void startAcquisition();
    void stopAcquisition();

    void messageReceived(bool state);

    String getLabelString();

private:

    void buttonClicked(Button* button);
    void labelTextChanged(Label* slider);
    void timerCallback();
    bool acquisitionIsActive;

    bool isEnabled;

    /** Called when a new message is received. */
    void actionListenerCallback(const String& message);

    /** A JUCE label used to display message text. */
    ScopedPointer<MessageLabel> incomingMessageDisplayArea;

    /** A JUCE label used to display message text. */
    ScopedPointer<MessageLabel> outgoingMessageDisplayArea;

    /** A JUCE button used to send messages. */
    ScopedPointer<Button> sendMessageButton;

    MessageCenter* messageCenter;

    Colour incomingBackground;
    Colour outgoingBackground;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageCenterEditor);

};

class MessageLabel : public Label
{
public:
    MessageLabel(const String& componentName=String::empty, const String& labelText=String::empty);
};

#endif  // MESSAGECENTEREDITOR_H_INCLUDED
