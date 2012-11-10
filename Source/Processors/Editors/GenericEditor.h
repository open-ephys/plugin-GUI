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

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor.h"
#include "../../AccessClass.h"

#include "../Channel.h"

#include <stdio.h>

class GenericProcessor;
class DrawerButton;
class TriangleButton;
class UtilityButton;
class ParameterEditor;
class ChannelSelector;
class Channel;


/**
  
  Base class for creating processor editors.

  If a processor doesn't havesign an editor defined, a GenericEditor will be used.

  Classes derived from this class must place their controls as child components.
  They shouldn't try to re-draw any aspects of their background.

  @see GenericProcessor, EditorViewport

*/

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
	void highlight();
	void deselect();
	bool getSelectionState();

	void enable();
	void disable();
	bool getEnabledState();
	void setEnabledState(bool);
	void startAcquisition();
	void stopAcquisition();

	String getName() {return name;}

	int desiredWidth;
	int nodeId;

	virtual void tabNumber(int t) {tNum = t;}
	int tabNumber() {return tNum;}

	virtual void switchSource(int) { }  // needed for MergerEditor
	virtual void switchSource() { }; // needed for MergerEditor

	GenericProcessor* getProcessor() const {return (GenericProcessor*) getAudioProcessor();}

	void fadeIn();

	bool isFading;

	float accumulator;

	virtual void switchDest() { }
	virtual void switchIO(int) { }

	virtual void buttonClicked(Button* button);
	virtual void buttonEvent(Button* button) {}
	virtual void sliderValueChanged(Slider* slider);
	virtual void sliderEvent(Slider* slider) {}
	virtual void editorWasClicked() {}

	bool checkDrawerButton(Button* button);
//	bool checkParameterButtons(Button* button);

	bool getRecordStatus(int chan);
	bool getAudioStatus(int chan);

	void selectChannels(Array<int>);

	void refreshColors();

	virtual void update();
	virtual void updateSettings() {}
	virtual void updateVisualizer() {}

	virtual void channelChanged(int chan) {}

	Array<int> getActiveChannels();

	Channel* getChannel(int chan);
	Channel* getEventChannel(int chan);

	Font titleFont;

	int getStartChannel();

protected:
	DrawerButton* drawerButton;
	int drawerWidth;

	virtual void addParameterEditors();

	ChannelSelector* channelSelector;

private:

	virtual void timerCallback();

	virtual void resized();

	Colour backgroundColor;
	ColourGradient backgroundGradient;

	bool isSelected;
	bool isEnabled;

	int tNum;

	String name;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);

};

/**
  
  Used to show and hide the ChannelSelector.

  @see GenericEditor, ChannelSelector

*/

class DrawerButton : public Button
{
public:
	DrawerButton(const String& name);
	~DrawerButton() {}
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
		
};

/**
  
  A button that displays a triangle facing up or down.

  @see GenericEditor

*/

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

/**
  
  A button that displays text.

  @see GenericEditor

*/

class UtilityButton : public Button
{
public:
    UtilityButton(const String& label_, Font font_);
    ~UtilityButton() {}

    void setCorners(bool UL, bool UR, bool LL, bool LR);
    void setRadius(float r);

    void setEnabledState(bool);
    bool getEnabledState() {return isEnabled;}

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    const String label;
    Font font;
    bool roundUL, roundUR, roundLL, roundLR;
    float radius;
    ColourGradient selectedGrad, selectedOverGrad, neutralGrad, neutralOverGrad;
    Colour fontColor;
    Path outlinePath;

    bool isEnabled;

    void resized();

};


#endif  // __GENERICEDITOR_H_DD406E71__
