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

#ifndef __BANDPASSFILTER_H_CED428E__
#define __BANDPASSFILTER_H_CED428E__

#include <ProcessorHeaders.h>
#include <DspLib.h>

#define CHANNELS_PER_THREAD 32

/** Holds settings for one stream's filters */

class BandpassFilterSettings
{
public:
    /** Constructor -- sets default values*/
    BandpassFilterSettings() {}

    /** Holds the sample rate for this stream*/
    float sampleRate;

    /** Holds the filters for one stream*/
    OwnedArray<Dsp::Filter> filters;

    /** Creates new filters when input settings change*/
    void createFilters (int numChannels, float sampleRate, double lowCut, double highCut);

    /** Updates filters when parameters change*/
    void updateFilters (double lowCut, double highCut);

    /** Sets filter parameters for one channel*/
    void setFilterParameters (double lowCut, double highCut, int channel);
};

/** Allows multi-threaded filtering */
class FilterJob : public ThreadPoolJob
{
public:
    /** Constructor */
    FilterJob (String name, Array<Dsp::Filter*> filters, Array<float*> channelPointer, int numSamples);

    /** Runs the job inside a thread */
    JobStatus runJob();

private:
    Array<Dsp::Filter*> filters;
    Array<float*> channelPointers;
    int numSamples;
    int numChannels;
};

/**
    Applies a Butterworth bandpass filter to the incoming data.

    The user can select the low- and high-frequency cutoffs,
    the channels to filter, and the number of threads to use.

    @see GenericProcessor, FilterEditor
*/
class TESTABLE BandpassFilter : public GenericProcessor
{
public:
    /** The class constructor, used to initialize any members. */
    BandpassFilter (bool headless = false);

    /** The class destructor, used to deallocate memory. */
    ~BandpassFilter() {}

    /** Registers the parameters for a given processor */
    void registerParameters() override;

    /** Creates the FilterEditor. */
    AudioProcessorEditor* createEditor() override;

    /** Filters incoming channels according to current parameters */
    void process (AudioBuffer<float>& buffer) override;

    /** Called whenever a parameter's value is changed (called by GenericProcessor::setParameter())*/
    void parameterValueChanged (Parameter* param) override;

    /** Called when upstream settings are changed.*/
    void updateSettings() override;

    /** Stops any ongoing worker threads */
    bool stopAcquisition() override;

private:
    StreamSettings<BandpassFilterSettings> settings;

    std::unique_ptr<ThreadPool> threadPool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandpassFilter);
};

#endif // __BANDPASSFILTER_H_CED428E__
