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

#include "SpikeDisplayEditor.h"
#include "SpikePlots.h"

#include <stdio.h>


SpikeDisplayNode::SpikeDisplayNode()
    : GenericProcessor  ("Spike Viewer"), 
      displayBufferSize (5),  
      redrawRequested  (false)
{
}


AudioProcessorEditor* SpikeDisplayNode::createEditor()
{
    editor = std::make_unique<SpikeDisplayEditor> (this);
    return editor.get();
}


void SpikeDisplayNode::updateSettings()
{
    electrodes.clear();
    electrodeMap.clear();

	for (auto spikeChannel : spikeChannels)
	{

        if (spikeChannel->isValid())
        {

            Electrode* elec = new Electrode();
            elec->numChannels = spikeChannel->getNumChannels();
            elec->name = spikeChannel->getName();
            elec->spikeChannel = spikeChannel;

            electrodes.add(elec);
        }
	}
}


bool SpikeDisplayNode::startAcquisition()
{
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();

	for (int i = 0; i < spikeChannels.size(); i ++)
	{
		Electrode* elec = electrodes[i];
	}

    editor->enable();

    totalCallbacks = 0;
    spikeCount = 0;

    return true;
}


bool SpikeDisplayNode::stopAcquisition()
{

    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->disable();

    return true;
}


void SpikeDisplayNode::setParameter(int param, float val)
{

    if (param == 10)
    {
        SpikeChannel* chan = spikeChannels[int(val)];

        String msg = "AUDIO SELECT ";
        msg += String(chan->getStreamId()) + " ";

        for (auto ch : chan->localChannelIndexes)
        {
            msg += String(ch) + " ";
        }

        //std::cout << "MESSAGE: " << msg << std::endl;
        broadcastMessage(msg);

    }
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
    Electrode* electrode = electrodes[i];
    electrode->spikePlot = sp;

    electrodeMap[electrode->spikeChannel] = sp;
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


void SpikeDisplayNode::process (AudioBuffer<float>& buffer)
{
    checkForEvents (true); // automatically calls 'handleEvent()'

    totalCallbacks++;
}


void SpikeDisplayNode::handleSpike(const SpikeChannel* spikeChannel, const EventPacket& spike, int samplePosition, const uint8* rawData)
{
	SpikePtr newSpike = Spike::deserialize(spike, spikeChannel);

    //std::cout << "SDN: " << newSpike->getSortedID() << std::endl;

    electrodeMap.at(spikeChannel)->addSpikeToBuffer(newSpike);
}

