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

#include "UIComponent.h"
#include <stdio.h>

UIComponent::UIComponent (MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio_) 
	: processorGraph(pgraph), audio(audio_), mainWindow(mainWindow_)

{	

	processorGraph->setUIComponent(this);

	infoLabel = new InfoLabel();

	dataViewport = new DataViewport ();
	addChildComponent(dataViewport);
	dataViewport->addTabToDataViewport("Info",infoLabel,0);

	std::cout << "Created data viewport." << std::endl;

	editorViewport = new EditorViewport();

	addAndMakeVisible(editorViewport);

	std::cout << "Created filter viewport." << std::endl;

	editorViewportButton = new EditorViewportButton(this);
	addAndMakeVisible(editorViewportButton);

	controlPanel = new ControlPanel(processorGraph, audio);
	addAndMakeVisible(controlPanel);

	std::cout << "Created control panel." << std::endl;

	processorList = new ProcessorList();
	addAndMakeVisible(processorList);

	std::cout << "Created filter list." << std::endl;

	messageCenter = new MessageCenter();
	addActionListener(messageCenter);
	addAndMakeVisible(messageCenter);

	std::cout << "Created message center." << std::endl;

	setBounds(0,0,500,400);

	std::cout << "Component width = " << getWidth() << std::endl;
	std::cout << "Component height = " << getHeight() << std::endl;

	std::cout << "UI component data viewport: " << dataViewport << std::endl;

	std::cout << "Finished UI stuff." << std::endl << std::endl << std::endl;

	processorGraph->setUIComponent(this);
	processorList->setUIComponent(this);
	editorViewport->setUIComponent(this);
	dataViewport->setUIComponent(this);
	controlPanel->getAudioEditor()->setUIComponent(this);
	controlPanel->setUIComponent(this);
    
    //processorGraph->sendActionMessage("Test.");

	//processorGraph->loadState();

#if JUCE_MAC
    MenuBarModel::setMacMainMenu(this);
    mainWindow->setMenuBar(0);
#else
    mainWindow->setMenuBar(this);
#endif
	
}

UIComponent::~UIComponent()
{
	deleteAndZero(infoLabel);
	deleteAllChildren();

	processorGraph = 0;
	audio = 0;
}

void UIComponent::resized()
{
	
	int w = getWidth();
	int h = getHeight();
	
	if (dataViewport != 0) 
	{
		int left, top, width, height;
		left = 6;
		top = 40;
		
		if (processorList->isOpen())
			left = 202;
		else
			left = 6;

		if (controlPanel->isOpen())
			top = 72;
		else
			top = 40;

		if (editorViewportButton->isOpen())
			height = h - top - 195;
		else
			height = h - top - 45;

		width = w - left - 5;

		dataViewport->setBounds(left, top, width, height);
	}

	if (editorViewportButton != 0)
	{
		editorViewportButton->setBounds(w-230, h-40, 225, 35);
	}
	
	if (editorViewport != 0) 
	{
		if (editorViewportButton->isOpen() && !editorViewport->isVisible())
			editorViewport->setVisible(true);
		else if (!editorViewportButton->isOpen() && editorViewport->isVisible())
			editorViewport->setVisible(false);

		editorViewport->setBounds(6,h-190,w-11,150);
	}

	if (controlPanel != 0)
	{
		if (controlPanel->isOpen())
			controlPanel->setBounds(201,6,w-210,64);
		else
			controlPanel->setBounds(201,6,w-210,32);
	}

	if (processorList != 0) 
	{
		if (processorList->isOpen())
			if (editorViewportButton->isOpen())
				processorList->setBounds(5,5,195,h-200);
			else
				processorList->setBounds(5,5,195,h-50);
		else
			processorList->setBounds(5,5,195,34);
	}

	if (messageCenter != 0)
		messageCenter->setBounds(6,h-35,w-241,30);

	// for debugging qpurposes:
	if (false)
	{
		dataViewport->setVisible(false);
		editorViewport->setVisible(false);
		processorList->setVisible(false);
		messageCenter->setVisible(false);
		controlPanel->setVisible(false);
		editorViewportButton->setVisible(false);
	}

}

void UIComponent::disableCallbacks()
{
	//sendActionMessage("Data acquisition terminated.");
	controlPanel->disableCallbacks();
}

void UIComponent::disableDataViewport()
{
	dataViewport->disableConnectionToEditorViewport();
}

void UIComponent::childComponentChanged()
{
	resized();
}




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// MENU BAR METHODS

const StringArray UIComponent::getMenuBarNames() {

	// StringArray names;
	// names.add("File");
	// names.add("Edit");
	// names.add("Help");

	const char* const names[] = { "File", "Edit", "View", "Help", 0 };

    return StringArray (names);

}

const PopupMenu UIComponent::getMenuForIndex(int menuIndex, const String& menuName)
{
	 ApplicationCommandManager* commandManager = &(mainWindow->commandManager);

     PopupMenu menu;

     if (menuIndex == 0)
     {
     	menu.addCommandItem (commandManager, openConfiguration);
        menu.addCommandItem (commandManager, saveConfiguration);
        
#if !JUCE_MAC
       	menu.addSeparator();
       	menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
#endif

       } else if (menuIndex == 1)
     {
     	menu.addCommandItem (commandManager, undo);
     	menu.addCommandItem (commandManager, redo);
     	menu.addSeparator();
     	menu.addCommandItem (commandManager, copySignalChain);
     	menu.addCommandItem (commandManager, pasteSignalChain);
     	menu.addSeparator();
     	menu.addCommandItem (commandManager, clearSignalChain);
     
     } else if (menuIndex == 2) {

     	menu.addCommandItem (commandManager, toggleProcessorList);
     	menu.addCommandItem (commandManager, toggleSignalChain);
     	menu.addCommandItem (commandManager, toggleFileInfo);
     
     } else if (menuIndex == 3)
     {
     	menu.addCommandItem (commandManager, showHelp);
     }

     return menu;

}

void UIComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	
 //
}

// ApplicationCommandTarget methods

ApplicationCommandTarget* UIComponent::getNextCommandTarget()
{
	// this will return the next parent component that is an ApplicationCommandTarget (in this
    // case, there probably isn't one, but it's best to use this method anyway).
	return findFirstTargetParentComponent();
}

void UIComponent::getAllCommands (Array <CommandID>& commands)
{
	 const CommandID ids[] = {openConfiguration,
	 					      saveConfiguration,
	 					      undo,
	 					      redo,
	 					      copySignalChain,
	 					      pasteSignalChain,
	 					      clearSignalChain,
	 					      toggleProcessorList,
	 					      toggleSignalChain,
	 					      toggleFileInfo,
	 					      showHelp};

	 commands.addArray (ids, numElementsInArray (ids));

}

void UIComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{

	bool acquisitionStarted = getAudioComponent()->callbacksAreActive();

	switch (commandID)
	{
	case openConfiguration:
		result.setInfo("Open configuration", "Load a saved processor graph.", "General", 0);
		result.addDefaultKeypress ('O', ModifierKeys::commandModifier);
		result.setActive(!acquisitionStarted);
		break;

	case saveConfiguration:
		result.setInfo("Save configuration", "Save the current processor graph.", "General", 0);
		result.addDefaultKeypress ('S', ModifierKeys::commandModifier);
		result.setActive(!acquisitionStarted);
		break;

	case undo:
		result.setInfo("Undo", "Undo the last action.", "General", 0);
		result.addDefaultKeypress ('Z', ModifierKeys::commandModifier);
		result.setActive(false);
		break;

	case redo:
		result.setInfo("Redo", "Undo the last action.", "General", 0);
		result.addDefaultKeypress ('Y', ModifierKeys::commandModifier);
		result.setActive(false);
		break;

	case copySignalChain:
		result.setInfo("Copy", "Copy a portion of the signal chain.", "General", 0);
		result.addDefaultKeypress ('C', ModifierKeys::commandModifier);
		result.setActive(false);
		break;

	case pasteSignalChain:
		result.setInfo("Paste", "Paste a portion of the signal chain.", "General", 0);
		result.addDefaultKeypress ('V', ModifierKeys::commandModifier);
		result.setActive(false);
		break;

	case clearSignalChain:
		result.setInfo("Clear signal chain", "Clear the current signal chain.", "General", 0);
		result.addDefaultKeypress (KeyPress::backspaceKey, ModifierKeys::commandModifier);
		result.setActive(!getEditorViewport()->isSignalChainEmpty() && !acquisitionStarted);
		break;

	case toggleProcessorList:
		result.setInfo("Processor List", "Show/hide Processor List.", "General", 0);
		result.addDefaultKeypress ('P', ModifierKeys::shiftModifier);
		result.setTicked(processorList->isOpen());
		break;

	case toggleSignalChain:
		result.setInfo("Signal Chain", "Show/hide Signal Chain.", "General", 0);
		result.addDefaultKeypress ('S', ModifierKeys::shiftModifier);
		result.setTicked(editorViewportButton->isOpen());
		break;

	case toggleFileInfo:
		result.setInfo("File Info", "Show/hide File Info.", "General", 0);
		result.addDefaultKeypress ('F', ModifierKeys::shiftModifier);
		result.setTicked(controlPanel->isOpen());
		break;

	case showHelp:
		result.setInfo("Show help...", "Show some freakin' help.", "General", 0);
		result.setActive(false);
		break;

	default:
		break;
	};

}

bool UIComponent::perform (const InvocationInfo& info)
{

	switch (info.commandID)
	{
	case openConfiguration:
	{
		sendActionMessage(getEditorViewport()->loadState());
		break;
	}
	case saveConfiguration:
	{
		sendActionMessage(getEditorViewport()->saveState());
		break;
	}
	case clearSignalChain:
		getEditorViewport()->clearSignalChain();
		break;

	case showHelp:
		std::cout << "SHOW ME SOME HELP!" << std::endl;
		break;

	case toggleProcessorList:
		processorList->toggleState();
		break;

	case toggleFileInfo:
		controlPanel->toggleState();
		break;

	case toggleSignalChain:
		editorViewportButton->toggleState();
		break;
		
	default:
		break;

	}

	return true;

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

EditorViewportButton::EditorViewportButton(UIComponent* ui) : UI(ui)
{
	open = true;
}

EditorViewportButton::~EditorViewportButton()
{
	
}

void EditorViewportButton::newOpenGLContextCreated()
{
	
	setUp2DCanvas();
	activateAntiAliasing();

	setClearColor(darkgrey);

	//glClearColor(0.23f, 0.23f, 0.23f, 1.0f); 

}


void EditorViewportButton::renderOpenGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawName();
	drawButton();
}

void EditorViewportButton::drawName()
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glRasterPos2f(8.0/getWidth(),0.75f);
	getFont(cpmono_light)->FaceSize(23);
	getFont(cpmono_light)->Render("SIGNAL CHAIN");
	
}

void EditorViewportButton::drawButton()
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glLineWidth(1.0f);

	glBegin(GL_LINE_LOOP);

	if (open)
	{
		glVertex2f(0.90,0.65);
		glVertex2f(0.925,0.35);
	} else {
		glVertex2f(0.95,0.35);
		glVertex2f(0.90,0.5);
	}
	glVertex2f(0.95,0.65);
	glEnd();

}

void EditorViewportButton::mouseDown(const MouseEvent& e)
{
	open = !open;
	UI->childComponentChanged();
	repaint();

}

void EditorViewportButton::toggleState()
{
	open = !open;
	UI->childComponentChanged();
	repaint();
}
