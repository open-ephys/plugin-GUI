/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "PeriStimulusTimeHistogramNode.h"
#include "../RecordNode/RecordNode.h"
#include "../SpikeSorter/SpikeSorter.h"
#include "../Editors/PeriStimulusTimeHistogramEditor.h"
#include "../Channel/Channel.h"
//#include "ISCAN.h"
#include <stdio.h>

#include "tictoc.h"



PeriStimulusTimeHistogramNode::PeriStimulusTimeHistogramNode()
    : GenericProcessor("PSTH"), displayBufferSize(5),  redrawRequested(false)
{
	trialCircularBuffer  = nullptr;
	eventFile = nullptr;
	isRecording = false;
	saveEyeTracking = saveTTLs = saveNetworkEvents = false;
	saveNetworkEventsWhenNotRecording = false;
	spikeSavingMode = 2;
	syncCounter = 0;
}


void PeriStimulusTimeHistogramNode::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("PSTH");
    mainNode->setAttribute("saveEyeTracking", saveEyeTracking);
    mainNode->setAttribute("saveTTLs", saveTTLs);
	mainNode->setAttribute("saveNetworkEvents", saveNetworkEvents);
	mainNode->setAttribute("saveNetworkEventsWhenNotRecording", saveNetworkEventsWhenNotRecording);
	mainNode->setAttribute("spikeSavingMode", spikeSavingMode);
}



void PeriStimulusTimeHistogramNode::loadCustomParametersFromXml()
{
	if (parametersAsXml != nullptr)
	{
		forEachXmlChildElement(*parametersAsXml, mainNode)
		{
			if (mainNode->hasTagName("PSTH"))
			{
				int numElectrodes = mainNode->getIntAttribute("numElectrodes");

				saveEyeTracking = mainNode->getBoolAttribute("saveEyeTracking");
				saveTTLs = mainNode->getBoolAttribute("saveTTLs");
				saveNetworkEvents = mainNode->getBoolAttribute("saveNetworkEvents");
				saveNetworkEventsWhenNotRecording = mainNode->getBoolAttribute("saveNetworkEventsWhenNotRecording");
				spikeSavingMode = mainNode->getIntAttribute("spikeSavingMode");
			}
		}
	}
}

PeriStimulusTimeHistogramNode::~PeriStimulusTimeHistogramNode()
{
    
}

AudioProcessorEditor* PeriStimulusTimeHistogramNode::createEditor()
{
    editor = new PeriStimulusTimeHistogramEditor(this,true);
    return editor;

}

void PeriStimulusTimeHistogramNode::allocateTrialCircularBuffer()
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
	params.ttlTrialLengthSec = 0;
	params.autoAddTTLconditions = true;
	params.buildTrialsPSTH = true;
	params.reconstructTTL = false;
	params.approximate = true;

	trialCircularBuffer = new TrialCircularBuffer(params);
}

void PeriStimulusTimeHistogramNode::updateSettings()
{
	delete trialCircularBuffer;
	trialCircularBuffer = nullptr;
	if (trialCircularBuffer  == nullptr && getSampleRate() > 0 && getNumInputs() > 0)
	{
		allocateTrialCircularBuffer();
		syncInternalDataStructuresWithSpikeSorter();
	}

	recordNode = getProcessorGraph()->getRecordNode();
//    diskWriteLock = recordNode->getLock();
}

void PeriStimulusTimeHistogramNode::setHardwareTriggerAlignmentChannel(int chan)
{
	trialCircularBuffer->setHardwareTriggerAlignmentChannel(chan);
}

bool PeriStimulusTimeHistogramNode::enable()
{
	
    std::cout << "PeriStimulusTimeHistogramNode::enable()" << std::endl;
	PeriStimulusTimeHistogramEditor* editor = (PeriStimulusTimeHistogramEditor*) getEditor();
    editor->enable();
		
    return true;

}

bool PeriStimulusTimeHistogramNode::disable()
{
	
    std::cout << "PeriStimulusTimeHistogramNode disabled!" << std::endl;
    PeriStimulusTimeHistogramEditor* editor = (PeriStimulusTimeHistogramEditor*) getEditor();
    editor->disable();
	
    return true;
}

void PeriStimulusTimeHistogramNode::toggleConditionVisibility(int cond)
{
	if (trialCircularBuffer  != nullptr)
	{
		trialCircularBuffer->toggleConditionVisibility(cond);
	}
}

void PeriStimulusTimeHistogramNode::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{
	//printf("Entering PeriStimulusTimeHistogramNode::process\n");
	// Update internal statistics 
    checkForEvents(events); 
	

	if (trialCircularBuffer  == nullptr && getSampleRate() > 0 && getNumInputs() > 0)  {
		allocateTrialCircularBuffer();
		syncInternalDataStructuresWithSpikeSorter();	
	} else if (trialCircularBuffer != nullptr)
	{
		trialCircularBuffer->process(buffer,getNumSamples(0),hardware_timestamp,software_timestamp);
	}


	// draw the PSTH
    if (redrawRequested)
    {
        redrawRequested = false;
    }
	//printf("Exitting PeriStimulusTimeHistogramNode::process\n");
}



void PeriStimulusTimeHistogramNode::dumpStartStopRecordEventToDisk(int64 ts, bool startRecord)
{
	const ScopedLock myScopedLock(diskWriteLock);
	#define SESSION 10
	uint8 eventType = SESSION;

	// write event type
	fwrite(&eventType, 1,1, eventFile); 
	// write event size
	int16 eventSize = 1 +2+ 8; // start/stop(1) + recordnumber (2) + timestamp (8)
    fwrite(&eventSize, 2,1, eventFile);
	// write event data
	// 1. Start/stop
	fwrite(&startRecord, 1,1, eventFile);
	// 2. record number
	fwrite(&recordingNumber, 2,1, eventFile);
	// 3. the software timestamp
	fwrite(&ts, 8,1, eventFile);


}

void PeriStimulusTimeHistogramNode::dumpNetworkEventToDisk(String S, int64 ts)
{
	const ScopedLock myScopedLock(diskWriteLock);
	
	uint8 eventType = NETWORK;

	// write event type
	fwrite(&eventType, 1,1, eventFile); 
	// write event size
	int16 eventSize = S.getNumBytesAsUTF8() + 8; // string length + timestamp
    fwrite(&eventSize, 2,1, eventFile);
	// write event data
	// 1. the network event string
	fwrite(S.toUTF8(), S.getNumBytesAsUTF8(), 1, eventFile);
	// 2. the software timestamp
	fwrite(&ts, 8,1, eventFile);

	
}

void PeriStimulusTimeHistogramNode::dumpSpikeEventToDisk(SpikeObject *s, bool dumpWave)
{
	const ScopedLock myScopedLock(diskWriteLock);



	uint8 eventType = SPIKE;

	// write event type
	fwrite(&eventType, 1,1, eventFile); 
	int16 eventSize = 8+8+2+2+2+2+2  + s->nChannels*4; // ts, ts, sorted id, electrode ID, num channels, num data per channel + 4x gains 

	if (dumpWave)
		eventSize += 2*(s->nSamples*s->nChannels); // uint16 per samples 

	// write event size
    fwrite(&eventSize, 2,1, eventFile);

	// write event data
	// 1. software ts
	fwrite(&s->timestamp_software, 8,1, eventFile);
	// 2. hardware ts
	fwrite(&s->timestamp, 8,1, eventFile);
	// 3. sorted ID
	fwrite(&s->sortedId, 2,1, eventFile);
	// 4. electrod ID
	fwrite(&s->electrodeID, 2,1, eventFile);

	// 5. Channel in which threshold was detected
	fwrite(&s->channel, 2,1, eventFile);

	// 6. num channels
	fwrite(&s->nChannels, 2,1, eventFile);

	// 7. gains
	fwrite(&s->gain, 4,s->nChannels, eventFile);

	// 8. num data points per channel
	fwrite(&s->nSamples, 2,1, eventFile);
	if (dumpWave)
		fwrite(&s->data, 2,s->nSamples*s->nChannels, eventFile);

	
}
void PeriStimulusTimeHistogramNode::dumpTTLeventToDisk(int channel, bool risingEdge, int64 ttl_timestamp_software, int64 ttl_timestamp_hardware, int samplePosition )
{
	const ScopedLock myScopedLock(diskWriteLock);
	uint8 eventType = TTL;

	// write event type
	fwrite(&eventType, 1,1, eventFile); 
	// write event size
	int16 eventSize = 1 + 2 + 8 + 8; // rising/falling(1) + software ts (8) + hardware ts (8)
    fwrite(&eventSize, 2,1, eventFile);
	// write event data
	// 1. rising/falling edge
	fwrite(&risingEdge, 1, 1, eventFile);
	// 2. channel
	fwrite(&channel, 2,1,eventFile);
	// 2. software ts
	fwrite(&ttl_timestamp_software, 8,1, eventFile);
	// 3. hardware ts
	fwrite(&ttl_timestamp_hardware, 8,1, eventFile);

	
}
/*
void PeriStimulusTimeHistogramNode::dumpEyeTrackingEventToDisk(EyePosition pos)
{
	diskWriteLock->enter();

	uint8 eventType = EYE_POSITION;

	// write event type
	fwrite(&eventType, 1,1, eventFile); 
	// write event size
	int16 eventSize = 8 + 8 + 8 + 8 + 8 +8 +8; //  x,y,xc,yc,pupil,timestamp
    fwrite(&eventSize, 2,1, eventFile);
	// write event data
	// 1. eye x position
	fwrite(&pos.x, 8,1, eventFile);
	// 2. eye y position
	fwrite(&pos.y, 8,1, eventFile);


	// 3. calibrated eye x position
	fwrite(&pos.xc, 8,1, eventFile);
	// 4. calibrated eye y position
	fwrite(&pos.yc, 8,1, eventFile);

	// 5. eye pupil
	fwrite(&pos.pupil, 8,1, eventFile);
	// 6. software timestamp
	fwrite(&pos.software_timestamp, 8,1, eventFile);
	// 7. hardware timestamp
	fwrite(&pos.hardware_timestamp, 8,1, eventFile);

	diskWriteLock->exit();
}
*/
void PeriStimulusTimeHistogramNode::dumpTimestampEventToDisk(int64 softwareTS,int64 hardwareTS)
{
	const ScopedLock myScopedLock(diskWriteLock);

	uint8 eventType = TIMESTAMP;

	// write event type
	fwrite(&eventType, 1,1, eventFile); 
	// write event size
	int16 eventSize = 8 + 8; //  software ts (8) + hardware ts (8)
    fwrite(&eventSize, 2,1, eventFile);
	// write event data
	// 1. software ts
	fwrite(&softwareTS, 8,1, eventFile);
	// 2. hardware ts
	fwrite(&hardwareTS, 8,1, eventFile);

	

}

void PeriStimulusTimeHistogramNode::syncInternalDataStructuresWithSpikeSorter()
{
	ProcessorGraph *g = getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Spike Sorter")
		{
			SpikeSorter *node = (SpikeSorter*)p[k];
			Array<Electrode*> electrodes = node->getElectrodes();

			// for each electrode, verify that 
			// 1. We have it in our internal structure 
			// 2. All channels match
			// 3. We have all sorted unit information
			trialCircularBuffer->syncInternalDataStructuresWithSpikeSorter(electrodes);
		}
	}
}


void PeriStimulusTimeHistogramNode::modifyTimeRange(double preSec_, double postSec_)
{
	ProcessorGraph *g = getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Spike Sorter")
		{
			SpikeSorter*node = (SpikeSorter*)p[k];
			Array<Electrode*> electrodes = node->getElectrodes();

			// for each electrode, verify that 
			// 1. We have it in our internal structure 
			// 2. All channels match
			// 3. We have all sorted unit information
			TrialCircularBufferParams params = trialCircularBuffer->getParams();
			params.preSec = preSec_;
			params.postSec = postSec_;
			delete trialCircularBuffer;
			trialCircularBuffer = new TrialCircularBuffer(params);
			trialCircularBuffer->syncInternalDataStructuresWithSpikeSorter(electrodes);

		}
	}
}

void PeriStimulusTimeHistogramNode::handleNetworkMessage(StringTS s)
{
 		bool redrawNeeded = trialCircularBuffer->parseMessage(s);
		if (redrawNeeded ){
			PeriStimulusTimeHistogramEditor* ed = (PeriStimulusTimeHistogramEditor*) getEditor();
			ed->updateCanvas();
		}
			
		if (isRecording && saveNetworkEvents)
		{
			dumpNetworkEventToDisk(s.getString(),s.timestamp);
		} else if (!isRecording && saveNetworkEvents && saveNetworkEventsWhenNotRecording)
		{
			networkEventsHistory.push(s);
		}

}
void PeriStimulusTimeHistogramNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
    //tictoc.Tic(9);
	static std::list<long> previousSpikesIDs;

	if (eventType == NETWORK)
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
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
		  
		  if (isRecording)
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
		  }

    } 
	if (eventType == TTL)
	{
	Time t;
	   const uint8* dataptr = event.getRawData();
	   int ttl_source = dataptr[1];
	   bool ttl_raise = dataptr[2] > 0;
	   int channel = dataptr[3];
	   int64 ttl_timestamp_hardware = timestamps[ttl_source] + samplePosition; // hardware time
	   int64 ttl_timestamp_software = t.getHighResolutionTicks(); // get software time
	   //int64  ttl_timestamp_software,ttl_timestamp_hardware;
	   //memcpy(&ttl_timestamp_software, dataptr+4, 8);
	   //memcpy(&ttl_timestamp_hardware, dataptr+12, 8);
	   if (ttl_raise)
			trialCircularBuffer->addTTLevent(channel,ttl_timestamp_software,ttl_timestamp_hardware, ttl_raise, true);
	   if (isRecording && saveTTLs)
		   dumpTTLeventToDisk(channel,ttl_raise,ttl_timestamp_software,ttl_timestamp_hardware,samplePosition );
	}

    if (eventType == SPIKE)
    {
        const uint8_t* dataptr = event.getRawData();
        int bufferSize = event.getRawDataSize();
        if (bufferSize > 0)
        {
            SpikeObject newSpike;
            unpackSpike(&newSpike, dataptr, bufferSize);

			if (newSpike.sortedId > 0) { // drop unsorted spikes
				trialCircularBuffer->addSpikeToSpikeBuffer(newSpike);
			}
			if (isRecording)
			{
				if  (spikeSavingMode == 1 && newSpike.sortedId > 0)
					dumpSpikeEventToDisk(&newSpike, false);
				else if (spikeSavingMode == 2 && newSpike.sortedId > 0)
					dumpSpikeEventToDisk(&newSpike, true);
				else if (spikeSavingMode == 3)
					dumpSpikeEventToDisk(&newSpike, true);
			}
        }
    }
   // tictoc.Toc(9);

}



String PeriStimulusTimeHistogramNode::generateHeader()
{

    String header = "header.format = 'Open Ephys Data Format'; \n";
    header += "header.version = 0.32;";
    header += "header.header_bytes = ";
    header += String(HEADER_SIZE);
    header += ";\n";
    header += "header.description = 'each record is size-varying. First is the record size (16-bit), followed by record type (uint8), followed by the actual data (which vary, depending on the type) '; \n";
    header += "header.date_created = '";
    header += recordNode->generateDateString();
    header += "';\n";
    header += "header.channel = '";
    header += "events";
    header += "';\n";
    header += "header.channelType = 'Event';\n";
    header += "header.sampleRate = ";
    // all channels need to have the same sample rate under the current scheme
    header += String(getSampleRate()); 
    header += ";\n";
    header += "header.blockLength = ";
    header += BLOCK_LENGTH;
    header += ";\n";
    header += "header.bufferSize = ";
    header += getAudioComponent()->getBufferSize();
    header += ";\n";
    header += "header.bitVolts = ";
	if (recordNode->channels.size() > 0)
	{
		header += String(recordNode->channels[0]->bitVolts);
	}
	else
	{
		header += String(getDefaultBitVolts());
	}

    header += ";\n";

	Time t;
    header += "header.ticksPerSec = ";
	header += String(t.getHighResolutionTicksPerSecond());
    header += ";\n";

    header = header.paddedRight(' ', HEADER_SIZE);
    return header;

}



void PeriStimulusTimeHistogramNode::openFile(String filename)
{
    std::cout << "OPENING FILE: " << filename << std::endl;

    File f = File(filename);
     
    bool fileExists = f.exists();
    
	const ScopedLock myScopedLock(diskWriteLock);

    eventFile = fopen(filename.toUTF8(), "ab");

    if (!fileExists)
    {
        // create and write header
        std::cout << "Writing header." << std::endl;
        String header = generateHeader();
		int headerSize = header.getNumBytesAsUTF8();
        std::cout << "File ID: " << eventFile << ", number of bytes: " << headerSize << std::endl;
        fwrite(header.toUTF8(), header.getNumBytesAsUTF8(), 1, eventFile);
        std::cout << "Wrote header." << std::endl;
    }
    else
    {
        std::cout << "File already exists, just opening." << std::endl;
    }
 
}


void PeriStimulusTimeHistogramNode::startRecording()
{
	if (!isRecording)
	{
        File dataDirectory = recordNode->getDataDirectory();

        if (dataDirectory.getFullPathName().length() == 0)
        {
            // temporary fix in case nothing is returned by the record node.
            dataDirectory = File::getSpecialLocation(File::userHomeDirectory); 
        }

        String baseDirectory = dataDirectory.getFullPathName();
		String eventChannelFilename = baseDirectory + dataDirectory.separatorString + "all_channels.events";
        openFile(eventChannelFilename);

		// dump network events that arrived when we weren't recording
		if (saveNetworkEventsWhenNotRecording)
		{
			while (networkEventsHistory.size() > 0)
			{
				StringTS s = networkEventsHistory.front();
				dumpNetworkEventToDisk(s.getString(),s.timestamp);
				networkEventsHistory.pop();
			}
		}

//		recordingNumber = recordNode->getRecordingNumber();
		Time t;
		dumpStartStopRecordEventToDisk(t.getHighResolutionTicks(), true);
		isRecording = true;

	}


}

void PeriStimulusTimeHistogramNode::stopRecording()
{
	if (isRecording)
	{
		// close files, etc.
		const ScopedLock myScopedLock(diskWriteLock);
		std::cout << "CLOSING EVENT FILE: " << std::endl;
		if (eventFile != nullptr)
			fclose(eventFile);

	
		isRecording = false;
	}
}
