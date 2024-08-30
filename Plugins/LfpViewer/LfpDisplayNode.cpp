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

#include "LfpDisplayNode.h"

#include <stdio.h>

#define MS_FROM_START Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - start) * 1000

using namespace LfpViewer;

LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor ("LFP Viewer")
{
    for (int displayIndex = 0; displayIndex <= 3; displayIndex++)
    {
        triggerChannels.add (-1);
        latestTrigger.add (-1);
        latestCurrentTrigger.add (-1);
    }
}

AudioProcessorEditor* LfpDisplayNode::createEditor()
{
    editor = std::make_unique<LfpDisplayEditor> (this);
    return editor.get();
}

void LfpDisplayNode::initialize (bool signalChainIsLoading)
{
    if (! signalChainIsLoading && ! headlessMode)
    {
        LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
        editor->initialize (signalChainIsLoading);
    }
}

void LfpDisplayNode::updateSettings()
{
    LOGD ("Setting num inputs on LfpDisplayNode to ", getNumInputs());

    int64 start = Time::getHighResolutionTicks();

    for (auto displayBuffer : displayBuffers)
    {
        displayBuffer->prepareToUpdate();
    }

    for (int ch = 0; ch < getNumInputs(); ch++)
    {
        const ContinuousChannel* channel = continuousChannels[ch];

        uint16 streamId = channel->getStreamId();
        String name = channel->getStreamName();

        String stream_key = getDataStream (streamId)->getKey();

        if (displayBufferMap.count (streamId) == 0)
        {
            displayBuffers.add (new DisplayBuffer (streamId, name, channel->getSampleRate()));
            displayBufferMap[streamId] = displayBuffers.getLast();
        }
        else
        {
            displayBufferMap[streamId]->sampleRate = channel->getSampleRate();
            displayBufferMap[streamId]->name = name;
        }
        //
        displayBufferMap[streamId]->addChannel (channel->getName(), // name
                                                ch, // index
                                                channel->getChannelType(), // type
                                                channel->isRecorded,
                                                0, // group
                                                channel->position.y, // ypos
                                                channel->getDescription());
    }

    Array<DisplayBuffer*> toDelete;

    for (auto displayBuffer : displayBuffers)
    {
        if (displayBuffer->isNeeded)
        {
            displayBuffer->update();
        }
        else
        {
            displayBufferMap.erase (displayBuffer->id);
            toDelete.add (displayBuffer);

            for (auto splitID : displayBuffer->displays)
            {
                LfpDisplayEditor* ed = (LfpDisplayEditor*) getEditor();
                ed->removeBufferForDisplay (splitID);
            }
        }
    }

    for (auto displayBuffer : toDelete)
    {
        displayBuffers.removeObject (displayBuffer, true);
    }

    LOGDD ("    Finished creating buffers in ", MS_FROM_START, " milliseconds");
}

void LfpDisplayNode::setSplitDisplays (Array<LfpDisplaySplitter*> splits)
{
    splitDisplays = splits;
}

uint16 LfpDisplayNode::getEventSourceId (const EventChannel* event)
{
    return event->getStreamId();
}

uint16 LfpDisplayNode::getChannelSourceId (const ChannelInfoObject* chan)
{
    return chan->getStreamId();
}

Array<DisplayBuffer*> LfpDisplayNode::getDisplayBuffers()
{
    Array<DisplayBuffer*> buffers;

    for (auto displayBuffer : displayBuffers)
    {
        if (displayBuffer->numChannels > 0)
            buffers.add (displayBuffer);
    }

    return buffers;
}

bool LfpDisplayNode::startAcquisition()
{
    if (! headlessMode)
    {
        LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
        editor->enable();
    }

    return true;
}

bool LfpDisplayNode::stopAcquisition()
{
    if (! headlessMode)
    {
        LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
        editor->disable();
    }

    for (auto split : splitDisplays)
    {
        Array<int> emptyArray = Array<int>();
        split->setFilteredChannels (emptyArray);
    }

    for (auto buffer : displayBuffers)
        buffer->ttlState = 0;

    return true;
}

void LfpDisplayNode::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex < 99)
    {
        triggerChannels.set (int (newValue), parameterIndex);
    }
    else
    {
        ContinuousChannel* chan = continuousChannels[int (newValue)];

        String msg = "AUDIO SELECT ";
        msg += String (chan->getStreamId()) + " ";
        msg += String (chan->getLocalIndex() + 1) + " ";

        broadcastMessage (msg);
    }
}

void LfpDisplayNode::startRecording()
{
    for (auto display : splitDisplays)
    {
        display->recordingStarted();
    }
}

void LfpDisplayNode::stopRecording()
{
    for (auto display : splitDisplays)
    {
        display->recordingStopped();
    }
}

void LfpDisplayNode::handleTTLEvent (TTLEventPtr event)
{
    const int eventId = event->getState() ? 1 : 0;
    const int eventChannel = event->getLine();
    const uint16 eventStreamId = event->getChannelInfo()->getStreamId();
    const int eventSourceNodeId = event->getChannelInfo()->getSourceNodeId();
    const int eventTime = int (event->getSampleNumber() - getFirstSampleNumberForBlock (eventStreamId));

    //LOGD("LFP Viewer received: ", eventSourceNodeId, " ", eventId, " ", event->getSampleNumber(), " ", getFirstSampleNumberForBlock(eventStreamId));

    if (eventId == 1)
    {
        for (int i = 0; i < 3; i++)

        {
            if (triggerChannels[i] == eventChannel)
            {
                if (splitDisplays[i]->selectedStreamId == eventStreamId)
                {
                    // if an event came in on the trigger channel
                    //std::cout << "Setting latest current trigger to " << eventTime << std::endl;
                    latestCurrentTrigger.set (i, eventTime);
                }
            }
        }
    }

    if (displayBufferMap.count (eventStreamId))
    {
        displayBufferMap[eventStreamId]->addEvent (eventTime, eventChannel, eventId, getNumSamplesInBlock (eventStreamId));
    }

    for (auto display : splitDisplays)
    {
        if (display->selectedStreamId == eventStreamId)
        {
            if (event->getWord() != 0)
                display->options->setTTLWord (String (event->getWord()));
        }
    }
}

void LfpDisplayNode::initializeEventChannels()
{
    for (int i = 0; i < 3; i++)
        latestCurrentTrigger.set (i, -1); // reset to -1

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSamplesInBlock (displayBuffer->id);
        displayBuffer->initializeEventChannel (numSamples);
    }
}

void LfpDisplayNode::finalizeEventChannels()
{
    for (int i = 0; i < 3; i++)
    {
        if (latestTrigger[i] == -1 && latestCurrentTrigger[i] > -1) // received a trigger, but not yet acknowledged
        {
            int triggerSample = latestCurrentTrigger[i] + splitDisplays[i]->displayBuffer->displayBufferIndices.getLast();
            //std::cout << "Setting latest trigger to " << triggerSample << std::endl;
            latestTrigger.set (i, triggerSample);
        }
    }

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSamplesInBlock (displayBuffer->id);
        displayBuffer->finalizeEventChannel (numSamples);
    }
}

void LfpDisplayNode::process (AudioBuffer<float>& buffer)
{
    initializeEventChannels();
    checkForEvents();
    finalizeEventChannels();

    for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
    {
        const uint16 streamId = continuousChannels[chan]->getStreamId();

        const uint32 nSamples = getNumSamplesInBlock (streamId);

        String streamKey = getDataStream (streamId)->getKey();

        displayBufferMap[streamId]->addData (buffer, chan, nSamples);
    }
}

int64 LfpDisplayNode::getLatestTriggerTime (int id) const
{
    return latestTrigger[id];
}

void LfpDisplayNode::acknowledgeTrigger (int id)
{
    latestTrigger.set (id, -1);
}

void LfpDisplayNode::handleBroadcastMessage (const String& msg, const int64 messageTimeMillis)
{
    var parsedMessage = JSON::parse (msg);

    if (! parsedMessage.isObject())
    {
        LOGC ("Failed to parse JSON message");

        return;
    }

    DynamicObject::Ptr jsonMessage = parsedMessage.getDynamicObject();

    if (jsonMessage == nullptr)
    {
        LOGC ("Json message did not map to dynamic object");

        return;
    }

    String pluginName = jsonMessage->getProperty ("plugin");

    if (pluginName != "LFPViewer")
        return;

    String command = jsonMessage->getProperty ("command");
    DynamicObject::Ptr payload = jsonMessage->getProperty ("payload").getDynamicObject();

    if (command == "filter")
    {
        if (payload.get() == nullptr)
        {
            LOGD ("Tried to filter in LFPViewer, but could not find a payload");

            return;
        }

        if (! payload->hasProperty ("stream_id"))
            return;

        auto& p = payload->getProperty ("stream_id");

        if (! p.isInt())
            return;

        auto filteredStreamId = p.operator int();

        if (! payload->hasProperty ("local_channel_indices"))
            return;

        auto& filteredLocalChidxsVar = payload->getProperty ("local_channel_indices");

        if (! filteredLocalChidxsVar.isArray())
            return;

        auto& filteredLocalChidxsV = *filteredLocalChidxsVar.getArray();
        Array<int> filteredLocalChidxs;

        for (const auto& chidx : filteredLocalChidxsV)
            filteredLocalChidxs.add ((int) chidx);

        for (auto& display : splitDisplays)
        {
            if (display->selectedStreamId == filteredStreamId)
            {
                display->setFilteredChannels (filteredLocalChidxs);
                display->shouldRebuildChannelList = true;
            }
        }
    }
}
