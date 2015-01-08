/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __EXAMPLEPROCESSOR_H_91811542__
#define __EXAMPLEPROCESSOR_H_91811542__

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"

/**

  This class serves as a template for creating new processors.

  If this were a real processor, this comment section would be used to
  describe the processor's function.

  @see GenericProcessor

*/

class ExampleProcessor : public GenericProcessor

{
public:

    /** The class constructor, used to initialize any members. */
    ExampleProcessor();

    /** The class destructor, used to deallocate memory */
    ~ExampleProcessor();

    /** Determines whether the processor is treated as a source. */
    bool isSource()
    {
        return false;
    }

    /** Determines whether the processor is treated as a sink. */
    bool isSink()
    {
        return false;
    }

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Processors can either use this method to add new data, manipulate existing
        data, or send data to an external target (such as a display or other hardware).

        Continuous signals arrive in the "buffer" variable, event data (such as TTLs
        and spikes) is contained in the "events" variable, and "nSamples" holds the
        number of continous samples in the current buffer (which may differ from the
        size of the buffer).
         */
    void process(AudioSampleBuffer& buffer, MidiBuffer& events);

    /** Any variables used by the "process" function _must_ be modified only through
        this method while data acquisition is active. If they are modified in any
        other way, the application will crash.  */
    void setParameter(int parameterIndex, float newValue);

private:

    // private members and methods go here
    //
    // e.g.:
    //
    // float threshold;
    // bool state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExampleProcessor);

};

#endif  // __EXAMPLEPROCESSOR_H_91811542__
