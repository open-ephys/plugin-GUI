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

    /** Creates the editor */
    AudioProcessorEditor* createEditor() override;

    /** Sends incoming spikes to the SpikeDisplayCanvas */
    void process (AudioBuffer<float>& buffer) override;

    /** Informs the SpikeDisplayNode when a redraw is needed*/
    void setParameter(int, float) override;

    /** Called for each incoming spike*/
	void handleSpike(SpikePtr spike) override;

    /** Creates a display for each incoming spike channel*/
    void updateSettings() override;

    /** Starts animation*/
    bool startAcquisition()   override;

    /** Stops animation*/
    bool stopAcquisition()  override;

    /** Returns the name of an electrode for a given index*/
    String getNameForElectrode (int i) const;

    /** Returns the channel count for an electrode*/
    int getNumberOfChannelsForElectrode (int i) const;

    /** Returns the total number of available electrodes*/
    int getNumElectrodes() const;

    /** Sets the corresponding spike plot for an electrode*/
    void addSpikePlotForElectrode (SpikePlot* sp, int i);

    /** Removes pointers to SpikePlot objects*/
    void removeSpikePlots();

private:
    
    struct Electrode
    {
        String name;

        int numChannels;

        SpikePlot* spikePlot;
        SpikeChannel* spikeChannel;
    };

    OwnedArray<Electrode> electrodes;

    std::map<const SpikeChannel*, SpikePlot*> electrodeMap;

    int spikeCount;
    int totalCallbacks;

    int displayBufferSize;

    bool redrawRequested;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayNode);
};


#endif  // SPIKEDISPLAYNODE_H_
