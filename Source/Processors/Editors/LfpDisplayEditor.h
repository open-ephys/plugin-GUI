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

#ifndef __LFPDISPLAYEDITOR_H_3438800D__
#define __LFPDISPLAYEDITOR_H_3438800D__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "../LfpDisplayNode.h"
#include "../Visualization/LfpDisplayCanvas.h"

//class FilterViewport;
//class DataViewport;
class DataWindow;
class LfpDisplayCanvas;

class SelectorButton : public DrawableButton
{
	public:
		SelectorButton();
		~SelectorButton();	
};

class LfpDisplayEditor : public GenericEditor,//,
				   	    // public Button::Listener,
				   	     public Slider::Listener
{
public:
	LfpDisplayEditor (GenericProcessor*);
	~LfpDisplayEditor();

	void buttonClicked (Button* button);
	void setBuffers (AudioSampleBuffer*, MidiBuffer*);
	//void setUIComponent (UIComponent* ui) {UI = ui;}

	void sliderValueChanged (Slider* slider);

	void enable();
	void disable();

	void updateNumInputs(int);
	void updateSampleRate(float);

private:	

	bool isPlaying;
	
	ScopedPointer <DataWindow> dataWindow;

	Slider* timebaseSlider;
	Slider* displayGainSlider;

	SelectorButton* windowSelector;
	SelectorButton* tabSelector;

	ScopedPointer <LfpDisplayCanvas> canvas;

	//AudioSampleBuffer* streamBuffer;
	//MidiBuffer* eventBuffer;
	//UIComponent* UI;
	//DataViewport* dataViewport;

	int tabIndex;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayEditor);

};

#endif  // __LFPDISPLAYEDITOR_H_3438800D__
