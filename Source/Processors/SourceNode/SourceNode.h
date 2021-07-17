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


/**
  Creates and controls a thread for reading data from external sources.

  @see GenericProcessor, SourceNodeEditor, DataThread, IntanThread
*/
class PLUGIN_API SourceNode : public GenericProcessor
                            , public Timer
                            , public ActionListener
{
public:
    SourceNode (const String& name, DataThreadCreator dt);
    ~SourceNode();

    void actionListenerCallback (const String& message) override;

    AudioProcessorEditor* createEditor() override;

    void setEnabledState (bool newState) override;

    void process (AudioBuffer<float>& buffer) override;

    void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int sampleNum) override;

    String handleConfigMessage(String msg) override;

    void broadcastDataThreadMessage(String msg);

    void setParameter (int parameterIndex, float newValue) override;

    void saveCustomParametersToXml (XmlElement* parentElement)  override;
    void loadCustomParametersFromXml()                          override;

    float getSampleRate(int subProcessorIdx = 0)        const override;
    float getDefaultSampleRate() const override;

    void requestChainUpdate();

	bool hasEditor() const override;

	bool generatesTimestamps() const override;

    bool startAcquisition()   override;
    bool stopAcquisition()  override;

    bool isSourcePresent() const;

    void acquisitionStopped();

	DataThread* getThread() const;

	int getTTLState() const;

    bool tryEnablingEditor();

	//void setChannelInfo(int channel, String name, float bitVolts);

private:
    void timerCallback() override;

    void updateSettings() override;

    int sourceCheckInterval;

    bool wasDisabled;

    ScopedPointer<DataThread> dataThread;
    Array<DataBuffer*> inputBuffers;

    uint64 timestamp;
    //uint64* eventCodeBuffer;
    //int* eventChannelState;
    OwnedArray<MemoryBlock> eventCodeBuffers;
	Array<uint64> eventStates;
	Array<EventChannel*> ttlChannels;

    int ttlState;
	void resizeBuffers();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNode);
};


#endif  // __SOURCENODE_H_DCE798F1__
