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

	processorGraph->loadState();

#if JUCE_MAC
	setMacMainMenu(this);
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
	
	if (dataViewport != 0) {
		if (processorList->isOpen() && editorViewportButton->isOpen())
			dataViewport->setBounds(202,40,w-207,h-235);
		else if (!processorList->isOpen() && editorViewportButton->isOpen())
			dataViewport->setBounds(6,40,w-11,h-235);
		else if (processorList->isOpen() && !editorViewportButton->isOpen())
			dataViewport->setBounds(202,40,w-207,h-85);
		else	
			dataViewport->setBounds(6,40,w-11,h-85);
	}

	if (editorViewportButton != 0)
	{
		editorViewportButton->setBounds(w-230, h-40, 225, 35);
	}
	
	if (editorViewport != 0) {
		if (editorViewportButton->isOpen() && !editorViewport->isVisible())
			editorViewport->setVisible(true);
		else if (!editorViewportButton->isOpen() && editorViewport->isVisible())
			editorViewport->setVisible(false);

		editorViewport->setBounds(6,h-190,w-11,150);
	}

	if (controlPanel != 0)
		controlPanel->setBounds(201,6,w-210,32);

	if (processorList != 0) {
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

	// for debugging purposes:
	if (false) {
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

	const char* const names[] = { "File", "Edit", "Help", 0 };

    return StringArray (names);

}

const PopupMenu UIComponent::getMenuForIndex(int menuIndex, const String& menuName)
{
	 ApplicationCommandManager* commandManager = &(mainWindow->commandManager);

     PopupMenu menu;

     if (menuIndex == 0)
     {
     	// menu.addItem (1, "Load configuration");
     	// menu.addItem (2, "Save configuration");
     	menu.addCommandItem (commandManager, loadConfiguration);
        menu.addCommandItem (commandManager, saveConfiguration);
        menu.addSeparator();
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
     } else if (menuIndex == 1)
     {
     	menu.addCommandItem (commandManager, clearSignalChain);
     	//menu.addItem (1, "Clear signal chain");
     } else if (menuIndex == 2)
     {
     	menu.addCommandItem (commandManager, showHelp);
     	//menu.addItem (1, "Show help...");
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
        // case, there probably isn't one, but it's best to use this method in your own apps).
	return findFirstTargetParentComponent();
}

void UIComponent::getAllCommands (Array <CommandID>& commands)
{
	 const CommandID ids[] = {loadConfiguration,
	 					      saveConfiguration,
	 					      clearSignalChain,
	 					      showHelp,
	 					      moveSelectionLeft,
	 					      moveSelectionRight};

	 commands.addArray (ids, numElementsInArray (ids));

}

void UIComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{

	switch (commandID)
	{
	case loadConfiguration:
		result.setInfo("Load configuration", "Load a saved processor graph.", "General", 0);
		result.addDefaultKeypress (T('1'), ModifierKeys::commandModifier);
		break;

	case saveConfiguration:
		result.setInfo("Save configuration", "Save the current processor graph.", "General", 0);
		break;

	case clearSignalChain:
		result.setInfo("Clear signal chain", "Clear the current signal chain.", "General", 0);
		break;

	case showHelp:
		result.setInfo("Show help...", "Show some freakin' help.", "General", 0);
		break;

	case moveSelectionLeft:
		result.setInfo("Move left", "Move left", "General", 0);
		result.addDefaultKeypress (KeyPress::leftKey, 0);// ModifierKeys::noModifiers);
		break;

	case moveSelectionRight:
		result.setInfo("Move right", "Move right", "General", 0);
		result.addDefaultKeypress (KeyPress::rightKey, 0);//ModifierKeys::noModifiers);
		break;

	// case quit:
	// 	result.setInfo("Quit", "Quit", "General", 0);
	// 	result.addDefaultKeypress (T('Q'), ModifierKeys::commandModifier);//ModifierKeys::noModifiers);
	// 	break;

	default:
		break;
	};

}

bool UIComponent::perform (const InvocationInfo& info)
{
	switch (info.commandID)
	{
	case loadConfiguration:
		std::cout << "LOAD THAT CONFIG!" << std::endl;
		break;

	case saveConfiguration:
		std::cout << "SAVE THAT CONFIG!" << std::endl;
		break;

	case clearSignalChain:
		std::cout << "CLEAR THAT SIGNAL CHAIN!" << std::endl;
		break;

	case showHelp:
		std::cout << "SHOW ME SOME HELP!" << std::endl;
		break;

	case moveSelectionRight:
		std::cout << "MOVE RIGHT!" << std::endl;
		break;

	case moveSelectionLeft:
		std::cout << "MOVE LEFT!" << std::endl;
		break;

	// case quit:
	// 	std::cout << "REQUEST TO QUIT." << std::endl;
	// 	mainWindow->closeButtonPressed();
	// 	break;
		
	default:
		break;

	}

	return true;

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

EditorViewportButton::EditorViewportButton(UIComponent* ui) : UI(ui)
{
	open = true;

	const unsigned char* buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_light_otf);
	size_t bufferSize = BinaryData::cpmono_light_otfSize;

	font = new FTPixmapFont(buffer, bufferSize);
	
}

EditorViewportButton::~EditorViewportButton()
{
	
}

void EditorViewportButton::newOpenGLContextCreated()
{
	
	glMatrixMode (GL_PROJECTION);

	glLoadIdentity();
	glOrtho (0, 1, 1, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);

	glClearColor(0.23f, 0.23f, 0.23f, 1.0f); 

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	font->FaceSize(23);
	font->Render("SIGNAL CHAIN");
	
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