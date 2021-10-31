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
  Looks for incoming Spike events and draws them to the SpikeDisplayCanvas.

  @see GenericProcessor, SpikeDisplayEditor, SpikeDisplayCanvas
*/
class SpikeDisplayNode :  public GenericProcessor
{
public:
    /** Constructor*/
    SpikeDisplayNode();
    
    /** Destructor*/
    ~SpikeDisplayNode() {}

    AudioProcessorEditor* createEditor() override;

    void process (AudioSampleBuffer& buffer) override;

    void setParameter (int parameterIndex, float newValue) override;

	void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;

    void updateSettings() override;

    bool startAcquisition()   override;
    bool stopAcquisition()  override;

    void startRecording()   override;
    void stopRecording()    override;

    String getNameForElectrode (int i) const;
    int getNumberOfChannelsForElectrode (int i) const;
    int getNumElectrodes() const;

    void addSpikePlotForElectrode (SpikePlot* sp, int i);
    void removeSpikePlots();

    bool checkThreshold (int, float, Spike*);


private:
    struct Electrode
    {
        String name;

        int numChannels;
        int recordIndex;
        int currentSpikeIndex;

        Array<float> displayThresholds;
        Array<float> detectorThresholds;

        OwnedArray<Spike> mostRecentSpikes;

		float bitVolts;

        SpikePlot* spikePlot;
    };

    OwnedArray<Electrode> electrodes;

    int displayBufferSize;
    bool redrawRequested;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayNode);
};


#endif  // SPIKEDISPLAYNODE_H_
