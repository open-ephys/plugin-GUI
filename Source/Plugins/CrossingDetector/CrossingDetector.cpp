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
    , pastStrict        (1.0f)
    , pastSpan          (1)
    , futureStrict      (1.0f)
    , futureSpan        (1)
    , sampsToShutoff    (-1)
    , sampsToReenable   (pastSpan)
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
    int currPastSpan = pastSpan;
    int currFutureSpan = futureSpan;

    if (currChan < 0 || currChan >= continuousBuffer.getNumChannels()) // (shouldn't really happen)
        return;

    int nSamples = getNumSamples(currChan);
    const float* rp = continuousBuffer.getReadPointer(currChan);

    // loop has two functions: detect crossings and turn on events for the end of the previous buffer and most of the current buffer,
    // or if an event is currently on, turn it off if it has been on for long enough.
    for (int i = -currFutureSpan + 1; i < nSamples; i++)
    {
        // atomic field access
        float currThresh = threshold;
        bool currPosOn = posOn;
        bool currNegOn = negOn;

        // if enabled, check whether to trigger an event (operates on [-currFutureSpan+1, nSamples - currFutureSpan] )
        bool turnOn = (i >= sampsToReenable && i <= nSamples - currFutureSpan && shouldTrigger(rp, nSamples, i, currThresh,
            currPosOn, currNegOn, currPastSpan, currFutureSpan));
        
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

    if (sampsToReenable >= -currFutureSpan)
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

    case pPastSpan:
        pastSpan = static_cast<int>(newValue);
        sampsToReenable = pastSpan;
        break;

    case pPastStrict:
        pastStrict = newValue;
        break;

    case pFutureSpan:
        futureSpan = static_cast<int>(newValue);
        break;

    case pFutureStrict:
        futureStrict = newValue;
        break;
    }
}

bool CrossingDetector::disable()
{
    // set this to pastSpan so that we don't trigger on old data when we start again.
    sampsToReenable = pastSpan;
    return true;
}

// private
bool CrossingDetector::shouldTrigger(const float* rpCurr, int nSamples, int t0, float currThresh,
    bool currPosOn, bool currNegOn, int currPastSpan, int currFutureSpan)
{
    if (!currPosOn && !currNegOn)
        return false;

    if (currPosOn && currNegOn)
        return shouldTrigger(rpCurr, nSamples, t0, currThresh, true, false, currPastSpan, currFutureSpan)
        || shouldTrigger(rpCurr, nSamples, t0, currThresh, false, true, currPastSpan, currFutureSpan);

    // at this point exactly one of posOn and negOn is true.

    int minInd = t0 - currPastSpan;
    int maxInd = t0 + currFutureSpan - 1;

    // check whether we have enough data
    if (minInd < -lastBuffer.size() || maxInd >= nSamples)
        return false;

    const float* rpLast = lastBuffer.end();

// allow us to treat the previous and current buffers as one array
#define rp(x) ((x)>=0 ? rpCurr[(x)] : rpLast[(x)])

    int numPastRequired = (int)ceil(currPastSpan * pastStrict);
    int numFutureRequired = (int)ceil(currFutureSpan * futureStrict);

    for (int i = minInd; i < t0 && numPastRequired > 0; i++)
        if (currPosOn ? rp(i) < currThresh : rp(i) > currThresh)
            numPastRequired--;

    if (numPastRequired == 0) // "prev" condition satisfied
    {
        for (int i = t0; i <= maxInd && numFutureRequired > 0; i++)
            if (currPosOn ? rp(i) > currThresh : rp(i) < currThresh)
                numFutureRequired--;

        if (numFutureRequired == 0) // "next" condition satisfied
            return true;
    }
    
    return false;

#undef rp
}
