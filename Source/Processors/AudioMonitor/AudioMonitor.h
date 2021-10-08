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
    AudioMonitor();
    ~AudioMonitor();

    void process (AudioBuffer<float>& buffer) override;
    
    void setParameter (int parameterIndex, float newValue) override;

    AudioProcessorEditor* createEditor() override;

    bool hasEditor() const  override { return true; }

    void updateSettings() override;

    /** Updates the audio buffer size*/
	void updatePlaybackBuffer();

    void prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock) override;

     /** Resets the connections prior to a new round of data acquisition. */
    void resetConnections() override;

    bool startAcquisition() override;

    void updateFilter(int i);
    
    void setMonitoredChannels(Array<int> activeChannels);
    Array<int> getMonitoredChannels();

    std::vector<bool> dataChannelStates;

private:
    void recreateBuffers();
    
    Array<int> activeChannels;

    std::map<int, std::unique_ptr<AudioBuffer<float>>> bufferA;
    std::map<int, std::unique_ptr<AudioBuffer<float>>> bufferB;

    std::map<int, int> numSamplesExpected;

    std::map<int, int> samplesInBackupBuffer;
    std::map<int, int> samplesInOverflowBuffer;
    std::map<int, double> sourceBufferSampleRate;
    double destBufferSampleRate;
	int estimatedSamples;

    bool isMuted;

    enum AudioOutputType {LEFT = 1, BOTH, RIGHT};

    AudioOutputType audioOutput; 

    std::map<int, bool> bufferSwap;

    // sample rate, timebase, and ratio info:
    std::map<int, double> ratio;

    // major objects:
    OwnedArray<Dsp::Filter> filters;

    std::unique_ptr<AudioBuffer<float>> tempBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMonitor);
};


#endif  // __AUDIOMONITOR_H__
