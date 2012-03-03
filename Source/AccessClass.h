/*
  ==============================================================================

    AccessClass.h
    Created: 1 Mar 2012 1:17:45pm
    Author:  jsiegle

  ==============================================================================
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
