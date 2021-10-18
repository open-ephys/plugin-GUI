/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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
    BandpassFilterSettings() { }

    /** Holds the sample rate for this stream*/
    float sampleRate;

    /** Holds the filters for one stream*/
    OwnedArray<Dsp::Filter> filters;

    /** Creates new filters when input settings change*/
    void createFilters(int numChannels, float sampleRate, double lowCut, double highCut);

    /** Updates filters when parameters change*/
    void updateFilters(double lowCut, double highCut);

    /** Sets filter parameters for one channel*/
    void setFilterParameters(double lowCut, double highCut, int channel);

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
    void process(AudioSampleBuffer& buffer) override;

    /** Called whenever a parameter's value is changed (called by GenericProcessor::setParameter())*/
    void parameterValueChanged(Parameter* param) override;

    /** Called when upstream settings are changed.*/
    void updateSettings() override;

private:

    StreamSettings<BandpassFilterSettings> settings;

    void setFilterParameters (double, double, int);

    uint16 currentStream;
    int currentChannel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterNode);
};

#endif  // __FILTERNODE_H_CED428E__
