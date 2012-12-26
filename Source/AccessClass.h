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

#ifdef WIN32
#include <Windows.h>
#endif
#include "../JuceLibraryCode/JuceHeader.h"

class UIComponent;
class EditorViewport;
class ProcessorList;
class DataViewport;
class ProcessorGraph;
class MessageCenter;
class ControlPanel;
class AudioComponent;

/**
  
  Allows subclasses to access important pointers within the application.

  When an object inherits from AccessClass, it makes it much more convenient to get and
  set pointers to other objects, such as the EditorViewport, ProcessorList, and
  ProcessorGraph that are used throughout the application. In addition, every subclass 
  of AccessClass automatically adds the MessageCenter as an ActionListener, which means
  messages sent by that object [using sendActionMessage("Message.")] will appear 
  in the MessageCenter by default.

  @see UIComponent, MessageCenter

*/

class AccessClass : public ActionBroadcaster
{
public:

	AccessClass() { }
	~AccessClass() { }
	
	/** Sets the object's UIComponent and copies all the necessary pointers
	    from the UIComponent. 

	    Automatically adds the MessageCenter as an ActionListener, which causes
	    messages sent using sendActionMessage("Message") to appear in the 
	    MessageCenter. */
	void setUIComponent(UIComponent*);

	/** Called within setUIComponent() to enable subclasses to update their 
	    members' pointers. */
	virtual void updateChildComponents() {}


	/** Returns a pointer to the application's EditorViewport. */
	EditorViewport* getEditorViewport() {return ev;}

	/** Returns a pointer to the application's DataViewport. */
	DataViewport* getDataViewport() {return dv;}

	/** Returns a pointer to the application's ProcessorList. */
	ProcessorList* getProcessorList() {return pl;}

	/** Returns a pointer to the application's ProcessorGraph. */
	ProcessorGraph* getProcessorGraph() {return pg;}

	/** Returns a pointer to the application's DataViewport. */
	ControlPanel* getControlPanel() {return cp;}

	/** Returns a pointer to the application's MessageCenter. */
	MessageCenter* getMessageCenter() {return mc;}

	/** Returns a pointer to the application's UIComponent. */
	UIComponent* getUIComponent() {return ui;}

	/** Returns a pointer to the application's AudioComponent. */
	AudioComponent* getAudioComponent() {return ac;}

private:

	UIComponent* ui;
	EditorViewport* ev;
	ProcessorList* pl;
	DataViewport* dv;
	ProcessorGraph* pg;
	ControlPanel* cp;
	MessageCenter* mc;
	AudioComponent* ac;

};


#endif  // __ACCESSCLASS_H_CE1DC2DE__
