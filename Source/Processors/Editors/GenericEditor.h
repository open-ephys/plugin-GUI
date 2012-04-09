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

  If a processor doesn't havesign an editor defined, a GenericEditor will be used.

  Classes derived from this class must place their controls as child components.
  They shouldn't try to re-draw any aspects of their background.

  @see GenericProcessor, EditorViewport

*/

class GenericProcessor;
class DrawerButton;
class EditorButton;
class ChannelSelectorButton;
class TriangleButton;
class PlusButton;

class GenericEditor : public AudioProcessorEditor,
                      public Timer,
                      public AccessClass,
                      public Button::Listener,
                      public Slider::Listener

{
public:
	GenericEditor (GenericProcessor* owner);
	virtual ~GenericEditor();

	void paint (Graphics& g);

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

	virtual void tabNumber(int t) {tNum = t;}
	int tabNumber() {return tNum;}

	virtual void switchSource(int) { }  // needed for MergerEditor
	virtual void switchSource() { }; // needed for MergerEditor

	GenericProcessor* getProcessor() const {return (GenericProcessor*) getAudioProcessor();}
	
	void createRadioButtons(int x, int y, int w, StringArray values, const String& name);
		
	void fadeIn();

	int radioGroupId;

	bool isFading;

	float accumulator;

	virtual void buttonClicked(Button* button);
	virtual void buttonEvent(Button* button) {}
	virtual void sliderValueChanged(Slider* slider) {}

	bool checkDrawerButton(Button* button);
	bool checkChannelSelectors(Button* button);

	void selectChannels(Array<int>);

	void refreshColors();

	virtual void update();
	virtual void updateVisualizer() {}

	Array<int> getActiveChannels();

	Array<bool> audioChannels;
	Array<bool> recordChannels;
	Array<bool> paramsChannels;

protected:
	DrawerButton* drawerButton;
	int drawerWidth;

	
private:

	virtual void timerCallback();

	virtual void resized();

	virtual int createChannelSelectors();
	virtual void removeChannelSelectors();
	virtual void destroyChannelSelectors();

	Colour backgroundColor;
	ColourGradient backgroundGradient;

	bool isSelected;
	bool isEnabled;

	int tNum;

	int numChannels;



	EditorButton* audioButton;
	EditorButton* recordButton;
	EditorButton* paramsButton;

	

	Array<ChannelSelectorButton*> channelSelectorButtons;

	ChannelSelectorButton* allButton;
	ChannelSelectorButton* noneButton;


	String name;

protected:

	Font titleFont;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);

};




class RadioButton : public Button
{
public:
    RadioButton(const String& name, int groupId, Font f);// : Button("Name") {configurationChanged = true;}
    ~RadioButton() {}

private:

    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown);

    Font buttonFont;
};

class DrawerButton : public Button
{
public:
	DrawerButton(const String& name);
	~DrawerButton() {}
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
		
};

class EditorButton : public Button
{
public:
	EditorButton(const String& name, Font f);
	~EditorButton() {}
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
	
	int type;
	Font buttonFont;
};

class ChannelSelectorButton : public Button
{
public:
	ChannelSelectorButton(const String& name, Font f);
	~ChannelSelectorButton() {}
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
	
	int type;
	Font buttonFont;
};

class TriangleButton : public Button
{
public:
    TriangleButton(int direction_) : Button("Arrow") 
        {direction = direction_;}
    ~TriangleButton() {}
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
    
    int direction;
};

class PlusButton : public Button
{
public:
    PlusButton() : Button("Plus") {}
    ~PlusButton() {}
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

};


#endif  // __GENERICEDITOR_H_DD406E71__
