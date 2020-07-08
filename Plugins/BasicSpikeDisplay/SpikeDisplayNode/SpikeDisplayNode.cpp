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

#include "SpikeDisplayNode.h"
#include "SpikeDisplayCanvas.h"

#include <stdio.h>


SpikeDisplayNode::SpikeDisplayNode()
    : GenericProcessor  ("Spike Viewer")
    , displayBufferSize (5)
    ,  redrawRequested  (false)
    , isRecording       (false)
{
    setProcessorType (PROCESSOR_TYPE_SINK);
}


SpikeDisplayNode::~SpikeDisplayNode()
{
}


AudioProcessorEditor* SpikeDisplayNode::createEditor()
{
    editor = new SpikeDisplayEditor (this);
    return editor;
}


void SpikeDisplayNode::updateSettings()
{
    //std::cout << "Setting num inputs on SpikeDisplayNode to " << getNumInputs() << std::endl;

    electrodes.clear();
	for (int i = 0; i < spikeChannelArray.size(); ++i)
	{

		Electrode* elec = new Electrode();
		elec->numChannels = spikeChannelArray[i]->getNumChannels();
		elec->bitVolts = spikeChannelArray[i]->getChannelBitVolts(0); //lets assume all channels have the same bitvolts
		elec->name = spikeChannelArray[i]->getName();
		elec->currentSpikeIndex = 0;
		elec->mostRecentSpikes.ensureStorageAllocated(displayBufferSize);

		for (int j = 0; j < elec->numChannels; ++j)
		{
			elec->displayThresholds.add(0);
			elec->detectorThresholds.add(0);
		}

		electrodes.add(elec);

	}
}


bool SpikeDisplayNode::enable()
{
    std::cout << "SpikeDisplayNode::enable()" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();

	//CoreServices::RecordNode::registerSpikeSource(this);
	for (int i = 0; i < spikeChannelArray.size(); i ++)
	{
		Electrode* elec = electrodes[i];
		//elec->recordIndex = CoreServices::RecordNode::addSpikeElectrode(spikeChannelArray[i]);
	}

    editor->enable();
    return true;
}


bool SpikeDisplayNode::disable()
{
    std::cout << "SpikeDisplayNode disabled!" << std::endl;

    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->disable();

    return true;
}


int SpikeDisplayNode::getNumberOfChannelsForElectrode (int i) const
{
    if (i > -1 && i < electrodes.size())
    {
        return electrodes[i]->numChannels;
    }
    else
    {
        return 0;
    }
}


String SpikeDisplayNode::getNameForElectrode (int i) const
{
    if (i > -1 && i < electrodes.size())
    {
        return electrodes[i]->name;
    }
    else
    {
        return " ";
    }
}


void SpikeDisplayNode::addSpikePlotForElectrode (SpikePlot* sp, int i)
{
    Electrode* e = electrodes[i];
    e->spikePlot = sp;
}


void SpikeDisplayNode::removeSpikePlots()
{
    for (int i = 0; i < getNumElectrodes(); ++i)
    {
        Electrode* e = electrodes[i];
        e->spikePlot = nullptr;
    }
}


int SpikeDisplayNode::getNumElectrodes() const
{
    return electrodes.size();
}


void SpikeDisplayNode::startRecording()
{
    setParameter (1, 0.0f); // need to use the 'setParameter' method to interact with 'process'
}


void SpikeDisplayNode::stopRecording()
{
    setParameter (0, 0.0f); // need to use the 'setParameter' method to interact with 'process'
}


void SpikeDisplayNode::setParameter (int param, float val)
{
    //std::cout<<"SpikeDisplayNode got Param:"<< param<< " with value:"<<val<<std::endl;

    if (param == 0) // stop recording
    {
        isRecording = false;
    }
    else if (param == 1)   // start recording
    {
        isRecording = true;
    }
    else if (param == 2)   // redraw
    {
        redrawRequested = true;
    }
}


void SpikeDisplayNode::process (AudioSampleBuffer& buffer)
{
    checkForEvents (true); // automatically calls 'handleEvent

    if (redrawRequested)
    {
        // update incoming thresholds
        for (int i = 0; i < getNumElectrodes(); ++i)
        {
            Electrode* e = electrodes[i];

            // update thresholds
            for (int j = 0; j < e->numChannels; ++j)
            {
                e->displayThresholds.set (j,
                                         e->spikePlot->getDisplayThresholdForChannel (j));

                e->spikePlot->setDetectorThresholdForChannel (j, e->detectorThresholds[j]);
            }

            // transfer buffered spikes to spike plot
            for (int j = 0; j < e->currentSpikeIndex; ++j)
            {
                //std::cout << "Transferring spikes." << std::endl;
                e->spikePlot->processSpikeObject (e->mostRecentSpikes[j]);
                e->currentSpikeIndex = 0;
            }
        }

        redrawRequested = false;
    }
}


void SpikeDisplayNode::handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition)
{
	SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
	if (!newSpike) return;

	int electrodeNum = getSpikeChannelIndex(newSpike);

	Electrode* e = electrodes[electrodeNum];
	// std::cout << electrodeNum << std::endl;

	bool aboveThreshold = false;

	// update threshold / check threshold
	for (int i = 0; i < e->numChannels; ++i)
	{
		e->detectorThresholds.set(i, float(newSpike->getThreshold(i))); // / float(newSpike.gain[i]));

		aboveThreshold = aboveThreshold | checkThreshold(i, e->displayThresholds[i], newSpike);
	}

	if (aboveThreshold)
	{
		// save spike
		if (false)
		{
			//CoreServices::RecordNode::writeSpike(newSpike, spikeInfo);
		}
		// add to buffer
		if (e->currentSpikeIndex < displayBufferSize)
		{
			//This releases the spike from the smart pointer to avoid copies, so it's done latest.
			e->mostRecentSpikes.set(e->currentSpikeIndex, newSpike.release());
			e->currentSpikeIndex++;
		}

		
	}
}


bool SpikeDisplayNode::checkThreshold (int chan, float thresh, SpikeEvent* s)
{
	int nSamples = getSpikeChannel(getSpikeChannelIndex(s))->getTotalSamples();

    for (int i = 0; i < nSamples-1; ++i)
    {
        if  (s->getDataPointer(chan)[i]  > thresh)
        {
            return true;
        }
    }

    return false;
}
