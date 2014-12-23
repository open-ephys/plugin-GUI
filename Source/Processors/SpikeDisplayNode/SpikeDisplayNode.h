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

#ifndef SPIKEDISPLAYNODE_H_
#define SPIKEDISPLAYNODE_H_

#include "../../JuceLibraryCode/JuceHeader.h"
#include "SpikeDisplayEditor.h"
#include "../Editors/VisualizerEditor.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Visualization/SpikeObject.h"

class DataViewport;
class SpikePlot;

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

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    void setParameter(int, float);

    void handleEvent(int, MidiMessage&, int);

    void updateSettings();

    bool enable();
    bool disable();

    void startRecording();
    void stopRecording();

    String getNameForElectrode(int i);
    int getNumberOfChannelsForElectrode(int i);
    int getNumElectrodes();

    void addSpikePlotForElectrode(SpikePlot* sp, int i);
    void removeSpikePlots();

    bool checkThreshold(int, float, SpikeObject&);

private:

    struct Electrode
    {
        String name;

        int numChannels;

        Array<float> displayThresholds;
        Array<float> detectorThresholds;

        Array<SpikeObject> mostRecentSpikes;
        int currentSpikeIndex;

        SpikePlot* spikePlot;

        int recordIndex;

    };

    Array<Electrode> electrodes;

    int displayBufferSize;
    bool redrawRequested;

    // members for recording
    bool isRecording;
 //   bool signalFilesShouldClose;
 //   RecordNode* recordNode;
 //   String baseDirectory;
 //   File dataDirectory;
 //   uint8_t* spikeBuffer;
 //   SpikeObject currentSpike;

 //   uint16 recordingNumber;

//    CriticalSection* diskWriteLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayNode);

};


#endif  // SPIKEDISPLAYNODE_H_
