/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2015 Open Ephys

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

GenericProcessor::GenericProcessor(const String& name_) :
    sourceNode(0), destNode(0), isEnabled(true), wasConnected(false),
    nextAvailableChannel(0), saveOrder(-1), loadOrder(-1), currentChannel(-1),
    editor(0), parametersAsXml(nullptr), sendSampleCount(true), name(name_),
    paramsWereLoaded(false), needsToSendTimestampMessage(false), timestampSet(false)
{
    settings.numInputs = settings.numOutputs = settings.sampleRate = 0;

}

GenericProcessor::~GenericProcessor()
{
}

AudioProcessorEditor* GenericProcessor::createEditor()
{
    editor = new GenericEditor(this, true);
    return editor;
}

void GenericProcessor::setNodeId(int id)
{
    nodeId = id;

    if (editor != 0)
    {
        editor->updateName();
    }

}

Parameter& GenericProcessor::getParameterByName(String name_)
{
    // doesn't work
    for (int i = 0; i < getNumParameters(); i++)
    {

        Parameter& p =  parameters.getReference(i);
        const String parameterName = p.getName();

        if (parameterName.compare(name_) == 0) // fails at this point
            return p;//parameters.getReference(i);
    }

    static Parameter nullParam = Parameter("VOID", false, -1);

    return nullParam;

}

Parameter& GenericProcessor::getParameterReference(int parameterIndex)
{

    return parameters.getReference(parameterIndex);

}

void GenericProcessor::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    std::cout << "Setting parameter" << std::endl;

    if (currentChannel >= 0)
    {
        Parameter& p =  parameters.getReference(parameterIndex);
        p.setValue(newValue, currentChannel);
    }

}

const String GenericProcessor::getParameterName(int parameterIndex)
{
    Parameter& p=parameters.getReference(parameterIndex);
    return p.getName();
}

const String GenericProcessor::getParameterText(int parameterIndex)
{
    Parameter& p = parameters.getReference(parameterIndex);
    return p.getDescription();
}

var GenericProcessor::getParameterVar(int parameterIndex, int parameterChannel)
{
    Parameter& p=parameters.getReference(parameterIndex);
    return p.operator[](parameterChannel);
}

void GenericProcessor::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{

}

void GenericProcessor::releaseResources()
{
    // use the disable() function instead
    // releaseResources() is called by Juce at unpredictable times
    // disable() is only called by the ProcessorGraph at the end of acquisition
}

int GenericProcessor::getNextChannel(bool increment)
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



void GenericProcessor::setSourceNode(GenericProcessor* sn)
{
    //std::cout << "My name is " << getName() << ". Setting source node." << std::endl;

    if (!isSource())
    {
        //	std::cout << " I am not a source." << std::endl;

        if (sn != 0)
        {

            //	std::cout << " The source is not blank." << std::endl;

            if (!sn->isSink())
            {
                //		std::cout << " The source is not a sink." << std::endl;

                if (sourceNode != sn)
                {

                    //			std::cout << " The source is new and named " << sn->getName() << std::endl;

                    if (this->isMerger())
                        setMergerSourceNode(sn);
                    else
                        sourceNode = sn;

                    sn->setDestNode(this);

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
            sn->setDestNode(this);
    }
}


void GenericProcessor::setDestNode(GenericProcessor* dn)
{


    //	std::cout << "My name is " << getName() << ". Setting dest node." << std::endl;

    if (!isSink())
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
                        setSplitterDestNode(dn);
                    else
                        destNode = dn;

                    dn->setSourceNode(this);

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
    settings.sampleRate = getDefaultSampleRate();

    // std::cout << "Record status size = " << recordStatus.size() << std::endl;

    if (recordStatus.size() < channels.size())
        recordStatus.resize(channels.size());

    if (monitorStatus.size() < channels.size())
        monitorStatus.resize(channels.size());

    for (int i = 0; i < channels.size(); i++)
    {
        // std::cout << channels[i]->getRecordState() << std::endl;
        recordStatus.set(i,channels[i]->getRecordState());
        monitorStatus.set(i,channels[i]->isMonitored);
    }

    channels.clear();
    eventChannels.clear();

}

void GenericProcessor::update()
{

    std::cout << getName() << " updating settings." << std::endl;
    // SO patched here to keep original channel names

    // save original channel names
    int oldNumChannels = channels.size();

    StringArray oldNames;

    for (int k = 0; k < oldNumChannels; k++)
    {
        oldNames.add(channels[k]->getName());
    }

    // ---- RESET EVERYTHING ---- ///
    clearSettings();


    if (sourceNode != 0) // copy settings from source node
    {
        // everything is inherited except numOutputs
        settings = sourceNode->settings;
        settings.numInputs = settings.numOutputs;
        settings.numOutputs = settings.numInputs;

        for (int i = 0; i < sourceNode->channels.size(); i++)
        {
            Channel* sourceChan = sourceNode->channels[i];
            Channel* ch = new Channel(*sourceChan);
            ch->setProcessor(this);
            ch->nodeIndex = i;
            ch->mappedIndex = i;

            if (i < recordStatus.size())
            {
                ch->setRecordState(recordStatus[i]);
                ch->isMonitored = monitorStatus[i];
            }

            channels.add(ch);
        }

        for (int i = 0; i < sourceNode->eventChannels.size(); i++)
        {
            Channel* sourceChan = sourceNode->eventChannels[i];
            Channel* ch = new Channel(*sourceChan);
            ch->setProcessor(this);
            eventChannels.add(ch);
        }

    }
    else // generate new settings
    {
        settings.sampleRate = getDefaultSampleRate();
        settings.numOutputs = getNumHeadstageOutputs() + getNumAdcOutputs() + getNumAuxOutputs();

        std::cout << getName() << " setting num outputs to " << settings.numOutputs << std::endl;
        int nidx = 0;

        for (int i = 0; i < getNumHeadstageOutputs(); i++)
        {
            Channel* ch = new Channel(this, i+1, HEADSTAGE_CHANNEL);
            ch->setProcessor(this);
            ch->sampleRate = getDefaultSampleRate();
            ch->bitVolts = getBitVolts(ch);
            ch->sourceNodeId = nodeId;
            ch->nodeIndex = nidx;
            ch->mappedIndex = nidx;

            if (i < recordStatus.size())
            {
                ch->setRecordState(recordStatus[i]);
            }
            else
            {
                if (isSource())
                    ch->setRecordState(true);
            }

            channels.add(ch);
            nidx++;
        }

        for (int j = 0; j < getNumAuxOutputs(); j++)
        {
            Channel* ch = new Channel(this, j+1, AUX_CHANNEL);
            ch->setProcessor(this);
            ch->sampleRate = getDefaultSampleRate();
            ch->bitVolts = getBitVolts(ch);
            ch->sourceNodeId = nodeId;
            ch->nodeIndex = nidx;
            ch->mappedIndex = nidx;

            if (j < recordStatus.size())
            {
                ch->setRecordState(recordStatus[j]);
            }
            else
            {
                if (isSource())
                    ch->setRecordState(true);
            }

            channels.add(ch);
            nidx++;
        }

        for (int k = 0; k < getNumAdcOutputs(); k++)
        {
            Channel* ch = new Channel(this, k+1, ADC_CHANNEL);
            ch->setProcessor(this);
            ch->sampleRate = getDefaultSampleRate();
            ch->bitVolts = getBitVolts(ch);
            ch->sourceNodeId = nodeId;
            ch->nodeIndex = nidx;
            ch->mappedIndex = nidx;

            if (k < recordStatus.size())
            {

                ch->setRecordState(recordStatus[k]);
            }
            else
            {
                if (isSource())
                    ch->setRecordState(true);
            }

            channels.add(ch);
            nidx++;
        }

        for (int m = 0; m < getNumEventChannels(); m++)
        {
            Channel* ch = new Channel(this, m+1, EVENT_CHANNEL);
            ch->sourceNodeId = nodeId;
            ch->nodeIndex = nidx;
            eventChannels.add(ch);
            ch->sampleRate = getDefaultSampleRate();
        }

    }

    if (this->isSink())
    {
        settings.numOutputs = 0;
    }

    updateSettings(); // allow processors to change custom settings

    // required for the ProcessorGraph to know the
    // details of this processor:
    setPlayConfigDetails(getNumInputs(),  // numIns
                         getNumOutputs(), // numOuts
                         44100.0,         // sampleRate
                         128);            // blockSize

    editor->update(); // allow the editor to update its settings

}

void GenericProcessor::setAllChannelsToRecord()
{

    recordStatus.resize(channels.size());

    for (int i = 0; i < channels.size(); i++)
    {
        recordStatus.set(i,true);
    }

    // std::cout << "Setting all channels to record for source." << std::endl;

}

void GenericProcessor::setRecording(bool state)
{
    GenericEditor* ed = getEditor();
    if (state)
    {
        if (ed != 0)
            ed->startRecording();
        startRecording();
        if (generatesTimestamps())
        {
            needsToSendTimestampMessage = true;
        }
    }
    else
    {
        if (ed != 0)
            ed->stopRecording();
        stopRecording();
        needsToSendTimestampMessage = false;
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
int GenericProcessor::getNumSamples(int channelNum)
{
    int sourceNodeId, nSamples;

    if (channelNum >= 0 && channelNum < channels.size())
        sourceNodeId = channels[channelNum]->sourceNodeId;
    else
        return 0;

    // std::cout << "Requesting samples for channel " << channelNum << " with source node " << sourceNodeId << std::endl;

    try
    {
        nSamples = numSamples.at(sourceNodeId);
    }
    catch (std::exception& e)
    {
        return 0;
    }

    //std::cout << nSamples << " were found." << std::endl;

    return nSamples;
}


/** Used to get the number of samples in a given buffer, for a given source node. */
void GenericProcessor::setNumSamples(MidiBuffer& events, int sampleIndex)
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
    memcpy(data+2, &si, 2);

    events.addEvent(data,       // spike data
                    4,          // total bytes
                    0); // sample index
}

/** Used to get the timestamp for a given buffer, for a given source node. */
int64 GenericProcessor::getTimestamp(int channelNum)
{
    int sourceNodeId;
    int64 ts;

    if (channelNum >= 0 && channelNum < channels.size())
        sourceNodeId = channels[channelNum]->sourceNodeId;
    else
        return 0;

    try
    {
        ts = timestamps.at(sourceNodeId);
    }
    catch (std::exception& e)
    {
        return 0;
    }

    return ts;
}

/** Used to set the timestamp for a given buffer, for a given channel. */
void GenericProcessor::setTimestamp(MidiBuffer& events, int64 timestamp)
{

    //std::cout << "Setting timestamp to " << timestamp << std:;endl;
    timestampSet = true;

    uint8 data[8];
    memcpy(data, &timestamp, 8);

    // generate timestamp
    addEvent(events,    // MidiBuffer
             TIMESTAMP, // eventType
             0,         // sampleNum
             nodeId,    // eventID
             0,      // eventChannel
             8,         // numBytes
             data,   // data
             true    // isTimestampEvent
            );

    //since the processor generating the timestamp won't get the event, add it to the map
    timestamps[nodeId] = timestamp;

    if (needsToSendTimestampMessage)
    {
        String eventString = "Processor: " + String(getNodeId()) + " start time: " + String(timestamp) + "@" + String(getSampleRate()) + "Hz";

        CharPointer_UTF8 data = eventString.toUTF8();

        addEvent(events,
                 MESSAGE,
                 0,
                 0,
                 0,
                 data.sizeInBytes(), //It doesn't hurt to send the end-string null and can help avoid issues
                 (uint8*)data.getAddress(),
                 true);

        needsToSendTimestampMessage = false;
    }
}

int GenericProcessor::processEventBuffer(MidiBuffer& events)
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

    //int numRead = 0;

    if (events.getNumEvents() > 0)
    {

        // int m = events.getNumEvents();
        //std::cout << getName() << " received " << m << " events." << std::endl;

        MidiBuffer::Iterator i(events);

        const uint8* dataptr;
        int dataSize;

        int samplePosition = -1;

        while (i.getNextEvent(dataptr, dataSize, samplePosition))
        {

            if (*dataptr == BUFFER_SIZE)
            {

                int16 nr;
                memcpy(&nr, dataptr+2, 2);

                numRead = nr;

                uint8 sourceNodeId;
                memcpy(&sourceNodeId, dataptr + 1, 1);

                numSamples[sourceNodeId] = numRead;

                //if (nodeId < 900)
                //    std::cout << nodeId << " got " << numRead << " samples for " << (int) sourceNodeId << std::endl;


            }
            else if (*dataptr == TIMESTAMP)
            {
                int64 ts;
                memcpy(&ts, dataptr+6, 8);

                uint8 sourceNodeId;
                memcpy(&sourceNodeId, dataptr + 1, 1);

                timestamps[sourceNodeId] = ts;

                //if (nodeId < 900)
                //    std::cout << nodeId << " got " << ts << " timestamp for " << (int) sourceNodeId << std::endl;

            }
            else
            {

                if (isWritableEvent(*dataptr) &&    // a TTL event
                    getNodeId() < 900 && // not handled by a specialized processor (e.g. AudioNode))
                    *(dataptr+4) > 0)    // that's flagged for saving
                {
                    // changing the const cast is dangerous, but probably necessary:
                    uint8* ptr = const_cast<uint8*>(dataptr);
                    *(ptr + 4) = 0; // set fifth byte of raw data to 0, so the event
                    // won't be saved twice
                }
            }
        }
    }

    return numRead;
}


int GenericProcessor::checkForEvents(MidiBuffer& midiMessages)
{

    if (midiMessages.getNumEvents() > 0)
    {

        // int m = midiMessages.getNumEvents();
        //std::cout << m << " events received by node " << getNodeId() << std::endl;

        MidiBuffer::Iterator i(midiMessages);
        MidiMessage message(0xf4);

        int samplePosition = 0;
        i.setNextSamplePosition(samplePosition);

        while (i.getNextEvent(message, samplePosition))
        {

            const uint8* dataptr = message.getRawData();

            handleEvent(*dataptr, message, samplePosition);

        }

    }

    return -1;

}

void GenericProcessor::addEvent(MidiBuffer& eventBuffer,
                                uint8 type,
                                int sampleNum,
                                uint8 eventId,
                                uint8 eventChannel,
                                uint8 numBytes,
                                uint8* eventData,
                                bool isTimestamp)
{

    /*If the processor doesn't generates timestamps, but needs to add events to the buffer anyway
    add the timestamp of the first input channel so the event is properly timestamped. We avoid this step for
    source modules that must always provide a timestamp, even if they don't generate it*/
    if (!isTimestamp && !timestampSet && !isSource() && !generatesTimestamps())
        setTimestamp(eventBuffer, getTimestamp(0));

	HeapBlock<uint8> data(static_cast<const size_t>(6 + numBytes));
    //uint8* data = new uint8[6+numBytes];

    data[0] = type;    // event type
    data[1] = nodeId;  // processor ID automatically added
    data[2] = eventId; // event ID (1 = on, 0 = off, usually)
    data[3] = eventChannel; // event channel
    data[4] = 1; // saving flag
    if (!isTimestamp)
        data[5] = (uint8)eventChannels[eventChannel]->sourceNodeId;  // source node ID (for nSamples)
    else
        data[5] = nodeId;
    memcpy(data + 6, eventData, numBytes);

    //std::cout << "Node id: " << data[1] << std::endl;

    eventBuffer.addEvent(data, 		// raw data
                         6 + numBytes, // total bytes
                         sampleNum);     // sample index

    //if (type == TTL)
    //	std::cout << "Adding event for channel " << (int) eventChannel << " with ID " << (int) eventId << std::endl;

}

// void GenericProcessor::unpackEvent(int type,
// 								   MidiMessage& event)
// {


// }

void GenericProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& eventBuffer)
{

    processEventBuffer(eventBuffer); // extract buffer sizes and timestamps,
    // set flag on all TTL events to zero

    timestampSet = false;

    process(buffer, eventBuffer);

}


/////// ---- LOADING AND SAVING ---- //////////


void GenericProcessor::saveToXml(XmlElement* parentElement)
{

    parentElement->setAttribute("NodeId", nodeId);

    saveCustomParametersToXml(parentElement);

    // loop through the channels

    for (int i = 0; i < channels.size(); i++)
    {

        if (!isSplitter() && !isMerger())
            saveChannelParametersToXml(parentElement, i);

        // channelName = String(i);
        // channelChildNode = parentElement->createNewChildElement("CHANNEL");
        // channelChildNode->setAttribute("name", channelName);
        // saveParametersToChannelsXml(channelChildNode, i);
    }

    for (int i = 0; i < eventChannels.size(); i++)
    {
        if (!isSplitter() && !isMerger())
            saveChannelParametersToXml(parentElement, i, true);

        // channelName=/**String("EventCh:")+*/String(i);
        // channelChildNode = parentElement->createNewChildElement("EVENTCHANNEL");
        // channelChildNode->setAttribute("name", channelName);
        // saveParametersToChannelsXml(channelChildNode, i);
    }

    // Save editor parameters:
    XmlElement* editorChildNode = parentElement->createNewChildElement("EDITOR");
    getEditor()->saveEditorParameters(editorChildNode);

}

void GenericProcessor::saveCustomParametersToXml(XmlElement* parentElement)
{

}

void GenericProcessor::saveChannelParametersToXml(XmlElement* parentElement, int channelNumber, bool isEventChannel)
{

    if (!isEventChannel)
    {
        XmlElement* channelInfo = parentElement->createNewChildElement("CHANNEL");
        channelInfo->setAttribute("name", String(channelNumber));
        channelInfo->setAttribute("number", channelNumber);

        bool p, r, a;

        getEditor()->getChannelSelectionState(channelNumber, &p, &r, &a);

        XmlElement* selectionState = channelInfo->createNewChildElement("SELECTIONSTATE");
        selectionState->setAttribute("param",p);
        selectionState->setAttribute("record",r);
        selectionState->setAttribute("audio",a);

        saveCustomChannelParametersToXml(channelInfo, channelNumber);

    }
    else
    {

        XmlElement* channelInfo = parentElement->createNewChildElement("EVENTCHANNEL");
        channelInfo->setAttribute("name", String(channelNumber));
        channelInfo->setAttribute("number", channelNumber);

        saveCustomChannelParametersToXml(channelInfo, channelNumber, true);

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

void GenericProcessor::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNum, bool isEventChannel)
{


}


void GenericProcessor::loadFromXml()
{

    update(); // make sure settings are updated


    if (!paramsWereLoaded)
    {

        std::cout << "Loading parameters for " << name << std::endl;

        if (parametersAsXml != nullptr)
        {
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

                    loadChannelParametersFromXml(xmlNode);
                }
                else if (xmlNode->hasTagName("EVENTCHANNEL"))
                {

                    loadChannelParametersFromXml(xmlNode, true);

                }

            }
        }

    }

    paramsWereLoaded = true;

}

void GenericProcessor::loadChannelParametersFromXml(XmlElement* channelInfo, bool isEventChannel)
{

    int channelNum = channelInfo->getIntAttribute("number");

    if (!isEventChannel)
    {

        forEachXmlChildElement(*channelInfo, subNode)
        {
            if (subNode->hasTagName("SELECTIONSTATE"))
            {

                getEditor()->setChannelSelectionState(channelNum - 1,
                                                      subNode->getBoolAttribute("param"),
                                                      subNode->getBoolAttribute("record"),
                                                      subNode->getBoolAttribute("audio"));
            }
        }
    }

    loadCustomChannelParametersFromXml(channelInfo, isEventChannel);

}



void GenericProcessor::loadCustomParametersFromXml()
{

}

void GenericProcessor::loadCustomChannelParametersFromXml(XmlElement* channelInfo, bool isEventChannel)
{

}

const String GenericProcessor::unusedNameString("xxx-UNUSED-OPEN-EPHYS-xxx");

const String GenericProcessor::getName() const
{
    return name;
}

bool GenericProcessor::hasEditor() const
{
    return false;
}

void GenericProcessor::reset() {}

void GenericProcessor::setCurrentProgramStateInformation(const void* data, int sizeInBytes) {}

void GenericProcessor::setStateInformation(const void* data, int sizeInBytes) {}

void GenericProcessor::getCurrentProgramStateInformation(MemoryBlock& destData) {}

void GenericProcessor::getStateInformation(MemoryBlock& destData) {}

void GenericProcessor::changeProgramName(int index, const String& newName) {}

void GenericProcessor::setCurrentProgram(int index) {}

const String GenericProcessor::getInputChannelName(int channelIndex) const
{
    return GenericProcessor::unusedNameString;
}

const String GenericProcessor::getOutputChannelName(int channelIndex) const
{
    return GenericProcessor::unusedNameString;
}

void GenericProcessor::getEventChannelNames(StringArray& Names)
{
}

float GenericProcessor::getParameter(int parameterIndex)
{
    return 1.0;
}

const String GenericProcessor::getProgramName(int index)
{
    return "";
}

bool GenericProcessor::isInputChannelStereoPair(int index) const
{
    return true;
}

bool GenericProcessor::isOutputChannelStereoPair(int index) const
{
    return true;
}

bool GenericProcessor::acceptsMidi() const
{
    return true;
}

bool GenericProcessor::producesMidi() const
{
    return true;
}

bool GenericProcessor::isParameterAutomatable(int parameterIndex)
{
    return false;
}

bool GenericProcessor::isMetaParameter(int parameterIndex)
{
    return false;
}

int GenericProcessor::getNumParameters()
{
    return parameters.size();
}

int GenericProcessor::getNumPrograms()
{
    return 0;
}

int GenericProcessor::getCurrentProgram()
{
    return 0;
}

bool GenericProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double GenericProcessor::getTailLengthSeconds() const
{
    return 1.0f;
}

float GenericProcessor::getSampleRate()
{
    return settings.sampleRate;
}

float GenericProcessor::getDefaultSampleRate()
{
    return 44100.0;
}

int GenericProcessor::getNumInputs()
{
    return settings.numInputs;
}

int GenericProcessor::getNumOutputs()
{
    return settings.numOutputs;
}

int GenericProcessor::getNumHeadstageOutputs()
{
    return 2;
}

int GenericProcessor::getNumAdcOutputs()
{
    return 0;
}

int GenericProcessor::getNumAuxOutputs()
{
    return 0;
}

int GenericProcessor::getNumEventChannels()
{
    return 0;
}

float GenericProcessor::getDefaultBitVolts()
{
    return 1.0;
}

float GenericProcessor::getBitVolts(Channel* chan)
{
    return 1.0;
}

void GenericProcessor::setCurrentChannel(int chan)
{
    currentChannel = chan;
}

int GenericProcessor::getNodeId()
{
    return nodeId;
}

GenericProcessor* GenericProcessor::getSourceNode()
{
    return sourceNode;
}

GenericProcessor* GenericProcessor::getDestNode()
{
    return destNode;
}

void GenericProcessor::switchIO(int) { }

void GenericProcessor::switchIO() { }

void GenericProcessor::setPathToProcessor(GenericProcessor* p) { }

void GenericProcessor::setMergerSourceNode(GenericProcessor* sn) { }

void GenericProcessor::setSplitterDestNode(GenericProcessor* dn) { }

bool GenericProcessor::generatesTimestamps()
{
    return false;
}

bool GenericProcessor::isSource()
{
    return false;
}

bool GenericProcessor::isSink()
{
    return false;
}

bool GenericProcessor::isSplitter()
{
    return false;
}

bool GenericProcessor::isMerger()
{
    return false;
}

bool GenericProcessor::isUtility()
{
    return false;
}

bool GenericProcessor::canSendSignalTo(GenericProcessor*)
{
    return true;
}

bool GenericProcessor::isReady()
{
    return isEnabled;
}

bool GenericProcessor::enable()
{
    return isEnabled;
}

bool GenericProcessor::disable()
{
    return true;
}

void GenericProcessor::startRecording() { }

void GenericProcessor::stopRecording() { }

bool GenericProcessor::enabledState()
{
    return isEnabled;
}

void GenericProcessor::enabledState(bool t)
{
    isEnabled = t;
}

void GenericProcessor::enableCurrentChannel(bool) {}

bool GenericProcessor::stillHasSource()
{
    return true;
}

AudioSampleBuffer* GenericProcessor::getContinuousBuffer()
{
    return 0;
}

MidiBuffer* GenericProcessor::getEventBuffer()
{
    return 0;
}

void GenericProcessor::handleEvent(int eventType, MidiMessage& event, int samplePosition) {}

GenericEditor* GenericProcessor::getEditor()
{
    return editor;
}

int GenericProcessor::totalNumberOfChannels()
{
    return channels.size() + eventChannels.size();
}

void GenericProcessor::updateSettings() {}

String GenericProcessor::interProcessorCommunication(String command)
{
    return String("OK");
};
