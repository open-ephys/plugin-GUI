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

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "../../AccessClass.h"

#include <stdio.h>

/**
  
  Automatically creates an interactive editor for selecting channels

  @see GenericEditor

*/

class ChannelSelectorButton;
class EditorButton;

class ChannelSelector : public Component,
						public Button::Listener,
						public AccessClass,
						public Timer
{
public:

	ChannelSelector(bool createButtons, Font& titleFont);
	~ChannelSelector();

	void buttonClicked(Button* button);

	Array<int> getActiveChannels();

	void setNumChannels(int);

	bool getRecordStatus(int chan);

	int getDesiredWidth();

private:

	EditorButton* audioButton;
	EditorButton* recordButton;
	EditorButton* paramsButton;
	EditorButton* allButton;
	EditorButton* noneButton;

	Array<ChannelSelectorButton*> parameterButtons;
	Array<ChannelSelectorButton*> audioButtons;
	Array<ChannelSelectorButton*> recordButtons;

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

};


class EditorButton : public Button
{
public:
	EditorButton(const String& name, Font& f);
	~EditorButton() {}

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

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
	
	int type;
	int num;
	Font buttonFont;
};


#endif  // __CHANNELSELECTOR_H_68124E35__
