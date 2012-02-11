/*
  ==============================================================================

    AudioEditor.h
    Created: 31 Jul 2011 10:28:36pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __AUDIOEDITOR_H_9D6F1FC3__
#define __AUDIOEDITOR_H_9D6F1FC3__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../AudioNode.h"
#include <stdio.h>

class AudioNode;

class MuteButton : public DrawableButton
{
	public:
		MuteButton();
		~MuteButton();	
};

class AudioEditor : public AudioProcessorEditor,
					public Button::Listener

{
public:
	AudioEditor (AudioNode* owner);
	~AudioEditor();

	void paint (Graphics& g);

	bool keyPressed (const KeyPress& key);

	void switchSelectedState();
	void select();
	void deselect();
	bool getSelectionState();

	String getName() {return name;}

	int desiredWidth;
	int nodeId;

	AudioProcessor* getProcessor() const {return getAudioProcessor();}	
	
private:

	void buttonClicked (Button* button);

	Colour backgroundColor;

	MuteButton* muteButton;

	bool isSelected;
	String name;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEditor);

};



#endif  // __AUDIOEDITOR_H_9D6F1FC3__
