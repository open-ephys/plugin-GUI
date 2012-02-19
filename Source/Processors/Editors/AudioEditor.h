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
