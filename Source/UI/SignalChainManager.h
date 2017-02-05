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

#ifndef __SIGNALCHAINMANAGER_H_948769B9__
#define __SIGNALCHAINMANAGER_H_948769B9__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Editors/GenericEditor.h"
#include "../AccessClass.h"

class GenericEditor;
class SignalChainTabButton;
class EditorViewport;

/**

  Provides helper functions for editing the signal chain.

  Created and owned by the EditorViewport.

  @see EditorViewport.

*/

class SignalChainManager
{
public:
    SignalChainManager(EditorViewport*, Array<GenericEditor*, CriticalSection>&,
                       Array<SignalChainTabButton*, CriticalSection>&);
    ~SignalChainManager();

    /** Updates the editors currently displayed by the EditorViewport.*/
    void updateVisibleEditors(GenericEditor* activeEditor, int index, int insertionPoint, int action);

    /** Creates a tab button for a new signal chain. */
    void createNewTab(GenericEditor* editor);

    /** Removes the tab button for a deleted signal chain. */
    void removeTab(int tabIndex);

    /** Scrolls the SignalChainTabButtons up, if there are more signal chains
    than can be viewed at once.*/
    void scrollUp();

    /** Scrolls the SignalChainTabButtons down, if there are more signal chains
    than can be viewed at once.*/
    void scrollDown();

    /** Clears the signal chain.*/
    void clearSignalChain();

	void updateProcessorSettings();

private:

    /** An array of all currently visible editors.*/
    Array<GenericEditor*, CriticalSection>& editorArray;

    /** An array of all existing signal chains (as referenced by their associated
    SignalChainTabButtons).*/
    Array<SignalChainTabButton*, CriticalSection>& signalChainArray;

    /** A pointer to the EditorViewport.*/
    EditorViewport* ev;

    /** Updates the visibility of SignalChainTabButtons.*/
    void refreshTabs();

    /** The index of the top tab (used for scrolling purposes).*/
    int topTab;

    const int tabSize;


};


#endif  // __SIGNALCHAINMANAGER_H_948769B9__
