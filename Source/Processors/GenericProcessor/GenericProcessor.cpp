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
#include "../../UI/UIComponent.h"
#include "../../AccessClass.h"

#include <exception>


const String GenericProcessor::m_unusedNameString ("xxx-UNUSED-OPEN-EPHYS-xxx");

GenericProcessor::GenericProcessor (const String& name)
    : sourceNode                    (0)
    , destNode                      (0)
    , isEnabled                     (true)
    , wasConnected                  (false)
    , nextAvailableChannel          (0)
    , saveOrder                     (-1)
    , loadOrder                     (-1)
    , currentChannel                (-1)
    , editor                        (nullptr)
    , parametersAsXml               (nullptr)
    , sendSampleCount               (true)
    , m_name                            (name)
    , m_isParamsWereLoaded              (false)
    , m_processorType                   (PROCESSOR_TYPE_UTILITY)
{
    settings.numInputs = settings.numOutputs = 0;
}


GenericProcessor::~GenericProcessor()
{
}


AudioProcessorEditor* GenericProcessor::createEditor()
{
    editor = new GenericEditor (this, true);
    return editor;
}


void GenericProcessor::setNodeId (int id)
{
    nodeId = id;

    if (editor != 0)
    {
        editor->updateName();
    }
}


Parameter* GenericProcessor::getParameterByName (String name)
{
    const int numParameters = getNumParameters();
    // doesn't work
    for (int i = 0; i < numParameters; ++i)
    {
        const auto parameter = parameters[i];
        const String parameterName = parameter->getName();

        if (parameterName.compare (name) == 0) // fails at this point
            return parameter;//parameters.getReference(i);
    }

    Parameter* nullParam = new Parameter ("VOID", false, -1);

    return nullParam;
}


Parameter* GenericProcessor::getParameterObject (int parameterIndex) const
{
    return parameters[parameterIndex];
}


void GenericProcessor::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);
    std::cout << "Setting parameter" << std::endl;

    if (currentChannel >= 0)
        parameters[parameterIndex]->setValue (newValue, currentChannel);
}


const String GenericProcessor::getParameterName (int parameterIndex)
{
    return parameters[parameterIndex]->getName();
}


const String GenericProcessor::getParameterText (int parameterIndex)
{
    return parameters[parameterIndex]->getDescription();
}


var GenericProcessor::getParameterVar (int parameterIndex, int parameterChannel)
{
    const auto parameter = parameters[parameterIndex];
    return parameter->operator[] (parameterChannel);
}


void GenericProcessor::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
}


void GenericProcessor::releaseResources()
{
    // use the disable() function instead
    // releaseResources() is called by Juce at unpredictable times
    // disable() is only called by the ProcessorGraph at the end of acquisition
}


int GenericProcessor::getNextChannel (bool increment)
{
    int chan = nextAvailableChannel;

    //std::cout << "Next channel: " << chan << ", num inputs: " << getNumInputs() << std::endl;

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


void GenericProcessor::setSourceNode (GenericProcessor* sn)
{
    //std::cout << "My name is " << getName() << ". Setting source node." << std::endl;

    if (! isSource())
    {
        //	std::cout << " I am not a source." << std::endl;

        if (sn != 0)
        {
            //	std::cout << " The source is not blank." << std::endl;

            if (! sn->isSink())
            {
                //		std::cout << " The source is not a sink." << std::endl;
                if (sourceNode != sn)
                {
                    //			std::cout << " The source is new and named " << sn->getName() << std::endl;

                    if (this->isMerger())
                        setMergerSourceNode (sn);
                    else
                        sourceNode = sn;

                    sn->setDestNode (this);
                }
                else
                {
                    //			std::cout << "  The source node is not new." << std::endl;
                }
            }
            else
            {
                //		std::cout << " The source is a sink." << std::endl;
                sourceNode = 0;
            }

        }
        else
        {
            //		std::cout << " The source is blank." << std::endl;
            sourceNode = 0;
        }
    }
    else
    {
        //	std::cout << " I am a source. I can't have a source node." << std::endl;

        if (sn != 0)
            sn->setDestNode (this);
    }
}


void GenericProcessor::setDestNode (GenericProcessor* dn)
{
    //	std::cout << "My name is " << getName() << ". Setting dest node." << std::endl;

    if (! isSink())
    {
        //	std::cout << "  I am not a sink." << std::endl;

        if (dn != 0)
        {
            //		std::cout << "  The dest node is not blank." << std::endl;
            if (!dn->isSource())
            {
                //		std::cout << "  The dest node is not a source." << std::endl;

                if (destNode != dn)
                {
                    //		std::cout << "  The dest node is new and named " << dn->getName() << std::endl;
                    //
                    if (this->isSplitter())
                        setSplitterDestNode (dn);
                    else
                        destNode = dn;

                    dn->setSourceNode (this);
                }
                else
                {
                    //		std::cout << "  The dest node is not new." << std::endl;
                }
            }
            else
            {
                //	std::cout << "  The dest node is a source." << std::endl;

                destNode = 0;
            }
        }
        else
        {
            //	std::cout << "  The dest node is blank." << std::endl;

            destNode = 0;
        }
    }
    else
    {
        //std::cout << "  I am a sink, I can't have a dest node." << std::endl;
        //if (dn != 0)
        //	dn->setSourceNode(this);
    }
}


void GenericProcessor::clearSettings()
{
    //std::cout << "Generic processor clearing settings." << std::endl;

    settings.originalSource = 0;
    settings.numInputs = 0;
    settings.numOutputs = 0;

    // std::cout << "Record status size = " << recordStatus.size() << std::endl;

    if (m_recordStatus.size() < dataChannelArray.size())
        m_recordStatus.resize (dataChannelArray.size());

    if (m_monitorStatus.size() < dataChannelArray.size())
        m_monitorStatus.resize (dataChannelArray.size());

    for (int i = 0; i < dataChannelArray.size(); ++i)
    {
        // std::cout << channels[i]->getRecordState() << std::endl;
        m_recordStatus.set    (i, dataChannelArray[i]->getRecordState());
        m_monitorStatus.set   (i, dataChannelArray[i]->isMonitored());
    }

    dataChannelArray.clear();
    eventChannelArray.clear();
	spikeChannelArray.clear();
	configurationObjectArray.clear();
}


void GenericProcessor::update()
{
    std::cout << getName() << " updating settings." << std::endl;

    // ---- RESET EVERYTHING ---- ///
    clearSettings();

    if (sourceNode != 0) // copy settings from source node
    {
        // everything is inherited except numOutputs
        settings = sourceNode->settings;
        settings.numInputs = settings.numOutputs;
        settings.numOutputs = settings.numInputs;

        for (int i = 0; i < sourceNode->dataChannelArray.size(); ++i)
        {
            DataChannel* sourceChan = sourceNode->dataChannelArray[i];
            DataChannel* ch = new DataChannel (*sourceChan);


            if (i < m_recordStatus.size())
            {
                ch->setRecordState (m_recordStatus[i]);
                ch->setMonitored( m_monitorStatus[i]);
            }

			ch->addToHistoricString(getName());
            dataChannelArray.add (ch);
        }

        for (int i = 0; i < sourceNode->eventChannelArray.size(); ++i)
        {
            EventChannel* sourceChan = sourceNode->eventChannelArray[i];
            EventChannel* ch = new EventChannel (*sourceChan);
            eventChannelArray.add (ch);
        }
		for (int i = 0; i < sourceNode->spikeChannelArray.size(); ++i)
		{
			SpikeChannel* sourceChan = sourceNode->spikeChannelArray[i];
			SpikeChannel* ch = new SpikeChannel(*sourceChan);
			spikeChannelArray.add(ch);
		}
		for (int i = 0; i < sourceNode->configurationObjectArray.size(); ++i)
		{
			ConfigurationObject* sourceChan = sourceNode->configurationObjectArray[i];
			ConfigurationObject* ch = new ConfigurationObject(*sourceChan);
			configurationObjectArray.add(ch);
		}
    }
    else // generate new settings
    {
        settings.numOutputs = getTotalNumOutputChannels();

        std::cout << getName() << " setting num outputs to " << settings.numOutputs << std::endl;
		createDataChannels(); //Only sources can create data channels
		for (int i = 0; i < dataChannelArray.size(); i++)
		{
			if (i < m_recordStatus.size())
				dataChannelArray[i]->setRecordState(m_recordStatus[i]);
			else
				if (isSource())
					dataChannelArray[i]->setRecordState(true);
		}
    }

	//Any processor, not only sources, can add new event and spike channels. It's best to do it in their dedicated methods
	createEventChannels();
	createSpikeChannels();
	createConfigurationObjects();

    if (this->isSink())
    {
        settings.numOutputs = 0;
    }

    updateSettings(); // allow processors to change custom settings


	//Recreate the channel indexes
	dataChannelMap.clear();
	eventChannelMap.clear();
	spikeChannelMap.clear();

	for (int i = 0; i < dataChannelArray.size(); i++)
	{
		DataChannel* channel = dataChannelArray[i];
		channel->m_nodeID = nodeId;
		uint32 sourceID = getProcessorFullId(channel->getSourceNodeID(), channel->getSubProcessorIdx());
		dataChannelMap[sourceID][channel->getSourceIndex()] = i;
	}
	for (int i = 0; i < eventChannelArray.size(); i++)
	{
		EventChannel* channel = eventChannelArray[i];
		channel->m_nodeID = nodeId;
		uint32 sourceID = getProcessorFullId(channel->getSourceNodeID(), channel->getSubProcessorIdx());
		eventChannelMap[sourceID][channel->getSourceIndex()] = i;
	}
	for (int i = 0; i < spikeChannelArray.size(); i++)
	{
		SpikeChannel* channel = spikeChannelArray[i];
		channel->m_nodeID = nodeId;
		uint32 sourceID = getProcessorFullId(channel->getSourceNodeID(), channel->getSubProcessorIdx());
		spikeChannelMap[sourceID][channel->getSourceIndex()] = i;
	}

	m_needsToSendTimestampMessages.clear();
	m_needsToSendTimestampMessages.insertMultiple(-1, false, getNumSubProcessors());

    // required for the ProcessorGraph to know the
    // details of this processor:
    setPlayConfigDetails (getNumInputs(),  // numIns
                          getNumOutputs(), // numOuts
                          44100.0,         // sampleRate
                          128);            // blockSize

    editor->update(); // allow the editor to update its settings
}

void GenericProcessor::createDataChannels()
{
	createDataChannelsByType(DataChannel::HEADSTAGE_CHANNEL);
	createDataChannelsByType(DataChannel::AUX_CHANNEL);
	createDataChannelsByType(DataChannel::ADC_CHANNEL);
}

void GenericProcessor::createDataChannelsByType(DataChannel::DataChannelTypes type)
{
	int nSub = getNumSubProcessors();
	for (int sub = 0; sub < nSub; sub++)
	{
		int nChans = getDefaultNumDataOutputs(type, sub);
		for (int i = 0; i < nChans; i++)
		{
			DataChannel* chan = new DataChannel(type, this, sub);
			chan->setSampleRate(getSampleRate(sub));
			chan->setBitVolts(getBitVolts(sub));
			chan->addToHistoricString(getName());
			chan->m_nodeID = nodeId;
			dataChannelArray.add(chan);
		}
	}
}

void GenericProcessor::createEventChannels()
{
	int nSub = getNumSubProcessors();
	for (int sub = 0; sub < nSub; sub++)
	{
		Array<DefaultEventInfo> events;
		getDefaultEventInfo(events, sub);
		int nChans = events.size();
		for (int i = 0; i < nChans; i++)
		{
			if (events[i].type != EventChannel::INVALID && events[i].nChannels > 0 && events[i].length > 0)
			{
				EventChannel* chan = new EventChannel(events[i].type, events[i].nChannels, events[i].length, this, sub);
				chan->setSampleRate(getSampleRate(sub));
				chan->m_nodeID = nodeId;
				eventChannelArray.add(chan);
			}
		}
	}
}

void GenericProcessor::getDefaultEventInfo(Array<DefaultEventInfo>& events, int subproc) const
{
	events.clear();
}

void GenericProcessor::createSpikeChannels() {};
void GenericProcessor::createConfigurationObjects() {};

void GenericProcessor::setAllChannelsToRecord()
{
    m_recordStatus.resize (dataChannelArray.size());

    for (int i = 0; i < dataChannelArray.size(); ++i)
    {
        m_recordStatus.set (i, true);
    }

    // std::cout << "Setting all channels to record for source." << std::endl;
}


void GenericProcessor::setRecording (bool state)
{
    GenericEditor* ed = getEditor();
    if (state)
    {
        if (ed != 0)
            ed->startRecording();

        startRecording();
        if (isGeneratesTimestamps())
        {
			m_needsToSendTimestampMessages.clearQuick();
			m_needsToSendTimestampMessages.insertMultiple(-1, true, getNumSubProcessors());
        }
    }
    else
    {
        if (ed != 0)
            ed->stopRecording();

        stopRecording();
		m_needsToSendTimestampMessages.clearQuick();
		m_needsToSendTimestampMessages.insertMultiple(-1, false, getNumSubProcessors());
    }
}


void GenericProcessor::enableEditor()
{
    GenericEditor* ed = getEditor();

    if (ed != 0)
        ed->startAcquisition();
}


void GenericProcessor::disableEditor()
{
    GenericEditor* ed = getEditor();

    if (ed != nullptr)
        ed->stopAcquisition();
}


/** Used to get the number of samples in a given buffer, for a given channel. */
uint32 GenericProcessor::getNumSamples (int channelNum) const
{
    int sourceNodeId = 0;
	int subProcessorId = 0;
    int nSamples     = 0;

    if (channelNum >= 0
        && channelNum < dataChannelArray.size())
    {
        sourceNodeId = dataChannelArray[channelNum]->getSourceNodeID();
		subProcessorId = dataChannelArray[channelNum]->getSubProcessorIdx();
    }
    else
    {
        return 0;
    }

    // std::cout << "Requesting samples for channel " << channelNum << " with source node " << sourceNodeId << std::endl;
	uint32 sourceID = getProcessorFullId(sourceNodeId, subProcessorId);
    try
    {
        nSamples = numSamples.at (sourceID);
    }
    catch (std::exception& e)
    {
        return 0;
    }

    //std::cout << nSamples << " were found." << std::endl;

    return nSamples;
}


/** Used to get the timestamp for a given buffer, for a given source node. */
uint64 GenericProcessor::getTimestamp (int channelNum) const
{
    int sourceNodeId = 0;
	int subProcessorIdx = 0;
    int64 ts         = 0;

    if (channelNum >= 0 
        && channelNum < dataChannelArray.size())
    {
        sourceNodeId = dataChannelArray[channelNum]->getSourceNodeID();
		subProcessorIdx = dataChannelArray[channelNum]->getSubProcessorIdx();
    }
    else
    {
        return 0;
    }

	uint32 sourceID = getProcessorFullId(sourceNodeId, subProcessorIdx);
    try
    {
        ts = timestamps.at (sourceID);
    }
    catch (std::exception& e)
    {
        return 0;
    }

    return ts;
}


/** Used to set the timestamp for a given buffer, for a given channel. */
void GenericProcessor::setTimestampAndSamples(uint64 timestamp, uint32 nSamples, int subProcessorIdx)
{
	/** Event packet structure
	* SYSTEM_EVENT - 1 byte
	* TIMESTAMP_AND_SAMPLES - 1 byte
	* Source processorID - 2 bytes
	* Source Subprocessor index - 2 bytes
	* Zero-fill (to maintain aligment with other events) - 2 bytes
	* Timestamp - 8 bytes
	* Buffer sample nimber - 4 bytes
	*/

	MidiBuffer& eventBuffer = *m_currentMidiBuffer;
    //std::cout << "Setting timestamp to " << timestamp << std:;endl;

	uint8 data[20];
	data[0] = SYSTEM_EVENT;
	data[1] = TIMESTAMP_AND_SAMPLES;
	*reinterpret_cast<uint16*>(data + 2) = nodeId;
	*reinterpret_cast<uint16*>(data + 4) = subProcessorIdx;
	data[6] = 0;
	data[7] = 0;
	*reinterpret_cast<uint64*>(data + 8) = timestamp;
	*reinterpret_cast<uint32*>(data + 16) = nSamples;

	eventBuffer.addEvent(data, 20, 0);

	uint32 sourceID = getProcessorFullId(nodeId, subProcessorIdx);

    //since the processor generating the timestamp won't get the event, add it to the map
    timestamps[sourceID] = timestamp;
	numSamples[sourceID] = nSamples;

    if (m_needsToSendTimestampMessages[subProcessorIdx])
    {
		/** Event packet structure
		* SYSTEM_EVENT - 1 byte
		* TIMESTAMP_SYNC_TEXT - 1 byte
		* Source processorID - 2 bytes
		* Source Subprocessor index - 2 bytes
		* Zero-fill (to maintain aligment with other events) - 2 bytes
		* Timestamp - 8 bytes
		* string - variable
		*/
        String eventString = "Processor: "
								+ getName()
								+ " Id: "
                                + String (nodeId) 
								+ " subProcessor: "
								+ String (subProcessorIdx);
                                + " start time: " 
                                + String (timestamp) 
                                + "@" 
                                + String (getSampleRate()) 
                                + "Hz";

		size_t textSize = eventString.getNumBytesAsUTF8();
		size_t dataSize = 17 + textSize;
		HeapBlock<char> data(dataSize, true);
		data[0] = SYSTEM_EVENT;
		data[1] = TIMESTAMP_SYNC_TEXT;
		*reinterpret_cast<uint16*>(data.getData() + 2) = nodeId;
		*reinterpret_cast<uint16*>(data.getData() + 4) = subProcessorIdx;
		*reinterpret_cast<uint64*>(data.getData() + 8) = timestamp;
		memcpy(data.getData() + 16, eventString.toUTF8(), textSize);

		eventBuffer.addEvent(data, dataSize, 0);

		m_needsToSendTimestampMessages.set(subProcessorIdx, false);
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
			if (*(dataptr + 0) & 0x7F == SYSTEM_EVENT && *(dataptr + 1) == TIMESTAMP_AND_SAMPLES)
			{
				uint16 sourceNodeID = *reinterpret_cast<const uint16*>(dataptr + 2);
				uint16 sourceSubProcessorIdx = *reinterpret_cast<const uint16*>(dataptr + 4);
				uint32 sourceID = getProcessorFullId(sourceNodeID, sourceSubProcessorIdx);

				uint64 timestamp = *reinterpret_cast<const uint64*>(dataptr + 8);
				uint32 nSamples = *reinterpret_cast<const uint32*>(dataptr + 16);
				numSamples[sourceID] = nSamples;
				timestamps[sourceID] = timestamp;
			}
			//set the "recorded" bit on the first byte. This will go away when the probe system is implemented.
			//doing a const cast is always a bad idea, but there's no better way to do this until whe change the event record system
			*const_cast<uint8*>(dataptr + 0) = *(dataptr + 0) | 0x80;
		}
	}

	return numRead;
}


int GenericProcessor::checkForEvents(bool checkForSpikes)
{
    if (m_currentMidiBuffer->getNumEvents() > 0)
    {
        // int m = midiMessages.getNumEvents();
        //std::cout << m << " events received by node " << getNodeId() << std::endl;

        MidiBuffer::Iterator i (*m_currentMidiBuffer);
        MidiMessage message (0xf4);

        int samplePosition = 0;
        i.setNextSamplePosition (samplePosition);

        while (i.getNextEvent (message, samplePosition))
        {
			uint16 sourceId = EventBase::getSourceID(message);
			uint16 subProc = EventBase::getSubProcessorIdx(message);
			uint16 index = EventBase::getSourceIndex(message);
			if (EventBase::getBaseType(message) == EventType::PROCESSOR_EVENT)
			{
				int index = getEventChannelIndex(index, sourceId, subProc);
				if (index >= 0)
					handleEvent(eventChannelArray[index], message, samplePosition);
			}
			else if (checkForSpikes && EventBase::getBaseType(message) == EventType::SPIKE_EVENT)
			{
				int index = getSpikeChannelIndex(index, sourceId, subProc);
				if (index >= 0)
					handleSpike(spikeChannelArray[index], message, samplePosition);
			}
        }
		return 0;
    }

    return -1;
}

void GenericProcessor::addEvent(int channelIndex, const Event* event, int sampleNum)
{
	addEvent(eventChannelArray[channelIndex], event, sampleNum);
}

void GenericProcessor::addEvent(const EventChannel* channel, const Event* event, int sampleNum)
{
	size_t size = channel->getDataSize() + channel->getTotalEventMetaDataSize() + EVENT_BASE_SIZE;
	HeapBlock<char> buffer(size);
	event->serialize(buffer, size);
	m_currentMidiBuffer->addEvent(buffer, size, sampleNum);
}

void GenericProcessor::addSpike(int channelIndex, const SpikeEvent& event, int sampleNum)
{
	addSpike(spikeChannelArray[channelIndex], event, sampleNum);
}

void GenericProcessor::addSpike(const SpikeChannel* channel, const SpikeEvent& event, int sampleNum)
{
	size_t size = channel->getDataSize() + channel->getTotalEventMetaDataSize() + SPIKE_BASE_SIZE + channel->getNumChannels()*sizeof(float);
	HeapBlock<char> buffer(size);
	event.serialize(buffer, size);
	m_currentMidiBuffer->addEvent(buffer, size, sampleNum);
}


void GenericProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& eventBuffer)
{
	m_currentMidiBuffer = &eventBuffer;
    processEventBuffer (); // extract buffer sizes and timestamps,
    // set flag on all TTL events to zero

    process (buffer);
}

const DataChannel* GenericProcessor::getDataChannel(int index) const
{
	return dataChannelArray[index];
}

const EventChannel* GenericProcessor::getEventChannel(int index) const
{
	return eventChannelArray[index];
}

const SpikeChannel* GenericProcessor::getSpikeChannel(int index) const
{
	return spikeChannelArray[index];
}

const ConfigurationObject* GenericProcessor::getConfigurationObject(int index) const
{
	return configurationObjectArray[index];
}

int GenericProcessor::getDataChannelIndex(int channelIdx, int processorID, int subProcessorIdx) const
{
	uint32 sourceID = getProcessorFullId(processorID, subProcessorIdx);
	try
	{
		return dataChannelMap.at(sourceID).at(channelIdx);
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

/////// ---- LOADING AND SAVING ---- //////////


void GenericProcessor::saveToXml (XmlElement* parentElement)
{
    parentElement->setAttribute ("NodeId", nodeId);

    saveCustomParametersToXml (parentElement);

    // loop through the channels

    for (int i = 0; i < dataChannelArray.size(); ++i)
    {
        if (! isSplitter() && ! isMerger())
            saveChannelParametersToXml (parentElement, i, InfoObjectCommon::DATA_CHANNEL);
    }

    for (int i = 0; i < eventChannelArray.size(); ++i)
    {
        if (! isSplitter() && ! isMerger())
            saveChannelParametersToXml (parentElement, i, InfoObjectCommon::EVENT_CHANNEL);
    }

	for (int i = 0; i < spikeChannelArray.size(); ++i)
	{
		if (!isSplitter() && !isMerger())
			saveChannelParametersToXml(parentElement, i, InfoObjectCommon::SPIKE_CHANNEL);
	}

    // Save editor parameters:
    XmlElement* editorChildNode = parentElement->createNewChildElement ("EDITOR");
    getEditor()->saveEditorParameters (editorChildNode);
}


void GenericProcessor::saveCustomParametersToXml (XmlElement* parentElement)
{
}

void GenericProcessor::saveChannelParametersToXml (XmlElement* parentElement, int channelNumber, InfoObjectCommon::InfoObjectType type)
{
	XmlElement* channelInfo;
    if ( type == InfoObjectCommon::DATA_CHANNEL)
    {
        channelInfo = parentElement->createNewChildElement ("CHANNEL");
        channelInfo->setAttribute ("name", String (channelNumber));
        channelInfo->setAttribute ("number", channelNumber);

        bool p, r, a;

        getEditor()->getChannelSelectionState (channelNumber, &p, &r, &a);

        XmlElement* selectionState = channelInfo->createNewChildElement ("SELECTIONSTATE");
        selectionState->setAttribute ("param", p);
        selectionState->setAttribute ("record", r);
        selectionState->setAttribute ("audio", a);
    }
	else if (type == InfoObjectCommon::EVENT_CHANNEL)
    {
        channelInfo = parentElement->createNewChildElement ("EVENTCHANNEL");
        channelInfo->setAttribute ("name", String (channelNumber));
        channelInfo->setAttribute ("number", channelNumber);

    }
	else if (type == InfoObjectCommon::SPIKE_CHANNEL)
	{
		channelInfo = parentElement->createNewChildElement("SPIKECHANNEL");
		channelInfo->setAttribute("name", String(channelNumber));
		channelInfo->setAttribute("number", channelNumber);
	}
	saveCustomChannelParametersToXml(channelInfo, channelNumber, type);

    // deprecated parameter configuration:
    //std::cout <<"Creating Parameters" << std::endl;
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

void GenericProcessor::saveCustomChannelParametersToXml (XmlElement* channelInfo, int channelNum, InfoObjectCommon::InfoObjectType type)
{
}


void GenericProcessor::loadFromXml()
{
    update(); // make sure settings are updated

    if (! m_isParamsWereLoaded)
    {
        std::cout << "Loading parameters for " << m_name << std::endl;

        if (parametersAsXml != nullptr)
        {
            // use parametersAsXml to restore state
            loadCustomParametersFromXml();

            // load editor parameters
            forEachXmlChildElement (*parametersAsXml, xmlNode)
            {
                if (xmlNode->hasTagName ("EDITOR"))
                {
                    getEditor()->loadEditorParameters (xmlNode);
                }
            }

            forEachXmlChildElement (*parametersAsXml, xmlNode)
            {
                if (xmlNode->hasTagName ("CHANNEL"))
                {
                    loadChannelParametersFromXml (xmlNode, InfoObjectCommon::DATA_CHANNEL);
                }
                else if (xmlNode->hasTagName ("EVENTCHANNEL"))
                {
                    loadChannelParametersFromXml (xmlNode, InfoObjectCommon::EVENT_CHANNEL);
                }
				else if (xmlNode->hasTagName("SPIKECHANNEL"))
				{
					loadChannelParametersFromXml(xmlNode, InfoObjectCommon::SPIKE_CHANNEL);
				}
            }
        }
    }

    m_isParamsWereLoaded = true;
}


void GenericProcessor::loadChannelParametersFromXml (XmlElement* channelInfo, InfoObjectCommon::InfoObjectType type)
{
    int channelNum = channelInfo->getIntAttribute ("number");

    if (type == InfoObjectCommon::DATA_CHANNEL)
    {
        forEachXmlChildElement (*channelInfo, subNode)
        {
            if (subNode->hasTagName ("SELECTIONSTATE"))
            {
                getEditor()->setChannelSelectionState (channelNum - 1,
                                                       subNode->getBoolAttribute ("param"),
                                                       subNode->getBoolAttribute ("record"),
                                                       subNode->getBoolAttribute ("audio"));
            }
        }
    }

    loadCustomChannelParametersFromXml (channelInfo, type);
}


void GenericProcessor::loadCustomParametersFromXml() { }
void GenericProcessor::loadCustomChannelParametersFromXml (XmlElement* channelInfo, InfoObjectCommon::InfoObjectType type) { }

void GenericProcessor::reset() {}

void GenericProcessor::setCurrentProgramStateInformation (const void* data, int sizeInBytes) {}
void GenericProcessor::setStateInformation               (const void* data, int sizeInBytes) {}

void GenericProcessor::getCurrentProgramStateInformation (MemoryBlock& destData) {}
void GenericProcessor::getStateInformation               (MemoryBlock& destData) {}

void GenericProcessor::changeProgramName (int index, const String& newName) {}
void GenericProcessor::setCurrentProgram (int index) {}

void GenericProcessor::setCurrentChannel (int chan)
{
    currentChannel = chan;
}


void GenericProcessor::setProcessorType (PluginProcessorType processorType)
{
    m_processorType = processorType;
}


//<DEPRECATED>
// ==================================================================
const String GenericProcessor::getInputChannelName  (int channelIndex) const { return GenericProcessor::m_unusedNameString; }
const String GenericProcessor::getOutputChannelName (int channelIndex) const { return GenericProcessor::m_unusedNameString; }
// ==================================================================

void GenericProcessor::getEventChannelNames (StringArray& Names) { }

const String GenericProcessor::getProgramName (int index)   { return ""; }
const String GenericProcessor::getName() const              { return m_name; }

int GenericProcessor::getCurrentChannel() const { return currentChannel; }

PluginProcessorType GenericProcessor::getProcessorType() const { return m_processorType; }

bool GenericProcessor::hasEditor() const { return false; }

bool GenericProcessor::isInputChannelStereoPair  (int index) const { return true; }
bool GenericProcessor::isOutputChannelStereoPair (int index) const { return true; }

bool GenericProcessor::acceptsMidi() const  { return true; }
bool GenericProcessor::producesMidi() const { return true; }

bool GenericProcessor::silenceInProducesSilenceOut() const  { return false; }

bool GenericProcessor::stillHasSource() const { return true; }

bool GenericProcessor::isParameterAutomatable   (int parameterIndex) const { return false; }
bool GenericProcessor::isMetaParameter          (int parameterIndex) const { return false; }

bool GenericProcessor::canSendSignalTo (GenericProcessor*) const { return true; }

bool GenericProcessor::isReady()                { return isEnabled; }
bool GenericProcessor::isEnabledState() const   { return isEnabled; }

bool GenericProcessor::isGeneratesTimestamps() const { return false; }

bool GenericProcessor::isFilter()        const  { return getProcessorType() == PROCESSOR_TYPE_FILTER;        }
bool GenericProcessor::isSource()        const  { return getProcessorType() == PROCESSOR_TYPE_SOURCE;        }
bool GenericProcessor::isSink()          const  { return getProcessorType() == PROCESSOR_TYPE_SINK;          }
bool GenericProcessor::isSplitter()      const  { return getProcessorType() == PROCESSOR_TYPE_SPLITTER;      }
bool GenericProcessor::isMerger()        const  { return getProcessorType() == PROCESSOR_TYPE_MERGER;        }
bool GenericProcessor::isUtility()       const  { return getProcessorType() == PROCESSOR_TYPE_UTILITY;       }

int GenericProcessor::getNumParameters()    { return parameters.size(); }
int GenericProcessor::getNumPrograms()      { return 0; }
int GenericProcessor::getCurrentProgram()   { return 0; }

int GenericProcessor::getNumInputs() const                  { return settings.numInputs; }
int GenericProcessor::getNumOutputs() const                 { return settings.numOutputs; }
int GenericProcessor::getNumOutputs(int subProcessorIdx) const //Defaults to only one subprocessor
{	
	if (subProcessorIdx == 0)
		return getNumOutputs();
	return 0;
}
int GenericProcessor::getDefaultNumDataOutputs(DataChannel::DataChannelTypes, int) const        { return 0; }

int GenericProcessor::getNodeId() const                     { return nodeId; }
int GenericProcessor::getTotalNumberOfChannels() const      { return dataChannelArray.size() + eventChannelArray.size() + spikeChannelArray.size(); }

double GenericProcessor::getTailLengthSeconds() const       { return 1.0f; }

float GenericProcessor::getParameter (int parameterIndex)   { return 1.0; }
float GenericProcessor::getDefaultSampleRate() const        { return 44100.0; }
float GenericProcessor::getSampleRate(int) const               { return getDefaultSampleRate(); }
float GenericProcessor::getDefaultBitVolts() const          { return 1.0; }
float GenericProcessor::getBitVolts(int) const				{ return getDefaultBitVolts(); }
float GenericProcessor::getBitVolts (const DataChannel* chan) const   { return 1.0; }

GenericProcessor* GenericProcessor::getSourceNode() const { return sourceNode; }
GenericProcessor* GenericProcessor::getDestNode()   const { return destNode; }

int GenericProcessor::getNumSubProcessors() const { return 1; }

GenericEditor* GenericProcessor::getEditor() const { return editor; }

AudioSampleBuffer* GenericProcessor::getContinuousBuffer() const { return 0; }
MidiBuffer* GenericProcessor::getEventBuffer() const             { return 0; }

void GenericProcessor::switchIO (int)   { }
void GenericProcessor::switchIO()       { }

void GenericProcessor::setPathToProcessor   (GenericProcessor* p)   { }
void GenericProcessor::setMergerSourceNode  (GenericProcessor* sn)  { }
void GenericProcessor::setSplitterDestNode  (GenericProcessor* dn)  { }

void GenericProcessor::startRecording() { }
void GenericProcessor::stopRecording()  { }

void GenericProcessor::updateSettings() { }

void GenericProcessor::enableCurrentChannel (bool) {}

void GenericProcessor::handleEvent(EventChannel* eventInfo, MidiMessage& event, int samplePosition) {}

void GenericProcessor::handleSpike(SpikeChannel* spikeInfo, MidiMessage& event, int samplePosition) {}

void GenericProcessor::setEnabledState (bool t)
{
    isEnabled = t;
}

bool GenericProcessor::enable()
{
    return isEnabled;
}

bool GenericProcessor::disable()
{
    return true;
}

GenericProcessor::DefaultEventInfo::DefaultEventInfo(EventChannel::EventChannelTypes t, unsigned int c, unsigned int l)
	:type(t),
	nChannels(c),
	length(l)
{
}

GenericProcessor::DefaultEventInfo::DefaultEventInfo()
	:type(EventChannel::INVALID),
	nChannels(0),
	length(0)
{
}