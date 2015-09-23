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

#ifndef __SIGNALGENERATOR_H_EAA44B0B__
#define __SIGNALGENERATOR_H_EAA44B0B__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "SignalGeneratorEditor.h"

/**

  Outputs synthesized data of one of 5 different waveform types.

  @see GenericProcessor, SignalGeneratorEditor

*/

class SignalGenerator : public GenericProcessor

{
public:

    SignalGenerator();
    ~SignalGenerator();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    void setParameter(int parameterIndex, float newValue);

    float getSampleRate()
    {
        return 44100.0;
    }

    float getDefaultBitVolts()
    {
        return 0.03;
    }

    AudioProcessorEditor* createEditor();
    bool hasEditor() const
    {
        return true;
    }

    bool enable();
    bool disable();

    bool isSource()
    {
        return true;
    }

    void updateSettings();

    int getDefaultNumOutputs()
    {
        return nOut;
    }

    int nOut;


private:

    double defaultFrequency;
    double defaultAmplitude;

    float generateSpikeSample(double amp, double phase, double noise);

    float sampleRateRatio;

    //void updateWaveform(int chan);

    void initializeParameters();

    enum wvfrm
    {
        TRIANGLE, SINE, SQUARE, SAW, NOISE, SPIKE
    };

    Array<var> waveformParameter;
    Array<int> waveformType;
    Array<double> frequency;
    Array<double> amplitude;
    Array<double> phase;
    Array<double> phasePerSample;
    Array<double> currentPhase;

    double previousPhase;
    int spikeIdx;
    int spikeDelay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalGenerator);

};





#endif  // __SIGNALGENERATOR_H_EAA44B0B__
