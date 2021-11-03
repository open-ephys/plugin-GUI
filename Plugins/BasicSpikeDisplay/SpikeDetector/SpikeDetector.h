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


class AbsValueThresholder : public Thresholder
{
public:
    AbsValueThresholder(int numChannels);
    virtual ~AbsValueThresholder() { }
    
    void setThreshold(int channel, float threshold);
    
    float getThreshold(int channel);
    
    Array<float>& getThresholds() {return thresholds;}
    
    bool checkSample(int channel, float sample);
    
private:
    
    Array<float> thresholds;
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
    ~SpikeDetector() { }

    /** Processes an incoming continuous buffer and places new spikes into the event buffer. */
    void process (AudioBuffer<float>& buffer) override;

    /** Called whenever the signal chain is altered. */
    void updateSettings() override;
    
    /** Parameter changed */
    void parameterValueChanged(Parameter* p) override;

    /** Called after acquisition is finished. */
    bool stopAcquisition() override;

    /** Creates the SpikeDetectorEditor. */
    AudioProcessorEditor* createEditor() override;

    /** Saves custom parameters */
    void saveCustomParametersToXml (XmlElement* parentElement)  override;
    
    /** Loads custom parameters*/
    void loadCustomParametersFromXml(XmlElement* xml)           override;


    // CREATE AND DELETE ELECTRODES
    // =====================================================================
    /** Adds a spike channel of a given type */
    void addSpikeChannel (const String& name, SpikeChannel::Type type, Array<const ContinuousChannel*> sourceChannels);

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

    float getDefaultThreshold() const;

    float getSample(int globalChannelIndex, int sampleIndex, AudioBuffer<float>& buffer);

    void addWaveformToSpikeBuffer (Spike::Buffer& s,
                                    int sampleIndex,
                                   AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetector);
};



#endif  // __SPIKEDETECTOR_H_3F920F95__
