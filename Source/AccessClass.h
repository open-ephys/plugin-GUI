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

#ifndef __ACCESSCLASS_H_CE1DC2DE__
#define __ACCESSCLASS_H_CE1DC2DE__

#include "../JuceLibraryCode/JuceHeader.h"

class UIComponent;
class EditorViewport;
class ProcessorList;
class DataViewport;
class ProcessorGraph;
class MessageCenter;
class ControlPanel;
class Configuration;
class AudioComponent;


/**
  
  Allows subclasses to access important pointers within the application.

  @see UIComponent

*/

class AccessClass : public ActionBroadcaster
{
public:

	AccessClass() { }
	~AccessClass() { }
	
	void setUIComponent(UIComponent*);

	EditorViewport* getEditorViewport() {return ev;}
	DataViewport* getDataViewport() {return dv;}
	ProcessorList* getProcessorList() {return pl;}
	ProcessorGraph* getProcessorGraph() {return pg;}
	ControlPanel* getControlPanel() {return cp;}
	MessageCenter* getMessageCenter() {return mc;}
	UIComponent* getUIComponent() {return ui;}
	Configuration* getConfiguration() {return cf;}
	AudioComponent* getAudioComponent() {return ac;}

private:

	UIComponent* ui;
	EditorViewport* ev;
	ProcessorList* pl;
	DataViewport* dv;
	ProcessorGraph* pg;
	ControlPanel* cp;
	MessageCenter* mc;
	Configuration* cf;
	AudioComponent* ac;

};


#endif  // __ACCESSCLASS_H_CE1DC2DE__
