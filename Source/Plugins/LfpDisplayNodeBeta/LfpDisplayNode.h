/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __LFPDISPLAYNODE_H_D969A379__
#define __LFPDISPLAYNODE_H_D969A379__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "LfpDisplayEditor.h"
#include "../../Processors/Editors/VisualizerEditor.h"
#include "../../Processors/GenericProcessor/GenericProcessor.h"


class DataViewport;

namespace LfpDisplayNodeBeta { 
    
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

    AudioProcessorEditor* createEditor();

    bool isSink()
    {
        return true;
    }

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    void setParameter(int, float);

    void updateSettings();

    bool enable();
    bool disable();

    void handleEvent(int, MidiMessage&, int);

    AudioSampleBuffer* getDisplayBufferAddress()
    {
        return displayBuffer;
    }
    int getDisplayBufferIndex(int chan)
    {
        return displayBufferIndex[chan];
    }

	CriticalSection* getMutex()
	{
		return &displayMutex;
	}

private:

    void initializeEventChannels();

    ScopedPointer<AudioSampleBuffer> displayBuffer;

    Array<int> displayBufferIndex;
    Array<int> eventSourceNodes;
    std::map<int, int> channelForEventSource;

    int numEventChannels;

    float displayGain; //
    float bufferLength; // s

    AbstractFifo abstractFifo;

    int64 bufferTimestamp;
    std::map<int, int> ttlState;
    float* arrayOfOnes;
    int totalSamples;

    bool resizeBuffer();

	CriticalSection displayMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayNode);

};
};



#endif  // __LFPDISPLAYNODE_H_D969A379__
