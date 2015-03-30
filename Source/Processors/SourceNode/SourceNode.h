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

class SourceNode : public GenericProcessor,
    public Timer,
    public ActionListener

{
public:

    // real member functions:
    SourceNode(const String& name);
    ~SourceNode();

    void enabledState(bool t);

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    void setParameter(int parameterIndex, float newValue);

    float getSampleRate();
    float getDefaultSampleRate();
    int getNumHeadstageOutputs();

    int getNumAuxOutputs();
    int getNumAdcOutputs();

    int getNumEventChannels();
    float getBitVolts(Channel* chan);

    void requestChainUpdate();

    void getEventChannelNames(StringArray& names);

    AudioProcessorEditor* createEditor();
    bool hasEditor() const
    {
        return true;
    }

    bool enable();
    bool disable();

    bool isReady();
	bool sourcePresent();

    bool isSource()
    {
        return true;
    }

    bool generatesTimestamps()
    {
        return true;
    }

    void acquisitionStopped();

    DataThread* getThread();

    void actionListenerCallback(const String& message);

    int getTTLState();

    void saveCustomParametersToXml(XmlElement* parentElement);
    void loadCustomParametersFromXml();

    bool tryEnablingEditor();

private:

    int numEventChannels;

    int sourceCheckInterval;

    bool wasDisabled;

    void timerCallback();

    ScopedPointer<DataThread> dataThread;
    DataBuffer* inputBuffer;

    uint64 timestamp;
    uint64* eventCodeBuffer;
    int* eventChannelState;

    int ttlState;

    void updateSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceNode);

};


#endif  // __SOURCENODE_H_DCE798F1__
