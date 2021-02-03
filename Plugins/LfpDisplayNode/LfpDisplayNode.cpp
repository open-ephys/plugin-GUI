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
    , bufferLength      (10.0f)
    , abstractFifo      (100)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    const int heapSize = 5000;
    arrayOfOnes = new float[heapSize];
    for (int n = 0; n < heapSize; ++n)
    {
        arrayOfOnes[n] = 1;
    }

    subprocessorToDraw = 0;
    numSubprocessors = -1;
    triggerSource = -1;
    latestTrigger = -1;
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

    if (numChannelsInSubprocessor.find(subprocessorToDraw) == numChannelsInSubprocessor.end())
    {
        // subprocessor to draw does not exist
        if (numSubprocessors == 0)
        {
            subprocessorToDraw = 0;
        }
        else
        {
            // there are channels, but none on the current subprocessorToDraw
            // default to the first subprocessor
            subprocessorToDraw = getDataSubprocId(0);
        }
    }

    int numChans = getNumSubprocessorChannels();
    int srate = getSubprocessorSampleRate(subprocessorToDraw);

    std::cout << "Re-setting num inputs on LfpDisplayNode to " << numChans << std::endl;
    if (numChans > 0)
    {
        std::cout << "Sample rate = " << srate << std::endl;
    }

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
        std::cout << "Adding channel " << numChans + i << " for event source node " << eventSourceNodes[i] << std::endl;

        ttlState[eventSourceNodes[i]] = 0;
    }
    
    // update the editor's subprocessor selection display and sample rate
    LfpDisplayEditor * ed = (LfpDisplayEditor*)getEditor();
    ed->updateSubprocessorSelectorOptions();

    resizeBuffer();
}

std::shared_ptr<AudioSampleBuffer> LfpDisplayNode::getDisplayBufferAddress() const
{ 
    if (displayBuffers.size() > 0)
        return displayBuffers[allSubprocessors.indexOf(subprocessorToDraw)];
    else
        return nullptr;
}

int LfpDisplayNode::getDisplayBufferIndex(int chan) const
{ 
    if (displayBufferIndices.size() > 0)
        return displayBufferIndices[allSubprocessors.indexOf(subprocessorToDraw)][chan];
    else
        return -1;
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

void LfpDisplayNode::setSubprocessor(uint32 sp)
{
    subprocessorToDraw = sp;
    std::cout << "LfpDisplayNode setting subprocessor to " << sp << std::endl;    
}

uint32 LfpDisplayNode::getSubprocessor() const
{
    return subprocessorToDraw;
}

int LfpDisplayNode::getNumSubprocessorChannels()
{
    if (subprocessorToDraw != 0)
    {
        return numChannelsInSubprocessor[subprocessorToDraw];
    }
    return 0;
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

bool LfpDisplayNode::resizeBuffer()
{
    LfpDisplayEditor * ed = (LfpDisplayEditor*)getEditor();
    allSubprocessors = ed->getInputSubprocessors();
    
    int totalResized = 0;

    if (true)
    {
        ScopedLock displayLock(displayMutex);

        for (int currSubproc = 0; currSubproc < numSubprocessors ; currSubproc++)
        {
            int nSamples = (int)getSubprocessorSampleRate(allSubprocessors[currSubproc]) * bufferLength;
            int nInputs = numChannelsInSubprocessor[allSubprocessors[currSubproc]];

            std::cout << "Resizing buffer for Subprocessor " << allSubprocessors[currSubproc] << ". Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

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
            if (eventChannel==triggerSource)
            {
                latestCurrentTrigger = eventTime;
            }
        }
        else
        {
            ttlState[eventSourceNodeId] &= ~(1LL << eventChannel);
        }

        int subProcIndex = allSubprocessors.indexOf(eventSourceNodeId);

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
    latestCurrentTrigger = -1;
    for (int i = 0 ; i < numSubprocessors ; i++)
    {
        const int chan = numChannelsInSubprocessor[allSubprocessors[i]];
        const int nSamples = getNumSourceSamples(allSubprocessors[i]);
        const int samplesLeft   = displayBuffers[i]->getNumSamples() - displayBufferIndices[i][chan];
        
        if (nSamples < samplesLeft)
        {

            displayBuffers[i]->copyFrom (chan,                                      // destChannel
                                     displayBufferIndices[i][chan],             // destStartSample
                                     arrayOfOnes,                               // source
                                     nSamples,                                  // numSamples
                                     float (ttlState[subprocessorToDraw]));     // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            displayBuffers[i]->copyFrom (chan,                                      // destChannel
                                     displayBufferIndices[i][chan],             // destStartSample
                                     arrayOfOnes,                               // source
                                     samplesLeft,                               // numSamples
                                     float (ttlState[subprocessorToDraw]));     // gain

            displayBuffers[i]->copyFrom (chan,                                      // destChannel
                                     0,                                         // destStartSample
                                     arrayOfOnes,                               // source
                                     extraSamples,                              // numSamples
                                     float (ttlState[subprocessorToDraw]));     // gain
        }
    }
}

void LfpDisplayNode::finalizeEventChannels()
{
    for (int i = 0 ; i < numSubprocessors ; i++){    
        const int chan          = numChannelsInSubprocessor[allSubprocessors[i]];
        const int index = displayBufferIndices[i][chan];
        const int samplesLeft   = displayBuffers[i]->getNumSamples() - index;
        const int nSamples      = getNumSourceSamples(allSubprocessors[i]);
        
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
    
    if (latestCurrentTrigger >= 0)
    {
        int chan = numChannelsInSubprocessor[subprocessorToDraw];
        int subProcIndex = allSubprocessors.indexOf(subprocessorToDraw);
        latestTrigger = latestCurrentTrigger + displayBufferIndices[subProcIndex][chan];
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
                currSubproc = allSubprocessors.indexOf(subProcId);

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

void LfpDisplayNode::setTriggerSource(int ch) {
  printf("Trigger source: %i\n", ch);
  triggerSource = ch;
}

int LfpDisplayNode::getTriggerSource() const {
  return triggerSource;
}

int64 LfpDisplayNode::getLatestTriggerTime() const {
  return latestTrigger;
}

void LfpDisplayNode::acknowledgeTrigger() {
  latestTrigger = -1;
}
