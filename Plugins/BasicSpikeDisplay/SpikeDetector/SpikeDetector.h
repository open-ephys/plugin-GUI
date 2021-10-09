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

#ifndef __SPIKEDETECTOR_H_3F920F95__
#define __SPIKEDETECTOR_H_3F920F95__

#include <ProcessorHeaders.h>

/** Holds the local settings for one Spike Channel*/
class SpikeChannelSettings
{
public:

    enum ThresholdType
    {
        FIXED = 1,
        STD,
        DYNAMIC,
        UNKNOWN = 100
    };

    /** Constructor -- sets default values*/
    SpikeChannelSettings(const SpikeChannel::Type type);

    /** Destructor */
    ~SpikeChannelSettings() {}

    /** Updates channel arrays*/
    void setChannelIndexes(Array<int> localChannelIndexes,
        Array<int> globalChannelIndexes,
        int maxLocalChannel);

    /** Channel name (editable)*/
    String name;

    /** Channel description (editable)*/
    String description;

    /** Channel type (can't be changed) */
    const SpikeChannel::Type type;

    /** Index of each channel within a stream */
    Array<int> localChannelIndexes;

    /** Index of each channel within the processor */
    Array<int> globalChannelIndexes;

    /** Determines whether a particular channel is used for spike detection*/
    Array<bool> detectSpikesOnChannel;

    /** Determines the threshold type */
    ThresholdType thresholdType;

    /** Holds the thresholds for each channel*/
    Array<float> thresholds;

    /** Determines whether the channel sends the full waveform*/
    bool sendFullWaveform;

    /** Number of pre-peak samples if full waveform is sent*/
    unsigned int prePeakSamples;

    /** Number of post-peak samples if full waveform is sent*/
    unsigned int postPeakSamples;

    /** Used to determine channels available for selection*/
    int maxLocalChannel;

    /** Total number of channels for this electrode type*/
    const int expectedChannelCount;

    /** Holds the current sample index for this electrode*/
    int currentSampleIndex;

    /** Determines whether this electrode should use the overflow buffer*/
    bool useOverflowBuffer;

    /** Pointer to the SpikeChannel object for this electrode*/
    SpikeChannel* spikeChannel;

    /** Restores sampleIndex / overflow buffer settings after ending acquisition*/
    void reset();

    /** Saves parameters to XML*/
    void toXml(XmlElement*);

    /** Loads parameters from XML*/
    void fromXml(XmlElement*);


};

/** Holds the settings for one stream*/
class SpikeDetectorSettings
{
public:

    /** Constructor*/
    SpikeDetectorSettings();

    /** Destructor*/
    ~SpikeDetectorSettings();

    /** Saves parameters to XML*/
    void toXml(XmlElement*);

    /** Loads parameters from XML*/
    void fromXml(XmlElement*);

    /** Array of settings objects, one for each spike channel in the stream.*/
    OwnedArray<SpikeChannelSettings> spikeChannels;

    /** Holds the next available channel index in this stream*/
    int nextAvailableChannel;

    /** Holds the next available electrode index in this stream*/
    int nextElectrodeIndex;
};



/**
    Detects spikes in a continuous signal and outputs events containing the spike data.

    @see GenericProcessor, SpikeDetectorEditor
*/
class SpikeDetector : public GenericProcessor
{
public:

    /** Constructor*/
    SpikeDetector();

    /** Destructor*/
    ~SpikeDetector();

    /** Processes an incoming continuous buffer and places new spikes into the event buffer. */
    void process (AudioSampleBuffer& buffer) override;

    /** Used to alter parameters during data acquisition. */
    void setParameter (int parameterIndex, float newValue) override;

    /** Called whenever the signal chain is altered. */
    void updateSettings() override;

    /** Called prior to start of acquisition. */
    bool startAcquisition() override;

    /** Called after acquisition is finished. */
    bool stopAcquisition() override;

    /** Creates the SpikeDetectorEditor. */
    AudioProcessorEditor* createEditor() override;

    void saveCustomParametersToXml (XmlElement* parentElement)  override;
    void loadCustomParametersFromXml()                          override;


    // CREATE AND DELETE ELECTRODES
    // =====================================================================
    /** Adds a spike channel of a given type; returns true if successful. */
    bool addSpikeChannel (SpikeChannel::Type type, uint16 streamId);

    /** Removes a spike channel with a given index. */
    bool removeSpikeChannel (int index, uint16 streamId);
    // =====================================================================

    /** Get channel array, for editing electrode settings*/
    Array<SpikeChannelSettings*> getSpikeChannelsForStream(uint16 streamId);

private:

    StreamSettings<SpikeDetectorSettings> settings;


    // INTERNAL BUFFERS
    // =====================================================================
    /** Extra samples are placed in this buffer to allow seamless
    transitions between callbacks. */
    AudioSampleBuffer overflowBuffer;
    // =====================================================================

    float getDefaultThreshold() const;

    float getSample(int& globalChannelIndex, int& sampleIndex, AudioBuffer<float>& buffer);

    //float getNextSample (int& chan);
    //float getCurrentSample (int& chan);
    //bool samplesAvailable (int nSamples);

    void addWaveformToSpikeObject (Spike::Buffer& s,
                                   int& peakIndex,
                                   int& electrodeNumber,
                                   int& currentChannel);

    uint16 currentStream;
    int currentElectrode;
    int currentChannelIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetector);
};



#endif  // __SPIKEDETECTOR_H_3F920F95__
