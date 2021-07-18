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
#include "GenericProcessor.h"

#include "../../AccessClass.h"
#include "../../Utils/Utils.h"
#include "../Editors/GenericEditor.h"

#include "../Settings/DataStream.h"
#include "../Settings/ProcessorInfo.h"
#include "../Settings/ConfigurationObject.h"
#include "../Settings/DeviceInfo.h"

#include "../Events/Event.h"
#include "../Events/Spike.h"

#include "../../Processors/MessageCenter/MessageCenterEditor.h"

#include <exception>

const String GenericProcessor::m_unusedNameString("xxx-UNUSED-OPEN-EPHYS-xxx");

GenericProcessor::GenericProcessor(const String& name)
	: GenericProcessorBase(name)
	, sourceNode(nullptr)
	, destNode(nullptr)
	, isEnabled(true)
	, wasConnected(false)
	, nextAvailableChannel(0)
	, saveOrder(-1)
	, loadOrder(-1)
	, currentChannel(-1)
	, editor(nullptr)
	, parametersAsXml(nullptr)
	, sendSampleCount(true)
	, m_processorType(PROCESSOR_TYPE_UTILITY)
	, m_name(name)
	, m_paramsWereLoaded(false)
{
	m_lastProcessTime = Time::getHighResolutionTicks();
}


GenericProcessor::~GenericProcessor()
{
}


AudioProcessorEditor* GenericProcessor::createEditor()
{
	editor = std::make_unique<GenericEditor>(this, true);

	return editor.get();
}


void GenericProcessor::setNodeId(int id)
{
	nodeId = id;

	if (editor != 0)
	{
		editor->updateName();
	}
}


Parameter* GenericProcessor::getParameterByName(String name)
{
	const int numParameters = getNumParameters();
	// doesn't work
	for (int i = 0; i < numParameters; ++i)
	{
		const auto parameter = parameters[i];
		const String parameterName = parameter->getName();

		if (parameterName.compare(name) == 0) // fails at this point
			return parameter;//parameters.getReference(i);
	}

	Parameter* nullParam = new Parameter("VOID", false, -1);

	return nullParam;
}


Parameter* GenericProcessor::getParameterByIndex(int parameterIndex) const
{
	return parameters[parameterIndex];
}


void GenericProcessor::setParameter(int parameterIndex, float newValue)
{
	editor->updateParameterButtons(parameterIndex);
	LOGD("Setting parameter");

	if (currentChannel >= 0)
		parameters[parameterIndex]->setValue(newValue, currentChannel);
}


int GenericProcessor::getNextChannel(bool increment)
{
	int chan = nextAvailableChannel;

	LOGDD("Next channel: ", chan, ", num inputs: ", getNumInputs());

	if (increment)
		nextAvailableChannel++;

	if (chan < getNumInputs())
		return chan;
	else
		return -1;
}


void GenericProcessor::resetConnections()
{
	nextAvailableChannel = 0;

	wasConnected = false;
}


void GenericProcessor::setSourceNode(GenericProcessor* sn)
{
    if (this->isMerger())
        setMergerSourceNode(sn);
    else
        sourceNode = sn;
}


void GenericProcessor::setDestNode(GenericProcessor* dn)
{
	
    if (this->isSplitter())
        setSplitterDestNode(dn);
    else
        destNode = dn;

}


void GenericProcessor::clearSettings()
{
	LOGDD("Generic processor clearing settings.");

	continuousChannels.clear();
	eventChannels.clear();
	spikeChannels.clear();
	configurationObjects.clear();
	sourceStreams.clear();
	streams.clear();

}

void GenericProcessor::copyDataStreamSettings(DataStream* stream)
{
	streams.add(stream); // pointer to original source

	for (auto continuousChannel : stream->getContinuousChannels())
	{
		continuousChannels.add(new ContinuousChannel(*continuousChannel));
		continuousChannels.getLast()->addProcessor(processorInfo.get());
	}

	for (auto eventChannel : stream->getEventChannels())
	{
		eventChannels.add(new EventChannel(*eventChannel));
		eventChannels.getLast()->addProcessor(processorInfo.get());
	}

	for (auto spikeChannel : stream->getSpikeChannels())
	{
		spikeChannels.add(new SpikeChannel(*spikeChannel));
		spikeChannels.getLast()->addProcessor(processorInfo.get());
	}
}


void GenericProcessor::update()
{
	LOGD(getName(), " updating settings.");

	clearSettings();

	processorInfo.reset();
	processorInfo = std::unique_ptr<ProcessorInfoObject>(new ProcessorInfoObject(this));
    
    if (!isMerger() && !isSplitter())
    {
        if (sourceNode != nullptr) 
        {
			// copy settings from source node

            for (auto stream : sourceNode->streams)
            {
				copyDataStreamSettings(stream);
            }

            for (auto configurationObject : sourceNode->configurationObjects)
            {
                configurationObjects.add(new ConfigurationObject(*configurationObject));
            }

			isEnabled = sourceNode->isEnabled;
        }
        else
        {
			// connect first processor in signal chain to message center

			const EventChannel* messageChannel = AccessClass::getMessageCenter()->messageCenter->getMessageChannel();

			eventChannels.add(new EventChannel(*messageChannel));

			std::cout << getNodeId() << " connected to Message Center" << std::endl;
        }
    }

	updateSettings(); // allow processors to change custom settings, 
					  // including creation of streams / channels and
					  // setting isEnabled variable

	updateChannelIndexMaps();

	m_needsToSendTimestampMessages.clear();
	m_needsToSendTimestampMessages.insertMultiple(-1, false, getNumDataStreams());

	// required for the ProcessorGraph to know the
	// details of this processor:
	setPlayConfigDetails(getNumInputs(),  // numIns
		getNumOutputs(), // numOuts
		44100.0,         // sampleRate (always 44100 Hz, default audio card rate)
		128);            // blockSize

	editor->update(isEnabled); // allow the editor to update its settings
}

void GenericProcessor::updateChannelIndexMaps()
{
	continuousChannelMap.clear();
	eventChannelMap.clear();
	spikeChannelMap.clear();

	for (int i = 0; i < continuousChannels.size(); i++)
	{
		ContinuousChannel* chan = continuousChannels[i];

		uint16 processorId = chan->getSourceNodeId();
		uint16 streamId = chan->getStreamId();
		uint16 localIndex = chan->getLocalIndex();

		continuousChannelMap[processorId][streamId][localIndex] = chan;
	}

	for (int i = 0; i < eventChannels.size(); i++)
	{
		EventChannel* chan = eventChannels[i];

		uint16 processorId = chan->getSourceNodeId();
		uint16 streamId = chan->getStreamId();
		uint16 localIndex = chan->getLocalIndex();

		eventChannelMap[processorId][streamId][localIndex] = chan;
	}

	for (int i = 0; i < spikeChannels.size(); i++)
	{
		SpikeChannel* chan = spikeChannels[i];

		uint16 processorId = chan->getSourceNodeId();
		uint16 streamId = chan->getStreamId();
		uint16 localIndex = chan->getLocalIndex();

		spikeChannelMap[processorId][streamId][localIndex] = chan;
	}
	
}

String GenericProcessor::handleConfigMessage(String msg)
{
	return "";
}


/*void GenericProcessor::createEventChannels()
{
	int nSub = getNumSubProcessors();
	for (int sub = 0; sub < nSub; sub++)
	{
		Array<DefaultEventInfo> events;
		getDefaultEventInfo(events, sub);
		int nChans = events.size();
		for (int i = 0; i < nChans; i++)
		{
			const DefaultEventInfo& info = events[i];
			if (info.type != EventChannel::INVALID && info.nChannels > 0 && info.length > 0)
			{
				EventChannel* chan = new EventChannel(info.type, info.nChannels, info.length, info.sampleRate, this, sub);
				chan->m_nodeID = nodeId;
				if (info.name.isNotEmpty())
					chan->setName(info.name);
				if (info.description.isNotEmpty())
					chan->setDescription(info.description);
				if (info.identifier.isNotEmpty())
					chan->setIdentifier(info.identifier);
				eventChannelArray.add(chan);
			}
		}
	}
}*/

void GenericProcessor::getDefaultEventInfo(Array<DefaultEventInfo>& events, int subproc) const
{
	events.clear();
}


/** Used to get the number of samples in a given buffer, for a given channel. */
uint32 GenericProcessor::getNumSamples(int channelNum) const
{
	if (channelNum >= 0
		&& channelNum < continuousChannels.size())
	{
		uint16 streamId = continuousChannels[channelNum]->getStreamId();

		try
		{
			return numSamples.at(streamId);;
		}
		catch (...)
		{
			return 0;
		}
	}

	return 0;
}


/** Used to get the timestamp for a given buffer, for a given source node. */
juce::uint64 GenericProcessor::getTimestamp(int channelNum) const
{

	if (channelNum >= 0
		&& channelNum < continuousChannels.size())
	{
		uint16 streamId = continuousChannels[channelNum]->getStreamId();

		try
		{
			return timestamps.at(streamId);;
		}
		catch (...)
		{
			return 0;
		}
	}

	return 0;
}

uint32 GenericProcessor::getNumSourceSamples(uint16 streamId) const
{
	return numSamples.at(streamId);
}

juce::uint64 GenericProcessor::getSourceTimestamp(uint16 streamId) const
{
	return timestamps.at(streamId);
}


void GenericProcessor::setTimestampAndSamples(juce::uint64 timestamp, uint32 nSamples, uint16 streamId)
{

	MidiBuffer& eventBuffer = *m_currentMidiBuffer;
	LOGDD("Setting timestamp to ", timestamp);

	HeapBlock<char> data;
	size_t dataSize = SystemEvent::fillTimestampAndSamplesData(data, this, streamId, timestamp, nSamples);

	eventBuffer.addEvent(data, dataSize, 0);

	//since the processor generating the timestamp won't get the event, add it to the map
	timestamps[streamId] = timestamp;
	numSamples[streamId] = nSamples;

	if (m_needsToSendTimestampMessages[streamId] && nSamples > 0)
	{
		HeapBlock<char> data;
		size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, this, streamId, timestamp, false);

		eventBuffer.addEvent(data, dataSize, 0);

		m_needsToSendTimestampMessages.set(streamId, false);
	}
}


int GenericProcessor::processEventBuffer()
{
	//
	// This loops through all events in the buffer, and uses the BUFFER_SIZE
	// events to determine the number of samples in the current buffer. If
	// there are multiple such events, the one with the highest number of
	// samples will be used.
	// This approach is not ideal, as it will become a problem if we allow
	// the sample rate to change at different points in the signal chain.
	//
	int numRead = 0;

	MidiBuffer& eventBuffer = *m_currentMidiBuffer;

	if (eventBuffer.getNumEvents() > 0)
	{
		MidiBuffer::Iterator i(eventBuffer);

		const uint8* dataptr;
		int dataSize;

		int samplePosition = -1;

		while (i.getNextEvent(dataptr, dataSize, samplePosition))
		{
			//TODO: remove the mask when the probe system is implemented
			if (static_cast<Event::Type> (*dataptr) == Event::Type::SYSTEM_EVENT 
				&& static_cast<SystemEvent::Type>(*(dataptr + 1) == SystemEvent::Type::TIMESTAMP_AND_SAMPLES))
			{
				uint16 sourceProcessorId = *reinterpret_cast<const uint16*>(dataptr + 2);
				uint16 sourceStreamId = *reinterpret_cast<const uint16*>(dataptr + 4);
				uint32 sourceChannelIndex = *reinterpret_cast<const uint16*>(dataptr + 6);

				juce::uint64 timestamp = *reinterpret_cast<const juce::uint64*>(dataptr + 8);
				uint32 nSamples = *reinterpret_cast<const uint32*>(dataptr + 16);
				numSamples[sourceStreamId] = nSamples;
				timestamps[sourceStreamId] = timestamp;
			}
			//else {
			//	std::cout << nodeId << " : " << " received event" << std::endl;
			//}
			//set the "recorded" bit on the first byte. This will go away when the probe system is implemented.
			//doing a const cast is always a bad idea, but there's no better way to do this until whe change the event record system
			//if (nodeId < 900) //If the processor is not a specialized one
			//	*const_cast<uint8*>(dataptr + 0) = *(dataptr + 0) | 0x80;
		}
	}

	return numRead;
}


int GenericProcessor::checkForEvents(bool checkForSpikes)
{
	//std::cout << nodeId << ": check for events" << std::endl;

	if (m_currentMidiBuffer->getNumEvents() > 0)
	{
		//Since adding events to the buffer inside this loop could be dangerous, create a temporary event buffer
		//so any call to addEvent will operate on it;
		MidiBuffer temporaryEventBuffer;
		MidiBuffer* originalEventBuffer = m_currentMidiBuffer;
		m_currentMidiBuffer = &temporaryEventBuffer;

		MidiBuffer::Iterator i(*originalEventBuffer);
		MidiMessage message(0xf4);

		int samplePosition = 0;
		i.setNextSamplePosition(samplePosition);

		while (i.getNextEvent(message, samplePosition))
		{
			uint16 sourceProcessorId = EventBase::getProcessorId(message);
			uint16 sourceStreamId = EventBase::getStreamId(message);
			uint16 sourceChannelIdx = EventBase::getChannelIndex(message);

			if (EventBase::getBaseType(message) == Event::Type::PROCESSOR_EVENT)
			{
				const EventChannel* eventChannel = getEventChannel(sourceProcessorId, sourceStreamId, sourceChannelIdx);

				if (eventChannel != nullptr)
				{
					handleEvent(eventChannel, message, samplePosition);

					if (eventChannel->getType() == EventChannel::Type::TTL)
					{
						getEditor()->setTTLState(sourceStreamId,
								TTLEvent::getBit(message),
								TTLEvent::getState(message)
							);
					}
				}
					

			}
			else if (EventBase::getBaseType(message) == Event::Type::SYSTEM_EVENT 
				    && SystemEvent::getSystemEventType(message) == SystemEvent::Type::TIMESTAMP_SYNC_TEXT)
			{
				handleTimestampSyncTexts(message);
			}
			else if (checkForSpikes && EventBase::getBaseType(message) == Event::Type::SPIKE_EVENT)
			{
				const SpikeChannel* spikeChannel = getSpikeChannel(sourceProcessorId, sourceStreamId, sourceChannelIdx);

				if (spikeChannel != nullptr)
					handleSpike(spikeChannel, message, samplePosition);
			}
		}
		//Restore the original buffer pointer and, if some new event has been added here, copy it to the original buffer
		m_currentMidiBuffer = originalEventBuffer;

		if (temporaryEventBuffer.getNumEvents() > 0)
		{
			m_currentMidiBuffer->addEvents(temporaryEventBuffer, 0, -1, 0);
			//std::cout << nodeId << " added " << temporaryEventBuffer.getNumEvents() << " events." << std::endl;
		}
			
		return 0;
	}

	return -1;
}

void GenericProcessor::addEvent(int channelIndex, const Event* event, int sampleNum)
{
	addEvent(eventChannels[channelIndex], event, sampleNum);
}

void GenericProcessor::addEvent(const EventChannel* channel, const Event* event, int sampleNum)
{
	size_t size = channel->getDataSize() + channel->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
	HeapBlock<char> buffer(size);
	event->serialize(buffer, size);
	m_currentMidiBuffer->addEvent(buffer, size, sampleNum >= 0 ? sampleNum : 0);
}

void GenericProcessor::broadcastMessage(String msg)
{
	AccessClass::getMessageCenter()->broadcastMessage(msg);
}

void GenericProcessor::addSpike(int channelIndex, const Spike* spike, int sampleNum)
{
	addSpike(spikeChannels[channelIndex], spike, sampleNum);
}

void GenericProcessor::addSpike(const SpikeChannel* channel, const Spike* spike, int sampleNum)
{
	size_t size = channel->getDataSize() 
		+ channel->getTotalEventMetadataSize() 
		+ SPIKE_BASE_SIZE 
		+ channel->getNumChannels()*sizeof(float);

	HeapBlock<char> buffer(size);

	spike->serialize(buffer, size);

	m_currentMidiBuffer->addEvent(buffer, size, sampleNum >= 0 ? sampleNum : 0);
}


void GenericProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& eventBuffer)
{
	m_currentMidiBuffer = &eventBuffer;

	processEventBuffer(); // extract buffer sizes and timestamps,

	m_lastProcessTime = Time::getHighResolutionTicks();

	process(buffer);

}

Array<const EventChannel*> GenericProcessor::getEventChannels()
{
	Array<const EventChannel*> channels;

	for (int i = 0; i < eventChannels.size(); i++)
	{
		channels.add(eventChannels[i]);
	}

	return channels;
}

const ContinuousChannel* GenericProcessor::getContinuousChannel(uint16 processorId, uint16 streamId, uint16 localIndex) const
{
	return continuousChannelMap.at(processorId).at(streamId).at(localIndex);
}

const EventChannel* GenericProcessor::getEventChannel(uint16 processorId, uint16 streamId, uint16 localIndex) const
{
	return eventChannelMap.at(processorId).at(streamId).at(localIndex); 
}

const SpikeChannel* GenericProcessor::getSpikeChannel(uint16 processorId, uint16 streamId, uint16 localIndex) const
{
	return spikeChannelMap.at(processorId).at(streamId).at(localIndex);
}

DataStream* GenericProcessor::getDataStream(uint16 streamId) const
{
	return dataStreamMap.at(streamId);
}

const ConfigurationObject* GenericProcessor::getConfigurationObject(int index) const
{
	return configurationObjects[index];
}

int GenericProcessor::getTotalEventChannels() const
{
	return eventChannels.size();
}

int GenericProcessor::getTotalSpikeChannels() const
{
	return spikeChannels.size();
}

int GenericProcessor::getTotalConfigurationObjects() const
{
	return configurationObjects.size();
}

/*int GenericProcessor::getContinuousChannelIndex(int channelIdx, uint32 streamId) const
{
	try
	{
		return continuousChannelMap.at(streamId).at(channelIdx);
	}
	catch (...)
	{
		return -1;
	}
}

int GenericProcessor::getEventChannelIndex(int channelIdx, int processorID, int subProcessorIdx) const
{
	uint32 sourceID = getProcessorFullId(processorID, subProcessorIdx);
	try
	{
		return eventChannelMap.at(sourceID).at(channelIdx);
	}
	catch (...)
	{
		return -1;
	}
}*/

/*int GenericProcessor::getEventChannelIndex(const Event* event) const
{
	return getEventChannelIndex(event->getSourceIndex(), event->getSourceID(), event->getSubProcessorIdx());
}

int GenericProcessor::getSpikeChannelIndex(int channelIdx, int processorID, int subProcessorIdx) const
{
	uint32 sourceID = getProcessorFullId(processorID, subProcessorIdx);
	try
	{
		return spikeChannelMap.at(sourceID).at(channelIdx);
	}
	catch (...)
	{
		return -1;
	}
}

int GenericProcessor::getSpikeChannelIndex(const SpikeEvent* event) const
{
	return getSpikeChannelIndex(event->getSourceIndex(), event->getSourceID(), event->getSubProcessorIdx());
}*/


/////// ---- LOADING AND SAVING ---- //////////


void GenericProcessor::saveToXml(XmlElement* parentElement)
{
	parentElement->setAttribute("NodeId", nodeId);

	saveCustomParametersToXml(parentElement);

	// loop through the channels

	for (int i = 0; i < continuousChannels.size(); ++i)
	{
		if (!isSplitter() && !isMerger())
			saveChannelParametersToXml(parentElement, continuousChannels[i]);
	}

	for (int i = 0; i < eventChannels.size(); ++i)
	{
		if (!isSplitter() && !isMerger())
			saveChannelParametersToXml(parentElement, eventChannels[i]);
	}

	for (int i = 0; i < spikeChannels.size(); ++i)
	{
		if (!isSplitter() && !isMerger())
			saveChannelParametersToXml(parentElement, spikeChannels[i]);
	}

	// Save editor parameters:
	XmlElement* editorChildNode = parentElement->createNewChildElement("EDITOR");
	getEditor()->saveEditorParameters(editorChildNode);
}


void GenericProcessor::saveCustomParametersToXml(XmlElement* parentElement)
{
}

void GenericProcessor::saveChannelParametersToXml(XmlElement* parentElement, InfoObject* channel)
{
	XmlElement* channelInfo;

	if (channel->getType() == InfoObject::Type::CONTINUOUS_CHANNEL)
	{
		channelInfo = parentElement->createNewChildElement("CHANNEL");
		channelInfo->setAttribute("name", channel->getName());
		channelInfo->setAttribute("number", channel->getGlobalIndex());

		bool p, r, a;

		getEditor()->getChannelSelectionState(channel->getGlobalIndex(), &p, &r, &a);

		XmlElement* selectionState = channelInfo->createNewChildElement("SELECTIONSTATE");
		selectionState->setAttribute("param", p);
		//selectionState->setAttribute("record", r);
		//selectionState->setAttribute("audio", a);
	}
	else if (channel->getType() == InfoObject::Type::EVENT_CHANNEL)
	{
		channelInfo = parentElement->createNewChildElement("EVENTCHANNEL");
		channelInfo->setAttribute("name", channel->getName());
		channelInfo->setAttribute("number", channel->getGlobalIndex());

	}
	else if (channel->getType() == InfoObject::Type::SPIKE_CHANNEL)
	{
		channelInfo = parentElement->createNewChildElement("SPIKECHANNEL");
		channelInfo->setAttribute("name", String(channel->getName()));
		channelInfo->setAttribute("number", channel->getGlobalIndex());
	}

	saveCustomChannelParametersToXml(channelInfo, channel);

	// deprecated parameter configuration:
	LOGDD("Creating Parameters");
	// int maxsize = parameters.size();
	// String parameterName;
	// String parameterValue;
	// XmlElement* parameterChildNode;

	// // save any attributes that belong to "Parameter" objects
	// for (int n = 0; n < maxsize; n++)
	// {
	//     parameterName = getParameterName(n);

	//     parameterChildNode = channelParent->createNewChildElement("PARAMETER");
	//     parameterChildNode->setAttribute("name", parameterName);

	//     var parameterVar = getParameterVar(n, channelNumber-1);
	//     parameterValue = parameterVar.toString();
	//     parameterChildNode->addTextElement(parameterValue);
	// }
}

void GenericProcessor::saveCustomChannelParametersToXml(XmlElement* channelInfo, InfoObject* channel)
{
}


void GenericProcessor::loadFromXml()
{

	if (parametersAsXml != nullptr)
	{
        if (!m_paramsWereLoaded)
        {
			LOGD("Loading parameters for ", m_name);

            // use parametersAsXml to restore state
            loadCustomParametersFromXml();

            // load editor parameters
            forEachXmlChildElement(*parametersAsXml, xmlNode)
            {
                if (xmlNode->hasTagName("EDITOR"))
                {
                    getEditor()->loadEditorParameters(xmlNode);
                }
            }

            forEachXmlChildElement(*parametersAsXml, xmlNode)
            {
                if (xmlNode->hasTagName("CHANNEL"))
                {
                    loadChannelParametersFromXml(xmlNode, InfoObject::Type::CONTINUOUS_CHANNEL);
                }
                else if (xmlNode->hasTagName("EVENTCHANNEL"))
                {
                    loadChannelParametersFromXml(xmlNode, InfoObject::Type::EVENT_CHANNEL);
                }
                else if (xmlNode->hasTagName("SPIKECHANNEL"))
                {
                    loadChannelParametersFromXml(xmlNode, InfoObject::Type::SPIKE_CHANNEL);
                }
            }
        }
	}

	m_paramsWereLoaded = true;
}


void GenericProcessor::loadChannelParametersFromXml(XmlElement* channelInfo, InfoObject::Type type)
{
	int channelNum = channelInfo->getIntAttribute("number");

	if (type == InfoObject::Type::CONTINUOUS_CHANNEL)
	{
		forEachXmlChildElement(*channelInfo, subNode)
		{
			if (subNode->hasTagName("SELECTIONSTATE"))
			{
				getEditor()->setChannelSelectionState(channelNum,
					subNode->getBoolAttribute("param"),
					subNode->getBoolAttribute("record"),
					subNode->getBoolAttribute("audio"));
			}
		}
	}

	loadCustomChannelParametersFromXml(channelInfo, type);
}


void GenericProcessor::loadCustomParametersFromXml() { }
void GenericProcessor::loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObject::Type type) { }

void GenericProcessor::setCurrentChannel(int chan)
{
	currentChannel = chan;
}


void GenericProcessor::setProcessorType(PluginProcessorType processorType)
{
	m_processorType = processorType;
}


bool GenericProcessor::canSendSignalTo(GenericProcessor*) const { return true; }

bool GenericProcessor::generatesTimestamps() const { return false; }

bool GenericProcessor::isFilter()        const  { return getProcessorType() == PROCESSOR_TYPE_FILTER; }
bool GenericProcessor::isSource()        const  { return getProcessorType() == PROCESSOR_TYPE_SOURCE; }
bool GenericProcessor::isSink()          const  { return getProcessorType() == PROCESSOR_TYPE_SINK; }
bool GenericProcessor::isSplitter()      const  { return getProcessorType() == PROCESSOR_TYPE_SPLITTER; }
bool GenericProcessor::isMerger()        const  { return getProcessorType() == PROCESSOR_TYPE_MERGER; }
bool GenericProcessor::isUtility()       const  { return getProcessorType() == PROCESSOR_TYPE_UTILITY; }
bool GenericProcessor::isRecordNode()    const  { return getProcessorType() == PROCESSOR_TYPE_RECORD_NODE; }

int GenericProcessor::getNumInputs() const  
{ 
	if (sourceNode != nullptr)
	{
		return sourceNode->continuousChannels.size();
	}
	else {
		return 0;
	}
}

int GenericProcessor::getNumOutputs() const   
{ 
	return continuousChannels.size(); 
}

int GenericProcessor::getNumOutputsForStream(int streamIdx) const
{
	return streams[streamIdx]->getChannelCount();
}

int GenericProcessor::getNodeId() const                     { return nodeId; }

float GenericProcessor::getDefaultSampleRate() const        { return 44100.0; }
float GenericProcessor::getSampleRate(int) const               { return getDefaultSampleRate(); }

GenericProcessor* GenericProcessor::getSourceNode() const { return sourceNode; }
GenericProcessor* GenericProcessor::getDestNode()   const { return destNode; }

int GenericProcessor::getNumDataStreams() const { return streams.size(); }

GenericEditor* GenericProcessor::getEditor() const { return editor.get(); }

AudioBuffer<float>* GenericProcessor::getContinuousBuffer() const { return 0; }
MidiBuffer* GenericProcessor::getEventBuffer() const             { return 0; }

void GenericProcessor::switchIO(int)   { }
void GenericProcessor::switchIO()       { }

void GenericProcessor::setPathToProcessor(GenericProcessor* p)   { }
void GenericProcessor::setMergerSourceNode(GenericProcessor* sn)  { }
void GenericProcessor::setSplitterDestNode(GenericProcessor* dn)  { }

bool GenericProcessor::startAcquisition() { return true; }
bool GenericProcessor::stopAcquisition() { return true; }

void GenericProcessor::startRecording() { }
void GenericProcessor::stopRecording()  { }

void GenericProcessor::updateSettings() { }

void GenericProcessor::handleEvent(const EventChannel* eventInfo, const EventPacket& packet, int samplePosition) {}

void GenericProcessor::handleSpike(const SpikeChannel* spikeInfo, const EventPacket& packet, int samplePosition) {}

void GenericProcessor::handleTimestampSyncTexts(const EventPacket& packet) {};

/*void GenericProcessor::setEnabledState(bool t)
{
	isEnabled = t;
    
    if (isEnabled)
        getEditor()->enable();
    else
        getEditor()->disable();
}*/


GenericProcessor::DefaultEventInfo::DefaultEventInfo(EventChannel::Type t, unsigned int c, unsigned int l, float s)
	:type(t),
	nChannels(c),
	length(l),
	sampleRate(s)
{
}

GenericProcessor::DefaultEventInfo::DefaultEventInfo()
	:type(EventChannel::INVALID),
	nChannels(0),
	length(0),
	sampleRate(44100)
{}

/*uint32 GenericProcessor::getProcessorFullId(uint16 sid, uint16 subid)
{
	return (uint32(sid) << 16) + subid;
}

uint16 GenericProcessor::getNodeIdFromFullId(uint32 fid)
{
	return (fid & 0xFFFF0000 ) >> 16;
}

uint16 GenericProcessor::getSubProcessorFromFullId(uint32 fid)
{
	return (fid & 0x0000FFFF);
}

int64 GenericProcessor::getLastProcessedsoftwareTime() const
{
	return m_lastProcessTime;
}

void ChannelCreationIndices::clearChannelCreationCounts()
{
	dataChannelCount = 0;
	dataChannelTypeCount.clear();
	eventChannelCount = 0;
	eventChannelTypeCount.clear();
	spikeChannelCount = 0;
	spikeChannelTypeCount.clear();
}*/
