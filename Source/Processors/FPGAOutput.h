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

#ifndef __FPGAOUTPUT_H_33275017__
#define __FPGAOUTPUT_H_33275017__

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"
#include "Editors/FPGAOutputEditor.h"
#include "DataThreads/FPGAThread.h"


/**

  Allows the signal chain to send outputs to the Open Ephys acquisition board.

  @see GenericProcessor, FPGAOutputEditor

*/


class FPGAOutput : public GenericProcessor,
		           public Timer
                   //public ActionBroadcaster

{
public:
	
	FPGAOutput();
	~FPGAOutput();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

    void handleEvent(int eventType, MidiMessage& event, int sampleNum);
    
	AudioProcessorEditor* createEditor();

	bool isSink() {return true;}
    
     void updateSettings();
	
private:

    int TTLchannel;

	void timerCallback();
    
    bool isEnabled;

    bool continuousStim;
    
    FPGAThread* dataThread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FPGAOutput);
    
    

};


#endif  // __FPGAOUTPUT_H_33275017__
