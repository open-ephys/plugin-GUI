/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __SOURCENODE_H_DCE798F1__
#define __SOURCENODE_H_DCE798F1__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"
#include <ftdi.h>
#include <stdio.h>
#include "DataThreads/DataThread.h"
#include "GenericProcessor.h"
#include "../UI/UIComponent.h"

/**

  Creates and controls a thread for reading data from external sources.

  @see GenericProcessor, SourceNodeEditor, DataThread, IntanThread

*/

class SourceNode : public GenericProcessor,
				   public Timer,
                   public ActionListener

{
public:

	// real member functions:
	SourceNode(const String& name);
	~SourceNode();

	void enabledState(bool t);

	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	float getSampleRate();
	float getDefaultSampleRate();
	int getDefaultNumOutputs();
	float getDefaultBitVolts();

	AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;}

	bool enable();
	bool disable();

	bool isReady();

	bool isSource() {return true;}

	void acquisitionStopped();

    DataThread* getThread();

    void actionListenerCallback(const String& message);

    int getTTLState();

private:

	int numEventChannels;

	int sourceCheckInterval;

	bool wasDisabled;

	void timerCallback();

	ScopedPointer<DataThread> dataThread;
	DataBuffer* inputBuffer;

	uint64 timestamp;
	int16* eventCodeBuffer;
	int* eventChannelState;


    int ttlState;

	void updateSettings();
	bool tryEnablingEditor();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNode);

};


#endif  // __SOURCENODE_H_DCE798F1__
