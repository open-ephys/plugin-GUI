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

#ifndef SPIKEDISPLAYNODE_H_
#define SPIKEDISPLAYNODE_H_

#include <ProcessorHeaders.h>
#include "SpikeDisplayEditor.h"

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

    AudioProcessorEditor* createEditor() override;

    void process (AudioSampleBuffer& buffer) override;

    void setParameter (int parameterIndex, float newValue) override;

	void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;

    void updateSettings() override;

    bool enable()   override;
    bool disable()  override;

    void startRecording()   override;
    void stopRecording()    override;

    String getNameForElectrode (int i) const;
    int getNumberOfChannelsForElectrode (int i) const;
    int getNumElectrodes() const;

    void addSpikePlotForElectrode (SpikePlot* sp, int i);
    void removeSpikePlots();

    bool checkThreshold (int, float, SpikeEvent*);


private:
    struct Electrode
    {
        String name;

        int numChannels;
        int recordIndex;
        int currentSpikeIndex;

        Array<float> displayThresholds;
        Array<float> detectorThresholds;

        OwnedArray<SpikeEvent> mostRecentSpikes;

		float bitVolts;

        SpikePlot* spikePlot;
    };

    OwnedArray<Electrode> electrodes;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayNode);
};


#endif  // SPIKEDISPLAYNODE_H_
