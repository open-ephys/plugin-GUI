
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
#include "RFMapper.h"

//If the processor uses a custom editor, it needs its header to instantiate it
#include "RFMapperEditor.h"

RFMapper::RFMapper()
    : GenericProcessor("RF Mapper"), redrawRequested(false), displayBufferSize(100)

{
	//Without a custom editor, generic parameter controls can be added
    //parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

    electrodes.clear();

}

RFMapper::~RFMapper()
{

}

/**
	If the processor uses a custom editor, this method must be present.
*/

AudioProcessorEditor* RFMapper::createEditor()
{
	editor = new RFMapperEditor(this, true);

	//std::cout << "Creating editor." << std::endl;

	return editor;
}

void RFMapper::updateSettings()
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


void RFMapper::setParameter(int param, float newValue)
{

    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);

    //threshold = newValue;

    //std::cout << float(p[0]) << std::endl;
    if (param == 2)   // redraw
    {
        redrawRequested = true;

    }
}

bool RFMapper::enable()
{
    std::cout << "RFMapper::enable()" << std::endl;
    RFMapperEditor* editor = (RFMapperEditor*) getEditor();

    editor->enable();
    return true;

}

bool RFMapper::disable()
{
    std::cout << "RFMapper disabled!" << std::endl;
    RFMapperEditor* editor = (RFMapperEditor*) getEditor();
    editor->disable();
    return true;
}

int RFMapper::getNumElectrodes()
{
	return electrodes.size();
}

void RFMapper::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{

    checkForEvents(events); // automatically calls 'handleEvent

    if (redrawRequested)
    {
    	//std::cout << "redraw" << std::endl;
        for (int i = 0; i < getNumElectrodes(); i++)
        {

            Electrode& e = electrodes.getReference(i);

            // transfer buffered spikes to spike plot
            for (int j = 0; j < e.currentSpikeIndex; j++)
            {
                //std::cout << "Transferring spikes." << std::endl;
                e.rfMap->processSpikeObject(e.mostRecentSpikes[j]);
                e.currentSpikeIndex = 0;
            }

        }

        redrawRequested = false;
    }

}

void RFMapper::addRFMapForElectrode(RFMap* rf, int i)
{
    Electrode& e = electrodes.getReference(i);
    e.rfMap = rf;
}


void RFMapper::handleEvent(int eventType, MidiMessage& event, int samplePosition)
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

    }

}
