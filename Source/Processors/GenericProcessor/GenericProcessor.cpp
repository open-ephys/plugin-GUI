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
    , m_isTimestampSet                  (false)
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
        m_monitorStatus.set   (i, dataChannelArray[i]->isMonitored);
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
            ch-> m_nodeID = getNodeId();


            if (i < m_recordStatus.size())
            {
                ch->setRecordState (m_recordStatus[i]);
                ch->isMonitored = m_monitorStatus[i];
            }

			ch->addToHistoricString(getName());
            dataChannelArray.add (ch);
        }

        for (int i = 0; i < sourceNode->eventChannelArray.size(); ++i)
        {
            EventChannel* sourceChan = sourceNode->eventChannelArray[i];
            EventChannel* ch = new EventChannel (*sourceChan);
			ch->m_nodeID = getNodeId();
            eventChannelArray.add (ch);
        }
		for (int i = 0; i < sourceNode->spikeChannelArray.size(); ++i)
		{
			SpikeChannel* sourceChan = sourceNode->spikeChannelArray[i];
			SpikeChannel* ch = new SpikeChannel(*sourceChan);
			ch->m_nodeID = getNodeId();
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
		uint32 sourceID = getProcessorFullId(channel->getSourceNodeID(), channel->getSubProcessorIdx());
		dataChannelMap[sourceID][channel->getSourceIndex()] = i;
	}
	for (int i = 0; i < eventChannelArray.size(); i++)
	{
		EventChannel* channel = eventChannelArray[i];
		uint32 sourceID = getProcessorFullId(channel->getSourceNodeID(), channel->getSubProcessorIdx());
		eventChannelMap[sourceID][channel->getSourceIndex()] = i;
	}
	for (int i = 0; i < spikeChannelArray.size(); i++)
	{
		SpikeChannel* channel = spikeChannelArray[i];
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
int GenericProcessor::getNumSamples (int channelNum) const
{
    int sourceNodeId = 0;
    int nSamples     = 0;

    if (channelNum >= 0
        && channelNum < channels.size())
    {
        sourceNodeId = channels[channelNum]->sourceNodeId;
    }
    else
    {
        return 0;
    }

    // std::cout << "Requesting samples for channel " << channelNum << " with source node " << sourceNodeId << std::endl;

    try
    {
        nSamples = numSamples.at (sourceNodeId);
    }
    catch (std::exception& e)
    {
        return 0;
    }

    //std::cout << nSamples << " were found." << std::endl;

    return nSamples;
}


/** Used to get the number of samples in a given buffer, for a given source node. */
void GenericProcessor::setNumSamples (MidiBuffer& events, int sampleIndex)
{
    // This amounts to adding a "buffer size" flag at a particular sample number,
    // and a new flag is added each time "setNumSamples" is called.
    // Thus, if the number of samples changes somewhere in the processing pipeline,
    // the old sample number will remain. This is a problem if the number of
    // samples gets smaller.
    // If we allow the sample rate to change (e.g., with a resampling node),
    // this code will have to be updated. The easiest approach will be for each
    // processor to ignore any buffer size events that don't come from its
    // immediate source.
    //

    uint8 data[4];

    int16 si = (int16) sampleIndex;

    data[0] = BUFFER_SIZE;  // most-significant byte
    data[1] = nodeId;       // least-significant byte
    memcpy (data + 2, &si, 2);

    events.addEvent (data,       // spike data
                     4,          // total bytes
                     0); // sample index
}


/** Used to get the timestamp for a given buffer, for a given source node. */
int64 GenericProcessor::getTimestamp (int channelNum) const
{
    int sourceNodeId = 0;
    int64 ts         = 0;

    if (channelNum >= 0 
        && channelNum < channels.size())
    {
        sourceNodeId = channels[channelNum]->sourceNodeId;
    }
    else
    {
        return 0;
    }

    try
    {
        ts = timestamps.at (sourceNodeId);
    }
    catch (std::exception& e)
    {
        return 0;
    }

    return ts;
}


/** Used to set the timestamp for a given buffer, for a given channel. */
void GenericProcessor::setTimestamp (MidiBuffer& events, int64 timestamp)
{
    //std::cout << "Setting timestamp to " << timestamp << std:;endl;
    m_isTimestampSet = true;

    uint8 data[8];
    memcpy (data, &timestamp, 8);

    // generate timestamp
    addEvent (events,    // MidiBuffer
              TIMESTAMP, // eventType
              0,         // sampleNum
              nodeId,    // eventID
              0,      // eventChannel
              8,         // numBytes
              data,   // data
              true);  // isTimestampEvent

    //since the processor generating the timestamp won't get the event, add it to the map
    timestamps[nodeId] = timestamp;

    if (m_isNeedsToSendTimestampMessage)
    {
        String eventString = "Processor: "
                                + String (getNodeId()) 
                                + " start time: " 
                                + String (timestamp) 
                                + "@" 
                                + String (getSampleRate()) 
                                + "Hz";

        CharPointer_UTF8 data = eventString.toUTF8();

        addEvent (events,
                  MESSAGE,
                  0,
                  0,
                  0,
                  data.sizeInBytes(), //It doesn't hurt to send the end-string null and can help avoid issues
                  (uint8*)data.getAddress(),
                  true);

        m_isNeedsToSendTimestampMessage = false;
    }
}


int GenericProcessor::processEventBuffer (MidiBuffer& events)
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

    if (events.getNumEvents() > 0)
    {
        MidiBuffer::Iterator i (events);

        const uint8* dataptr;
        int dataSize;

        int samplePosition = -1;

        while (i.getNextEvent (dataptr, dataSize, samplePosition))
        {
            if (*dataptr == BUFFER_SIZE)
            {
                int16 nr;
                memcpy (&nr, dataptr + 2, 2);

                numRead = nr;

                uint8 sourceNodeId;
                memcpy (&sourceNodeId, dataptr + 1, 1);

                numSamples[sourceNodeId] = numRead;
            }
            else if (*dataptr == TIMESTAMP)
            {
                int64 ts;
                memcpy (&ts, dataptr + 6, 8);

                uint8 sourceNodeId;
                memcpy (&sourceNodeId, dataptr + 1, 1);

                timestamps[sourceNodeId] = ts;
            }
            else
            {

                if (isWritableEvent (*dataptr)  // a TTL event
                    && getNodeId() < 900        // not handled by a specialized processor (e.g. AudioNode))
                    && *(dataptr + 4) > 0)        // that's flagged for saving
                {
                    // changing the const cast is dangerous, but probably necessary:
                    uint8* ptr = const_cast<uint8*> (dataptr);
                    *(ptr + 4) = 0; // set fifth byte of raw data to 0, so the event
                    // won't be saved twice
                }
            }
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

void GenericProcessor::addEvent(int channelIndex, const Event& event, int sampleNum)
{
	addEvent(eventChannelArray[channelIndex], event, sampleNum);
}

void GenericProcessor::addEvent(const EventChannel* channel, const Event& event, int sampleNum)
{
	size_t size = channel->getDataSize() + channel->getTotalEventMetaDataSize() + EVENT_BASE_SIZE;
	HeapBlock<char> buffer(size);
	event.serialize(buffer, size);
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

    m_isTimestampSet = false;

    process (buffer, eventBuffer);
}


/////// ---- LOADING AND SAVING ---- //////////


void GenericProcessor::saveToXml (XmlElement* parentElement)
{
    parentElement->setAttribute ("NodeId", nodeId);

    saveCustomParametersToXml (parentElement);

    // loop through the channels

    for (int i = 0; i < channels.size(); ++i)
    {
        if (! isSplitter() && ! isMerger())
            saveChannelParametersToXml (parentElement, i);
    }

    for (int i = 0; i < eventChannels.size(); ++i)
    {
        if (! isSplitter() && ! isMerger())
            saveChannelParametersToXml (parentElement, i, true);
    }

    // Save editor parameters:
    XmlElement* editorChildNode = parentElement->createNewChildElement ("EDITOR");
    getEditor()->saveEditorParameters (editorChildNode);
}


void GenericProcessor::saveCustomParametersToXml (XmlElement* parentElement)
{
}

void GenericProcessor::saveChannelParametersToXml (XmlElement* parentElement, int channelNumber, bool isEventChannel)
{
    if (! isEventChannel)
    {
        XmlElement* channelInfo = parentElement->createNewChildElement ("CHANNEL");
        channelInfo->setAttribute ("name", String (channelNumber));
        channelInfo->setAttribute ("number", channelNumber);

        bool p, r, a;

        getEditor()->getChannelSelectionState (channelNumber, &p, &r, &a);

        XmlElement* selectionState = channelInfo->createNewChildElement ("SELECTIONSTATE");
        selectionState->setAttribute ("param", p);
        selectionState->setAttribute ("record", r);
        selectionState->setAttribute ("audio", a);

        saveCustomChannelParametersToXml (channelInfo, channelNumber);
    }
    else
    {
        XmlElement* channelInfo = parentElement->createNewChildElement ("EVENTCHANNEL");
        channelInfo->setAttribute ("name", String (channelNumber));
        channelInfo->setAttribute ("number", channelNumber);

        saveCustomChannelParametersToXml (channelInfo, channelNumber, true);
    }

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

void GenericProcessor::saveCustomChannelParametersToXml (XmlElement* channelInfo, int channelNum, bool isEventChannel)
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
                    loadChannelParametersFromXml (xmlNode);
                }
                else if (xmlNode->hasTagName ("EVENTCHANNEL"))
                {
                    loadChannelParametersFromXml (xmlNode, true);
                }
            }
        }
    }

    m_isParamsWereLoaded = true;
}


void GenericProcessor::loadChannelParametersFromXml (XmlElement* channelInfo, bool isEventChannel)
{
    int channelNum = channelInfo->getIntAttribute ("number");

    if (! isEventChannel)
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

    loadCustomChannelParametersFromXml (channelInfo, isEventChannel);
}


void GenericProcessor::loadCustomParametersFromXml() { }
void GenericProcessor::loadCustomChannelParametersFromXml (XmlElement* channelInfo, bool isEventChannel) { }

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
int GenericProcessor::getNumDataOutputs(DataChannel::DataChannelTypes) const        { return 0; }
int GenericProcessor::getNumEventOutputs(EventChannel::EventChannelTypes) const           { return 0; }
int GenericProcessor::getNodeId() const                     { return nodeId; }
int GenericProcessor::getTotalNumberOfChannels() const      { return dataChannelArray.size() + eventChannelArray.size() + spikeChannelArray.size(); }

double GenericProcessor::getTailLengthSeconds() const       { return 1.0f; }

float GenericProcessor::getParameter (int parameterIndex)   { return 1.0; }
float GenericProcessor::getDefaultSampleRate() const        { return 44100.0; }
float GenericProcessor::getSampleRate(int) const               { return getDefaultSampleRate(); }
float GenericProcessor::getDefaultBitVolts() const          { return 1.0; }
float GenericProcessor::getBitVolts (DataChannel* chan) const   { return 1.0; }

GenericProcessor* GenericProcessor::getSourceNode() const { return sourceNode; }
GenericProcessor* GenericProcessor::getDestNode()   const { return destNode; }

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

