/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include <stdio.h>
#include "PhaseDetector.h"
#include "Editors/PhaseDetectorEditor.h"

PhaseDetector::PhaseDetector()
    : GenericProcessor("Phase Detector"),
      maxFrequency(20), isIncreasing(true), canBeTriggered(false), selectedChannel(-1),
      triggerOnPeak(true), outputEventChannel(3)

{

    peakIntervals = new int[NUM_INTERVALS];

    randomNumberGenerator.setSeed(Time::currentTimeMillis());

    //parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

}

PhaseDetector::~PhaseDetector()
{

}

AudioProcessorEditor* PhaseDetector::createEditor()
{
    editor = new PhaseDetectorEditor(this, true);

    std::cout << "Creating editor." << std::endl;

    return editor;
}



void PhaseDetector::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    editor->updateParameterButtons(parameterIndex);
    
    if (parameterIndex == 1)
    {
        selectedChannel = (int) newValue;
    } else if (parameterIndex == 2)
    {
        outputEventChannel = (int) newValue; // -1 means don't send any events
    }

}

void PhaseDetector::updateSettings()
{

    minSamplesToNextPeak = int(getSampleRate()/maxFrequency);

}

bool PhaseDetector::enable()
{
    nSamplesSinceLastPeak = 0;
    lastSample = 0.0f;
    isIncreasing = false;
    numPeakIntervals = 0;

    return true;
}

void PhaseDetector::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    // MOVED GATING TO PULSE PAL OUTPUT!
    // now use to randomize phase for next trial

    //std::cout << "GOT EVENT." << std::endl;

    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        // int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);
        //int eventTime = event.getTimeStamp();

        //     //	std::cout << "Received event from " << eventNodeId << ", channel "
        //     //          << eventChannel << ", with ID " << eventId << std::endl;

        //     if (eventId == 1 && eventChannel == 5)
        //     {
        //         canBeTriggered = true;
        //     }
        if (eventId == 0 && eventChannel == 1)
        {
            triggerOnPeak = randomNumberGenerator.nextBool();

        }

    }

}

void PhaseDetector::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events,
                            int& nSamples)
{

    checkForEvents(events);

    if (selectedChannel >= 0 && selectedChannel < buffer.getNumChannels())
    {

        for (int i = 0; i < nSamples; i++)
        {

            float  sample = *buffer.getSampleData(selectedChannel, i);

            if (!triggerOnPeak)
                sample = -sample; // invert


            if (sample > lastSample && !isIncreasing)
            {

                // entering rising phase
                isIncreasing = true;
                nSamplesSinceLastPeak++;

            }
            else if (sample < lastSample && isIncreasing && nSamplesSinceLastPeak >= minSamplesToNextPeak)
            {

                numPeakIntervals++;


                //std::cout << "GOT EVENT." << std::endl;

                // entering falling phase (just reached peak or trough)
                //if (true)
                if (outputEventChannel > -1)
                    addEvent(events, TTL, i, 1, outputEventChannel);


                peakIntervals[numPeakIntervals % NUM_INTERVALS] = nSamplesSinceLastPeak;

                isIncreasing = false;

                nSamplesSinceLastPeak = 0;

                estimateFrequency();

            }
            else
            {

                // either rising or falling
                nSamplesSinceLastPeak++;

                if (nSamplesSinceLastPeak == 500 && outputEventChannel > -1)
                {
                    addEvent(events, TTL, i, 0, outputEventChannel);
                }

            }

            lastSample = sample;

        }
    }


}

void PhaseDetector::estimateFrequency()
{

    int N = (numPeakIntervals < NUM_INTERVALS) ? numPeakIntervals
            : NUM_INTERVALS;

    int sum = 0;

    for (int i = 0; i < N; i++)
    {
        sum += peakIntervals[i];
    }

    estimatedFrequency = getSampleRate()/(float(sum)/float(N));


}

