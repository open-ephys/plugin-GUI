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
	: GenericProcessor	("Crossing Detector")
	, threshold			(START_THRESH)
	, direction			(START_DIRECTION)
	, inputChan			(START_INPUT)
	, eventChan			(START_OUTPUT)
	, eventDuration		(START_DURATION)
	, timeout			(START_TIMEOUT)
	, fracPrev			(START_FRAC_PREV)
	, numPrev			(START_NUM_PREV)
	, fracNext			(START_FRAC_NEXT)
	, numNext			(START_NUM_NEXT)
	, sampsToShutoff	(-1)
	, sampsToReenable	(numPrev)
	, shutoffChan		(-1)
{}

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

	// event-related metadata!
	eventMetaDataDescriptors.clearQuick();

	MetaDataDescriptor* eventLevelDesc = new MetaDataDescriptor(MetaDataDescriptor::FLOAT, 1, "Event level",
		"Actual voltage level at sample where event occurred", "crossing.eventLevel");
	chan->addEventMetaData(eventLevelDesc);
	eventMetaDataDescriptors.add(eventLevelDesc);

	MetaDataDescriptor* dataChanDesc = new MetaDataDescriptor(MetaDataDescriptor::INT32, 1, "Data channel",
		"Index of the subscribed data channel", "crossing.dataChannel");
	chan->addEventMetaData(dataChanDesc);
	eventMetaDataDescriptors.add(dataChanDesc);

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

/*
* Precondition: eventMetaDataDescriptors has been populated through addEventMetaDataFields (called on 'update')
*/
void CrossingDetector::populateMetaDataArray(EventChannel* chan, MetaDataValueArray& mdArray, float eventLevel) const
{
	
}

void CrossingDetector::process(AudioSampleBuffer& continuousBuffer)
{
    // atomic field access
    int currChan = inputChan;
    int currNumPrev = numPrev;
    int currNumNext = numNext;
	float currThresh = threshold;
	CrossingDirection currDirection = direction;

    if (currChan < 0 || currChan >= continuousBuffer.getNumChannels()) // (shouldn't really happen)
        return;

    int nSamples = getNumSamples(currChan);
    const float* rp = continuousBuffer.getReadPointer(currChan);

    // loop has two functions: detect crossings and turn on events for the end of the previous buffer and most of the current buffer,
    // or if an event is currently on, turn it off if it has been on for long enough.
    for (int i = -currNumNext + 1; i < nSamples; i++)
    {
        // if enabled, check whether to trigger an event (operates on [-currNumNext+1, nSamples - currNumNext] )
		bool turnOn = (i >= sampsToReenable && i <= nSamples - currNumNext && shouldTrigger(rp, nSamples, i,
			currDirection, currNumPrev, currNumNext));
		
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

			MetaDataValue* eventLevelVal = new MetaDataValue(*eventMetaDataDescriptors[0]);
			eventLevelVal->setValue(rp[eventTime]);
			mdArray.add(eventLevelVal);

			MetaDataValue* dataChanVal = new MetaDataValue(*eventMetaDataDescriptors[1]);
			dataChanVal->setValue(static_cast<int32>(currChan));
			mdArray.add(dataChanVal);

			MetaDataValue* threshVal = new MetaDataValue(*eventMetaDataDescriptors[2]);
			threshVal->setValue(currThresh);
			mdArray.add(threshVal);

			MetaDataValue* posOnVal = new MetaDataValue(*eventMetaDataDescriptors[3]);
			posOnVal->setValue((currDirection == dPos || currDirection == dPosOrNeg) ? 1 : 0);
			mdArray.add(posOnVal);

			MetaDataValue* negOnVal = new MetaDataValue(*eventMetaDataDescriptors[4]);
			negOnVal->setValue((currDirection == dNeg || currDirection == dPosOrNeg) ? 1 : 0);
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
				int realEventChan = (shutoffChan != -1 ? shutoffChan : eventChan);
				uint8 ttlData = 1 << realEventChan;
				TTLEventPtr event = TTLEvent::createTTLEvent(eventChannelPtr, timestamp, &ttlData,
					sizeof(uint8), mdArray, realEventChan);
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

    case pDirection:
        direction = static_cast<CrossingDirection>(static_cast<int>(newValue));
        break;

    case pInputChan:
        if (getNumInputs() > newValue)
            inputChan = static_cast<int>(newValue);
        break;

    case pEventChan:
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
	// force signal change update, in case settings have changed during acquisition
	CoreServices::updateSignalChain(getEditor());
    return true;
}

float CrossingDetector::getThreshold()
{
    return threshold;
}

int CrossingDetector::getEventDuration()
{
    return eventDuration;
}

int CrossingDetector::getTimeout()
{
    return timeout;
}

float CrossingDetector::getFracPrev()
{
    return fracPrev;
}

int CrossingDetector::getNumPrev()
{
    return numPrev;
}

float CrossingDetector::getFracNext()
{
    return fracNext;
}

int CrossingDetector::getNumNext()
{
    return numNext;
}

// private
bool CrossingDetector::shouldTrigger(const float* rpCurr, int nSamples, int t0, CrossingDirection dir, int nPrev, int nNext)
{
    if (dir == dNone)
        return false;

    if (dir == dPosOrNeg)
        return shouldTrigger(rpCurr, nSamples, t0, dPos, nPrev, nNext) || shouldTrigger(rpCurr, nSamples, t0, dNeg, nPrev, nNext);

    // atomic field access
    float currThresh = threshold;

    int minInd = t0 - nPrev;
    int maxInd = t0 + nNext - 1;

    // check whether we have enough data
    if (minInd < -lastBuffer.size() || maxInd >= nSamples)
        return false;

    const float* rpLast = lastBuffer.end();

// allow us to treat the previous and current buffers as one array
#define rp(x) ((x)>=0 ? rpCurr[(x)] : rpLast[(x)])

    int numPrevRequired = (int)ceil(nPrev * fracPrev);
    int numNextRequired = (int)ceil(nNext * fracNext);

    for (int i = minInd; i < t0 && numPrevRequired > 0; i++)
        if (dir == dPos ? rp(i) < currThresh : rp(i) > currThresh)
            numPrevRequired--;

    if (numPrevRequired == 0) // "prev" condition satisfied
    {
        for (int i = t0; i <= maxInd && numNextRequired > 0; i++)
            if (dir == dPos ? rp(i) > currThresh : rp(i) < currThresh)
                numNextRequired--;

        if (numNextRequired == 0) // "next" condition satisfied
            return true;
    }
    
    return false;

#undef rp
}
