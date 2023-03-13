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

#include "../../TestableExport.h"
#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "../GenericProcessor/GenericProcessor.h"

class MessageCenterEditor;

/**

  Allows the application to display messages to the user.

  Also distributes broadcast messages to all plugins in the signal chain.

  The MessageCenter is located along the bottom left of the application window.

  @see UIComponent

*/

class TESTABLE MessageCenter : public GenericProcessor

{
public:

    /** Constructor */
    MessageCenter();

    /** Destructor */
    ~MessageCenter() { }

    /** Handle incoming data and decide which files and events to write to disk. */
    void process(AudioSampleBuffer& buffer) override;

    /** Called when new events arrive. */
    void setParameter(int parameterIndex, float newValue) override;

    /** Creates the MessageCenterEditor (located in the UI component). */
    AudioProcessorEditor* createEditor() override;

    /** A pointer to the Message Center editor. */
    ScopedPointer<MessageCenterEditor> messageCenterEditor;

    /** Returns a pointer to the Message Center event channel*/
    const EventChannel* getMessageChannel();

    /** Returns a pointer to the Message Center DataStream*/
    DataStream* getMessageStream();

    /** Informs the editor that acquisition has started*/
    bool startAcquisition() override;

    /** Informs the editor that acquisition has ended*/
    bool stopAcquisition() override;

    /** Creates the Message Center event channel*/
	void addSpecialProcessorChannels();
private:

    bool newEventAvailable;

    ScopedPointer<EventChannel> eventChannel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageCenter);

};



#endif  // __MESSAGECENTER_H_2695FC38__
