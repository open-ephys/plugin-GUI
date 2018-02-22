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

#ifndef __ACCESSCLASS_H_CE1DC2DE__
#define __ACCESSCLASS_H_CE1DC2DE__

#include "../JuceLibraryCode/JuceHeader.h"

class UIComponent;
class EditorViewport;
class ProcessorList;
class DataViewport;
class ProcessorGraph;
class MessageCenterEditor;
class ControlPanel;
class AudioComponent;
class GraphViewer;
class PluginManager;
class GenericProcessor;



namespace AccessClass
{


/** Sets the object's UIComponent and copies all the necessary pointers
	from the UIComponent.
	*/
void setUIComponent(UIComponent*);

void shutdownBroadcaster();


/** Returns a pointer to the application's EditorViewport. */
EditorViewport* getEditorViewport();

/** Returns a pointer to the application's DataViewport. */
DataViewport* getDataViewport();

/** Returns a pointer to the application's ProcessorList. */
ProcessorList* getProcessorList();

/** Returns a pointer to the application's ProcessorGraph. */
ProcessorGraph* getProcessorGraph();

/** Returns a pointer to the application's DataViewport. */
ControlPanel* getControlPanel();

/** Returns a pointer to the application's MessageCenter. */
MessageCenterEditor* getMessageCenter();

/** Returns a pointer to the application's UIComponent. */
UIComponent* getUIComponent();

/** Returns a pointer to the application's AudioComponent. */
AudioComponent* getAudioComponent();

/** Returns a pointer to the application's GraphViewer. */
GraphViewer* getGraphViewer();

/** Returns a pointer to the application's PluginManager. */
PluginManager* getPluginManager();

ActionBroadcaster* getBroadcaster();

//Methods to access some private members of GenericProcessors.
//Like all of the AccessClass methods, this ones are meant to be
//used by various internal parts of the core GUI which need access
//to those members, while keeping them inaccessible by normal plugins

class ExternalProcessorAccessor
{

public:
	static MidiBuffer* getMidiBuffer(GenericProcessor* proc);
};

};


#endif  // __ACCESSCLASS_H_CE1DC2DE__
