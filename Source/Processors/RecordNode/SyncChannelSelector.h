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
#include "../Editors/GenericEditor.h"
#include "../../Utils/Utils.h"

class SyncChannelSelector;

/** 

Allows the user to select the TTL line to use for synchronization

*/
class SyncChannelButton : public Button	
{
public:

	/** Constructor */
	SyncChannelButton(int id, SyncChannelSelector* parent);

	/** Destructor */
	~SyncChannelButton();

	/** Returns the ID for this button's stream*/
    int getId() { return id; };

private:

	int id; 
	SyncChannelSelector* parent;
    int width;
    int height;
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class SetButton : public Button	
{
public:

	/** Constructor */
	SetButton(const String& name);

	/** Destructor */
	~SetButton();

private:
	/** Renders the button*/
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};


class SyncChannelSelector : 
	public Component,
	public Button::Listener
{
public:

	class Listener
	{
	public:
		virtual ~Listener() { }
		virtual void channelStateChanged(Array<int> selectedChannels) = 0;
	};

	/** Constructor */
	SyncChannelSelector(Listener* listener, std::vector<bool> channelStates);
	//SyncChannelSelector(int nChans, int selectedChannelIdx, bool isPrimary);

	/** Destructor */
	~SyncChannelSelector();

	int getSelectedChannel() { return selectedChannelIdx; }

	/** Mouse listener methods*/
	void mouseDown(const MouseEvent &event);
	void mouseMove(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);

	/** Responds to button clicks*/
	void buttonClicked(Button*);

	int nChannels;
	int selectedId;
	bool isPrimary;
    
    bool detectedChange;

	int buttonSize;
	int nRows;

	int width;
	int height;

	OwnedArray<SyncChannelButton> buttons;

private:
	
	Listener* listener;

    ScopedPointer<SetButton> setPrimaryStreamButton;
    
	int selectedChannelIdx = 0;
};

#endif // SYNCCHANNEL_SELECTOR_H_INCLUDED
