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

#ifndef __MESSAGECENTER_H_2695FC38__
#define __MESSAGECENTER_H_2695FC38__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../TestableExport.h"
#include <queue>
#include <stdio.h>

#include "../GenericProcessor/GenericProcessor.h"
#include "MessageCenterEditor.h"

/**

  Allows the application to display messages to the user.

  Also distributes broadcast messages to all plugins in the signal chain.

  The MessageCenter is located along the bottom left of the application window.

  @see UIComponent

*/

class TESTABLE MessageCenter : public GenericProcessor,
                               public ActionListener

{
public:
    /** Constructor */
    MessageCenter();

    /** Destructor */
    ~MessageCenter() {}

    /** Handle incoming data and decide which files and events to write to disk. */
    void process (AudioBuffer<float>& buffer) override;

    /** Called when new events arrive. */
    void setParameter (int parameterIndex, float newValue) override;

    /** Creates the MessageCenterEditor (located in the UI component). */
    AudioProcessorEditor* createEditor() override;

    /** Enables the "send" button */
    bool startAcquisition() override;

    /** Disables the "send" button */
    bool stopAcquisition() override;

    /** Returns a pointer to the Message Center event channel*/
    const EventChannel* getMessageChannel();

    /** Returns a pointer to the Message Center DataStream*/
    DataStream* getMessageStream();

    /** Creates the Message Center event channel*/
    void addSpecialProcessorChannels();

    /** Called when a new message is received. */
    void actionListenerCallback (const String& message);

    /** Sends a broadcast message to all processors */
    void broadcastMessage (const String& msg);

    /** Sends a broadcast message to all processors for a specified system time */
    void broadcastMessage (const String& msg, const int64 systemTimeMilliseconds);

    /** Sends a broadcast message and adds it to the editor */
    void addOutgoingMessage (const String& msg, const int64 systemTimeMilliseconds);

    /** Saves a message for later use */
    void addSavedMessage (const String& msg);

    /** Clears all saved messages */
    void clearSavedMessages();

    /** Returns messages that have been sent previously */
    Array<String>& getSavedMessages();

    /** Stores saved messages in settings file */
    void saveStateToXml (XmlElement* xml);

    /** Loads saved messages from settings file */
    void loadStateFromXml (XmlElement* xml);

private:
    /** A pointer to the Message Center editor. */
    ScopedPointer<MessageCenterEditor> messageCenterEditor;

    /** Holds a message string, plus the system time at
    which the message was created */
    struct Message
    {
        String message;
        int64 systemTimeMilliseconds;
    };

    /** Holds incoming messages */
    std::queue<Message> messageQueue;

    bool newEventAvailable;

    ScopedPointer<EventChannel> eventChannel;

    Array<String> savedMessages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageCenter);
};

#endif // __MESSAGECENTER_H_2695FC38__
