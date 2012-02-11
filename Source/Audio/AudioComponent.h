/*
  ==============================================================================

    AudioComponent.h
    Created: 7 May 2011 1:35:05pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __AUDIOCOMPONENT_H_D97C73CF__
#define __AUDIOCOMPONENT_H_D97C73CF__

#include "../../JuceLibraryCode/JuceHeader.h"

class AudioComponent : public AudioDeviceManager {

public:
	AudioComponent();
	~AudioComponent();

	void beginCallbacks();
	void endCallbacks();

	void connectToProcessorGraph(AudioProcessorGraph* processorGraph);
	void disconnectProcessorGraph();

	bool callbacksAreActive();

private:

	bool isPlaying;

	AudioProcessorPlayer* graphPlayer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioComponent);

};










#endif

