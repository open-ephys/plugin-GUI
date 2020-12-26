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
    , displayGain       (1)
    , bufferLength      (2.0f)
    , abstractFifo      (100)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    const int heapSize = 5000;
    arrayOfOnes = new float[heapSize];
    for (int n = 0; n < heapSize; ++n)
    {
        arrayOfOnes[n] = 1;
    }

    numDisplays = 1;
    numSubprocessors = -1;
}


LfpDisplayNode::~LfpDisplayNode()
{
    delete[] arrayOfOnes;
}


AudioProcessorEditor* LfpDisplayNode::createEditor()
{
    editor = new LfpDisplayEditor (this, true);
    return editor;
}


void LfpDisplayNode::updateSettings()
{

    std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;

    numChannelsInSubprocessor.clear();
    subprocessorSampleRate.clear();

    for (int i = 0; i < getNumInputs(); i++)
    {
        uint32 channelSubprocessor = getDataSubprocId(i);

        numChannelsInSubprocessor.insert({ channelSubprocessor, 0 }); // (if not already there)
        numChannelsInSubprocessor[channelSubprocessor]++;

        subprocessorSampleRate.insert({ channelSubprocessor, getDataChannel(i)->getSampleRate() });
    }
    
    numSubprocessors = numChannelsInSubprocessor.size();

    displayBuffers.clear();

    for (int i = 0; i < numSubprocessors; i++)
        displayBuffers.push_back(std::make_shared<AudioSampleBuffer> (8, 100));

    displayBufferIndices.resize(numSubprocessors);

    channelIndices.resize(numSubprocessors);

    setDefaultSubprocessors();

    // int numChans = getNumSubprocessorChannels();
    // int srate = getSubprocessorSampleRate(subprocessorToDraw);

    // std::cout << "Re-setting num inputs on LfpDisplayNode to " << numChans << std::endl;
    // if (numChans > 0)
    // {
    //     std::cout << "Sample rate = " << srate << std::endl;
    // }

    eventSourceNodes.clear();
    ttlState.clear();

    for (int i = 0; i < eventChannelArray.size(); ++i)
    {
        uint32 sourceId = getEventSourceId(eventChannelArray[i]);
 
        if (!eventSourceNodes.contains(sourceId))
        {
            eventSourceNodes.add(sourceId);
        }
    }

    for (int i = 0; i < eventSourceNodes.size(); ++i)
    {
        // std::cout << "Adding channel " << numChans + i << " for event source node " << eventSourceNodes[i] << std::endl;

        ttlState[eventSourceNodes[i]] = 0;
    }
    
    updateInputSubprocessors();

    resizeBuffer();
}

uint32 LfpDisplayNode::getEventSourceId(const EventChannel* event)
{
    return getProcessorFullId(event->getTimestampOriginProcessor(), event->getTimestampOriginSubProcessor());
}

uint32 LfpDisplayNode::getChannelSourceId(const InfoObjectCommon* chan)
{
    return getProcessorFullId(chan->getSourceNodeID(), chan->getSubProcessorIdx());
}

uint32 LfpDisplayNode::getDataSubprocId(int chan) const
{
    if (chan < 0 || chan >= getTotalDataChannels())
    {
        return 0;
    }

    return getChannelSourceId(getDataChannel(chan));
}

void LfpDisplayNode::setSubprocessor(uint32 sp, int id)
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
}

void LfpDisplayNode::setDefaultSubprocessors()
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
        std::cout << "****** DEFAULT SUBP FOR #"<< i << " = " << subprocessorToDraw[i] << std::endl;
    }
}

float LfpDisplayNode::getSubprocessorSampleRate(uint32 subprocId)
{
    auto entry = subprocessorSampleRate.find(subprocId);
    if (entry != subprocessorSampleRate.end())
    {
        return entry->second;
    }
    return 0.0f;
}

void LfpDisplayNode::updateInputSubprocessors()
{
    // clear out the old data
    inputSubprocessors.clear();
    subprocessorNames.clear();
    
	if (getTotalDataChannels() != 0)
	{
		for (int i = 0, len = getTotalDataChannels(); i < len; ++i)
		{
            const DataChannel* ch = getDataChannel(i);
            uint16 sourceNodeId = ch->getSourceNodeID();
			uint16 subProcessorIdx = ch->getSubProcessorIdx();
            uint32 subProcFullId = GenericProcessor::getProcessorFullId(sourceNodeId, subProcessorIdx);

			bool added = inputSubprocessors.add(subProcFullId);

            if (added)
            {
                String sourceName = ch->getSourceName();
                subprocessorNames.set(subProcFullId,
                    sourceName + " " + String(sourceNodeId) + "/" + String(subProcessorIdx));
            }
		}
	}
}

bool LfpDisplayNode::resizeBuffer()
{   
    int totalResized = 0;

    if (true)
    {
        ScopedLock displayLock(displayMutex);

        for (int currSubproc = 0; currSubproc < numSubprocessors ; currSubproc++)
        {
            int nSamples = (int)getSubprocessorSampleRate(inputSubprocessors[currSubproc]) * bufferLength;
            int nInputs = numChannelsInSubprocessor[inputSubprocessors[currSubproc]];

            std::cout << "Resizing buffer for Subprocessor " << inputSubprocessors[currSubproc] << ". Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

            if (nSamples > 0 && nInputs > 0)
            {
                abstractFifo.setTotalSize(nSamples);
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

}


bool LfpDisplayNode::enable()
{

    if (resizeBuffer())
    {
        LfpDisplayEditor* editor = (LfpDisplayEditor*)getEditor();
        editor->enable();
        return true;
    }
    else
    {
        return false;
    }

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
        float eventSampleRate = getSubprocessorSampleRate(eventSourceNodeId);

        if (eventSampleRate == 0)
        {
            // shouldn't happen for any real event channel at this point
            return;
        }
        
        //std::cout << "Received event on channel " << eventChannel << std::endl;
        //std::cout << "Copying to channel " << channelForEventSource[eventSourceNodeId] << std::endl;
        
        if (eventId == 1)
        {
            ttlState[eventSourceNodeId] |= (1LL << eventChannel);
            for(int i = 0; i < numDisplays; i++)
            {
                if (eventChannel==triggerSource[i])
                {
                    latestCurrentTrigger.set(i, eventTime);
                }
            }
        }
        else
        {
            ttlState[eventSourceNodeId] &= ~(1LL << eventChannel);
        }

        int subProcIndex = inputSubprocessors.indexOf(eventSourceNodeId);

        const int chan          = numChannelsInSubprocessor[eventSourceNodeId];
        const int index         = (displayBufferIndices[subProcIndex][chan] + eventTime) % displayBuffers[subProcIndex]->getNumSamples();
        const int samplesLeft   = displayBuffers[subProcIndex]->getNumSamples() - index;
        const int nSamples      = getNumSourceSamples(eventSourceNodeId) - eventTime;

        if (nSamples < samplesLeft)
        {
            displayBuffers[subProcIndex]->copyFrom(chan,                                 // destChannel
                                    index,                                // destStartSample
                                    arrayOfOnes,                          // source
                                    nSamples,                             // numSamples
                                    float(ttlState[eventSourceNodeId]));  // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            displayBuffers[subProcIndex]->copyFrom(chan,                                 // destChannel
                                    index,                                // destStartSample
                                    arrayOfOnes,                          // source
                                    samplesLeft,                          // numSamples
                                    float(ttlState[eventSourceNodeId]));  // gain

            displayBuffers[subProcIndex]->copyFrom(chan,                                 // destChannel
                                    0,                                    // destStartSample
                                    arrayOfOnes,                          // source
                                    extraSamples,                         // numSamples
                                    float(ttlState[eventSourceNodeId]));  // gain
        }

        //         std::cout << "Received event from " << eventSourceNodeId
        //                   << " on channel " << eventChannel
        //                   << " with value " << eventId
        //                   << " at timestamp " << event.getTimeStamp() << std::endl;
    }
}


void LfpDisplayNode::initializeEventChannels()
{

    //std::cout << "Initializing events..." << std::endl
    latestCurrentTrigger.insertMultiple(0, -1, numDisplays);

    for (int i = 0 ; i < numSubprocessors ; i++)
    {
        const int chan = numChannelsInSubprocessor[inputSubprocessors[i]];
        const int nSamples = getNumSourceSamples(inputSubprocessors[i]);
        const int samplesLeft   = displayBuffers[i]->getNumSamples() - displayBufferIndices[i][chan];
        
        if (nSamples < samplesLeft)
        {

            displayBuffers[i]->copyFrom (chan,                                      // destChannel
                                     displayBufferIndices[i][chan],             // destStartSample
                                     arrayOfOnes,                               // source
                                     nSamples,                                  // numSamples
                                     float (ttlState[inputSubprocessors[i]]));     // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            displayBuffers[i]->copyFrom (chan,                                      // destChannel
                                     displayBufferIndices[i][chan],             // destStartSample
                                     arrayOfOnes,                               // source
                                     samplesLeft,                               // numSamples
                                     float (ttlState[inputSubprocessors[i]]));     // gain

            displayBuffers[i]->copyFrom (chan,                                      // destChannel
                                     0,                                         // destStartSample
                                     arrayOfOnes,                               // source
                                     extraSamples,                              // numSamples
                                     float (ttlState[inputSubprocessors[i]]));     // gain
        }
    }
}

void LfpDisplayNode::finalizeEventChannels()
{
    for (int i = 0 ; i < numSubprocessors ; i++){    
        const int chan          = numChannelsInSubprocessor[inputSubprocessors[i]];
        const int index         = displayBufferIndices[i][chan];
        const int samplesLeft   = displayBuffers[i]->getNumSamples() - index;
        const int nSamples      = getNumSourceSamples(inputSubprocessors[i]);
        
        int newIdx = 0;
        
        if (nSamples < samplesLeft)
        {
            newIdx = displayBufferIndices[i][chan] + nSamples;
        }
        else
        {
            newIdx = nSamples - samplesLeft;
        }
        
        displayBufferIndices[i][chan] = newIdx;
    }
    
    for(int i = 0; i < numDisplays; i++)
    {
        if (latestCurrentTrigger[i] >= 0)
        {
            int chan = numChannelsInSubprocessor[subprocessorToDraw[i]];
            int subProcIndex = inputSubprocessors.indexOf(subprocessorToDraw[i]);
            latestTrigger.set(i, latestCurrentTrigger[i] + displayBufferIndices[subProcIndex][chan]);
        }
    }  
}


void LfpDisplayNode::process (AudioSampleBuffer& buffer)
{
    // 1. place any new samples into the displayBuffer
    //std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    if (true)
    {
        ScopedLock displayLock(displayMutex);

        if (true)
        {
            initializeEventChannels();
            checkForEvents(); // see if we got any TTL events
            finalizeEventChannels();
        }

        if (true)
        {
            int channelIndex = -1;
            channelIndices.insertMultiple(0, -1, numSubprocessors);
            uint32 subProcId = 0;
            int currSubproc = -1;

            for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
            {
                subProcId =  getDataSubprocId(chan);
                currSubproc = inputSubprocessors.indexOf(subProcId);

                channelIndices.set(currSubproc, channelIndices[currSubproc] + 1);

                const int samplesLeft = displayBuffers[currSubproc]->getNumSamples() - displayBufferIndices[currSubproc][channelIndices[currSubproc]];
                const int nSamples = getNumSamples(chan);

                if (nSamples < samplesLeft)
                {
                    displayBuffers[currSubproc]->copyFrom(channelIndices[currSubproc],                      // destChannel
                        displayBufferIndices[currSubproc][channelIndices[currSubproc]],  // destStartSample
                        buffer,                    // source
                        chan,                      // source channel
                        0,                         // source start sample
                        nSamples);                 // numSamples

                    displayBufferIndices[currSubproc][channelIndices[currSubproc]] = displayBufferIndices[currSubproc][channelIndices[currSubproc]] + nSamples;
                }
                else
                {
                    const int extraSamples = nSamples - samplesLeft;

                    displayBuffers[currSubproc]->copyFrom(channelIndices[currSubproc],                      // destChannel
                        displayBufferIndices[currSubproc][channelIndices[currSubproc]],  // destStartSample
                        buffer,                    // source
                        chan,                      // source channel
                        0,                         // source start sample
                        samplesLeft);              // numSamples

                    displayBuffers[currSubproc]->copyFrom(channelIndices[currSubproc],                      // destChannel
                        0,                         // destStartSample
                        buffer,                    // source
                        chan,                      // source channel
                        samplesLeft,               // source start sample
                        extraSamples);             // numSamples

                    displayBufferIndices[currSubproc][channelIndices[currSubproc]] = extraSamples;
                }

            }
        }
    }
}

void LfpDisplayNode::setNumberOfDisplays(int num)
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
