/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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
#include "GenericProcessor.h"

/**

  This is a description of the processor's function.

  @see GenericProcessor

*/

class ExampleProcessor : public GenericProcessor

{
public:
	
	ExampleProcessor();
	~ExampleProcessor();

    bool isSource() {return false;}
    bool isSink() {return false;}
	
	void process (AudioSampleBuffer &buffer, MidiBuffer &events, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

private:

    // private variables and methods go here
    // 
    // e.g.:
    //
    // float threshold;
    // bool state;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExampleProcessor);

};

#endif  // __EXAMPLEPROCESSOR_H_91811542__
