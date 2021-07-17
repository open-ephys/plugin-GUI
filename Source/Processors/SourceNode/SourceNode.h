/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __SOURCENODE_H_DCE798F1__
#define __SOURCENODE_H_DCE798F1__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include "../DataThreads/DataThread.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../../UI/UIComponent.h"

#include "../Events/Event.h"

/**
  Creates and controls a DataThread for reading data from hardware devices

  @see GenericProcessor, SourceNodeEditor, DataThread
*/
class PLUGIN_API SourceNode : public GenericProcessor
                            , public Timer
{
public:
    /* Constructor */
    SourceNode (const String& name, DataThreadCreator dt);

    /* Destructor */
    ~SourceNode();

    /* Create a custom editor. */
    AudioProcessorEditor* createEditor() override;

    /* Copies samples from the DataThread's DataBuffer into the GUI's processing buffers. */
    void process (AudioBuffer<float>& buffer) override;

    /* Passes TEXT event messages to the DataThread, via handleMessage() */
    void handleEvent(const EventChannel* eventInfo, const EventPacket& packet, int sampleNum) override;

    /* Passes configuration messages to the DataThread, via handleConfigMessage() */
    String handleConfigMessage(String msg) override;

    /* Broadcasts a message from the DataThread to all other processors*/
    void broadcastDataThreadMessage(String msg);

    //void setParameter (int parameterIndex, float newValue) override;

    /* Saves custom parameters to the settings file*/
    void saveCustomParametersToXml (XmlElement* parentElement)  override;

    /* Loads custom parameters from the settings file*/
    void loadCustomParametersFromXml()                          override;

    /* Gets the sample rate for a particular subprocessor*/
    float getSampleRate(int subProcessorIdx = 0) const override;

    /* Gets the default sample rate*/
    float getDefaultSampleRate() const override;

    /* Allows the DataThread to update the signal chain*/
    void requestSignalChainUpdate();

	/* Registers this processor as a timestamp generator*/
	bool generatesTimestamps() const override;

    /* Starts the DataThread*/
    bool startAcquisition()   override;

    /* Stops the DataThread*/
    bool stopAcquisition()  override;

    /* Returns true if the DataThread found the source hardware*/
    bool isSourcePresent() const;

    /* Called by the DataThread to indicate that the connection to the source was lost*/
    void connectionLost();

    /* Returns a pointer to the DataThread*/
	DataThread* getThread() const;

    /* Enables editor after a connection to the data source is re-established*/
    bool tryEnablingEditor();

private:

    /* Periodically checks for a connection to the data source.*/
    void timerCallback() override;

    /* Passes channel configuration objects to the DataThread*/
    void updateSettings() override;

    /* Updates the size of the DataBuffers*/
    void resizeBuffers();

    /* Interval (in ms) for checking for the data source*/
    int sourceCheckInterval;

    bool wasDisabled;

    int numStreams;

    ScopedPointer<DataThread> dataThread;
    Array<DataBuffer*> inputBuffers;

    uint64 timestamp;

    OwnedArray<MemoryBlock> eventCodeBuffers;
	Array<uint64> eventStates;
	Array<EventChannel*> ttlChannels;

    int ttlState;
	

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNode);
};


#endif  // __SOURCENODE_H_DCE798F1__
