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

#ifndef __SIGNALCHAINMANAGER_H_948769B9__
#define __SIGNALCHAINMANAGER_H_948769B9__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Editors/GenericEditor.h"
#include "../AccessClass.h"


/**
  
  Helper functions for editing the signal chain.

  Created and owned by the EditorViewport.

  @see EditorViewport.

*/

class GenericEditor;
class SignalChainTabButton;
class EditorViewport;

class SignalChainManager : AccessClass
{
public:
	SignalChainManager(EditorViewport*, Array<GenericEditor*, CriticalSection>&,
	 				   Array<SignalChainTabButton*, CriticalSection>&);
	~SignalChainManager();

	void updateVisibleEditors(GenericEditor* activeEditor, int index, int insertionPoint, int action);

	void createNewTab(GenericEditor* editor);
	void removeTab(int tabIndex);

private:	

	Array<GenericEditor*, CriticalSection>& editorArray;
	Array<SignalChainTabButton*, CriticalSection>& signalChainArray;

	EditorViewport* ev;

};


#endif  // __SIGNALCHAINMANAGER_H_948769B9__
