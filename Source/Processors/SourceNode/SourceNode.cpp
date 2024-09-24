/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "SourceNode.h"
#include "../../AccessClass.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "../SourceNode/SourceNodeEditor.h"
#include <stdio.h>

#include "../../Utils/Utils.h"

#include "../Events/Event.h"
#include "../Settings/DataStream.h"

SourceNode::SourceNode (const String& name_, DataThreadCreator dataThreadCreator)
    : GenericProcessor (name_)
{
    setProcessorType (Plugin::Processor::SOURCE);

    dataThread = dataThreadCreator (this);

    if (dataThread != nullptr)
    {
        if (! dataThread->foundInputSource())
        {
            isEnabled = false;
        }
        resizeBuffers();
    }
    else
    {
        isEnabled = false;
    }

    // check for input source every few seconds
    startTimer (sourceCheckInterval);
}

SourceNode::~SourceNode()
{
    if (dataThread->isThreadRunning())
    {
        LOGD (getName(), "forcing DataThread to stop.");
        dataThread->stopThread (500);
    }
}

bool SourceNode::generatesTimestamps() const
{
    return true;
}

DataThread* SourceNode::getThread() const
{
    return dataThread;
}

//This is going to be quite slow, since is reallocating everything, but it's the
//safest way to handle a possible varying number of subprocessors
void SourceNode::resizeBuffers()
{
    inputBuffers.clear();
    eventCodeBuffers.clear();
    eventStates.clear();

    if (dataThread != nullptr)
    {
        dataThread->resizeBuffers();

        for (int i = 0; i < dataStreams.size(); i++)
        {
            inputBuffers.add (dataThread->getBufferAddress (i));
            eventCodeBuffers.add (new MemoryBlock (10000 * sizeof (uint64)));
            eventStates.add (0);
        }
    }
}

void SourceNode::initialize (bool signalChainIsLoading)
{
    dataThread->initialize (signalChainIsLoading);
}

bool SourceNode::isReady()
{
    if (dataThread != nullptr)
    {
        return dataThread->isReady();
    }
    return false;
}

void SourceNode::registerParameters()
{
    dataThread->registerParameters();
}

void SourceNode::requestSignalChainUpdate()
{
    CoreServices::updateSignalChain (getEditor());
}

void SourceNode::updateSettings()
{
    if (dataThread)
    {
        dataThread->updateSettings (&continuousChannels,
                                    &eventChannels, // must return 1 for every stream
                                    &spikeChannels,
                                    &dataStreams,
                                    &devices,
                                    &configurationObjects);

        resizeBuffers();

        //std::cout << " Source node num continuous channels: " << continuousChannels.size() << std::endl;

        for (int i = 0; i < continuousChannels.size(); i++)
            continuousChannels[i]->addProcessor (this);

        for (int i = 0; i < eventChannels.size(); i++)
            eventChannels[i]->addProcessor (this);

        for (int i = 0; i < spikeChannels.size(); i++)
            spikeChannels[i]->addProcessor (this);

        for (int i = 0; i < dataStreams.size(); i++)
            dataStreams[i]->addProcessor (this);

        isEnabled = dataThread->foundInputSource();

        LOGD (getName(), " isEnabled = ", isEnabled, " (updateSettings)");
    }
}

void SourceNode::parameterValueChanged (Parameter* parameter)
{
    dataThread->parameterValueChanged (parameter);
}

float SourceNode::getSampleRate (int streamId) const
{
    if (dataThread != nullptr)
    {
        for (auto& stream : dataStreams)
            if (stream->getStreamId() == streamId)
                return stream->getSampleRate();
    }
    return 44100.0;
}

float SourceNode::getDefaultSampleRate() const
{
    if (dataThread != nullptr)
        return dataStreams[0]->getSampleRate();
    else
        return 44100.0;
}

AudioProcessorEditor* SourceNode::createEditor()
{
    if (dataThread != nullptr)
    {
        editor = dataThread->createEditor (this);
    }
    else
    {
        editor = nullptr;
    }

    if (editor == nullptr)
    {
        editor = std::make_unique<SourceNodeEditor> (this);
    }

    return editor.get();
}

bool SourceNode::tryEnablingEditor()
{
    if (! isSourcePresent())
    {
        //LOGD("No input source found.");
        return false;
    }

    //LOGD("isEnabled = ", isEnabled, " (tryEnablingEditor)");

    if (isEnabled)
    {
        // If we're already enabled (e.g. if we're being called again
        // due to timerCallback()), then there's no need to go through
        // the editor again.
        //LOGD("We're already enabled; returning.");
        return true;
    }

    LOGD (getName(), " -- input source found!");

    CoreServices::updateSignalChain (getEditor());

    return true;
}

void SourceNode::timerCallback()
{
    if (! tryEnablingEditor() && isEnabled)
    {
        LOGD ("Input source lost.");
        isEnabled = false;

        CoreServices::updateSignalChain (getEditor());
    }
}

bool SourceNode::isSourcePresent() const
{
    return dataThread && dataThread->foundInputSource();
}

bool SourceNode::startAcquisition()
{
    if (isSourcePresent())
    {
        stopTimer(); // stop checking for source connection

        dataThread->startAcquisition();
        return true;
    }
    else
    {
        return false;
    }
}

bool SourceNode::stopAcquisition()
{
    if (dataThread != nullptr)
        dataThread->stopAcquisition();

    eventStates.clear();

    for (int i = 0; i < dataStreams.size(); i++)
    {
        eventStates.add (0);
    }

    startTimer (sourceCheckInterval); // timer to check for connected source

    return true;
}

void SourceNode::connectionLost()
{
    CoreServices::setAcquisitionStatus (false);

    CoreServices::sendStatusMessage ("Data acquisition stopped by " + getName());

    CoreServices::updateSignalChain (getEditor());

    startTimer (sourceCheckInterval); // timer to check for re-established connection
}

String SourceNode::handleConfigMessage (const String& msg)
{
    return dataThread->handleConfigMessage (msg);
}

void SourceNode::handleBroadcastMessage (const String& msg, const int64 messageTimeMilliseconds)
{
    dataThread->handleBroadcastMessage (msg, messageTimeMilliseconds);
}

void SourceNode::broadcastDataThreadMessage (const String& msg)
{
    broadcastMessage (msg);
}

void SourceNode::sendDataThreadConfigMessage (GenericProcessor* destProcessor, const String& msg)
{
    sendConfigMessage (destProcessor, msg);
}

void SourceNode::process (AudioBuffer<float>& buffer)
{
    int copiedChannels = 0;

    for (int streamIdx = 0; streamIdx < inputBuffers.size(); streamIdx++)
    {
        int channelsToCopy = getNumOutputsForStream (streamIdx);

        int nSamples = inputBuffers[streamIdx]->readAllFromBuffer (buffer,
                                                                   &sampleNumber,
                                                                   &timestamp,
                                                                   static_cast<uint64*> (eventCodeBuffers[streamIdx]->getData()),
                                                                   buffer.getNumSamples(),
                                                                   &timestampSampleIndex,
                                                                   copiedChannels,
                                                                   channelsToCopy);

        //std::cout << getNodeId() << " " << streamIdx << " " << nSamples << std::endl;

        copiedChannels += channelsToCopy;

        //if (getFirstSampleNumberForBlock(dataStreams[streamIdx]->getStreamId()) > sampleNumber)
        //    std::cout << "SET ERROR: " << getNodeId() << " " << dataStreams[streamIdx]->getStreamId() << std::endl;

        setTimestampAndSamples (sampleNumber,
                                timestamp,
                                nSamples,
                                dataStreams[streamIdx]->getStreamId());

        if (eventChannels[streamIdx])
        {
            int maxTTLBits = eventChannels[streamIdx]->getMaxTTLBits();

            uint64 lastCode = eventStates[streamIdx];

            for (int sample = 0; sample < nSamples; ++sample)
            {
                uint64 currentCode = *(static_cast<uint64*> (eventCodeBuffers[streamIdx]->getData()) + sample);

                //If there has been no change to the TTL word, avoid doing anything at all here
                if (lastCode != currentCode)
                {
                    //Create a TTL event for each bit that has changed
                    for (uint8 c = 0; c < maxTTLBits; ++c)
                    {
                        if (((currentCode >> c) & 0x01) != ((lastCode >> c) & 0x01))
                        {
                            TTLEventPtr event = TTLEvent::createTTLEvent (eventChannels[streamIdx],
                                                                          sampleNumber + sample,
                                                                          c,
                                                                          (currentCode >> c) & 0x01,
                                                                          currentCode);

                            addEvent (event, sample);
                        }
                    }

                    lastCode = currentCode;
                }
            }
            eventStates.set (streamIdx, lastCode);
        }
    }
}
