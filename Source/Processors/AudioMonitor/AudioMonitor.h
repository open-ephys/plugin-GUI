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


#ifndef __AUDIOMONITOR_H__
#define __AUDIOMONITOR_H__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../GenericProcessor/GenericProcessor.h"
#include "../Dsp/Dsp.h"

#define MAX_CHANNELS 4

/**
  Reads data from a file.

  @see GenericProcessor
*/
class AudioMonitor : public GenericProcessor
{
public:
    
    /** Constructor */
    AudioMonitor();
    
    /** Destructor*/
    ~AudioMonitor() { }

    /** Re-samples, filters, and copies selected channels*/
    void process (AudioBuffer<float>& buffer) override;
    
    /** Creates the custom UI for the AudioMonitor*/
    AudioProcessorEditor* createEditor() override;

    /** Specifies that two extra output channels should be added*/
    void updateSettings() override;

    /** Updates the audio buffer size*/
	void updatePlaybackBuffer();

    /** Updates the resampling ratio for each channel*/
    void prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock) override;
    
    /** Called whenever a parameter's value is changed (called by GenericProcessor::setParameter())*/
    void parameterValueChanged(Parameter* param) override;

     /** Resets the connections prior to a new round of data acquisition. */
    void resetConnections() override;

    /** Updates the bandpass filter parameters, given the currently monitored stream*/
    void updateFilter(int i, uint16 streamId);

private:
    
    /** Re-sets the copy buffers prior to acquisition*/
    void recreateBuffers();
    
    std::map<int, std::unique_ptr<AudioBuffer<float>>> bufferA;
    std::map<int, std::unique_ptr<AudioBuffer<float>>> bufferB;

    /** Per-channel buffer state information*/
    std::map<int, double> samplesInBackupBuffer;
    std::map<int, double> samplesInOverflowBuffer;
    std::map<int, double> sourceBufferSampleRate;
    std::map<int, bool> bufferSwap;
    
    std::map<int, double> numSamplesExpected;
    std::map<int, double> ratio;
    
    double destBufferSampleRate;
    double estimatedSamples;

    /** 4 bandpass filters (1 per selected channel)*/
    OwnedArray<Dsp::Filter> bandpassfilters;
    
    /** 4 antialiasing filters (1 per selected channel)*/
    OwnedArray<Dsp::Filter> antialiasingfilters;

    /** Holds the data for one channel, before it's copied to the output*/
    std::unique_ptr<AudioBuffer<float>> tempBuffer;
    
    /** Only one stream can be monitored at a time*/
    uint16 selectedStream;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMonitor);
};


#endif  // __AUDIOMONITOR_H__
