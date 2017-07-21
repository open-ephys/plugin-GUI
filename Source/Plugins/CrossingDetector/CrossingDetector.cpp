/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#include "CrossingDetector.h"
#include "CrossingDetectorEditor.h"

CrossingDetector::CrossingDetector()
    : GenericProcessor  ("Crossing Detector")
    , threshold         (0.0f)
    , posOn             (true)
    , negOn             (false)
    , inputChan         (0)
    , eventChan         (0)
    , eventDuration     (100)
    , timeout           (1000)
    , fracPrev          (1.0f)
    , numPrev           (1)
    , fracNext          (1.0f)
    , numNext           (1)
    , sampsToShutoff    (-1)
    , sampsToReenable   (numPrev)
    , shutoffChan       (-1)
{
    setProcessorType(PROCESSOR_TYPE_FILTER);
}

CrossingDetector::~CrossingDetector() {}

AudioProcessorEditor* CrossingDetector::createEditor()
{
    editor = new CrossingDetectorEditor(this);
    return editor;
}

void CrossingDetector::createEventChannels()
{
    // add detection event channel
    const DataChannel* in = getDataChannel(inputChan);
    float sampleRate = in ? in->getSampleRate() : CoreServices::getGlobalSampleRate();
    EventChannel* chan = new EventChannel(EventChannel::TTL, 8, 1, sampleRate, this);
    chan->setName("Crossing detector output");
    chan->setDescription("Triggers whenever the input signal crosses a voltage threshold.");
    chan->setIdentifier("crossing.event");

    // metadata storing source data channel
    if (in)
    {
        MetaDataDescriptor sourceChanDesc(MetaDataDescriptor::UINT16, 3, "Source Channel",
            "Index at its source, Source processor ID and Sub Processor index of the channel that triggers this event", "source.channel.identifier.full");
        MetaDataValue sourceChanVal(sourceChanDesc);
        uint16 sourceInfo[3];
        sourceInfo[0] = in->getSourceIndex();
        sourceInfo[1] = in->getSourceNodeID();
        sourceInfo[2] = in->getSubProcessorIdx();
        sourceChanVal.setValue(static_cast<const uint16*>(sourceInfo));
        chan->addMetaData(sourceChanDesc, sourceChanVal);
    }

    // event-related metadata!
    eventMetaDataDescriptors.clearQuick();

    MetaDataDescriptor* eventLevelDesc = new MetaDataDescriptor(MetaDataDescriptor::FLOAT, 1, "Event level",
        "Actual voltage level at sample where event occurred", "crossing.eventLevel");
    chan->addEventMetaData(eventLevelDesc);
    eventMetaDataDescriptors.add(eventLevelDesc);

    MetaDataDescriptor* threshDesc = new MetaDataDescriptor(MetaDataDescriptor::FLOAT, 1, "Threshold",
        "Monitored voltage threshold", "crossing.threshold");
    chan->addEventMetaData(threshDesc);
    eventMetaDataDescriptors.add(threshDesc);

    MetaDataDescriptor* posOnDesc = new MetaDataDescriptor(MetaDataDescriptor::UINT8, 1, "Ascending on",
        "Equals 1 if an event is triggered for ascending crossings", "crossing.positive");
    chan->addEventMetaData(posOnDesc);
    eventMetaDataDescriptors.add(posOnDesc);

    MetaDataDescriptor* negOnDesc = new MetaDataDescriptor(MetaDataDescriptor::UINT8, 1, "Descending on",
        "Equals 1 if an event is triggered for descending crossings", "crossing.negative");
    chan->addEventMetaData(negOnDesc);
    eventMetaDataDescriptors.add(negOnDesc);

    eventChannelPtr = eventChannelArray.add(chan);
}

void CrossingDetector::process(AudioSampleBuffer& continuousBuffer)
{
    // atomic field access
    int currChan = inputChan;
    int currNumPrev = numPrev;
    int currNumNext = numNext;

    if (currChan < 0 || currChan >= continuousBuffer.getNumChannels()) // (shouldn't really happen)
        return;

    int nSamples = getNumSamples(currChan);
    const float* rp = continuousBuffer.getReadPointer(currChan);

    // loop has two functions: detect crossings and turn on events for the end of the previous buffer and most of the current buffer,
    // or if an event is currently on, turn it off if it has been on for long enough.
    for (int i = -currNumNext + 1; i < nSamples; i++)
    {
        // atomic field access
        float currThresh = threshold;
        bool currPosOn = posOn;
        bool currNegOn = negOn;

        // if enabled, check whether to trigger an event (operates on [-currNumNext+1, nSamples - currNumNext] )
        bool turnOn = (i >= sampsToReenable && i <= nSamples - currNumNext && shouldTrigger(rp, nSamples, i, currThresh,
            currPosOn, currNegOn, currNumPrev, currNumNext));
        
        // if not triggering, check whether event should be shut off (operates on [0, nSamples) )
        bool turnOff = (!turnOn && i >= 0 && i == sampsToShutoff);

        if (turnOn || turnOff)
        {
            // actual sample when event fires (start of current buffer if turning on and the crossing was in prev. buffer.)
            int eventTime = turnOn ? std::max(i, 0) : i;
            int64 timestamp = getTimestamp(currChan) + eventTime;

            // construct the event's metadata array
            // The order of metadata has to match the order they are stored in createEventChannels.
            MetaDataValueArray mdArray;

            int mdInd = 0;
            MetaDataValue* eventLevelVal = new MetaDataValue(*eventMetaDataDescriptors[mdInd++]);
            eventLevelVal->setValue(rp[eventTime]);
            mdArray.add(eventLevelVal);

            MetaDataValue* threshVal = new MetaDataValue(*eventMetaDataDescriptors[mdInd++]);
            threshVal->setValue(currThresh);
            mdArray.add(threshVal);

            MetaDataValue* posOnVal = new MetaDataValue(*eventMetaDataDescriptors[mdInd++]);
            posOnVal->setValue(static_cast<uint8>(posOn));
            mdArray.add(posOnVal);

            MetaDataValue* negOnVal = new MetaDataValue(*eventMetaDataDescriptors[mdInd++]);
            negOnVal->setValue(static_cast<uint8>(negOn));
            mdArray.add(negOnVal);
            
            if (turnOn)
            {
                // add event
                uint8 ttlData = 1 << eventChan;
                TTLEventPtr event = TTLEvent::createTTLEvent(eventChannelPtr, timestamp, &ttlData,
                    sizeof(uint8), mdArray, eventChan);
                addEvent(eventChannelPtr, event, eventTime);

                // schedule event turning off and timeout period ending
                sampsToShutoff = eventTime + eventDuration;
                sampsToReenable = eventTime + timeout;
            }
            else
            {
                // add (turning-off) event
                uint8 ttlData = 0;
                int realEventChan = (shutoffChan != -1 ? shutoffChan : eventChan);
                TTLEventPtr event = TTLEvent::createTTLEvent(eventChannelPtr, timestamp, 
                    &ttlData, sizeof(uint8), mdArray, realEventChan);
                addEvent(eventChannelPtr, event, eventTime);

                // reset shutoffChan (now eventChan has been changed)
                shutoffChan = -1;
            }
        }
    }

    if (sampsToShutoff >= nSamples)
        // shift so it is relative to the next buffer
        sampsToShutoff -= nSamples;
    else
        // no scheduled shutoff, so keep it at -1
        sampsToShutoff = -1;

    if (sampsToReenable >= -currNumNext)
        // shift so it is relative to the next buffer
        sampsToReenable -= nSamples;

    // save this buffer for the next execution
    lastBuffer.clearQuick();
    lastBuffer.addArray(rp, nSamples);
}

// all new values should be validated before this function is called!
void CrossingDetector::setParameter(int parameterIndex, float newValue)
{
    switch (parameterIndex)
    {
    case pThreshold:
        threshold = newValue;
        break;

    case pPosOn:
        posOn = static_cast<bool>(newValue);
        break;

    case pNegOn:
        negOn = static_cast<bool>(newValue);
        break;

    case pInputChan:
        if (getNumInputs() > newValue)
            inputChan = static_cast<int>(newValue);
        break;

    case pEventChan:
        // if we're in the middle of an event, keep track of the old channel until it's done.
        if (sampsToShutoff > -1)
            shutoffChan = eventChan;
        eventChan = static_cast<int>(newValue);
        break;

    case pEventDur:
        eventDuration = static_cast<int>(newValue);
        break;

    case pTimeout:
        timeout = static_cast<int>(newValue);
        break;

    case pNumPrev:
        numPrev = static_cast<int>(newValue);
        sampsToReenable = numPrev;
        break;

    case pFracPrev:
        fracPrev = newValue;
        break;

    case pNumNext:
        numNext = static_cast<int>(newValue);
        break;

    case pFracNext:
        fracNext = newValue;
        break;
    }
}

bool CrossingDetector::disable()
{
    // set this to numPrev so that we don't trigger on old data when we start again.
    sampsToReenable = numPrev;
    return true;
}

// private
bool CrossingDetector::shouldTrigger(const float* rpCurr, int nSamples, int t0, float currThresh,
    bool currPosOn, bool currNegOn, int currNumPrev, int currNumNext)
{
    if (!currPosOn && !currNegOn)
        return false;

    if (currPosOn && currNegOn)
        return shouldTrigger(rpCurr, nSamples, t0, currThresh, true, false, currNumPrev, currNumNext)
        || shouldTrigger(rpCurr, nSamples, t0, currThresh, false, true, currNumPrev, currNumNext);

    // at this point exactly one of posOn and negOn is true.

    int minInd = t0 - currNumPrev;
    int maxInd = t0 + currNumNext - 1;

    // check whether we have enough data
    if (minInd < -lastBuffer.size() || maxInd >= nSamples)
        return false;

    const float* rpLast = lastBuffer.end();

// allow us to treat the previous and current buffers as one array
#define rp(x) ((x)>=0 ? rpCurr[(x)] : rpLast[(x)])

    int numPrevRequired = (int)ceil(currNumPrev * fracPrev);
    int numNextRequired = (int)ceil(currNumNext * fracNext);

    for (int i = minInd; i < t0 && numPrevRequired > 0; i++)
        if (currPosOn ? rp(i) < currThresh : rp(i) > currThresh)
            numPrevRequired--;

    if (numPrevRequired == 0) // "prev" condition satisfied
    {
        for (int i = t0; i <= maxInd && numNextRequired > 0; i++)
            if (currPosOn ? rp(i) > currThresh : rp(i) < currThresh)
                numNextRequired--;

        if (numNextRequired == 0) // "next" condition satisfied
            return true;
    }
    
    return false;

#undef rp
}
