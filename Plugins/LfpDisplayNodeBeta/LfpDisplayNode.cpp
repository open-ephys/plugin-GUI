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

using namespace LfpDisplayNodeBeta;


LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor  ("LFP Viewer Beta")
    , displayGain       (1)
    , bufferLength      (20.0f) // seconds
    , abstractFifo      (100)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    displayBuffer = new AudioSampleBuffer (8, 100); // channels, samples
    // (Size is later changed by resizeBuffer() to [bufferLength = 20] seconds.)

    const int heapSize = 5000;
    arrayOfOnes = new float[heapSize];
    for (int n = 0; n < heapSize; ++n)
    {
        arrayOfOnes[n] = 1;
    }
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

    channelForEventSource.clear();
    eventSourceNodes.clear();
    ttlState.clear();

    for (int i = 0; i < eventChannelArray.size(); ++i)
    {
        uint32 sourceID = getChannelSourceID(eventChannelArray[i]);
        if (!eventSourceNodes.contains(sourceID))
        {
            eventSourceNodes.add(sourceID);

        }
    }

    numEventChannels = eventSourceNodes.size();

    std::cout << "Found " << numEventChannels << " event channels." << std::endl;

    for (int i = 0; i < eventSourceNodes.size(); ++i)
    {
        std::cout << "Adding channel " << getNumInputs() + i << " for event source node " << eventSourceNodes[i] << std::endl;

        channelForEventSource[eventSourceNodes[i]] = getNumInputs() + i;
        ttlState[eventSourceNodes[i]] = 0;
    }

    displayBufferIndex.clear();
    displayBufferIndex.insertMultiple (0, 0, getNumInputs() + numEventChannels);
}

uint32 LfpDisplayNode::getChannelSourceID(const EventChannel* event) const
{
        return getProcessorFullId(event->getTimestampOriginProcessor(), event->getTimestampOriginSubProcessor());
}

bool LfpDisplayNode::resizeBuffer()
{
    int nSamples = (int) getSampleRate() * bufferLength;
    int nInputs = getNumInputs();

    std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

    if (nSamples > 0 && nInputs > 0)
    {
        abstractFifo.setTotalSize (nSamples);
        displayBuffer->setSize (nInputs + numEventChannels, nSamples); // add extra channels for TTLs
        displayBuffer->clear();
        //displayBufferIndex.clear();
        //displayBufferIndex.insertMultiple(0, 0, nInputs + numEventChannels);

        return true;
    }
    else
    {
        return false;
    }
}


bool LfpDisplayNode::enable()
{
    if (resizeBuffer())
    {
        LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
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
    eventValueChanges.clear();
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

std::list<int> LfpDisplayNode::eventStateChannels() const {
  std::list<int> cc;
  for (uint32 id: eventSourceNodes)
    cc.push_back(channelForEventSource.find(id)->second);
  return cc;
}                 

void LfpDisplayNode::handleEvent(const EventChannel* eventInfo,
                                 const MidiMessage& event, int samplePosition) {
  if (Event::getEventType(event) == EventChannel::TTL) {
    TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);
    const int eventChannel = ttl->getChannel();
    const int eventTime = samplePosition;
    const uint32 eventSourceNodeId = getChannelSourceID(eventInfo);
    int val = 1<<eventChannel;
    if (!ttl->getState())
      val = -val;
    eventValueChanges[eventSourceNodeId].push_back(EventValueChange(eventTime, val));
  }
}

void LfpDisplayNode::initializeEventChannels() {
  for (int i=0; i<eventSourceNodes.size(); i++) {
    uint32 src = eventSourceNodes[i];
    eventValueChanges[src] = std::list<EventValueChange>();
  }
}

void LfpDisplayNode::copyToEventChannel(uint32 src,
                                        int t0,
                                        int t1,
                                        float value) {
  int dispBufSamps = displayBuffer->getNumSamples();
  int chan = channelForEventSource[src];
  int destStart = t0 + displayBufferIndex[chan];
  int n = t1 - t0;
  while (destStart + n >= dispBufSamps) {
    int m = dispBufSamps - destStart;
    displayBuffer->copyFrom(chan, destStart, arrayOfOnes, m, value);
    n -= m;
    destStart = 0;
  }
  if (n>0)
    displayBuffer->copyFrom(chan, destStart, arrayOfOnes, n, value);
}

void LfpDisplayNode::finalizeEventChannels() {
  for (int i=0; i<eventSourceNodes.size(); ++i) {
    uint32 src = eventSourceNodes[i];
    int chan = channelForEventSource[src];
    int index = displayBufferIndex[chan];
    int nSamples = getNumSourceSamples(src);
    int t0 = 0;
    for (EventValueChange const &evc: eventValueChanges[src]) {
      int t1 = evc.eventTime;
      copyToEventChannel(src, t0, t1, ttlState[src]);
      if (evc.eventVal>0)
        ttlState[src] |= evc.eventVal;
      else
        ttlState[src] &= ~evc.eventVal;
      t0 = t1;
    }
    copyToEventChannel(src, t0, nSamples, ttlState[src]);
    displayBufferIndex.set(chan, (index + nSamples)
                           % displayBuffer->getNumSamples());
  }
}

void LfpDisplayNode::copyDataToDisplay(int chan, AudioSampleBuffer &srcbuf) {
  int n = getNumSamples(chan);
  int idx = displayBufferIndex[chan];
  int buflen = displayBuffer->getNumSamples();
  int t0 = 0;
  while (idx + n >= buflen) {
    int m = buflen - idx;
    displayBuffer->copyFrom(chan, idx, srcbuf, chan, t0, m);
    t0 += m;
    n -= m;
    idx = 0;
  }
  if (n>0) {
    displayBuffer->copyFrom(chan, idx, srcbuf, chan, t0, n);
    idx += n;
  }
  displayBufferIndex.set(chan, idx);
}

void LfpDisplayNode::process (AudioSampleBuffer& buffer)
{
    // 1. place any new samples into the displayBuffer
    //std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    initializeEventChannels();
    checkForEvents(); // see if we got any TTL events 
    ScopedLock displayLock(displayMutex);
    finalizeEventChannels();
    for (int chan = 0; chan < buffer.getNumChannels(); ++chan) 
      copyDataToDisplay(chan, buffer);
}

