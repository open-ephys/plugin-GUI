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

#ifndef __GENERICEDITOR_H_DD406E71__
#define __GENERICEDITOR_H_DD406E71__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor.h"
#include "../../AccessClass.h"
#include <stdio.h>

/**
  
  Base class for creating processor editors.

  If a processor doesn't have an editor defined, a GenericEditor will be used.

  Classes derived from this class must place their controls as child components.
  They shouldn't try to re-draw any aspects of their background.

  @see GenericProcessor, FilterViewport

*/

class GenericProcessor;
class FilterViewport;

class GenericEditor : public AudioProcessorEditor,
                      public Timer,
                      public AccessClass

{
public:
	GenericEditor (GenericProcessor* owner);//, FilterViewport* vp);
	virtual ~GenericEditor();

	void paint (Graphics& g);
	//void setViewport(FilterViewport*);

	bool keyPressed (const KeyPress& key);

	void switchSelectedState();
	void select();
	void deselect();
	bool getSelectionState();

	void enable();
	void disable();
	bool getEnabledState();
	void setEnabledState(bool);

	String getName() {return name;}

	int desiredWidth;
	int nodeId;

	void tabNumber(int t) {tNum = t;}
	int tabNumber() {return tNum;}

	//FilterViewport* viewport;
	//Configuration* config;

	//void setConfiguration(Configuration* cf) {config = cf;}
	//Configuration* getConfiguration() {return config;}

	AudioProcessor* getProcessor() const {return getAudioProcessor();}
	
	void createRadioButtons(int x, int y, int w, StringArray values, const String& name);
		

	void fadeIn();

	int radioGroupId;

	bool isFading;

	float accumulator;
	
private:

	virtual void timerCallback();

	Colour backgroundColor;

	bool isSelected;
	bool isEnabled;

	int tNum;



	Font titleFont;

	String name;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);

};




class RadioButton : public Button
{
public:
    RadioButton(const String& name, int groupId);// : Button("Name") {configurationChanged = true;}
    ~RadioButton() {}

private:

    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown);

    Font buttonFont;
};




#endif  // __GENERICEDITOR_H_DD406E71__
