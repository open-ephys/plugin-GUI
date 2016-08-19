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

#ifndef SWRDETECTOR_H_INCLUDED
#define SWRDETECTOR_H_INCLUDED

#ifdef _WIN32
#include <Windows.h>
#include <queue>
#endif

#include <ProcessorHeaders.h>

using namespace std;

/**

  Uses RMS calculations of long and short sliding windows to detect a sharp wave ripple.

  @see GenericProcessor, SWRDetectorEditor

*/

class SWRDetector : public GenericProcessor

{
public:

    SWRDetector();
    ~SWRDetector();


    bool hasEditor() const
    {
        return true;
    }

    bool enable();

    AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer, MidiBuffer& events);
    void setParameter(int parameterIndex, float newValue);

    void addModule();
    void setActiveModule(int);

private:

    const double longTimeWindow = 2.000;
    const double shortTimeWindow = 0.008;
    const double thresholdSWREventTime = 0.008;

    class SlidingWindow
    {
    public:
        void updateSumOfSquares(float sample, float sampleRate);
        void setTimeWindow(double time);
        float calculateRMS();

    private:
        queue<float> slidingWindow;
        double timeWindow;
        float sumOfSquares = 0;
        float RMSValue = 0;
    };


    struct DetectorModule
    {
        int inputChan;
        int gateChan;
        int outputChan;
        bool isActive;
        float lastSample;
        int samplesSinceTrigger;
        int shortGreaterThanLongCount;
        int numSWRDetected;
        bool wasTriggered;
        float thresholdComparisonConstant;
        float eventStimulationTime;
        SlidingWindow shortWindow;
        SlidingWindow longWindow;
    };

    Array<DetectorModule> modules;

    int activeModule;

    void handleEvent(int eventType, MidiMessage& event, int sampleNum);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SWRDetector);

};

#endif  // SWRDETECTOR_H_INCLUDED
