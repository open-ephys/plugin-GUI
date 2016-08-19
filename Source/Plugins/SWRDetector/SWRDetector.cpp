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
#include <queue>
#include "SWRDetector.h"
#include "SWRDetectorEditor.h"


using namespace std;

SWRDetector::SWRDetector()
    : GenericProcessor("SWR Detector")

{

}

SWRDetector::~SWRDetector()
{

}

AudioProcessorEditor* SWRDetector::createEditor()
{
    editor = new SWRDetectorEditor(this, true);

    std::cout << "Creating editor." << std::endl;

    return editor;
}

void SWRDetector::addModule()
{
    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.outputChan = -1;
    m.gateChan = -1;
    m.isActive = true;
    m.lastSample = 0.0f;
    m.samplesSinceTrigger = 5000;
    m.wasTriggered = false;
    m.numSWRDetected = 0;
    m.shortGreaterThanLongCount = 0;
    m.thresholdComparisonConstant = 3.0;
    m.eventStimulationTime = 0.05;

    modules.add(m);
}

void SWRDetector::setActiveModule(int i)
{
    activeModule = i;

}


void SWRDetector::setParameter(int parameterIndex, float newValue)
{

    DetectorModule& module = modules.getReference(activeModule);

    if (parameterIndex == 2)   // inputChan
    {
        module.inputChan = (int)newValue;
    }
    else if (parameterIndex == 3)   // outputChan
    {
        module.outputChan = (int)newValue;
    }
    else if (parameterIndex == 4)   // gateChan
    {
        module.gateChan = (int)newValue;
        if (module.gateChan < 0)
        {
            module.isActive = true;
        }
        else
        {
            module.isActive = false;
        }
    }
    else if (parameterIndex == 5) // RMS threshold comparison constant
    {
        module.thresholdComparisonConstant = newValue;
    }
    else if (parameterIndex == 6) // event stimulation time
    {
        module.eventStimulationTime = newValue;
    }

}

bool SWRDetector::enable()
{
    return true;
}

void SWRDetector::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{

    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        // int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr + 2);
        int eventChannel = *(dataptr + 3);

        for (int i = 0; i < modules.size(); i++)
        {
            DetectorModule& module = modules.getReference(i);

            if (module.gateChan == eventChannel)
            {
                if (eventId)
                    module.isActive = true;
                else
                    module.isActive = false;
            }
        }

    }

}

void SWRDetector::SlidingWindow::setTimeWindow(double time)
{
    timeWindow = time;
}

void SWRDetector::SlidingWindow::updateSumOfSquares(float sample, float sampleRate)
{
    if (slidingWindow.size() >= (sampleRate * timeWindow))
    {
        sumOfSquares -= pow(slidingWindow.front(), 2);
        slidingWindow.pop();
    }

    sumOfSquares += pow(sample, 2);
    slidingWindow.push(sample);
}

float SWRDetector::SlidingWindow::calculateRMS()
{
    return sqrt(sumOfSquares / (slidingWindow.size()));
}



void SWRDetector::process(AudioSampleBuffer& buffer,
                          MidiBuffer& events)
{
    checkForEvents(events);

    // loop through the modules
    for (int i = 0; i < modules.size(); i++)
    {
        DetectorModule& module = modules.getReference(i);

        module.longWindow.setTimeWindow(longTimeWindow);
        module.shortWindow.setTimeWindow(shortTimeWindow);

        // check to see if it's active and has a channel
        if (module.isActive && module.outputChan >= 0 &&
                module.inputChan >= 0 &&
                module.inputChan < buffer.getNumChannels())
        {
            for (int i = 0; i < getNumSamples(module.inputChan); i++)
            {
                const float sample = *buffer.getReadPointer(module.inputChan, i);

                module.longWindow.updateSumOfSquares(sample, getSampleRate());
                module.shortWindow.updateSumOfSquares(sample, getSampleRate());

                if (module.shortWindow.calculateRMS() > (module.thresholdComparisonConstant * module.longWindow.calculateRMS()))
                {
                    module.shortGreaterThanLongCount++;
                }
                else
                {
                    module.shortGreaterThanLongCount = 0;
                }

                if (module.shortGreaterThanLongCount >= static_cast<int>(shortTimeWindow * getSampleRate()))
                {
                    module.numSWRDetected++;
                    module.shortGreaterThanLongCount = 0;
                    addEvent(events, TTL, i, 1, module.outputChan);
                    module.samplesSinceTrigger = 0;
                    module.wasTriggered = true;
                }

                module.lastSample = sample;

                if (module.wasTriggered)
                {
                    if (module.samplesSinceTrigger >= (module.eventStimulationTime * getSampleRate()))
                    {
                        addEvent(events, TTL, i, 0, module.outputChan);
                        module.wasTriggered = false;
                    }
                    else
                    {
                        module.samplesSinceTrigger++;
                    }
                }

            }


        }

    }

}



