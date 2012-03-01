/*
  ==============================================================================

    AccessClass.h
    Created: 1 Mar 2012 1:17:45pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __ACCESSCLASS_H_CE1DC2DE__
#define __ACCESSCLASS_H_CE1DC2DE__



class UIComponent;
class FilterViewport;
class FilterList;
class DataViewport;
class ProcessorGraph;
class MessageCenter;
class ControlPanel;
class Configuration;

class AccessClass
{
public:

	AccessClass() { }
	~AccessClass() { }
	
	void setUIComponent(UIComponent*);

	FilterViewport* getFilterViewport() {return fv;}
	DataViewport* getDataViewport() {return dv;}
	FilterList* getFilterList() {return fl;}
	ProcessorGraph* getProcessorGraph() {return pg;}
	ControlPanel* getControlPanel() {return cp;}
	MessageCenter* getMessageCenter() {return mc;}
	UIComponent* getUIComponent() {return ui;}
	Configuration* getConfiguration() {return cf;}

private:

	UIComponent* ui;
	FilterViewport* fv;
	FilterList* fl;
	DataViewport* dv;
	ProcessorGraph* pg;
	ControlPanel* cp;
	MessageCenter* mc;
	Configuration* cf;

};


#endif  // __ACCESSCLASS_H_CE1DC2DE__
