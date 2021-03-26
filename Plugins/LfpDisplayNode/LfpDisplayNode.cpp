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
  //  , displayGain       (1)
  //  , bufferLength      (2.0f)
  //  , abstractFifo      (100)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    

    //numDisplays = 1; // should not be necessary
    //numSubprocessors = 1; // default to 1
    
   // displayBuffers.clear();
   // for (int i = 0; i < numSubprocessors; i++)
    //    displayBuffers.push_back(std::make_shared<AudioSampleBuffer>(8, 100));

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
        }

        displayBufferMap[id]->addChannel(getDataChannel(ch)->getName());
    }
    

    for (auto displayBuffer : displayBuffers)
    {
        displayBuffer->update();
    }


    //for (int i = 0; i < numSubprocessors; i++)
        

    //displayBufferIndices.resize(numSubprocessors);

    //channelIndices.resize(numSubprocessors);

    //setDefaultSubprocessors();

    // int numChans = getNumSubprocessorChannels();
    // int srate = getSubprocessorSampleRate(subprocessorToDraw);

    // std::cout << "Re-setting num inputs on LfpDisplayNode to " << numChans << std::endl;
    // if (numChans > 0)
    // {
    //     std::cout << "Sample rate = " << srate << std::endl;
    // }

    //eventSourceNodes.clear();

    //for (int i = 0; i < eventChannelArray.size(); ++i)
    //{
    //    uint32 sourceId = getEventSourceId(eventChannelArray[i]);
 
    //    if (!eventSourceNodes.contains(sourceId))
    //    {
    //        eventSourceNodes.add(sourceId);
    //    }
    //}

    //for (int i = 0; i < eventSourceNodes.size(); ++i)
    //{
        // std::cout << "Adding channel " << numChans + i << " for event source node " << eventSourceNodes[i] << std::endl;

     //   ttlState[eventSourceNodes[i]] = 0;
    //}
    
    //updateInputSubprocessors();

    //resizeBuffer();
}

uint32 LfpDisplayNode::getEventSourceId(const EventChannel* event)
{
    return getProcessorFullId(event->getTimestampOriginProcessor(), event->getTimestampOriginSubProcessor());
}

uint32 LfpDisplayNode::getChannelSourceId(const InfoObjectCommon* chan)
{
    return getProcessorFullId(chan->getSourceNodeID(), chan->getSubProcessorIdx());
}

/*uint32 LfpDisplayNode::getDataSubprocId(int chan) const
{
    if (chan < 0 || chan >= getTotalDataChannels())
    {
        return 0;
    }

    return getChannelSourceId(getDataChannel(chan));
}*/

/*void LfpDisplayNode::setSubprocessor(uint32 sp, int id)
{
    subprocessorToDraw.set(id, sp);
    std::cout << "LfpDisplayNode setting subprocessor of display #"<< id << " to " << sp << std::endl;    
}

uint32 LfpDisplayNode::getSubprocessor(int id) const
{
    return subprocessorToDraw[id];
}

int LfpDisplayNode::getNumSubprocessorChannels(int id)
{
    if (subprocessorToDraw[id] != 0)
    {
        return numChannelsInSubprocessor[subprocessorToDraw[id]];
    }
    return 0;
}*/

/*void LfpDisplayNode::setDefaultSubprocessors()
{
    for(int i = 0; i < numDisplays; i++)
    {
        if (numChannelsInSubprocessor.find(subprocessorToDraw[i]) == numChannelsInSubprocessor.end())
        {
            // subprocessor to draw does not exist
            if (numSubprocessors == 0)
            {
                return;
            }
            else
            {
                // there are channels, but none on the current subprocessorToDraw
                // default to the first subprocessor
                subprocessorToDraw.set(i, getDataSubprocId(0));
            }
        }
    }
}*/

/*float LfpDisplayNode::getSubprocessorSampleRate(uint32 subprocId)
{
    auto entry = subprocessorSampleRate.find(subprocId);
    if (entry != subprocessorSampleRate.end())
    {
        return entry->second;
    }
    return 0.0f;
}*/

String LfpDisplayNode::getSubprocessorName(int channel)
{
    // clear out the old data
    //inputSubprocessors.clear();
    //subprocessorNames.clear();
    
	if (getTotalDataChannels() != 0)
	{
		//for (int i = 0, len = getTotalDataChannels(); i < len; ++i)
		//{
            const DataChannel* ch = getDataChannel(channel);
            uint16 sourceNodeId = ch->getSourceNodeID();
			uint16 subProcessorIdx = ch->getSubProcessorIdx();
            uint32 subProcFullId = GenericProcessor::getProcessorFullId(sourceNodeId, subProcessorIdx);

            String name = ch->getSourceName() + " " + String(sourceNodeId) + "/" + String(subProcessorIdx);

            return name;

	}
}

/*bool LfpDisplayNode::resizeBuffer()
{   
    int totalResized = 0;

    if (true)
    {
        ScopedLock displayLock(displayMutex);

        for (int currSubproc = 0; currSubproc < numSubprocessors ; currSubproc++)
        {
           // int nSamples = (int)getSubprocessorSampleRate(inputSubprocessors[currSubproc]) * bufferLength;
           // int nInputs = numChannelsInSubprocessor[inputSubprocessors[currSubproc]];

            std::cout << "Resizing buffer for Subprocessor " << inputSubprocessors[currSubproc] << ". Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

            if (nSamples > 0 && nInputs > 0)
            {
                //abstractFifo.setTotalSize(nSamples);
                displayBuffers[currSubproc]->setSize(nInputs + 1, nSamples); // add extra channel for TTLs
                displayBuffers[currSubproc]->clear();

                displayBufferIndices[currSubproc].clear();
                displayBufferIndices[currSubproc].insert(displayBufferIndices[currSubproc].end(), nInputs + 1, 0);

                channelIndices.clear();

                totalResized++;
            }
        }
    }
    
    if (totalResized == numSubprocessors)
    {
        return true;
    }

    return false;

}*/


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

/*void LfpDisplayNode::setNumberOfDisplays(int num)
{
    numDisplays = num;

    subprocessorToDraw.clear();
    subprocessorToDraw.insertMultiple(0, 0, num);
    setDefaultSubprocessors();

    triggerSource.clear();
    triggerSource.insertMultiple(0, -1, num);

    latestTrigger.clear();
    latestTrigger.insertMultiple(0, -1, num);

    latestCurrentTrigger.clear();
    latestCurrentTrigger.resize(num);
}*/

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
