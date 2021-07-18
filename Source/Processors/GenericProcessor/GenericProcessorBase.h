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

#ifndef __GENERICPROCESSORBASE_H_1F469DAF__
#define __GENERICPROCESSORBASE_H_1F469DAF__

#include <JuceHeader.h>

#include "../PluginManager/PluginClass.h"


/**
    Base class for GenericProcessor - implements some required Juce Methods
*/
class PLUGIN_API GenericProcessorBase : public AudioProcessor
{
public:
    /** Constructor (sets the processor's name). */
    GenericProcessorBase (const String& name_);

    /** Destructor. */
    virtual ~GenericProcessorBase();

    /** Returns the name of the processor. */
    const String getName() const override;

    /** Called by JUCE as soon as a processor is created, as well as before the start of audio callbacks.
        To avoid starting data acquisition prematurely, use the enable() function instead. */
    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;

    /** Called by JUCE as soon as audio callbacks end. Use disable() instead. */
    void releaseResources() override;

    /** All processors are expected to have editors by default.*/
    virtual bool hasEditor() const override;

    /** JUCE method. Not used.*/
    void reset() override;

    /** JUCE method. Not used.*/
    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override;

    /** JUCE method. Not used.*/
    void setStateInformation (const void* data, int sizeInBytes) override;

    /** JUCE method. Not used.*/
    void getCurrentProgramStateInformation (MemoryBlock& destData) override;

    /** JUCE method. Not used.*/
    void getStateInformation (MemoryBlock& destData) override;

    /** JUCE method. Not used.*/
    void changeProgramName (int index, const String& newName) override;

    /** JUCE method. Not used.*/
    void setCurrentProgram (int index) override;

    /** Returns the current active channel. */
    int getCurrentChannel() const;

    /** Returns the name of the parameter with a given index.*/
    const String getParameterName (int parameterIndex) override;

    /** Returns additional details about the parameter with a given index.*/
    const String getParameterText (int parameterIndex) override;

    /** Returns the current value of a parameter with a given index.
     Currently set to always return 1. See getParameterVar below*/
    float getParameter (int parameterIndex) override;

    /** JUCE method. Not used.*/
    const String getProgramName (int index) override;

    /** All processors can accept MIDI (event) data by default.*/
    bool acceptsMidi() const override;

    /** All processors can produce MIDI (event) data by default.*/
    bool producesMidi() const override;

    /** JUCE method. Not used.*/
    bool isParameterAutomatable (int parameterIndex) const override;

    /** JUCE method. Not used.*/
    bool isMetaParameter (int parameterIndex) const override;

    /** Returns the number of user-editable parameters for this processor.*/
    int getNumParameters() override;

    /** JUCE method. Not used.*/
    int getNumPrograms() override;

    /** JUCE method. Not used.*/
    int getCurrentProgram() override;

    /** JUCE method. Not used.*/
    double getTailLengthSeconds() const override;

    /** JUCE method. Not used.*/
    bool isInputChannelStereoPair (int index) const;

    /** JUCE method. Not used.*/
    bool isOutputChannelStereoPair (int index) const;

    /** JUCE method. Not used.*/
    bool silenceInProducesSilenceOut() const;

private:

    const String m_name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessorBase);
};


#endif  // __GENERICPROCESSORBASE_H_1F469DAF__
