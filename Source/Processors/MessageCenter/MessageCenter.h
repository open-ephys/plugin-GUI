/*
    -----------------------------------------------------------------

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

#ifndef __MESSAGECENTER_H_2695FC38__
#define __MESSAGECENTER_H_2695FC38__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "../GenericProcessor/GenericProcessor.h"

class MessageCenterEditor;

/**

  Allows the application to display messages to the user.

  The MessageCenter is located along the bottom left of the application window.

  @see UIComponent

*/

class MessageCenter : public GenericProcessor

{
public:
    MessageCenter();
    ~MessageCenter();

    /** Handle incoming data and decide which files and events to write to disk. */
    void process(AudioSampleBuffer& buffer, MidiBuffer& eventBuffer);

    /** Called when new events arrive. */
    void setParameter(int parameterIndex, float newValue);

    /** Creates the MessageCenterEditor (located in the UI component). */
    AudioProcessorEditor* createEditor();

    /** A pointer to the Message Center editor. */
    ScopedPointer<MessageCenterEditor> messageCenterEditor;

    bool enable();
    bool disable();

    void startRecording()
    {
        isRecording = true;
        needsToSendTimestampMessage = true;
    }
    void stopRecording()
    {
        isRecording = false;
        needsToSendTimestampMessage = false;
    }

    void setSourceNodeId(int id);
    int getSourceNodeId();

    void addSourceProcessor(GenericProcessor* p);
    void removeSourceProcessor(GenericProcessor* p);

    int64 getTimestamp(bool softwareTime = false);
private:

    bool newEventAvailable;
    bool isRecording;
    int sourceNodeId;
    GenericProcessor* timestampSource;
    int64 lastTime, softTimestamp;
    bool needsToSendTimestampMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageCenter);

};



#endif  // __MESSAGECENTER_H_2695FC38__
