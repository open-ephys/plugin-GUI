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

#ifndef __FILTERNODE_H_CED428E__
#define __FILTERNODE_H_CED428E__

#include <ProcessorHeaders.h>

#include <DspLib.h>


/** Holds settings for one stream's filters*/

class BandpassFilterSettings
{

public:

    /** Constructor -- sets default values*/
    BandpassFilterSettings();

    /** Destructor */
    ~BandpassFilterSettings() {}

    /** High filter cutoff */
    double highCut;

    /** Low filter cutoff */
    double lowCut;

    /** Array of channels which will not be filtered. */
    Array<bool> channelMask;

    /** Determines whether this stream's filter is enabled.*/
    bool isEnabled;

    /** Holds the sample rate for this stream*/
    float sampleRate;

    /** Holds the filters for one stream*/
    OwnedArray<Dsp::Filter> filters;

    /** Creates new filters when input settings change*/
    void createFilters(int numChannels, float sampleRate);

    /** Updates filters when parameters change*/
    void updateFilters();

    /** Sets filter parameters for one channel*/
    void setFilterParameters(double lowCut, double highCut, int channel);

    /** Saves parameters to XML*/
    void toXml(XmlElement*);

    /** Loads parameters from XML*/
    void fromXml(XmlElement*);

    /** Converts mask channels to string*/
    String channelMaskToString(Array<bool> channelMask);

    /** Converts string to mask channels*/
    void channelMaskFromString(String);

};

/**
    Filters data using a filter from the DSP library.

    The user can select the low- and high-frequency cutoffs.

    @see GenericProcessor, FilterEditor
*/
class FilterNode : public GenericProcessor
{
public:

    /** The class constructor, used to initialize any members. */
    FilterNode();

    /** The class destructor, used to deallocate memory. */
    ~FilterNode();

    /** Creates the FilterEditor. */
    AudioProcessorEditor* createEditor() override;

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Processors can either use this method to add new data, manipulate existing
        data, or send data to an external target (such as a display or other hardware).

        Continuous signals arrive in the "buffer" variable, event data (such as TTLs
        and spikes) is contained in the "events" variable, and "nSamples" holds the
        number of continous samples in the current buffer (which may differ from the
        size of the buffer).
    */
    void process (AudioSampleBuffer& buffer) override;

    /** Used to update parameters while the process() loop is running*/
    void setParameter (int parameterIndex, float newValue) override;

    /** Called whenever a parameter value is changed*/
    void parameterValueChanged(Parameter* param) override;

    /** Called when upstream settings are changed.*/
    void updateSettings() override;

    /** Returns the low cutoff for a particular stream*/
    double getLowCutValue  (uint16 streamId) ;

    /** Sets the low cutoff for a particular stream*/
    void setLowCutValue(uint16 streamId, double value);

    /** Returns the high cutoff for a particular stream*/
    double getHighCutValue(uint16 streamId) ;

    /** Sets the high cutoff for a particular stream*/
    void setHighCutValue(uint16 streamId, double value);

    /** Sets whether or not the filters for a particular stream are enabled*/
    void setEnabledState(uint16 streamId, bool isEnabled);

    /** Sets whether or not the filters for a particular stream are enabled*/
    bool getEnabledState(uint16 streamId);

    /** Sets whether or not the filters for a particular stream are enabled*/
    void setChannelMask(uint16 streamId, Array<int> channels);

    /** Sets whether or not the filters for a particular stream are enabled*/
    Array<bool> getChannelMask(uint16 streamId);

    /** Saving custom parameters*/
    void saveCustomParametersToXml(XmlElement* xml) override;

    /** Loading custom parameters*/
    void loadCustomParametersFromXml() override;


private:

    StreamSettings<BandpassFilterSettings> settings;

    void setFilterParameters (double, double, int);

    uint16 currentStream;
    int currentChannel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterNode);
};

#endif  // __FILTERNODE_H_CED428E__
