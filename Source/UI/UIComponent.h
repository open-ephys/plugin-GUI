/*
  ==============================================================================

    UIComponent.h
    Created: 30 Apr 2011 8:33:05pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __UICOMPONENT_H_D97C73CF__
#define __UICOMPONENT_H_D97C73CF__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "InfoLabel.h"
#include "ControlPanel.h"
#include "FilterList.h"
#include "FilterViewport.h"
#include "DataViewport.h"
#include "MessageCenter.h"
#include "Configuration.h"
#include "../Processors/DisplayNode.h"
#include "../Processors/ProcessorGraph.h"
#include "../Audio/AudioComponent.h"


class UIComponent : public Component,
				    //public ActionBroadcaster,
				    public DragAndDropContainer // required for 
				    				            // drag-and-drop
				    				            // internal components

{
public: 
	UIComponent(ProcessorGraph* pgraph, AudioComponent* audio);
	~UIComponent();

	FilterViewport* getFilterViewport() {return filterViewport;}
	DataViewport* getDataViewport() {return dataViewport;}
	Configuration* getConfiguration() {return config;}

	//void transmitMessage(const String& message);

private:

	DataViewport* dataViewport;
	FilterViewport* filterViewport;
	FilterList* filterList;
	ControlPanel* controlPanel;
	MessageCenter* messageCenter;
	Configuration* config;
	InfoLabel* infoLabel;

	ProcessorGraph* processorGraph;
	AudioComponent* audio;

	void resized();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIComponent);
	
};

#endif  // __UICOMPONENT_H_D97C73CF__
