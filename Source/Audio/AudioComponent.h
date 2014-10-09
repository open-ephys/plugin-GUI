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

class AudioComponent
{

public:
    /** Constructor. Finds the audio component (if there is one), and sets the
    default sample rate and buffer size.*/
    AudioComponent();
    ~AudioComponent();

    /** Begins the audio callbacks that drive data acquisition.*/
    void beginCallbacks();

    /** Stops the audio callbacks that drive data acquisition.*/
    void endCallbacks();

    /** Connects the AudioComponent to the ProcessorGraph (crucial for any sort of
    data acquisition; done at startup).*/
    void connectToProcessorGraph(AudioProcessorGraph* processorGraph);

    /** Disconnects the AudioComponent to the ProcessorGraph (only done when the application
    is about to close).*/
    void disconnectProcessorGraph();

    /** Returns true if the audio callbacks are active, false otherwise.*/
    bool callbacksAreActive();

    /** Restarts communication with the audio device in order to update settings
    or just prior the start of data acquisition callbacks.*/
    void restartDevice();

    /** Stops communication with the selected audio device (to conserve CPU load
    when callbacks are not active).*/
    void stopDevice();

    /** Returns the buffer size (in samples) currently being used.*/
    int getBufferSize();

    /** Returns the buffer size (in ms) currently being used.*/
    int getBufferSizeMs();

    /** Sets the buffer size in samples.*/
    void setBufferSize(int);

    AudioDeviceManager deviceManager;

private:

    bool isPlaying;

    ScopedPointer<AudioProcessorPlayer> graphPlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioComponent);

};










#endif

