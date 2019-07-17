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

#ifndef __LFPDISPLAYNODE_H_BETA__
#define __LFPDISPLAYNODE_H_BETA__

#include <ProcessorHeaders.h>
#include "LfpDisplayEditor.h"


class DataViewport;

namespace LfpDisplayNodeBeta 
{

/**

  Holds data in a displayBuffer to be used by the LfpDisplayCanvas
  for rendering continuous data streams.

  @see GenericProcessor, LfpDisplayEditor, LfpDisplayCanvas

*/
class LfpDisplayNode :  public GenericProcessor

{
public:
    LfpDisplayNode();
    ~LfpDisplayNode();

    AudioProcessorEditor* createEditor() override;

    void process (AudioSampleBuffer& buffer) override;

    void setParameter (int parameterIndex, float newValue) override;

    void updateSettings() override;

    bool enable()   override;
    bool disable()  override;

	void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition = 0) override;

    AudioSampleBuffer* getDisplayBufferAddress() const { return displayBuffer; }

    int getDisplayBufferIndex (int chan) const { return displayBufferIndex[chan]; }

    CriticalSection* getMutex() { return &displayMutex; }


private:
    void initializeEventChannels();

    ScopedPointer<AudioSampleBuffer> displayBuffer;

    Array<int> displayBufferIndex;
    Array<uint32> eventSourceNodes;
    std::map<uint32, int> channelForEventSource;

    int numEventChannels;

    float displayGain; //
    float bufferLength; // s

    AbstractFifo abstractFifo;

    int64 bufferTimestamp;
    std::map<uint32, uint64> ttlState;
    float* arrayOfOnes;
    int totalSamples;

    bool resizeBuffer();

    CriticalSection displayMutex;

	uint32 getChannelSourceID(const EventChannel* event) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayNode);
};
};



#endif  // __LFPDISPLAYNODE_H_BETA__
