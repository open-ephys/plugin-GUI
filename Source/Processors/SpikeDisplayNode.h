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

#ifndef SPIKEDISPLAYNODE_H_
#define SPIKEDISPLAYNODE_H_

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/SpikeDisplayEditor.h"
#include "Editors/VisualizerEditor.h"
#include "GenericProcessor.h"
#include "Visualization/SpikeObject.h"

class DataViewport;

/**

 Takes in MidiEvents and extracts SpikeObjects from the MidiEvent buffers.
 Those Events are then held in a queue until they are pulled by the SpikeDisplayCanvas.

  @see GenericProcessor, SpikeDisplayEditor, SpikeDisplayCanvas

*/

class SpikeDisplayNode :  public GenericProcessor
{
public:

    SpikeDisplayNode();
    ~SpikeDisplayNode();

    AudioProcessorEditor* createEditor();

    bool isSink()
    {
        return true;
    }

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);

    void setParameter(int, float);

    void handleEvent(int, MidiMessage&, int);

    //void updateSettings();

    bool enable();
    bool disable();
    
    void startRecording();
    void stopRecording();

    MidiBuffer* getSpikeBufferAddress()
    {
        return eventBuffer;
    }

    String getNameForElectrode(int i);
    int getNumberOfChannelsForElectrode(int i);
    int getNumElectrodes();

private:

    int numberOfSources;

    ScopedPointer<MidiBuffer> eventBuffer;

    int bufferSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayNode);

};


#endif  // SPIKEDISPLAYNODE_H_
