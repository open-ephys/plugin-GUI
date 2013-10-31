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

#include "GenericProcessor.h"
#include "../UI/UIComponent.h"

GenericProcessor::GenericProcessor(const String& name_) : AccessClass(),
    sourceNode(0), destNode(0), isEnabled(true), wasConnected(false),
    nextAvailableChannel(0), saveOrder(-1), loadOrder(-1), currentChannel(-1),
     parametersAsXml(nullptr),  name(name_), paramsWereLoaded(false)
{
}

GenericProcessor::~GenericProcessor()
{
}

AudioProcessorEditor* GenericProcessor::createEditor()
{
    editor = new GenericEditor(this, true);
    return editor;
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
    settings.originalSource = 0;
    settings.numInputs = 0;
    settings.numOutputs = 0;
    settings.sampleRate = getDefaultSampleRate();

    recordStatus.clear();

    for (int i = 0; i < channels.size(); i++)
    {
        recordStatus.add(channels[i]->getRecordState());
    }

    channels.clear();
    eventChannels.clear();

}

void GenericProcessor::update()
{

    std::cout << getName() << " updating settings." << std::endl;

    clearSettings();

    if (sourceNode != 0)
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
            ch->bitVolts = ch->bitVolts*getDefaultBitVolts();

            if (i < recordStatus.size())
            {
                ch->setRecordState(recordStatus[i]);
            }

            channels.add(ch);
        }

        for (int i = 0; i < sourceNode->eventChannels.size(); i++)
        {
            Channel* sourceChan = sourceNode->eventChannels[i];
            Channel* ch = new Channel(*sourceChan);
            ch->sampleRate = getDefaultSampleRate();
            ch->bitVolts = getDefaultBitVolts();
            eventChannels.add(ch);
        }

    }
    else
    {

        settings.numOutputs = getDefaultNumOutputs();
        settings.sampleRate = getDefaultSampleRate();

        for (int i = 0; i < getNumOutputs(); i++)
        {
            Channel* ch = new Channel(this, i);
            ch->sampleRate = getDefaultSampleRate();
            ch->bitVolts = getDefaultBitVolts();

             if (i < recordStatus.size())
            {
                ch->setRecordState(recordStatus[i]);
            }

            channels.add(ch);
        }

    }

    if (this->isSink())
    {
        settings.numOutputs = 0;
    }

    updateSettings(); // custom settings code

    // required for the ProcessorGraph to know the
    // details of this processor:
    setPlayConfigDetails(getNumInputs(),  // numIns
                         getNumOutputs(), // numOuts
                         44100.0,         // sampleRate
                         128);            // blockSize

    editor->update(); // update editor settings

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

int GenericProcessor::getNumSamples(MidiBuffer& events)
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

        // int m = events.getNumEvents();
        //std::cout << getName() << " received " << m << " events." << std::endl;

        MidiBuffer::Iterator i(events);

        const uint8* dataptr;
        int dataSize;

        int samplePosition = -5;

        while (i.getNextEvent(dataptr, dataSize, samplePosition))
        {

            if (*dataptr == BUFFER_SIZE)
            {
                
                numRead = samplePosition;

            } else if (*dataptr == TTL &&    // a TTL event
                        getNodeId() < 900 && // not handled by a specialized processor (e.g. AudioNode))
                        *(dataptr+1) > 0)    // that's flagged for saving
            {
                // changing the const cast is dangerous, but probably necessary:
                uint8* ptr = const_cast<uint8*>(dataptr);
                *(ptr + 1) = 0; // set second byte of raw data to 0, so the event
                                // won't be saved twice
            }
        }
    }

    return numRead;
}

void GenericProcessor::setNumSamples(MidiBuffer& events, int sampleIndex)
{
    //
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

    uint8 data[2];

    data[0] = BUFFER_SIZE;  // most-significant byte
    data[1] = nodeId;       // least-significant byte

    events.addEvent(data,       // spike data
                    2,          // total bytes
                    sampleIndex); // sample index

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
                                uint8* eventData)
{
    uint8* data = new uint8[4+numBytes];

    data[0] = type;    // event type
    data[1] = nodeId;  // processor ID automatically added
    data[2] = eventId; // event ID
    data[3] = eventChannel; // event channel
    memcpy(data + 4, eventData, numBytes);

    eventBuffer.addEvent(data, 		// raw data
                         4 + numBytes, // total bytes
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

    int nSamples = getNumSamples(eventBuffer); // finds buffer size and sets save
                                               // flag on all TTL events to zero

    process(buffer, eventBuffer, nSamples);

    setNumSamples(eventBuffer, nSamples); // adds it back,
    // even if it's unchanged

}


/////// ---- LOADING AND SAVING ---- //////////


void GenericProcessor::saveToXml(XmlElement* parentElement)
{

    parentElement->setAttribute("NodeId", nodeId);

    saveCustomParametersToXml(parentElement);

    // loop through the channels

    for (int i = 0; i < channels.size(); i++)
    {

        saveChannelParametersToXml(parentElement, i);

        // channelName = String(i);
        // channelChildNode = parentElement->createNewChildElement("CHANNEL");
        // channelChildNode->setAttribute("name", channelName);
        // saveParametersToChannelsXml(channelChildNode, i);
    }

    for (int i = 0; i < eventChannels.size(); i++)
    {
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

    std::cout << "Loading parameters for " << name << std::endl;

    if (!paramsWereLoaded)
    {

        if (parametersAsXml != nullptr)
        {
            // use parametersAsXml to restore state
            loadCustomParametersFromXml();

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
                else if (xmlNode->hasTagName("EDITOR"))
                {
                    getEditor()->loadEditorParameters(xmlNode);
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
