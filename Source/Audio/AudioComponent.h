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

#ifndef __AUDIOCOMPONENT_H_D97C73CF__
#define __AUDIOCOMPONENT_H_D97C73CF__

#include "../../JuceLibraryCode/JuceHeader.h"

/**
  
  Interfaces with system audio hardware.

  Uses the audio card to generate the callbacks to run the ProcessorGraph
  during data acquisition.

  Sends output to the audio card for audio monitoring.

  Determines the initial size of the sample buffer (crucial for
  real-time feedback latency).

  @see MainWindow, ProcessorGraph

*/

class AudioComponent {

public:
	AudioComponent();
	~AudioComponent();

	void beginCallbacks();
	void endCallbacks();

	void connectToProcessorGraph(AudioProcessorGraph* processorGraph);
	void disconnectProcessorGraph();

	bool callbacksAreActive();

  AudioDeviceManager deviceManager;

private:

	bool isPlaying;

	AudioProcessorPlayer* graphPlayer;

  

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioComponent);

};










#endif

