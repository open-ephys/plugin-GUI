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

#ifndef CAR_H_INCLUDED
#define CAR_H_INCLUDED

#include <ProcessorHeaders.h>

/** Holds settings for one stream's CAR*/

class CARSettings
{

public: 

    /** Constructor -- sets default values*/
    CARSettings();

    /** Destructor */
    ~CARSettings() {}

    /** Buffer to hold average */
    AudioSampleBuffer m_avgBuffer;

};


/**
    This is a simple filter that subtracts the average of a subset of channels from 
    another subset of channels. The gain parameter allows you to subtract a percentage of the total avg.

    See Ludwig et al. 2009 Using a common average reference to improve cortical
    neuron recordings from microelectrode arrays. J. Neurophys, 2009 for a detailed
    discussion
*/
class CommonAverageRef : public GenericProcessor
{
public:
    /** The class constructor, used to initialize any members. */
    CommonAverageRef();

    /** The class destructor, used to deallocate memory. */
    ~CommonAverageRef();

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Processors can either use this method to add new data, manipulate existing
        data, or send data to an external target (such as a display or other hardware).

        Continuous signals arrive in the "buffer" variable, event data (such as TTLs
        and spikes) is contained in the "events" variable, and "nSamples" holds the
        number of continous samples in the current buffer (which may differ from the
        size of the buffer).
    */
    void process (AudioBuffer<float>& buffer) override;

    /** Called when upstream settings are changed.*/
    void updateSettings() override;

    /** Returns the current gain level that is set in the processor */
    float getGainLevel(uint16 streamId);

    /** Sets the new gain level that will be used in the processor */
    void setGainLevel (uint16 streamId, float newGain);

    /** Creates the CAREditor. */
    AudioProcessorEditor* createEditor() override;

private:

    StreamSettings<CARSettings> settings;

    // ==================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CommonAverageRef);
};



#endif  // CAR_H_INCLUDED
