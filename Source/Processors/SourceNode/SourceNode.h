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

    void process (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    void setParameter (int parameterIndex, float newValue) override;

    void getEventChannelNames (StringArray& names) override;

    void saveCustomParametersToXml (XmlElement* parentElement)  override;
    void loadCustomParametersFromXml()                          override;

    float getSampleRate()        const override;
    float getDefaultSampleRate() const override;

    float getBitVolts (Channel* chan) const override;

    int getNumHeadstageOutputs() const override;
    int getNumAuxOutputs()       const override;
    int getNumAdcOutputs()       const override;

    int getNumEventChannels() const override;

    void requestChainUpdate();

    bool hasEditor() const override { return true; }

    bool isGeneratesTimestamps() const override { return true; }

    bool enable()   override;
    bool disable()  override;

    bool isReady() override;

    bool isSourcePresent() const;

    void acquisitionStopped();

    DataThread* getThread() const { return dataThread; }

    int getTTLState() const { return ttlState; }

    bool tryEnablingEditor();


private:
    void timerCallback() override;

    void updateSettings() override;

    int numEventChannels;
    int sourceCheckInterval;

    bool wasDisabled;

    ScopedPointer<DataThread> dataThread;
    DataBuffer* inputBuffer;

    uint64 timestamp;
    //uint64* eventCodeBuffer;
    //int* eventChannelState;
    HeapBlock<uint64> eventCodeBuffer;
    HeapBlock<int> eventChannelState;

    int ttlState;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNode);
};


#endif  // __SOURCENODE_H_DCE798F1__
