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

#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include <stdio.h>

using namespace LfpViewer;


LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor  ("LFP Viewer")
{
    setProcessorType (PROCESSOR_TYPE_SINK);
}


LfpDisplayNode::~LfpDisplayNode()
{
   
}


AudioProcessorEditor* LfpDisplayNode::createEditor()
{
    editor = new LfpDisplayEditor (this, true);
    return editor;
}


void LfpDisplayNode::updateSettings()
{

    std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;

    for (auto displayBuffer : displayBuffers)
    {
        displayBuffer->prepareToUpdate();
    }

    for (int ch = 0; ch < getNumInputs(); ch++)
    {
        uint32 id = getChannelSourceId(getDataChannel(ch));

        if (!displayBufferMap.contains(id))
        {
            String name = getSubprocessorName(ch);
            displayBuffers.add(new DisplayBuffer(id, name, getDataChannel(ch)->getSampleRate()));
            displayBufferMap.set(id, displayBuffers.getLast());

            // also add channel positions
        }

        displayBufferMap[id]->addChannel(getDataChannel(ch)->getName(), ch);
    }
    
    for (auto displayBuffer : displayBuffers)
    {
        displayBuffer->update();
    }
}

uint32 LfpDisplayNode::getEventSourceId(const EventChannel* event)
{
    return getProcessorFullId(event->getTimestampOriginProcessor(), event->getTimestampOriginSubProcessor());
}

uint32 LfpDisplayNode::getChannelSourceId(const InfoObjectCommon* chan)
{
    return getProcessorFullId(chan->getSourceNodeID(), chan->getSubProcessorIdx());
}

String LfpDisplayNode::getSubprocessorName(int channel)
{

	if (getTotalDataChannels() != 0)
	{
        const DataChannel* ch = getDataChannel(channel);
        uint16 sourceNodeId = ch->getSourceNodeID();
		uint16 subProcessorIdx = ch->getSubProcessorIdx();
        uint32 subProcFullId = GenericProcessor::getProcessorFullId(sourceNodeId, subProcessorIdx);

        String name = ch->getSourceName() + " " + String(sourceNodeId) + "/" + String(subProcessorIdx);

        return name;
    }
    else {
        return " ";
    }
}


bool LfpDisplayNode::enable()
{


    LfpDisplayEditor* editor = (LfpDisplayEditor*)getEditor();
    editor->enable();

    return true;

}


bool LfpDisplayNode::disable()
{
    LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
    editor->disable();
    return true;
}


void LfpDisplayNode::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);
    //
    //Sets Parameter in parameters array for processor
    parameters[parameterIndex]->setValue (newValue, currentChannel);

    //std::cout << "Saving Parameter from " << currentChannel << ", channel ";

    LfpDisplayEditor* ed = (LfpDisplayEditor*) getEditor();
    if (ed->canvas != 0)
        ed->canvas->setParameter (parameterIndex, newValue);
}


void LfpDisplayNode::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
{
    if (Event::getEventType(event) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);
        
        //int eventNodeId = *(dataptr+1);
        const int eventId = ttl->getState() ? 1 : 0;
        const int eventChannel = ttl->getChannel();
        const int eventTime = samplePosition;

        // find sample rate of event channel
        uint32 eventSourceNodeId = getEventSourceId(eventInfo);
        float eventSampleRate = displayBuffers[eventSourceNodeId]->sampleRate;

        if (eventSampleRate == 0)
        {
            // shouldn't happen for any real event channel at this point
            return;
        }
        
        //std::cout << "Received event on channel " << eventChannel << std::endl;
        //std::cout << "Copying to channel " << channelForEventSource[eventSourceNodeId] << std::endl;
        
        if (eventId == 1)
        {
            for(int i = 0; i < 3; i++)
            {
                if (eventChannel == triggerSource[i])
                {
                    latestCurrentTrigger.set(i, eventTime);
                }
            }
        }
        //int subProcIndex = inputSubprocessors.indexOf(eventSourceNodeId);

        displayBufferMap[eventSourceNodeId]->addEvent(eventTime, eventChannel, eventId,
                                                      getNumSourceSamples(eventSourceNodeId)
                                                      );

        

        //         std::cout << "Received event from " << eventSourceNodeId
        //                   << " on channel " << eventChannel
        //                   << " with value " << eventId
        //                   << " at timestamp " << event.getTimeStamp() << std::endl;
    }
}


void LfpDisplayNode::initializeEventChannels()
{

    //std::cout << "Initializing events..." << std::endl
    latestCurrentTrigger.insertMultiple(0, -1, 3);

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSourceSamples(displayBuffer->id);
        displayBuffer->initializeEventChannel(numSamples);
    }
}

void LfpDisplayNode::finalizeEventChannels()
{
    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSourceSamples(displayBuffer->id);
        displayBuffer->finalizeEventChannel(numSamples);

    }

    for (int i = 0; i < 3; i++)
    {
        if (latestCurrentTrigger[i] >= 0)
        {
            // int chan = numChannelsInSubprocessor[subprocessorToDraw[i]];
           //  int subProcIndex = inputSubprocessors.indexOf(subprocessorToDraw[i]);
           //  latestTrigger.set(i, latestCurrentTrigger[i] + displayBufferIndices[subProcIndex][chan]);
        }
    }
}

void LfpDisplayNode::process (AudioSampleBuffer& buffer)
{

    initializeEventChannels();
    checkForEvents();
    finalizeEventChannels();

    for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
    {
        uint32 subProcId = getChannelSourceId(getDataChannel(chan));
        //currSubproc = inputSubprocessors.indexOf(subProcId);

        const int nSamples = getNumSamples(chan);

        displayBufferMap[subProcId]->addData(buffer, chan, nSamples);
    }
}

void LfpDisplayNode::setTriggerSource(int ch, int id)
{
  printf("Trigger source: %i\n", ch);
  triggerSource.set(id, ch);
}

int LfpDisplayNode::getTriggerSource(int id) const
{
  return triggerSource[id];
}

int64 LfpDisplayNode::getLatestTriggerTime(int id) const
{
  return latestTrigger[id];
}

void LfpDisplayNode::acknowledgeTrigger(int id)
{
  latestTrigger.set(id, -1);
}
