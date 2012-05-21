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

#ifndef __DATAVIEWPORT_H_B38FE628__
#define __DATAVIEWPORT_H_B38FE628__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../AccessClass.h"

class GenericEditor;

/**
  
  Holds tabs for visualizers.

  Editors that create OpenGL visualizers can either place them
  in the DataViewport, for easy access on small monitors, or in
  a separate window, for maximum flexibility.

  @see GenericEditor, InfoLabel, LfpDisplayCanvas

*/

class DataViewport : public TabbedComponent,
                     public AccessClass

{
public: 
	DataViewport();
	~DataViewport();

	/** Adds a new tab and returns the tab index.*/
    int addTabToDataViewport(String tabName, Component* componentToAdd, GenericEditor* editor);

    /** Removes a tab with a specified index.*/
    void destroyTab(int);

    /** Select a tab with a specified index.*/
    void selectTab(int);

    /** Informs the component of the current tab that it's now active.*/
    void currentTabChanged(int newIndex, const String& newTabName);

    /** Prevent DataViewport from signaling EditorViewport when changing tabs.*/
    void disableConnectionToEditorViewport();

private:

	Array<int> tabArray;
  Array<GenericEditor*> editorArray;
	void paint(Graphics& g);
	int tabDepth;

  bool shutdown;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataViewport);
	
};



#endif  // __DATAVIEWPORT_H_B38FE628__
