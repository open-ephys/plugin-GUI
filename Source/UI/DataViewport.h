/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

  Holds tabs containing the application's visualizers.

  The DataViewport sits in the center of the MainWindow
  and is always visible. Editors that create data
  visualizations can place them in the
  DataViewport for easy access on small monitors, or in
  a separate window for maximum flexibility.

  This class is a subclass of juce_TabbedComponent.h

  @see GenericEditor, InfoLabel, LfpDisplayCanvas

*/

class DataViewport : public TabbedComponent
{
public:
    DataViewport();
    ~DataViewport();

    /** Adds a new visualizer within a tab and returns the tab index.*/
    int addTabToDataViewport(String tabName, Component* componentToAdd, GenericEditor* editor);

    /** Removes a tab with a specified index.*/
    void destroyTab(int);

    /** Selects a tab with a specified index.*/
    void selectTab(int);

    /** Informs the component within the current tab that it's now active.*/
    void currentTabChanged(int newIndex, const String& newTabName);

    /** Prevents the DataViewport from signaling EditorViewport when changing tabs.*/
    void disableConnectionToEditorViewport();

private:

    /** Maps original tab indices to their location within the DataViewport. */
    Array<int> tabArray;

    /** Maps processor editors to their respective tabs within the DataViewport. */
    Array<GenericEditor*> editorArray;
    void paint(Graphics& g);
    int tabDepth;
    int tabIndex;

    bool shutdown;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataViewport);

};



#endif  // __DATAVIEWPORT_H_B38FE628__
