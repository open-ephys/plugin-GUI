/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "../Dsp/Dsp.h"
#include "../GenericProcessor/GenericProcessor.h"

/**
  Creates and holds the filter settings for one input stream in the AudioMonitor.
*/
class AudioMonitorSettings
{
public:
    AudioMonitorSettings();

    int numChannels;
    float sampleRate;
    double destBufferSampleRate;
    double estimatedSamples;

    /** Bandpass filters (1 per channel)*/
    OwnedArray<Dsp::Filter> bandpassfilters;

    /** Antialiasing filters (1 per channel)*/
    OwnedArray<Dsp::Filter> antialiasingfilters;
    std::map<int, std::unique_ptr<AudioBuffer<float>>> bufferA;
    std::map<int, std::unique_ptr<AudioBuffer<float>>> bufferB;

    /** Per-channel buffer state information*/
    std::map<int, double> samplesInBackupBuffer;
    std::map<int, double> samplesInOverflowBuffer;
    std::map<int, bool> bufferSwap;
    double numSamplesExpected;
    double ratio;

    /** Creates new filters when input settings change*/
    void createFilters (int numChannels, float sampleRate);

    /** Updates filters when parameters change*/
    void updateAntiAliasingFilterParameters();

    /** Re-sets the copy buffers prior to starting acquisition*/
    void setOutputSampleRate (double outputSampleRate, double estimatedSamplesPerBlock);
};

/**
  Streams data from incoming continuous channels to the computer’s audio output.

  @see GenericProcessor
*/
class AudioMonitor : public GenericProcessor
{
public:
    /** Constructor */
    AudioMonitor();

    /** Destructor*/
    ~AudioMonitor() {}

    /** Add and register parameters*/
    void registerParameters() override;

    /** Re-samples, filters, and copies selected channels*/
    void process (AudioBuffer<float>& buffer) override;

    /** Creates the custom UI for the AudioMonitor*/
    AudioProcessorEditor* createEditor() override;

    /** Specifies that two extra output channels should be added*/
    void updateSettings() override;

    /** Updates the audio buffer size*/
    void updatePlaybackBuffer();

    /** Updates the resampling ratio for each channel*/
    void prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock) override;

    /** Called whenever a parameter's value is changed (called by GenericProcessor::setParameter())*/
    void parameterValueChanged (Parameter* param) override;

    /** Resets the connections prior to a new round of data acquisition. */
    void resetConnections() override;

    /** Allows other processors to configure the Audio Monitor during acquisition*/
    void handleBroadcastMessage (const String& message, const int64 messageTimeMillis) override;

    /** Sets the selected stream and updates the filter parameters*/
    void setSelectedStream (uint16 streamId);

private:
    /** AudioMonitor settings for each input stream*/
    StreamSettings<AudioMonitorSettings> settings;

    /** Holds the data for one channel, before it's copied to the output*/
    std::unique_ptr<AudioBuffer<float>> tempBuffer;

    /** Only one stream can be monitored at a time*/
    uint16 selectedStream;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioMonitor);
};

#endif // __AUDIOMONITOR_H__
