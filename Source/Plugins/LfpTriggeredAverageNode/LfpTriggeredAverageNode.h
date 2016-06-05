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

#ifndef __LFPTRIGAVGNODE_H_D969A379__
#define __LFPTRIGAVGNODE_H_D969A379__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "LfpTriggeredAverageEditor.h"
#include "../Editors/VisualizerEditor.h"
#include "../GenericProcessor/GenericProcessor.h"

class DataViewport;

/**
    Displays the average of a continuous signal, triggered on a certain event channel.

    @see GenericProcessor, LfpTriggeredAverageEditor, LfpDisplayCanvas
*/
class LfpTriggeredAverageNode :  public GenericProcessor
{
public:
    LfpTriggeredAverageNode();
    ~LfpTriggeredAverageNode();

    AudioProcessorEditor* createEditor() override;

    void process (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    void setParameter (int parameterIndex, float newValue) override;

    void updateSettings() override;

    bool enable()   override;
    bool disable()  override;

    void handleEvent (int, MidiMessage&) override;

    AudioSampleBuffer* getDisplayBufferAddress() const { return displayBuffer; }
    int getDisplayBufferIndex() const { return displayBufferIndex; }


private:
    void initializeEventChannel();

    ScopedPointer<AudioSampleBuffer> displayBuffer;
    ScopedPointer<MidiBuffer> eventBuffer;

    int displayBufferIndex;
    int displayBufferIndexEvents;

    float displayGain; //
    float bufferLength; // s

    AbstractFifo abstractFifo;

    int64 bufferTimestamp;
    int ttlState;
    float* arrayOfOnes;
    int totalSamples;

    //Time timer;

    bool resizeBuffer();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpTriggeredAverageNode);
};


#endif  // __LFPTRIGAVGNODE_H_D969A379__
