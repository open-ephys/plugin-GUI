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

#ifndef __ARDUINOOUTPUT_H_F7BDA585__
#define __ARDUINOOUTPUT_H_F7BDA585__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "Editors/ArduinoOutputEditor.h"
#include "Serial/ofArduino.h"
#include "GenericProcessor.h"


/** 

	*UNDER CONSTRUCTION*

	Provides a serial interface to an Arduino board.

	Based on Open Frameworks ofArduino class

	@see GenericProcessor

*/

class ArduinoOutput : public GenericProcessor
{
public:
	
	ArduinoOutput();
	~ArduinoOutput();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &events, int& nSamples);
	
	void setParameter (int parameterIndex, float newValue);

    void handleEvent(int eventType, MidiMessage& event);

    bool enable();
    bool disable();
    
	AudioProcessorEditor* createEditor();

	bool isSink() {return true;}
	
private:

	ofArduino arduino;

	bool state;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArduinoOutput);

};




#endif  // __ARDUINOOUTPUT_H_F7BDA585__
