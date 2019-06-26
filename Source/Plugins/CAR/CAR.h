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


#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>

/**
    This is a simple filter that subtracts the average of all other channels from 
    each channel. The gain parameter allows you to subtract a percentage of the total avg.

    See Ludwig et al. 2009 Using a common average reference to improve cortical
    neuron recordings from microelectrode arrays. J. Neurophys, 2009 for a detailed
    discussion
*/
class CAR : public GenericProcessor
{
public:
    /** The class constructor, used to initialize any members. */
    CAR();

    /** The class destructor, used to deallocate memory */
    ~CAR();

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

    /** Returns the current gain level that is set in the processor */
    float getGainLevel();

    /** Sets the new gain level that will be used in the processor */
    void setGainLevel (float newGain);

    /** Creates the CAREditor. */
    AudioProcessorEditor* createEditor() override;

    Array<int> getReferenceChannels() const     { return m_referenceChannels; }
    Array<int> getAffectedChannels()  const     { return m_affectedChannels; }

    void setReferenceChannels (const Array<int>& newReferenceChannels);
    void setAffectedChannels  (const Array<int>& newAffectedChannels);

    void setReferenceChannelState (int channel, bool newState);
    void setAffectedChannelState  (int channel, bool newState);

    /** Saving/loading channel parameters */
    void saveCustomChannelParametersToXml(XmlElement* channelElement,
        int channelNumber, InfoObjectCommon::InfoObjectType channelType);
    void loadCustomChannelParametersFromXml(XmlElement* channelElement,
        InfoObjectCommon::InfoObjectType channelType);

private:
    LinearSmoothedValueAtomic<float> m_gainLevel;

    AudioSampleBuffer m_avgBuffer;

    /** We should add this for safety to prevent any app crashes or invalid data processing.
        Since we use m_referenceChannels and m_affectedChannels arrays in the process() function,
        which works in audioThread, we may stumble upon the situation when we start changing
        either reference or affected channels by copying array and in the middle of copying process
        we will be interrupted by audioThread. So it most probably will lead to app crash or
        processing incorrect channels.
    */
    CriticalSection objectLock;

    /** Array of channels which will be used to calculate mean signal. */
    Array<int> m_referenceChannels;

    /** Array of channels that will be affected by adding/substracting of mean signal of reference channels */
    Array<int> m_affectedChannels;

    // ==================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAR);
};



#endif  // CAR_H_INCLUDED
