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

#ifndef __CHANNELSELECTOR_H_68124E35__
#define __CHANNELSELECTOR_H_68124E35__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "../../AccessClass.h"

#include "../Channel.h"

#include <stdio.h>


class ChannelSelectorButton;
class EditorButton;

/**
  
  Automatically creates an interactive editor for selecting channels.

  @see GenericEditor

*/


class ChannelSelector : public Component,
						public Button::Listener,
						public AccessClass,
						public Timer
{
public:

	/** constructor */
	ChannelSelector(bool createButtons, Font& titleFont);

	/** destructor */
	~ChannelSelector();

	/** button callback */
	void buttonClicked(Button* button);

	/** Return an array of selected channels. */
	Array<int> getActiveChannels();

	/** Set the selected channels. */
	void setActiveChannels(Array<int>);

	/** Set the total number of channels. */
	void setNumChannels(int);

	/** Return whether a particular channel should be recording. */
	bool getRecordStatus(int chan);

	/** Return whether a particular channel should be monitored. */
	bool getAudioStatus(int chan);

	/** Return component's desired width. */
	int getDesiredWidth();

	void startAcquisition();

	void stopAcquisition();

	void inactivateButtons();

	void activateButtons();

	void setRadioStatus(bool);

	void paramButtonsToggledByDefault(bool t) {paramsToggled = t;}
	//void paramButtonsActiveByDefault(bool t) {paramsActive = t;}
    
    bool eventsOnly;

private:

	EditorButton* audioButton;
	EditorButton* recordButton;
	EditorButton* paramsButton;
	EditorButton* allButton;
	EditorButton* noneButton;

	Array<ChannelSelectorButton*> parameterButtons;
	Array<ChannelSelectorButton*> audioButtons;
	Array<ChannelSelectorButton*> recordButtons;

	bool paramsToggled;
	bool paramsActive;
	bool radioStatus;

	bool isNotSink;
	bool moveRight;
	bool moveLeft;

	int offsetLR;
	int offsetUD;

	int parameterOffset;
	int audioOffset;
	int recordOffset;

	int desiredOffset;

	void resized();

	void addButton();
	void removeButton();
	void refreshButtonBoundaries();

	void timerCallback();

	void paint(Graphics& g);

	Font& titleFont;

	enum {AUDIO, RECORD, PARAMETER};

	bool acquisitionIsActive;

};

/**
  
  A button within the ChannelSelector representing an individual channel.

  @see ChannelSelector

*/

class EditorButton : public Button
{
public:
	EditorButton(const String& name, Font& f);
	~EditorButton() {}

	bool getState() {return isEnabled;}

	void setState(bool state) 
	{
		isEnabled = state;

		if (!state)
		{
			removeListener((Button::Listener*) getParentComponent());
		} else {
			addListener((Button::Listener*) getParentComponent());
		}

		repaint();
	}

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
	
	void resized();

	Path outlinePath;

	int type;
	Font buttonFont;

	bool isEnabled;

	ColourGradient selectedGrad, selectedOverGrad, neutralGrad, neutralOverGrad;
};

class ChannelSelectorButton : public Button
{
public:
	ChannelSelectorButton(int num, int type, Font& f);
	~ChannelSelectorButton() {}

	int getType() {return type;}
	int getChannel() {return num;}
	//Channel* getChannel() {return ch;}
	void setActive(bool t);

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
	
	//Channel* ch;

	int type;
	int num;
	Font buttonFont;
	bool isActive;
};


#endif  // __CHANNELSELECTOR_H_68124E35__
