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
	/** Constructor. Loads fonts and creates default buttons.*/
	GenericEditor (GenericProcessor* owner);

	/** Destructor.*/
	virtual ~GenericEditor();

	/** Draws the editor's background.*/
	void paint (Graphics& g);

	/** Called whenever a key is pressed and the editor has keyboard focus.*/
	bool keyPressed (const KeyPress& key);

	/** Toggles the editor's selection state.*/
	void switchSelectedState();

	/** Highlights an editor and calls editorWasClicked().*/
	void select();

	/** Highlights an editor.*/
	void highlight();

	/** Deselects an editor.*/
	void deselect();

	/** Returns an editor's selection state.*/
	bool getSelectionState();

	/** Used to enable an editor's processor.*/
	void enable();

	/** Used to disable an editor's processor.*/
	void disable();

	/** Returns whether or not the editor's processor is enabled (i.e., whether it's able to handle data.*/
	bool getEnabledState();

	/** Used to enable or disable an editor's processor.*/
	void setEnabledState(bool);

	/** Called just prior to the start of acquisition, to allow the editor to prepare.*/
	void startAcquisition();

	/** Called after the end of acquisition.*/
	void stopAcquisition();

	/** Returns the name of the editor.*/
	String getName() {return name;}

	/** Determines how wide the editor will be drawn. */
	int desiredWidth;

	/** The unique integer ID of the editor's processor. */
	int nodeId;

	/** Sets the number of the editor's associated tab in the DataViewport. */
	virtual void tabNumber(int t) {tNum = t;}

	/** Returns the number of the editor's associated tab in the DataViewport. */
	int tabNumber() {return tNum;}

	/** Required for MergerEditor only.*/
	virtual void switchSource(int) { }

	/** Required for MergerEditor only.*/
	virtual void switchSource() { }

	/** Returns the processor associated with an editor.*/
	GenericProcessor* getProcessor() const {return (GenericProcessor*) getAudioProcessor();}

	/** Causes the editor to fade in when it first appears in the EditorViewport. */
	void fadeIn();

	/** Indicates whether or not the editor is in the processof fading in. */
	bool isFading;

	/** Used to control the speed at which the editor fades in. */
	float accumulator;

	/** Required for SplitterEditor only.*/
	virtual void switchDest() { }

	/** Required for SplitterEditor and MergerEditor only.*/
	virtual void switchIO(int) { }

	/** Handles button clicks for all editors. Deals with clicks on the editor's
        title bar and channel selector drawer. */
	virtual void buttonClicked(Button* button);

	/** Called by buttonClicked(). Deals with clicks on custom buttons. Subclasses of
	    GenericEditor should modify this method only.*/
	virtual void buttonEvent(Button* button) {}

	/** Handles slider events for all editors. */
	virtual void sliderValueChanged(Slider* slider);

	/** Called by sliderValueChanged(). Deals with clicks on custom sliders. Subclasses
	    of GenericEditor should modify this method only.*/
	virtual void sliderEvent(Slider* slider) {}

	/** Required for opening displays in a VisualizerEditor. Hopefully will be deprecated soon.*/
	virtual void editorWasClicked() {}

	/** Checks to see if a button click occurred on the ChannelSelector drawer button.*/
	bool checkDrawerButton(Button* button);

	/** Returns the record status of a given channel from the ChannelSelector.*/
	bool getRecordStatus(int chan);

	/** Returns the audio monitoring status of a given channel from the ChannelSelector.*/
	bool getAudioStatus(int chan);

	/** Selects all the channels in the input array.*/
	void selectChannels(Array<int>);

	/** Refreshes an editor's background colors when the user selects new ones with the ColourSelector.*/
	void refreshColors();

	/** Called when an editor's processor updates its settings (mainly to update channel count).*/
	virtual void update();

	/** Called by the update() method to allow the editor to update its custom settings.*/
	virtual void updateSettings() {}

	/** Allows an editor to update the settings of its visualizer (such as channel count and sample rate).*/
	virtual void updateVisualizer() {}

	/** Used by SpikeDetectorEditor. */
	virtual void channelChanged(int chan) {}

	/** Returns all selected channels from the ChannelSelector. */
	Array<int> getActiveChannels();

	/** An array of pointers to ParameterEditors created based on the Parameters of an editor's underlying processor. */
	Array<ParameterEditor*> parameterEditors;

	/** Returns the Channel object for a given continuous channel number. */
	Channel* getChannel(int chan);

	/** Returns the Channel object for a given event channel number. */
	Channel* getEventChannel(int chan);

	/** Stores the font used to display the editor's name. */
	Font titleFont;

protected:

	/** A pointer to the button that opens the drawer for the ChannelSelector. */
	DrawerButton* drawerButton;

	/** Determines the width of the ChannelSelector drawer when opened. */
	int drawerWidth;

	/** Can be overridden to customize the layout of ParameterEditors. */
	virtual void addParameterEditors();

	/** A pointer to the editor's ChannelSelector. */
	ChannelSelector* channelSelector;

private:

	/** Used for fading in the editor. */
	virtual void timerCallback();

	/** Called when the boundaries of the editor are updated. */
	virtual void resized();

	/** Stores the editor's background color. */
	Colour backgroundColor;

	/** Stores the editor's background gradient. */
	ColourGradient backgroundGradient;

	bool isSelected;
	bool isEnabled;

	int tNum;

	String name;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);

};

/**
  
  Used to show and hide the ChannelSelector.

  Appears on the right-hand size of all editors (except SplitterEditor and MergerEditor).

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

  Useful for incrementing or decrementing values (as in SpikeDetectorEditor).

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
