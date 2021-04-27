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

#include <stdio.h>

using namespace LfpViewer;


LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor  ("LFP Viewer")
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    for (int i = 0; i <= 3; i++)
    {
        triggerChannels.add(-1);
        latestTrigger.add(-1);
        latestCurrentTrigger.add(-1);
    }
        

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

    //std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;

    for (auto displayBuffer : displayBuffers)
    {
        displayBuffer->prepareToUpdate();
    }

    for (int ch = 0; ch < getNumInputs(); ch++)
    {
        uint32 id = getChannelSourceId(getDataChannel(ch));

        if (displayBufferMap.count(id) == 0)
        {
            String name = getSubprocessorName(ch);

            
            displayBuffers.add(new DisplayBuffer(id, name, getDataChannel(ch)->getSampleRate()));
            displayBufferMap[id] = displayBuffers.getLast();

        }
        else {
            displayBufferMap[id]->sampleRate = getDataChannel(ch)->getSampleRate();
        }

        int depthId = getDataChannel(ch)->findMetaData(MetaDataDescriptor::FLOAT, 1, "depth-value");
        int groupId = getDataChannel(ch)->findMetaData(MetaDataDescriptor::INT32, 1, "channel-group");

        float channelDepth = 0;
        float channelGroup = 0;
   
        if (depthId > -1)
        {
            const MetaDataValue* val = getDataChannel(ch)->getMetaDataValue(depthId);
            val->getValue(&channelDepth);
        }

        if (groupId > -1)
        {
            const MetaDataValue* val = getDataChannel(ch)->getMetaDataValue(groupId);
            val->getValue(&channelGroup);
        }

        displayBufferMap[id]->addChannel(getDataChannel(ch)->getName(), ch, channelGroup, channelDepth);
    }

    Array<DisplayBuffer*> toDelete;
    
    for (auto displayBuffer : displayBuffers)
    {

        if (displayBuffer->isNeeded)
        {
            displayBuffer->update();
            //std::cout << "Updating displayBuffer with id " << displayBuffer->id << std::endl;
        }
        else {

            //std::cout << "Erasing displayBuffer with id " << displayBuffer->id << std::endl;
            displayBufferMap.erase(displayBuffer->id);
            toDelete.add(displayBuffer);

            for (auto splitID : displayBuffer->displays)
            {
                LfpDisplayEditor* ed = (LfpDisplayEditor*)getEditor();
                ed->removeBufferForDisplay(splitID);
            }
        }
        
    }

    for (auto displayBuffer : toDelete)
    {
        displayBuffers.removeObject(displayBuffer, true);
    }

   
    //std::cout << "Total display buffers: " << displayBuffers.size() << std::endl;

    // TODO: add event channels separately, as they may have a different source
}

void LfpDisplayNode::setSplitDisplays(Array<LfpDisplaySplitter*> splits)
{
    splitDisplays = splits;
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

        int subprocessorNameId = ch->findMetaData(MetaDataDescriptor::CHAR, 64, "subprocessor-name");

        String name;

        if (subprocessorNameId > -1)
        {
           
            const MetaDataValue* val = ch->getMetaDataValue(subprocessorNameId);
            String stringValue;
            val->getValue(stringValue);

            name = ch->getSourceName() + " " + stringValue;

           // std::cout << "Sb name: " << name << std::endl;
        }
        else {

            uint16 sourceNodeId = ch->getSourceNodeID();
            uint16 subProcessorIdx = ch->getSubProcessorIdx();
            uint32 subProcFullId = GenericProcessor::getProcessorFullId(sourceNodeId, subProcessorIdx);

            name = ch->getSourceName() + " " + String(sourceNodeId) + "/" + String(subProcessorIdx);
        }

        return name;
    }
    else {
        return " ";
    }
}

Array<DisplayBuffer*> LfpDisplayNode::getDisplayBuffers()
{
    Array<DisplayBuffer*> buffers;

    for (auto displayBuffer : displayBuffers)
    {
        if (displayBuffer->numChannels > 0)
            buffers.add(displayBuffer);
    }

    return buffers;
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
    //std::cout << "Setting trigger channel for display index " << int(newValue) << " to " << parameterIndex << std::endl;
    triggerChannels.set(int(newValue), parameterIndex);
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
        //float eventSampleRate = displayBuffers[eventSourceNodeId]->sampleRate;

        //if (eventSampleRate == 0)
        //{
            // shouldn't happen for any real event channel at this point
       //     return;
       // }
        
        //std::cout << "Received event on channel " << eventChannel << std::endl;
        //std::cout << "Copying to channel " << channelForEventSource[eventSourceNodeId] << std::endl;
        
        //int subProcIndex = inputSubprocessors.indexOf(eventSourceNodeId);

        if (eventId == 1)
        {
            for (int i = 0; i < 3; i++)

            {
                if (triggerChannels[i] == eventChannel)
                {
                    // if an event came in on the trigger channel
                    //std::cout << "Setting latest current trigger to " << eventTime << std::endl;
                    latestCurrentTrigger.set(i, eventTime);

                }
            }
        }

        if (displayBufferMap.count(eventSourceNodeId))
        {
            displayBufferMap[eventSourceNodeId]->addEvent(eventTime, eventChannel, eventId,
                getNumSourceSamples(eventSourceNodeId)
            );

           
        }

        else {

            for (auto displayBuffer : displayBuffers)
            {
                displayBuffer->addEvent(eventTime, eventChannel, eventId,
                    getNumSourceSamples(displayBuffer->id)
                );
                
            }

        }

       /*          std::cout << "Received event from " << eventSourceNodeId
                           << " on channel " << eventChannel
                           << " with value " << eventId
                           << " at timestamp " << event.getTimeStamp() << std::endl;*/
    }
}


void LfpDisplayNode::initializeEventChannels()
{
    latestCurrentTrigger.insertMultiple(0, -1, 3); // reset to -1

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSourceSamples(displayBuffer->id);
        displayBuffer->initializeEventChannel(numSamples);
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
            latestTrigger.set(i, triggerSample);
        }
    }

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSourceSamples(displayBuffer->id);
        displayBuffer->finalizeEventChannel(numSamples);
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

        const int nSamples = getNumSamples(chan);

        displayBufferMap[subProcId]->addData(buffer, chan, nSamples);
    }
}

//void LfpDisplayNode::setTriggerSource(int ch, int id)
//{
//  printf("Trigger source: %i\n", ch);
//  triggerSource.set(id, ch);
//}

//int LfpDisplayNode::getTriggerSource(int id) const
//{
 // return triggerSource[id];
//}

int64 LfpDisplayNode::getLatestTriggerTime(int id) const
{
  return latestTrigger[id];
}

void LfpDisplayNode::acknowledgeTrigger(int id)
{
  latestTrigger.set(id, -1);
  //std::cout << "Display " << id << " acknowledging trigger." << std::endl;
}
