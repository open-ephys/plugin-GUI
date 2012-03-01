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
#include "../Processors/ProcessorGraph.h"
#include "../Audio/AudioComponent.h"
#include "../MainWindow.h"

#include "../OpenGL.h"

/**
  
  Creates objects for user interaction.

  The UIComponent can relay messages its sub-components, such as
  the MessageCenter

  @see ControlPanel, FilterList, FilterViewport, DataViewport,
       MessageCenter, Configuration

*/

class MainWindow;
class FilterList;

class FilterViewportButton;

class UIComponent : public Component,
				    public ActionBroadcaster,
				    public MenuBarModel,
				    public DragAndDropContainer // required for 
				    				            // drag-and-drop
				    				            // internal components
				    

{
public: 
	UIComponent(MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio);
	~UIComponent();

	FilterViewport* getFilterViewport() {return filterViewport;}
	FilterList* getFilterList() {return filterList;}
	DataViewport* getDataViewport() {return dataViewport;}
	Configuration* getConfiguration() {return config;}
	ProcessorGraph* getProcessorGraph() {return processorGraph;}
	ControlPanel* getControlPanel() {return controlPanel;}
	MessageCenter* getMessageCenter() {return messageCenter;}
	UIComponent* getUIComponent() {return this;}

	//void transmitMessage(const String& message);
	void disableCallbacks();

	void childComponentChanged();

	const StringArray getMenuBarNames();
	const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName);
	void menuItemSelected(int menuItemID, int topLevelMenuIndex);

private:

	DataViewport* dataViewport;
	FilterViewport* filterViewport;
	FilterViewportButton* filterViewportButton;
	FilterList* filterList;
	ControlPanel* controlPanel;
	MessageCenter* messageCenter;
	Configuration* config;
	InfoLabel* infoLabel;

	MainWindow* mainWindow;

	ProcessorGraph* processorGraph;
	AudioComponent* audio;

	void resized();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIComponent);
	
};



class FilterViewportButton : public OpenGLComponent
{
public:
	FilterViewportButton(UIComponent* ui);
	~FilterViewportButton();

	bool isOpen() {return open;}

	void newOpenGLContextCreated();
	void renderOpenGL();

	void drawName();
	void drawButton();

	void mouseDown(const MouseEvent& e);

private:	

	UIComponent* UI;
	bool open;

	FTPixmapFont* font;

};

#endif  // __UICOMPONENT_H_D97C73CF__
