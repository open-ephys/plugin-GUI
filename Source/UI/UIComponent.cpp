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

#include "UIComponent.h"
#include "../Processors/PluginManager/PluginManager.h"
#include <stdio.h>

#include "InfoLabel.h"
#include "ControlPanel.h"
#include "ProcessorList.h"
#include "EditorViewport.h"
#include "MessageCenterButton.h"
#include "DataViewport.h"
#include "../Processors/MessageCenter/MessageCenterEditor.h"
#include "GraphViewer.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "../Audio/AudioComponent.h"
#include "../MainWindow.h"

	UIComponent::UIComponent(MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio_)
: mainWindow(mainWindow_), processorGraph(pgraph), audio(audio_), messageCenterIsCollapsed(true)

{

	processorGraph->createDefaultNodes();

	messageCenterEditor = (MessageCenterEditor*) processorGraph->getMessageCenter()->createEditor();
	addActionListener(messageCenterEditor);
	
	LOGD("Created message center.");

	infoLabel = new InfoLabel();
	LOGD("Created info label.");

	graphViewer = new GraphViewer();
	LOGD("Created graph viewer.");

	dataViewport = new DataViewport();
	addChildComponent(dataViewport);
	dataViewport->addTabToDataViewport("Info", infoLabel);
	dataViewport->addTabToDataViewport("Graph", graphViewer->getGraphViewport());

	LOGD("Created data viewport.");

    signalChainTabComponent = new SignalChainTabComponent();
    addAndMakeVisible(signalChainTabComponent);
    
	editorViewport = new EditorViewport(signalChainTabComponent);
	//addAndMakeVisible(editorViewport);
    
	LOGD("Created editor viewport.");

	editorViewportButton = new EditorViewportButton(this);
	addAndMakeVisible(editorViewportButton);

	controlPanel = new ControlPanel(processorGraph, audio);
	addAndMakeVisible(controlPanel);
    
	LOGD("Created control panel.");

	processorList = new ProcessorList(&processorListViewport);
	processorListViewport.setViewedComponent(processorList,false);
	processorListViewport.setScrollBarsShown(false,false);
	addAndMakeVisible(&processorListViewport);
    
    messageCenterButton.addListener(this);
    addAndMakeVisible(messageCenterEditor);
    addAndMakeVisible(&messageCenterButton);
    
	processorList->setVisible(true);
	processorList->setBounds(0,0,195,processorList->getTotalHeight());
	LOGD("Created filter list.");

	pluginManager = new PluginManager();
	LOGD("Created plugin manager");

	setBounds(0,0,500,400);

	AccessClass::setUIComponent(this);

	getPluginManager()->loadAllPlugins();

	getProcessorList()->fillItemList();
	controlPanel->updateRecordEngineList();

	processorGraph->updateBufferSize(); // needs to happen after processorGraph gets the right pointers

#if JUCE_MAC
	MenuBarModel::setMacMainMenu(this);
	mainWindow->setMenuBar(0);
#else
	mainWindow->setMenuBar(this);
	mainWindow->getMenuBarComponent()->setName("MainMenu");
#endif

}

UIComponent::~UIComponent()
{
	dataViewport->destroyTab(0); // get rid of tab for InfoLabel

	if (pluginInstaller)
	{
		pluginInstaller->setVisible(false);
		delete pluginInstaller;
	}

	AccessClass::shutdownBroadcaster();
}

/** Returns a pointer to the EditorViewport. */
EditorViewport* UIComponent::getEditorViewport()
{
	return editorViewport;
}

/** Returns a pointer to the ProcessorList. */
ProcessorList* UIComponent::getProcessorList()
{
	return processorList;
}

/** Returns a pointer to the DataViewport. */
DataViewport* UIComponent::getDataViewport()
{
	return dataViewport;
}

/** Returns a pointer to the ProcessorGraph. */
ProcessorGraph* UIComponent::getProcessorGraph()
{
	return processorGraph;
}

/** Returns a pointer to the GraphViewer. */
GraphViewer* UIComponent::getGraphViewer()
{
	return graphViewer;
}


/** Returns a pointer to the ControlPanel. */
ControlPanel* UIComponent::getControlPanel()
{
	return controlPanel;
}

/** Returns a pointer to the MessageCenterEditor. */
MessageCenterEditor* UIComponent::getMessageCenter()
{
	return messageCenterEditor;
}

/** Returns a pointer to the UIComponent. */
UIComponent* UIComponent::getUIComponent()
{
	return this;
}

/** Returns a pointer to the AudioComponent. */
AudioComponent* UIComponent::getAudioComponent()
{
	return audio;
}

PluginManager* UIComponent::getPluginManager()
{
	return pluginManager;
}

PluginInstaller* UIComponent::getPluginInstaller()
{
    if (pluginInstaller == nullptr)
	{
		pluginInstaller = new PluginInstaller(this->mainWindow, false);
	}
	return pluginInstaller;
}

void UIComponent::buttonClicked(Button* button)
{
    if (button == &messageCenterButton)
    {
        messageCenterButton.switchState();
        
        messageCenterIsCollapsed = !messageCenterIsCollapsed;
        
        resized();
    }
}

void UIComponent::resized()
{

	int w = getWidth();
	int h = getHeight();

	if (editorViewportButton != nullptr)
	{
		editorViewportButton->setBounds(w-230, h-40, 225, 35);

		if (h < 300 && editorViewportButton->isOpen())
			editorViewportButton->toggleState();

		if (h < 200)
			editorViewportButton->setBounds(w-230,h-40+200-h,225,35);
	}

	if (signalChainTabComponent != nullptr)
	{
		if (editorViewportButton->isOpen() && !signalChainTabComponent->isVisible())
        {
            signalChainTabComponent->setVisible(true);
        }
			
		else if (!editorViewportButton->isOpen() && signalChainTabComponent->isVisible())
        {
            signalChainTabComponent->setVisible(false);
        }
			
		signalChainTabComponent->setBounds(6,h-200,w-11,160);
	}

	if (controlPanel != nullptr)
	{

		int controlPanelWidth;
		int addHeight = 0;
		int leftBound;

		if (!editorViewport->isSignalChainLocked())
		{
			if (w >= 460)
			{
				leftBound = 202;
				controlPanelWidth = w - 210;
			}
			else
			{
				leftBound = w - 258;
				controlPanelWidth = w - leftBound;
			}
		}
		else {
			leftBound = 6;
			controlPanelWidth = w - 12;
		}

		if (controlPanelWidth < 750)
		{
			addHeight = 750-controlPanelWidth;

			if (addHeight > 32)
				addHeight = 32;
		}

		if (controlPanelWidth < 570)
		{
			addHeight = 32 + 570-controlPanelWidth;

			if (addHeight > 64)
				addHeight = 64;
		}

		if (controlPanel->isOpen())
			controlPanel->setBounds(leftBound,6,controlPanelWidth,64+addHeight);
		else
			controlPanel->setBounds(leftBound,6,controlPanelWidth,32+addHeight);
	}

	if (processorList != nullptr)
	{
		if (editorViewport->isSignalChainLocked())
		{
			processorList->setVisible(false);
		}
			
		else {

			processorList->setVisible(true);
			
			if (processorList->isOpen())
			{
				if (editorViewportButton->isOpen())
					processorListViewport.setBounds(5, 5, 195, h - 210);
				else
					processorListViewport.setBounds(5, 5, 195, h - 50);

				processorListViewport.setScrollBarsShown(false, false, true, false);

			}
			else
			{
				processorListViewport.setBounds(5, 5, 195, 34);
				processorListViewport.setScrollBarsShown(false, false);
				processorListViewport.setViewPosition(0, 0);
			}

			if (w < 460)
				processorListViewport.setBounds(5 - 460 + getWidth(), 5, 195, processorList->getHeight());
		}
		
	}

	if (dataViewport != nullptr)
	{
		int left, top, width, height;
		left = 6;
		top = 40;

		if (processorList->isOpen() && !editorViewport->isSignalChainLocked())
			left = processorListViewport.getX()+processorListViewport.getWidth()+2;
		else
			left = 6;

		top = controlPanel->getHeight()+8;

		if (editorViewportButton->isOpen())
			height = h - top - 205;
		else
			height = h - top - 45;

		width = w - left - 5;

		dataViewport->setBounds(left, top, width, height);

		if (h < 200)
			dataViewport->setVisible(false);
		else
			dataViewport->setVisible(true);

	}

	if (messageCenterEditor != nullptr)
	{
        if (messageCenterIsCollapsed)
        {
            messageCenterEditor->collapse();
            messageCenterEditor->setBounds(6,h-35,w-241,30);
            
        } else {
            messageCenterEditor->expand();
            messageCenterEditor->setBounds(6,h-305,w-241,300);
        }
	}
    

    //if (messageCenterIsCollapsed)
   // {
    messageCenterButton.setBounds((w-241)/2,h-35,30,30);
   // } else {
   //     messageCenterButton.setBounds((w-241)/2,h-305,30,30);
   // }

	// for debugging purposes:
	if (false)
	{
		dataViewport->setVisible(false);
		editorViewport->setVisible(false);
		processorList->setVisible(false);
		messageCenterEditor->setVisible(false);
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

StringArray UIComponent::getMenuBarNames()
{

	const char* const names[] = { "File", "Edit", "View", "Help", 0 };

	return StringArray(names);

}

PopupMenu UIComponent::getMenuForIndex(int menuIndex, const String& menuName)
{
	ApplicationCommandManager* commandManager = &(mainWindow->commandManager);

	PopupMenu menu;

	if (menuIndex == 0)
	{
		menu.addCommandItem(commandManager, openSignalChain);
        menu.addSeparator();
		menu.addCommandItem(commandManager, saveSignalChain);
		menu.addCommandItem(commandManager, saveSignalChainAs);
        menu.addSeparator();
		menu.addCommandItem(commandManager, reloadOnStartup);
		menu.addSeparator();
		menu.addCommandItem(commandManager, toggleHttpServer);
		menu.addSeparator();
		menu.addCommandItem(commandManager, openDefaultConfigWindow);
		menu.addSeparator();
		menu.addCommandItem(commandManager, openPluginInstaller);

#if !JUCE_MAC
		menu.addSeparator();
		menu.addCommandItem(commandManager, StandardApplicationCommandIDs::quit);
#endif

	}
	else if (menuIndex == 1)
	{
		menu.addCommandItem(commandManager, undo);
		menu.addCommandItem(commandManager, redo);
		menu.addSeparator();
		menu.addCommandItem(commandManager, copySignalChain);
		menu.addCommandItem(commandManager, pasteSignalChain);
		menu.addSeparator();
		menu.addCommandItem(commandManager, clearSignalChain);
		menu.addSeparator();
		menu.addCommandItem(commandManager, lockSignalChain);

	}
	else if (menuIndex == 2)
	{

		PopupMenu clockMenu;
		clockMenu.addCommandItem(commandManager, setClockModeDefault);
		clockMenu.addCommandItem(commandManager, setClockModeHHMMSS);

		menu.addCommandItem(commandManager, toggleProcessorList);
		menu.addCommandItem(commandManager, toggleSignalChain);
		menu.addCommandItem(commandManager, toggleFileInfo);
		menu.addSeparator();
		menu.addSubMenu("Clock mode", clockMenu);
		menu.addSeparator();
		menu.addCommandItem(commandManager, resizeWindow);

	}
	else if (menuIndex == 3)
	{
		menu.addCommandItem(commandManager, showHelp);
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

void UIComponent::getAllCommands(Array <CommandID>& commands)
{
	const CommandID ids[] = {openSignalChain,
		saveSignalChain,
		saveSignalChainAs,
        loadPluginSettings,
        savePluginSettings,
		reloadOnStartup,
		undo,
		redo,
		copySignalChain,
		pasteSignalChain,
		clearSignalChain,
		lockSignalChain,
		toggleProcessorList,
		toggleSignalChain,
		toggleHttpServer,
		toggleFileInfo,
		setClockModeDefault,
		setClockModeHHMMSS,
		showHelp,
		resizeWindow,
		openPluginInstaller,
		openDefaultConfigWindow
	};

	commands.addArray(ids, numElementsInArray(ids));

}

void UIComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{

	bool acquisitionStarted = getAudioComponent()->callbacksAreActive();

	switch (commandID)
	{
		case openSignalChain:
			result.setInfo("Open...", "Open a saved signal chain.", "General", 0);
			result.addDefaultKeypress('O', ModifierKeys::commandModifier);
			result.setActive(!acquisitionStarted);
			break;

		case saveSignalChain:
			result.setInfo("Save", "Save the current signal chain.", "General", 0);
			result.addDefaultKeypress('S', ModifierKeys::commandModifier);
			break;

		case saveSignalChainAs:
			result.setInfo("Save as...", "Save the current signal chain with a new name.", "General", 0);
			result.addDefaultKeypress('S', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
			break;
            
        case loadPluginSettings:
            result.setInfo("Load plugin settings...", "Load saved plugin settings.", "General", 0);
            result.setActive(!acquisitionStarted);
            break;

        case savePluginSettings:
            result.setInfo("Save plugin settings...", "Save the settings of the selected plugin.", "General", 0);
            break;

		case reloadOnStartup:
			result.setInfo("Reload on startup", "Load the last used configuration on startup.", "General", 0);
			result.setActive(!acquisitionStarted);
			result.setTicked(mainWindow->shouldReloadOnStartup);
			break;

		case toggleHttpServer:
			result.setInfo("Enable HTTP Server", "Enable the HTTP server on port 37497.", "General", 0);
			result.setActive(!acquisitionStarted);
			result.setTicked(mainWindow->shouldEnableHttpServer);
			break;

		case undo:
			result.setInfo("Undo", "Undo the last action.", "General", 0);
			result.addDefaultKeypress('Z', ModifierKeys::commandModifier);
			result.setActive(!acquisitionStarted && getEditorViewport()->undoManager.canUndo() && !getEditorViewport()->isSignalChainLocked());
			break;

		case redo:
			result.setInfo("Redo", "Undo the last action.", "General", 0);
			result.addDefaultKeypress('Z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
			result.setActive(!acquisitionStarted && getEditorViewport()->undoManager.canRedo() && !getEditorViewport()->isSignalChainLocked());
			break;

		case copySignalChain:
			result.setInfo("Copy", "Copy selected processors.", "General", 0);
			result.addDefaultKeypress('C', ModifierKeys::commandModifier);
			result.setActive(!acquisitionStarted && getEditorViewport()->editorIsSelected() && !getEditorViewport()->isSignalChainLocked());
			break;

		case pasteSignalChain:
			result.setInfo("Paste", "Paste processors.", "General", 0);
			result.addDefaultKeypress('V', ModifierKeys::commandModifier);
			result.setActive(!acquisitionStarted && getEditorViewport()->canPaste() && !getEditorViewport()->isSignalChainLocked());
			break;

		case clearSignalChain:
			result.setInfo("Clear signal chain", "Clear the current signal chain.", "General", 0);
			result.addDefaultKeypress(KeyPress::backspaceKey, ModifierKeys::commandModifier);
			result.setActive(!getEditorViewport()->isSignalChainEmpty() && !acquisitionStarted && !getEditorViewport()->isSignalChainLocked());
			break;

		case lockSignalChain:
			result.setInfo("Lock signal chain", "Disable signal chain edits.", "General", 0);
			result.addDefaultKeypress('L', ModifierKeys::commandModifier);
			result.setTicked(getEditorViewport()->isSignalChainLocked());
			break;

		case toggleProcessorList:
			result.setInfo("Processor List", "Show/hide Processor List.", "General", 0);
			result.addDefaultKeypress('P', ModifierKeys::shiftModifier);
			result.setActive(!editorViewport->isSignalChainLocked());
			result.setTicked(processorList->isOpen());
			break;

		case toggleSignalChain:
			result.setInfo("Signal Chain", "Show/hide Signal Chain.", "General", 0);
			result.addDefaultKeypress('S', ModifierKeys::shiftModifier);
			result.setTicked(editorViewportButton->isOpen());
			break;

		case toggleFileInfo:
			result.setInfo("File Info", "Show/hide File Info.", "General", 0);
			result.addDefaultKeypress('F', ModifierKeys::shiftModifier);
			result.setTicked(controlPanel->isOpen());
			break;

		case setClockModeDefault:
			result.setInfo("Default", "Set clock mode to default.", "General", 0);
			result.setTicked(controlPanel->clock->getMode() == Clock::DEFAULT);
			break;

		case setClockModeHHMMSS:
			result.setInfo("HH:MM:SS", "Set clock mode to HH:MM:SS.", "General", 0);
			result.setTicked(controlPanel->clock->getMode() == Clock::HHMMSS);
			break;

		case openPluginInstaller:
			result.setInfo("Plugin Installer", "Launch the plugin installer.", "General", 0);
			result.addDefaultKeypress('P', ModifierKeys::commandModifier);
			break;
		
		case openDefaultConfigWindow:
			result.setInfo("Load a default config", "Load a default configuration", "General", 0);
			result.addDefaultKeypress('D', ModifierKeys::commandModifier);
			break;

		case showHelp:
			result.setInfo("Online documentation...", "Launch the GUI's documentation website in a browser.", "General", 0);
			result.setActive(true);
			break;

		case resizeWindow:
			result.setInfo("Reset window bounds", "Reset window bounds", "General", 0);
			break;

		default:
			break;
	};

}

bool UIComponent::perform(const InvocationInfo& info)
{

	switch (info.commandID)
	{
		case openSignalChain:
			{
				FileChooser fc("Choose a settings file to load...",
						CoreServices::getDefaultUserSaveDirectory(),
						"*",
						true);

				if (fc.browseForFileToOpen())
				{
					currentConfigFile = fc.getResult();
					getEditorViewport()->loadState(currentConfigFile);
				}
				else
				{
					sendActionMessage("No file selected.");
				}

				break;
			}
		case saveSignalChain:
			{

				if (currentConfigFile.exists())
				{
					sendActionMessage(getEditorViewport()->saveState(currentConfigFile));
				}
				else
				{
					FileChooser fc("Choose the file name...",
							CoreServices::getDefaultUserSaveDirectory(),
							"*",
							true);

					if (fc.browseForFileToSave(true))
					{
						currentConfigFile = fc.getResult();
						LOGD(currentConfigFile.getFileName());
						sendActionMessage(getEditorViewport()->saveState(currentConfigFile));
					}
					else
					{
						sendActionMessage("No file chosen.");
					}
				}

				break;
			}

		case saveSignalChainAs:
			{

				FileChooser fc("Choose the file name...",
						CoreServices::getDefaultUserSaveDirectory(),
						"*",
						true);

				if (fc.browseForFileToSave(true))
				{
					currentConfigFile = fc.getResult();
					LOGD(currentConfigFile.getFileName());
					sendActionMessage(getEditorViewport()->saveState(currentConfigFile));
				}
				else
				{
					sendActionMessage("No file chosen.");
				}

				break;
			}
            
        case loadPluginSettings:
        {
            FileChooser fc("Choose a settings file to load...",
                    CoreServices::getDefaultUserSaveDirectory(),
                    "*",
                    true);

            if (fc.browseForFileToOpen())
            {
                currentConfigFile = fc.getResult();
                sendActionMessage(getEditorViewport()->loadPluginState(currentConfigFile));
            }
            else
            {
                sendActionMessage("No file selected.");
            }

            break;
        }
        case savePluginSettings:
        {

            FileChooser fc("Choose the file name...",
                    CoreServices::getDefaultUserSaveDirectory(),
                    "*",
                    true);

            if (fc.browseForFileToSave(true))
            {
                currentConfigFile = fc.getResult();
                LOGD(currentConfigFile.getFileName());
                sendActionMessage(getEditorViewport()->savePluginState(currentConfigFile));
            }
            else
            {
                sendActionMessage("No file chosen.");
            }

            break;
        }

		case reloadOnStartup:
			{
				mainWindow->shouldReloadOnStartup = !mainWindow->shouldReloadOnStartup;

			}
			break;

		case toggleHttpServer:

			mainWindow->shouldEnableHttpServer = !mainWindow->shouldEnableHttpServer;

			if (mainWindow->shouldEnableHttpServer) {
				mainWindow->enableHttpServer();
			}
			else {
				mainWindow->disableHttpServer();
			}
			break;

        case undo:
            {
                getEditorViewport()->undo();
                break;
            }
            
        case redo:
            {
                getEditorViewport()->redo();
                break;
            }
            
        case copySignalChain:
            {
                getEditorViewport()->copySelectedEditors();
                break;
            }
            
        case pasteSignalChain:
            {
                getEditorViewport()->paste();
                break;
            }
                
		case clearSignalChain:
			{
				getEditorViewport()->clearSignalChain();
				break;
			}

		case lockSignalChain:
		{
			
			if (getEditorViewport()->isSignalChainLocked())
			{
				getEditorViewport()->lockSignalChain(false);
				resized();
				//processorList->unlock();
				
			}
			else {
				getEditorViewport()->lockSignalChain(true);
				resized();
				//processorList->lock();
			}
			
			break;
		}


		case showHelp:
			{
				URL url = URL("https://open-ephys.github.io/gui-docs/");
				url.launchInDefaultBrowser();
				break;
			}

		case toggleProcessorList:
			processorList->toggleState();
			break;

		case toggleFileInfo:
			controlPanel->toggleState();
			break;

		case toggleSignalChain:
			editorViewportButton->toggleState();
			break;

		case resizeWindow:
			mainWindow->centreWithSize(1200, 800);
			break;

		case setClockModeDefault:
			controlPanel->clock->setMode(Clock::DEFAULT);
			break;

		case setClockModeHHMMSS:
			controlPanel->clock->setMode(Clock::HHMMSS);
			break;

		case openPluginInstaller:
			{
				if (pluginInstaller == nullptr)
				{
					pluginInstaller = new PluginInstaller(this->mainWindow);
				}
				pluginInstaller->setVisible(true);
				pluginInstaller->toFront(true);
				break;
			}

		case openDefaultConfigWindow:
			{
				defaultConfigWindow = std::make_unique<DefaultConfigWindow>(this->mainWindow);
				break;
			}

		default:
			break;

	}

	return true;

}


void UIComponent::saveStateToXml(XmlElement* xml)
{
	XmlElement* uiComponentState = xml->createNewChildElement("UICOMPONENT");
	uiComponentState->setAttribute("isProcessorListOpen",processorList->isOpen());
	uiComponentState->setAttribute("isEditorViewportOpen",editorViewportButton->isOpen());
}

void UIComponent::loadStateFromXml(XmlElement* xml)
{
	for (auto* xmlNode : xml->getChildWithTagNameIterator("UICOMPONENT"))
	{

        bool isProcessorListOpen = xmlNode->getBoolAttribute("isProcessorListOpen");
        bool isEditorViewportOpen = xmlNode->getBoolAttribute("isEditorViewportOpen");

        if (!isProcessorListOpen)
        {
            processorList->toggleState();
        }

        if (!isEditorViewportOpen)
        {
            editorViewportButton->toggleState();
        }

	}
}

StringArray UIComponent::getRecentlyUsedFilenames()
{
	return controlPanel->getRecentlyUsedFilenames();
}

void UIComponent::setRecentlyUsedFilenames(const StringArray& filenames)
{
	controlPanel->setRecentlyUsedFilenames(filenames);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

EditorViewportButton::EditorViewportButton(UIComponent* ui) : UI(ui)
{
	open = true;

	buttonFont = Font("CP Mono", "Light", 25);

}

EditorViewportButton::~EditorViewportButton()
{

}


void EditorViewportButton::paint(Graphics& g)
{

	g.fillAll(Colour(58,58,58));

	g.setColour(Colours::white);
	g.setFont(buttonFont);
	g.drawText("SIGNAL CHAIN", 10, 0, getWidth(), getHeight(), Justification::left, false);

	g.setColour(Colours::white);

	Path p;

	float h = getHeight();
	float w = getWidth()-5;

	if (open)
	{
		p.addTriangle(w-h+0.3f*h, 0.7f*h,
				w-h+0.5f*h, 0.3f*h,
				w-h+0.7f*h, 0.7f*h);
	}
	else
	{
		p.addTriangle(w-h+0.3f*h, 0.5f*h,
				w-h+0.7f*h, 0.3f*h,
				w-h+0.7f*h, 0.7f*h);
	}

	PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

	g.strokePath(p, pst);

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
