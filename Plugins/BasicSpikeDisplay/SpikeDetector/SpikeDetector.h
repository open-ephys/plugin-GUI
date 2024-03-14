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

class SpikeDetectorSettings
{
public:
    /** Constructor -- initializes values*/
    SpikeDetectorSettings();
    
    int nextAvailableChannel;
    
    int singleElectrodeCount;
    int stereotrodeCount;
    int tetrodeCount;

};

enum ThresholderType {
    ABS = 0,
    STD,
    DYN
};


/** 
    Thresholder based on signal absolute value.

    If a sample is below the threshold value,
    a spike will be triggered.

*/
class AbsValueThresholder : public Thresholder
{
public:

    /** Constructor */
    AbsValueThresholder(int numChannels);

    /** Destructor */
    virtual ~AbsValueThresholder() { }

    /** Checks whether a sample should trigger a spike*/
    bool checkSample(int channel, float sample);
    
    /** Sets the threshold for a given channel*/
    void setThreshold(int channel, float threshold);
    
    /** Gets the threshold for a given channel*/
    float getThreshold(int channel);
    
    /** Gets an array of thresholds for all channels*/
    Array<float>& getThresholds() {return thresholds;}
    
private:
    
    Array<float> thresholds;
};

/**
    Thresholder based on the standard deviation
    of an input signal.

    If a sample is below a multiple of the standard
    deviation, a spike will be triggered.

*/
class StdDevThresholder : public Thresholder
{
public:

    /** Constructor*/
    StdDevThresholder(int numChannels);

    /** Destructor */
    virtual ~StdDevThresholder() { }

    /** Checks whether a sample should trigger a spike*/
    bool checkSample(int channel, float sample);

    /** Sets the threshold for a given channel*/
    void setThreshold(int channel, float threshold);

    /** Gets the threshold for a given channel*/
    float getThreshold(int channel);

    /** Gets an array of thresholds for all channels*/
    Array<float>& getThresholds() { return thresholds; }

private:

    /** Computes the standard deviation of a given channel*/
    void computeStd(int channel);

    Array<float> thresholds;
    Array<float> stdLevels;
    Array<float> stds;
    OwnedArray<Array<float>> sampleBuffer;
    Array<int> bufferIndex;

    const int bufferSize = 4000;
    const int skipSamples = 50;

    int index;

};

/**
    Thresholder based on method from Quian Quiroga et al.
    https://pubmed.ncbi.nlm.nih.gov/15228749/

    Thr = 4 * s
    s = median{ |x| / 0.6745 }
*/
class DynamicThresholder : public Thresholder
{
public:

    /** Constructor */
    DynamicThresholder(int numChannels);

    /** Destructor */
    virtual ~DynamicThresholder() { }

    /** Checks whether a sample should trigger a spike*/
    bool checkSample(int channel, float sample);

    /** Sets the threshold for a given channel*/
    void setThreshold(int channel, float threshold);

    /** Gets the threshold for a given channel*/
    float getThreshold(int channel);

    /** Gets an array of thresholds for all channels*/
    Array<float>& getThresholds() { return thresholds; }

private:

    /** Computes sigma value used for dynamic thresholding*/
    void computeSigma(int channel);

    Array<float> thresholds;
    Array<float> sigmaLevels;
    Array<float> medians;
    OwnedArray< std::vector<float>> sampleBuffer;
    Array<int> bufferIndex;

    const int bufferSize = 4000;
    const int skipSamples = 50;

    const float scalar = 0.6745f;

    int index;
    
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
    void process (AudioBuffer<float>& buffer) override;

    /** Called whenever the signal chain is altered. */
    void updateSettings() override;
    
    /** Parameter changed */
    void parameterValueChanged(Parameter* p) override;

    /** Called when acquisition is started*/
    bool startAcquisition() override;

    /** Called after acquisition is finished. */
    bool stopAcquisition() override;

    /** Creates the SpikeDetectorEditor. */
    AudioProcessorEditor* createEditor() override;

    /** Saves spike channels to the settings file */
    void saveCustomParametersToXml (XmlElement* parentElement)  override;
    
    /** Loads spike channels from the settings file */
    void loadCustomParametersFromXml(XmlElement* xml)           override;

    /** Ensures that selected channel names are unique across all channels in a stream */
    String ensureUniqueName(String name, uint16 streamId);


    // CREATE AND DELETE ELECTRODES
    // =====================================================================
    /** Adds a spike channel of a given type */
    SpikeChannel* addSpikeChannel(SpikeChannel::Type type,
                          uint16 currentStream,
                          int startChannel = -1,
                          String name = "");

    /** Removes a spike channel, based on a SpikeChannel pointer. */
    void removeSpikeChannel (SpikeChannel*);
    // =====================================================================

    /** Get array of local SpikeChannel objects for a given dataStream*/
    Array<SpikeChannel*> getSpikeChannelsForStream(uint16 streamId);

private:

    // INTERNAL BUFFERS
    // =====================================================================
    /** Extra samples are placed in this buffer to allow seamless
    transitions between callbacks. */
    AudioBuffer<float> overflowBuffer;
    // =====================================================================

    /** Returns the sample value at a given index, taking into account 
        the overflow buffer */
    float getSample(int globalChannelIndex, int sampleIndex, AudioBuffer<float>& buffer);

    /** Adds a waveform (starting a given sample) to spike data buffer*/
    void addWaveformToSpikeBuffer (Spike::Buffer& s,
                                    int sampleIndex,
                                   AudioBuffer<float>& buffer);
    
    /** Checks whether a spike channel has been loaded, to prevent double-loading
        when there is a Merger in the signal chain */
    bool alreadyLoaded(String name, SpikeChannel::Type type, int stream_source, String stream_name);

    StreamSettings<SpikeDetectorSettings> settings;

    int totalCallbacks;
    int spikeCount;

    int nextAvailableChannel;
    int singleElectrodeCount;
    int stereotrodeCount;
    int tetrodeCount;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetector);
};



#endif  // __SPIKEDETECTOR_H_3F920F95__
