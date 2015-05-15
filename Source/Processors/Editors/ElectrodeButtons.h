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

/**

  Used to select individual electrodes within a multichannel electrode.

  @see SpikeDetectorEditor.

*/


class ElectrodeButton : public Button
{
public:
	ElectrodeButton(int chan_);
	~ElectrodeButton();

	int getChannelNum();
    void setChannelNum(int i);
    void setChannelNum(int i, bool changeButtonText);


private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    int chan;
};

/**

  Utility button for Editors.

  @see SpikeDetectorEditor

*/

class ElectrodeEditorButton : public Button
{
public:
	ElectrodeEditorButton(const String& name_, Font font_);
	~ElectrodeEditorButton();
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    const String name;

    Font font;

};



#endif  // __ELECTRODEBUTTONS_H_BDCEE716__
