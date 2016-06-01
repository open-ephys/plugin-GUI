
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
    : GenericProcessor("Spike Raster"), redrawRequested(false), displayBufferSize(100)

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

    for (int i = 0; i < eventChannels.size(); i++)
    {
        ChannelType type = eventChannels[i]->getType();

        if (type == ELECTRODE_CHANNEL)
        {

            Electrode elec;
			elec.numChannels = static_cast<SpikeChannel*>(eventChannels[i]->extraData.get())->numChannels;

            elec.name = eventChannels[i]->getName();
            elec.currentSpikeIndex = 0;
            elec.mostRecentSpikes.ensureStorageAllocated(displayBufferSize);

            for (int j = 0; j < elec.numChannels; j++)
            {
                elec.displayThresholds.add(0);
                elec.detectorThresholds.add(0);
            }

            electrodes.add(elec);

        }
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
	r->setSampleRate(settings.sampleRate);
}

void SpikeRaster::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{

    checkForEvents(events); // automatically calls 'handleEvent

    if (redrawRequested)
    {
    	canvas->setTimestamp(getTimestamp(0));

    	//std::cout << "redraw" << std::endl;
        for (int i = 0; i < getNumElectrodes(); i++)
        {

            Electrode& e = electrodes.getReference(i);

            // transfer buffered spikes to spike plot
            for (int j = 0; j < e.currentSpikeIndex; j++)
            {
                //std::cout << "Transferring spikes." << std::endl;
                canvas->processSpikeObject(e.mostRecentSpikes[j]);
                e.currentSpikeIndex = 0;
            }

        }

        //redrawRequested = true;
    }



}

void SpikeRaster::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{

    //std::cout << "Received event of type " << eventType << std::endl;

    if (eventType == SPIKE)
    {

        const uint8_t* dataptr = event.getRawData();
        int bufferSize = event.getRawDataSize();

        if (bufferSize > 0)
        {

            SpikeObject newSpike;

            bool isValid = unpackSpike(&newSpike, dataptr, bufferSize);

            if (isValid)
            {
                int electrodeNum = newSpike.source;

                Electrode& e = electrodes.getReference(electrodeNum);
                // std::cout << electrodeNum << std::endl;

                // add to buffer
                if (e.currentSpikeIndex < displayBufferSize)
                {
                    //  std::cout << "Adding spike " << e.currentSpikeIndex + 1 << std::endl;
                    e.mostRecentSpikes.set(e.currentSpikeIndex, newSpike);
                    e.currentSpikeIndex++;
                }

            }

        }

    } else if (eventType == TTL)
    {
    	const uint8* dataptr = event.getRawData();

        //int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);
        uint8 sourceNodeId = event.getNoteNumber();

		int64 timestamp = timestamps[sourceNodeId] + samplePosition;

        if (eventId == 1)
        {
        	canvas->processEvent(eventChannel, timestamp);
        }
    }

}
