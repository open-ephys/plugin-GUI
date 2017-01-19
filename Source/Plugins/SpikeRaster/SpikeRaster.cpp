
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


#include <stdio.h>
#include "SpikeRaster.h"

#include "SpikeRasterEditor.h"

SpikeRaster::SpikeRaster()
    : GenericProcessor("Spike Raster"), displayBufferSize(100), redrawRequested(false)

{
	//Without a custom editor, generic parameter controls can be added
    //parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

    electrodes.clear();

}

SpikeRaster::~SpikeRaster()
{

}

/**
	If the processor uses a custom editor, this method must be present.
*/

AudioProcessorEditor* SpikeRaster::createEditor()
{
	editor = new SpikeRasterEditor(this, true);

	//std::cout << "Creating editor." << std::endl;

	return editor;
}

void SpikeRaster::updateSettings()
{
    //std::cout << "Setting num inputs on SpikeDisplayNode to " << getNumInputs() << std::endl;

    electrodes.clear();

	for (int i = 0; i < spikeChannelArray.size(); i++)
	{

		Electrode* elec = new Electrode();
		elec->numChannels = spikeChannelArray[i]->getNumChannels();

		elec->name = spikeChannelArray[i]->getName();
		elec->currentSpikeIndex = 0;
		elec->mostRecentSpikes.ensureStorageAllocated(displayBufferSize);

		for (int j = 0; j < elec->numChannels; j++)
		{
			elec->displayThresholds.add(0);
			elec->detectorThresholds.add(0);
		}

		electrodes.add(elec);

	}

}


void SpikeRaster::setParameter(int param, float newValue)
{


    if (param == 2)   // redraw
    {
        redrawRequested = true;

    }
}

bool SpikeRaster::enable()
{
    std::cout << "SpikeRaster::enable()" << std::endl;
    SpikeRasterEditor* editor = (SpikeRasterEditor*) getEditor();

    editor->enable();
    return true;

}

bool SpikeRaster::disable()
{
    std::cout << "SpikeRaster disabled!" << std::endl;
    SpikeRasterEditor* editor = (SpikeRasterEditor*) getEditor();
    editor->disable();
    return true;
}

int SpikeRaster::getNumElectrodes()
{
	return electrodes.size();
}

void SpikeRaster::setRasterPlot(RasterPlot* r)
{
	canvas = r;
	//A bit of a hack since a processor doesn't actually have a sample rate unless it is a source one
	r->setSampleRate(dataChannelArray[0]->getSampleRate());
}

void SpikeRaster::process(AudioSampleBuffer& buffer)
{

    checkForEvents(true); // automatically calls 'handleEvent

    if (redrawRequested)
    {
    	canvas->setTimestamp(getTimestamp(0));

    	//std::cout << "redraw" << std::endl;
        for (int i = 0; i < getNumElectrodes(); i++)
        {

            Electrode* e = electrodes[i];

            // transfer buffered spikes to spike plot
            for (int j = 0; j < e->currentSpikeIndex; j++)
            {
                //std::cout << "Transferring spikes." << std::endl;
                canvas->processSpikeObject(e->mostRecentSpikes[j]);
                e->currentSpikeIndex = 0;
            }

        }

        //redrawRequested = true;
    }



}

void SpikeRaster::handleSpike(const SpikeChannel* channelInfo, const MidiMessage& event, int samplePosition)
{

	//std::cout << "Received event of type " << eventType << std::endl;


	SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, channelInfo);

	if (newSpike > 0)
	{


		int electrodeNum = spikeChannelArray.indexOf(channelInfo);
		if (electrodeNum < 0) return;

		Electrode* e = electrodes[electrodeNum];
		// std::cout << electrodeNum << std::endl;

		// add to buffer
		if (e->currentSpikeIndex < displayBufferSize)
		{
			//  std::cout << "Adding spike " << e.currentSpikeIndex + 1 << std::endl;
			e->mostRecentSpikes.set(e->currentSpikeIndex, newSpike.release());
			e->currentSpikeIndex++;
		}

	}

}


void SpikeRaster::handleEvent(const EventChannel* channelInfo, const MidiMessage& event, int samplePosition)
{
	if (Event::getEventType(event) == EventChannel::TTL)
	{
		TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, channelInfo);
		int64 timestamp = getSourceTimestamp(ttl->getSourceID(), ttl->getSubProcessorIdx()) + samplePosition;

		if (ttl->getState())
		{
			canvas->processEvent(ttl->getChannel(), timestamp);
		}
	}
}