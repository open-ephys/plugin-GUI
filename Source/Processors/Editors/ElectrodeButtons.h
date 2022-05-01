/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __ELECTRODEBUTTONS_H_BDCEE716__
#define __ELECTRODEBUTTONS_H_BDCEE716__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

/**

  Each button represents one electrode, which can be enabled, disabled,
  or reordered.

  @see ChannelMappingEditor

*/
class PLUGIN_API ElectrodeButton : public Button
{
public:
    /** Constructor*/
	ElectrodeButton(int chan_, Colour defaultColour = Colours::orange);

    /** Destructor*/
	~ElectrodeButton();

    /** Returns the channel number*/
	int getChannelNum();

    /** Updates the channel number*/
    void setChannelNum(int i);

private:
    /** Draws the button.*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    /** Holds the button's channel number (should be 1-based indexing)*/
    int chan;
    
    Colour defaultColour;
};

/**
  Utility button for setting the global state of ElectrodeButtons

  @see ChannelMappingEditor

*/

class PLUGIN_API ElectrodeEditorButton : public Button
{
public:
    /** Constructor*/
	ElectrodeEditorButton(const String& name_);
	
    /** Destructor*/
    ~ElectrodeEditorButton();
private:

    /** Draws the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    /** Holds the button's name*/
    const String name;

};



#endif  // __ELECTRODEBUTTONS_H_BDCEE716__
