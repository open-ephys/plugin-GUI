
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


#include <stdio.h>
#include "PSTHProcessor.h"

//If the processor uses a custom editor, it needs its header to instantiate it
//#include "ExampleEditor.h"

PSTHProcessor::PSTHProcessor()
	: GenericProcessor("PSTH Sink"), displayBufferSize(5), redrawRequested(false)

{
	trialCircularBuffer = nullptr;
	isRecording = false;
	saveEyeTracking = saveTTLs = saveNetworkEvents = false;
	saveNetworkEventsWhenNotRecording = false;
	spikeSavingMode = 2;
	syncCounter = 0;

}

void PSTHProcessor::saveCustomParametersToXml(XmlElement* parentElement)
{
	XmlElement* mainNode = parentElement->createNewChildElement("PSTH");
	mainNode->setAttribute("saveEyeTracking", saveEyeTracking);
	mainNode->setAttribute("saveTTLs", saveTTLs);
	mainNode->setAttribute("saveNetworkEvents", saveNetworkEvents);
	mainNode->setAttribute("saveNetworkEventsWhenNotRecording", saveNetworkEventsWhenNotRecording);
	mainNode->setAttribute("spikeSavingMode", spikeSavingMode);
}



void PSTHProcessor::loadCustomParametersFromXml()
{
	if (parametersAsXml != nullptr)
	{
		forEachXmlChildElement(*parametersAsXml, mainNode)
		{
			if (mainNode->hasTagName("PSTH"))
			{
				//int numElectrodes = mainNode->getIntAttribute("numElectrodes");

				saveEyeTracking = mainNode->getBoolAttribute("saveEyeTracking");
				saveTTLs = mainNode->getBoolAttribute("saveTTLs");
				saveNetworkEvents = mainNode->getBoolAttribute("saveNetworkEvents");
				saveNetworkEventsWhenNotRecording = mainNode->getBoolAttribute("saveNetworkEventsWhenNotRecording");
				spikeSavingMode = mainNode->getIntAttribute("spikeSavingMode");
			}
		}
	}
}

PSTHProcessor::~PSTHProcessor()
{

}

AudioProcessorEditor* PSTHProcessor::createEditor()
{
	editor = new PSTHEditor(this, true);
	return editor;

}

void PSTHProcessor::allocateTrialCircularBuffer()
{
	TrialCircularBufferParams params;
	params.numChannels = getNumInputs();
	params.numTTLchannels = 8;
	params.sampleRate = getSampleRate();
	params.maxTrialTimeSeconds = 5.0;
	params.preSec = 0.1;
	params.postSec = 0.5;
	params.maxTrialsInMemory = 200;
	params.binResolutionMS = 1;
	params.desiredSamplingRateHz = 600;
	params.ttlSupressionTimeSec = 1.0;
	params.ttlTrialLengthSec = 1.0;
	params.autoAddTTLconditions = true;
	params.buildTrialsPSTH = true;
	params.reconstructTTL = false;
	params.approximate = true;

	trialCircularBuffer = new TrialCircularBuffer(params);
}

void PSTHProcessor::updateSettings()
{
	trialCircularBuffer = nullptr;
	if (trialCircularBuffer == nullptr && getSampleRate() > 0 && getNumInputs() > 0)
	{
		allocateTrialCircularBuffer();
		syncInternalDataStructuresWithSpikeSorter();
	}
	electrodeChannels.clear();
	for (int k = 0; k < eventChannels.size(); k++)
	{
		if ((eventChannels[k]->type == ELECTRODE_CHANNEL) &&
			(static_cast<SpikeChannel*>(eventChannels[k]->extraData.get())->dataType == SpikeChannel::Sorted))
			electrodeChannels.add(eventChannels[k]);
	}


	//    diskWriteLock = recordNode->getLock();
}

void PSTHProcessor::setHardwareTriggerAlignmentChannel(int chan)
{
	if (trialCircularBuffer != nullptr)
	{
		trialCircularBuffer->setHardwareTriggerAlignmentChannel(chan);
	}
}

bool PSTHProcessor::enable()
{

	std::cout << "PSTHProcessor::enable()" << std::endl;
	PSTHEditor* editor = (PSTHEditor*)getEditor();
	editor->enable();
	/*
	CoreServices::RecordNode::registerSpikeSource(this);
	for (int i = 0; i < electrodeChannels.size(); i++)
	{
		SpikeRecordInfo* recElec = new SpikeRecordInfo();
		recElec->name = electrodeChannels[i]->name;
		recElec->numChannels = static_cast<SpikeChannel*>(electrodeChannels[i]->extraData.get())->numChannels;
		recElec->sampleRate = settings.sampleRate;
		electrodeChannels[i]->recordIndex = CoreServices::RecordNode::addSpikeElectrode(recElec);
	}
	*/
	return true;

}

bool PSTHProcessor::disable()
{

	std::cout << "PSTHProcessor disabled!" << std::endl;
	PSTHEditor* editor = (PSTHEditor*)getEditor();
	editor->disable();

	return true;
}

void PSTHProcessor::toggleConditionVisibility(int cond)
{
	if (trialCircularBuffer != nullptr)
	{
		trialCircularBuffer->toggleConditionVisibility(cond);
	}
}

void PSTHProcessor::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{
	//printf("Entering PSTHProcessor::process\n");
	// Update internal statistics
	checkForEvents(events);


	if (trialCircularBuffer == nullptr && getSampleRate() > 0 && getNumInputs() > 0)
	{
		allocateTrialCircularBuffer();
		syncInternalDataStructuresWithSpikeSorter();
	}
	else if (trialCircularBuffer != nullptr)
	{
		trialCircularBuffer->process(buffer, getNumSamples(0), hardware_timestamp, software_timestamp);
	}


	// draw the PSTH
	if (redrawRequested)
	{
		redrawRequested = false;
	}
	//printf("Exitting PSTHProcessor::process\n");
}



void PSTHProcessor::syncInternalDataStructuresWithSpikeSorter()
{
	Array<Electrode*> electrodes;

	for (int k = 0; k<eventChannels.size(); k++)
	{
		if ((eventChannels[k]->type == ELECTRODE_CHANNEL) &&
			(static_cast<SpikeChannel*>(eventChannels[k]->extraData.get())->dataType == SpikeChannel::Sorted))
		{
			electrodes.add(static_cast<Electrode*>(eventChannels[k]->extraData->dataPtr));
		}
		// for each electrode, verify that
		// 1. We have it in our internal structure
		// 2. All channels match
		// 3. We have all sorted unit information
		trialCircularBuffer->syncInternalDataStructuresWithSpikeSorter(electrodes);
	}
}


void PSTHProcessor::modifyTimeRange(double preSec_, double postSec_)
{
	/*
	ProcessorGraph* g = AccessClass::getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k = 0; k<p.size(); k++)
	{
		if (p[k]->getName() == "Spike Sorter")
		{
			SpikeSorter* node = (SpikeSorter*)p[k];
			Array<Electrode*> electrodes = node->getElectrodes();

			// for each electrode, verify that
			// 1. We have it in our internal structure
			// 2. All channels match
			// 3. We have all sorted unit information
			TrialCircularBufferParams params = trialCircularBuffer->getParams();
			params.preSec = preSec_;
			params.postSec = postSec_;
			trialCircularBuffer = new TrialCircularBuffer(params);
			trialCircularBuffer->syncInternalDataStructuresWithSpikeSorter(electrodes);

		}
	}
	*/
}

void PSTHProcessor::handleNetworkMessage(StringTS s)
{
	bool redrawNeeded = trialCircularBuffer->parseMessage(s);
	if (redrawNeeded)
	{
		PSTHEditor* ed = (PSTHEditor*)getEditor();
		ed->updateCanvas();
	}

	/*	if (isRecording && saveNetworkEvents)
	{
	dumpNetworkEventToDisk(s.getString(),s.timestamp);
	} else if (!isRecording && saveNetworkEvents && saveNetworkEventsWhenNotRecording)
	{
	networkEventsHistory.push(s);
	}*/

}
void PSTHProcessor::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
	//tictoc.Tic(9);
	static std::list<long> previousSpikesIDs;

	uint8 eventId = *((uint8*)event.getRawData() + 2);

	if ((eventType == MESSAGE) && (eventId > 0)) //to differentiate network events from simple messages
	{
		//tictoc.Tic(10);

		StringTS s(event);
		handleNetworkMessage(s);
		//tictoc.Toc(10);
	}
	/*
	if (eventType == EYE_POSITION)
	{
	if (saveEyeTracking && isRecording)
	{
	EyePosition pos;
	const uint8* dataptr = event.getRawData();
	memcpy(&pos.x, dataptr+4,8);
	memcpy(&pos.y, dataptr+4+8,8);
	memcpy(&pos.xc, dataptr+4+8+8,8);
	memcpy(&pos.yc, dataptr+4+8+8+8,8);
	memcpy(&pos.pupil, dataptr+4+8+8+8+8,8);
	memcpy(&pos.software_timestamp, dataptr+4+8+8+8+8+8,8);
	memcpy(&pos.hardware_timestamp, dataptr+4+8+8+8+8+8+8,8);
	dumpEyeTrackingEventToDisk(pos);
	}
	}*/
	if (eventType == TIMESTAMP)
	{
		const uint8* dataptr = event.getRawData();
		memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		software_timestamp = timer.getHighResolutionTicks(); //memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes

		/* if (isRecording)
		{
		if (syncCounter == 0)
		{
		// we keep dumping these because software timestamps are known to be fickle.
		// best practice is to do robust linear fitting in post processing to find out
		// the optimal correspondence between the two.
		dumpTimestampEventToDisk(software_timestamp,hardware_timestamp);
		}

		syncCounter++;
		if (syncCounter > 10)
		syncCounter = 0;
		}*/

	}
	if (eventType == TTL)
	{

		const uint8* dataptr = event.getRawData();
		int ttl_source = dataptr[1];
		bool ttl_raise = dataptr[2] > 0;
		int channel = dataptr[3]; // channel number
		int64 ttl_timestamp_hardware = timestamps[ttl_source] + samplePosition; // hardware time
		int64 ttl_timestamp_software = timer.getHighResolutionTicks(); // get software time
		//int64  ttl_timestamp_software,ttl_timestamp_hardware;
		//memcpy(&ttl_timestamp_software, dataptr+4, 8);
		//memcpy(&ttl_timestamp_hardware, dataptr+12, 8);

		trialCircularBuffer->addTTLevent(channel, ttl_timestamp_software, ttl_timestamp_hardware, ttl_raise, true);

	}

	if (eventType == SPIKE)
	{
		const uint8_t* dataptr = event.getRawData();

		int bufferSize = event.getRawDataSize();

		if (bufferSize > 0)
		{
			SpikeObject newSpike;
			unpackSpike(&newSpike, dataptr, bufferSize);

			if (newSpike.sortedId > 0)   // drop unsorted spikes
			{
				trialCircularBuffer->addSpikeToSpikeBuffer(newSpike);
			}

			if (isRecording)
			{
				if (spikeSavingMode == 1 && newSpike.sortedId > 0)
					CoreServices::RecordNode::writeSpike(newSpike, electrodeChannels[newSpike.source]->recordIndex);
				//dumpSpikeEventToDisk(&newSpike, false);
				else if (spikeSavingMode == 2 && newSpike.sortedId > 0)
					CoreServices::RecordNode::writeSpike(newSpike, electrodeChannels[newSpike.source]->recordIndex);
				//dumpSpikeEventToDisk(&newSpike, true);
				else if (spikeSavingMode == 3)
					CoreServices::RecordNode::writeSpike(newSpike, electrodeChannels[newSpike.source]->recordIndex);
				//dumpSpikeEventToDisk(&newSpike, true);
			}
		}
	}
	// tictoc.Toc(9);

}

void PSTHProcessor::startRecording()
{
	isRecording = true;
}

void PSTHProcessor::stopRecording()
{
	isRecording = false;
}