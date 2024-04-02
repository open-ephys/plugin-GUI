/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#ifndef SYNCCHANNEL_SELECTOR_H_INCLUDED
#define SYNCCHANNEL_SELECTOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Utils/Utils.h"

class SyncLineSelector;

/** 

Allows the user to select the TTL line to use for synchronization

*/
class PLUGIN_API SyncChannelButton : public Button	
{
public:

	/** Constructor */
	SyncChannelButton(int id, SyncLineSelector* parent);

	/** Destructor */
	~SyncChannelButton();

	/** Returns the ID for this button's stream*/
    int getId() { return id; };

private:

	int id; 
	SyncLineSelector* parent;
    int width;
    int height;
	Colour btnColor;

	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class PLUGIN_API SetPrimaryButton : public Button	
{
public:

	/** Constructor */
	SetPrimaryButton(const String& name);

	/** Destructor */
	~SetPrimaryButton();

private:
	/** Renders the button*/
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};


class PLUGIN_API SyncLineSelector : 
	public Component,
	public Button::Listener
{
public:

	class Listener
	{
	public:
		virtual ~Listener() { }

		// Called when the selected sync line changes
		virtual void selectedLineChanged(int selectedLine) = 0;

		// Called when the user sets the primary stream for synchronization
		virtual void primaryStreamChanged() = 0;
	};

	/** Constructor */
	SyncLineSelector(Listener* listener, int numChans, int selectedLine, bool isPrimary);
	//SyncLineSelector(int nChans, int selectedChannelIdx, bool isPrimary);

	/** Destructor */
	~SyncLineSelector();

	int getSelectedChannel() { return selectedLine; }

	/** Mouse listener methods*/
	void mouseDown(const MouseEvent &event);
	void mouseMove(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);

	/** Responds to button clicks*/
	void buttonClicked(Button*);

	int nChannels;
	bool isPrimary;
    
    bool detectedChange;

	int buttonSize;
	int nRows;

	int width;
	int height;

	OwnedArray<SyncChannelButton> buttons;

	Array<Colour> lineColors;

private:
	
	Listener* listener;

    ScopedPointer<SetPrimaryButton> setPrimaryStreamButton;
    
	int selectedLine = 0;
};

#endif // SYNCCHANNEL_SELECTOR_H_INCLUDED
