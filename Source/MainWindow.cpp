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

#include "MainWindow.h"
#include "UI/UIComponent.h"
#include "UI/EditorViewport.h"
#include <stdio.h>
//-----------------------------------------------------------------------


	MainWindow::MainWindow(const File& fileToLoad)
: DocumentWindow(JUCEApplication::getInstance()->getApplicationName(),
		Colour(Colours::black),
		DocumentWindow::allButtons)
{

	setResizable(true,      // isResizable
			false);   // useBottomCornerRisizer -- doesn't work very well

	shouldReloadOnStartup = true;

	// Create ProcessorGraph and AudioComponent, and connect them.
	// Callbacks will be set by the play button in the control panel

	processorGraph = new ProcessorGraph();
	std::cout << std::endl;
	std::cout << "Created processor graph." << std::endl;
	std::cout << std::endl;

	audioComponent = new AudioComponent();
	std::cout << "Created audio component." << std::endl;

	audioComponent->connectToProcessorGraph(processorGraph);

	setContentOwned(new UIComponent(this, processorGraph, audioComponent), true);

	UIComponent* ui = (UIComponent*) getContentComponent();

	commandManager.registerAllCommandsForTarget(ui);
	commandManager.registerAllCommandsForTarget(JUCEApplication::getInstance());

	ui->setApplicationCommandManagerToWatch(&commandManager);

	addKeyListener(commandManager.getKeyMappings());

	loadWindowBounds();
	setUsingNativeTitleBar(true);
	Component::addToDesktop(getDesktopWindowStyleFlags());  // prevents the maximize
	// button from randomly disappearing
	setVisible(true);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(500, 500, 10000, 10000);

    if (!fileToLoad.getFullPathName().isEmpty())
    {
        ui->getEditorViewport()->loadState(fileToLoad);
    }
	else if (shouldReloadOnStartup)
	{
		File lastConfig = CoreServices::getSavedStateDirectory().getChildFile("lastConfig.xml");
		File recoveryConfig = CoreServices::getSavedStateDirectory().getChildFile("recoveryConfig.xml");

		if(lastConfig.existsAsFile())
		{
			if(compareConfigFiles(lastConfig, recoveryConfig))
			{
				ui->getEditorViewport()->loadState(lastConfig);
			}
			else
			{
				bool loadRecovery = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Reloading Settings",
																"It looks like the GUI crashed during your last run, " 
																"causing the configured settings to not save properly. "
																"Do you want to load the recovery config instead?",
																"Yes", "No");
				
				if(loadRecovery)
					ui->getEditorViewport()->loadState(recoveryConfig);
				else
					ui->getEditorViewport()->loadState(lastConfig);
			}
		}
	}

}

MainWindow::~MainWindow()
{

	if (audioComponent->callbacksAreActive())
	{
		audioComponent->endCallbacks();
		processorGraph->disableProcessors();
	}

	saveWindowBounds();

	audioComponent->disconnectProcessorGraph();
	UIComponent* ui = (UIComponent*) getContentComponent();
	ui->disableDataViewport();

	File lastConfig = CoreServices::getSavedStateDirectory().getChildFile("lastConfig.xml");
	File recoveryConfig = CoreServices::getSavedStateDirectory().getChildFile("recoveryConfig.xml");
	ui->getEditorViewport()->saveState(lastConfig);
	ui->getEditorViewport()->saveState(recoveryConfig);

	setMenuBar(0);

#if JUCE_MAC
	MenuBarModel::setMacMainMenu(0);
#endif

}

void MainWindow::closeButtonPressed()
{

	JUCEApplication::getInstance()->systemRequestedQuit();

}

void MainWindow::shutDownGUI()
{
	if (audioComponent->callbacksAreActive())
	{
		audioComponent->endCallbacks();
	}

	processorGraph->disableProcessors();
}

void MainWindow::saveWindowBounds()
{
	std::cout << std::endl;
	std::cout << "Saving window bounds." << std::endl;
	std::cout << std::endl;

	File file = CoreServices::getSavedStateDirectory().getChildFile("windowState.xml");

	XmlElement* xml = new XmlElement("MAINWINDOW");

	xml->setAttribute("version", JUCEApplication::getInstance()->getApplicationVersion());
	xml->setAttribute("shouldReloadOnStartup", shouldReloadOnStartup);

	XmlElement* bounds = new XmlElement("BOUNDS");
	bounds->setAttribute("x",getScreenX());
	bounds->setAttribute("y",getScreenY());
	bounds->setAttribute("w",getContentComponent()->getWidth());
	bounds->setAttribute("h",getContentComponent()->getHeight());
	bounds->setAttribute("fullscreen", isFullScreen());

	xml->addChildElement(bounds);

	XmlElement* recentDirectories = new XmlElement("RECENTDIRECTORYNAMES");

	UIComponent* ui = (UIComponent*) getContentComponent();

	StringArray dirs = ui->getRecentlyUsedFilenames();

	for (int i = 0; i < dirs.size(); i++)
	{
		XmlElement* directory = new XmlElement("DIRECTORY");
		directory->setAttribute("name", dirs[i]);
		recentDirectories->addChildElement(directory);
	}

	xml->addChildElement(recentDirectories);

	String error;

	if (! xml->writeToFile(file, String::empty))
		error = "Couldn't write to file";

	delete xml;
}

void MainWindow::loadWindowBounds()
{

	std::cout << std::endl;
	std::cout << "Loading window bounds." << std::endl;
	std::cout << std::endl;

	File file = CoreServices::getSavedStateDirectory().getChildFile("windowState.xml");

	XmlDocument doc(file);
	XmlElement* xml = doc.getDocumentElement();

	if (xml == 0 || ! xml->hasTagName("MAINWINDOW"))
	{

		std::cout << "File not found." << std::endl;
		delete xml;
		centreWithSize(800, 600);

	}
	else
	{

		String description;

		shouldReloadOnStartup = xml->getBoolAttribute("shouldReloadOnStartup", false);

		forEachXmlChildElement(*xml, e)
		{

			if (e->hasTagName("BOUNDS"))
			{

				int x = e->getIntAttribute("x");
				int y = e->getIntAttribute("y");
				int w = e->getIntAttribute("w");
				int h = e->getIntAttribute("h");

				// bool fs = e->getBoolAttribute("fullscreen");

				// without the correction, you get drift over time
#ifdef WIN32
				setTopLeftPosition(x,y); //Windows doesn't need correction
#else
				setTopLeftPosition(x,y-27);
#endif
				getContentComponent()->setBounds(0,0,w-10,h-33);
				//setFullScreen(fs);
			}
			else if (e->hasTagName("RECENTDIRECTORYNAMES"))
			{

				StringArray filenames;

				forEachXmlChildElement(*e, directory)
				{

					if (directory->hasTagName("DIRECTORY"))
					{
						filenames.add(directory->getStringAttribute("name"));
					}
				}

				UIComponent* ui = (UIComponent*) getContentComponent();
				ui->setRecentlyUsedFilenames(filenames);

			}

		}

		delete xml;
	}
	// return "Everything went ok.";
}


bool MainWindow::compareConfigFiles(File file1, File file2)
{
	XmlDocument lcDoc(file1);
	XmlDocument rcDoc(file2);
	
	std::unique_ptr<XmlElement> lcXml (lcDoc.getDocumentElement());
	std::unique_ptr<XmlElement> rcXml (rcDoc.getDocumentElement());

	if(rcXml == 0 || ! rcXml->hasTagName("SETTINGS"))
	{
		std::cout << "Recovery config is inavlid. Loading lastConfig.xml" << std::endl;
	}

	auto lcSig = lcXml->getChildByName("SIGNALCHAIN");
	auto rcSig = rcXml->getChildByName("SIGNALCHAIN");

	if(lcSig == nullptr)
	{
		if(rcSig != nullptr)
			return false;
	}
	else
	{
		if(rcSig != nullptr)
		{
			if(!lcSig->isEquivalentTo(rcSig, false))
				return false;
		}
	}

	auto lcAudio = lcXml->getChildByName("AUDIO");
	auto rcAudio = rcXml->getChildByName("AUDIO");

	if(!lcAudio->isEquivalentTo(rcAudio, false))
		return false;

	return true;
}
