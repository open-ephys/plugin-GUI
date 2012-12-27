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

#ifndef __EDITORVIEWPORTBUTTONS_H_2657C51D__
#define __EDITORVIEWPORTBUTTONS_H_2657C51D__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"

/**
  
  Allows the user to scroll through visible editors when
  there are more than can fit within the boundaries of the
  EditorViewport.

  @see EditorViewport.

*/

class EditorScrollButton : public DrawableButton
{
public:
	EditorScrollButton(int type);
	~EditorScrollButton();

	void setActive(bool);

	bool isActive;

	enum type {LEFT, RIGHT};

	int direction;

	DrawablePath inactive, activeNormal, activeOver, activeDown;

};

/**
  
  Allows the user to scroll through signal chains when
  there are more than can fit within the boundaries of the
  EditorViewport.

  @see EditorViewport.

*/

class SignalChainScrollButton : public DrawableButton
{
public:
	SignalChainScrollButton(int type);
	~SignalChainScrollButton();

	void setActive(bool);

	enum type {UP, DOWN};

	bool isActive;

	int direction;

	DrawablePath inactive, activeNormal, activeOver, activeDown;
};



#endif  // __EDITORVIEWPORTBUTTONS_H_2657C51D__
