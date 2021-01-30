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

#ifndef __LFPDISPLAYNODE_H_Alpha__
#define __LFPDISPLAYNODE_H_Alpha__

#include <ProcessorHeaders.h>
#include "LfpDisplayEditor.h"

#include <map>

class DataViewport;

namespace LfpViewer
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

    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int samplePosition = 0) override;

    std::shared_ptr<AudioSampleBuffer> getDisplayBufferAddress() const;

    int getDisplayBufferIndex(int chan) const;

    CriticalSection* getMutex() { return &displayMutex; }

    void setSubprocessor(uint32 sp);
    uint32 getSubprocessor() const;

    int getNumSubprocessorChannels();

    float getSubprocessorSampleRate(uint32 subprocId);

    uint32 getDataSubprocId(int chan) const;

    void setTriggerSource(int ch);
    int getTriggerSource() const;
    int64 getLatestTriggerTime() const;
    void acknowledgeTrigger();
private:
    void initializeEventChannels();
    void finalizeEventChannels();

    std::vector<std::shared_ptr<AudioSampleBuffer>> displayBuffers;

    std::vector<std::vector<int>> displayBufferIndices;
    Array<int> channelIndices;

    Array<uint32> eventSourceNodes;

    float displayGain; //
    float bufferLength; // s

    AbstractFifo abstractFifo;

    int64 bufferTimestamp;
    std::map<uint32, uint64> ttlState;
    float* arrayOfOnes;
    int totalSamples;
    int triggerSource;
    int64 latestTrigger; // overall timestamp
    int latestCurrentTrigger; // within current input buffer
 

    bool resizeBuffer();

    int numSubprocessors;
    uint32 subprocessorToDraw;
    SortedSet<uint32> allSubprocessors; 
    std::map<uint32, int> numChannelsInSubprocessor;
    std::map<uint32, float> subprocessorSampleRate;

    CriticalSection displayMutex;

    static uint32 getEventSourceId(const EventChannel* event);
    static uint32 getChannelSourceId(const InfoObjectCommon* chan);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayNode);
};
};



#endif  // __LFPDISPLAYNODE_H_Alpha__
