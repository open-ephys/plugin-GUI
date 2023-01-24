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
#include "Utils/OpenEphysHttpServer.h"
#include "UI/UIComponent.h"
#include "UI/EditorViewport.h"
#include <stdio.h>


MainWindow::MainWindow(const File& fileToLoad)
: DocumentWindow(JUCEApplication::getInstance()->getApplicationName(),
		Colour(Colours::black),
		DocumentWindow::allButtons)
{
    configsDir = CoreServices::getSavedStateDirectory();
	if(!configsDir.getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
		configsDir = configsDir.getChildFile("configs-api" + String(PLUGIN_API_VER));
	
	if(!configsDir.isDirectory())
		configsDir.createDirectory();

    File activityLog = configsDir.getChildFile("activity.log");
    if (activityLog.exists())
        activityLog.deleteFile();
	
	OELogger::instance().createLogFile(activityLog.getFullPathName().toStdString());

	std::cout << "Session Start Time: " << Time::getCurrentTime().toString(true, true, true, true) << std::endl;
	std::cout << std::endl;
	LOGC("Open Ephys GUI v", JUCEApplication::getInstance()->getApplicationVersion(), " (Plugin API v", PLUGIN_API_VER, ")");
	LOGC(SystemStats::getJUCEVersion());
	LOGC("Operating System: ", SystemStats::getOperatingSystemName());
	LOGC("CPU: ", SystemStats::getCpuModel(), " (", SystemStats::getNumCpus(), " core)");
	std::cout << std::endl;

	setResizable(true,      // isResizable
			false);   // useBottomCornerRisizer -- doesn't work very well

	shouldReloadOnStartup = true;
	shouldEnableHttpServer = true;
	openDefaultConfigWindow = false;

	// Create ProcessorGraph and AudioComponent, and connect them.
	// Callbacks will be set by the play button in the control panel

	LOGD("Creating processor graph...");
	processorGraph = std::make_unique<ProcessorGraph>();
	
	LOGD("Creating audio component...");
	audioComponent = std::make_unique<AudioComponent>();
	
	LOGD("Connecting audio component to processor graph...");
	audioComponent->connectToProcessorGraph(processorGraph.get());

	LOGD("Creating UI component...");
	setContentOwned(new UIComponent(this, processorGraph.get(), audioComponent.get()), true);

	UIComponent* ui = (UIComponent*) getContentComponent();

	commandManager.registerAllCommandsForTarget(ui);
	commandManager.registerAllCommandsForTarget(JUCEApplication::getInstance());

	ui->setApplicationCommandManagerToWatch(&commandManager);

	addKeyListener(commandManager.getKeyMappings());

	LOGD("Loading window bounds.");
	loadWindowBounds();
	setUsingNativeTitleBar(true);
	Component::addToDesktop(getDesktopWindowStyleFlags());  // prevents the maximize
														    // button from randomly disappearing
	setVisible(true);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(500, 500, 10000, 10000);

	// Set main window icon to display
	#ifdef __APPLE__
    	File iconDir = File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/Resources");
	#else
		File iconDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
        Image windowIcon = ImageFileFormat::loadFrom(iconDir.getChildFile("icon-small.png"));
        if (auto peer = getPeer())
            peer->setIcon(windowIcon);
	#endif
	
	// Load a specific state of the GUI (custom, default, last-saved, or recovery config)
    if (!fileToLoad.getFullPathName().isEmpty())
    {
        ui->getEditorViewport()->loadState(fileToLoad);
    }
	else if(openDefaultConfigWindow)
	{
		if(defaultConfigWindow == nullptr)
			defaultConfigWindow = std::make_unique<DefaultConfigWindow>(this);
	}
	else if (shouldReloadOnStartup)
	{
		File lastConfig = configsDir.getChildFile("lastConfig.xml");
		File recoveryConfig = configsDir.getChildFile("recoveryConfig.xml");

		if(lastConfig.existsAsFile())
		{
			LOGD("Comparing configurations...");

			if(compareConfigFiles(lastConfig, recoveryConfig))
			{
				ui->getEditorViewport()->loadState(lastConfig);
			}
			else
			{
				LOGD("Detected difference between recoveryConfig and lastConfig; displaying alert window...");
				int loadRecovery = AlertWindow::showYesNoCancelBox(AlertWindow::WarningIcon, "Reloading Settings",
																"It looks like the GUI crashed during your last run, " 
																"causing the configured settings to not save properly. "
																"Which configuration do you want to load?",
																"Recovery Config", "Last Config", "Empty Signal Chain");
				
				if (loadRecovery == 1)
				{
					LOGA("User chose OK, loading recoveryConfig...");
					ui->getEditorViewport()->loadState(recoveryConfig);
				}
				else if(loadRecovery == 2)
				{
					LOGA("User chose cancel, loading lastConfig...");
					ui->getEditorViewport()->loadState(lastConfig);
				}
					
			}
		}
	}

	http_server_thread = std::make_unique<OpenEphysHttpServer>(processorGraph.get());

	if (shouldEnableHttpServer) {
		enableHttpServer();
	}
	else {
		disableHttpServer();
	}

}

MainWindow::~MainWindow()
{

	if (audioComponent->callbacksAreActive())
	{
		audioComponent->endCallbacks();
		processorGraph->stopAcquisition();
	}
    
	saveWindowBounds();

	audioComponent->disconnectProcessorGraph();
	UIComponent* ui = (UIComponent*) getContentComponent();
	ui->disableDataViewport();
	
	File lastConfig = configsDir.getChildFile("lastConfig.xml");
	File recoveryConfig = configsDir.getChildFile("recoveryConfig.xml");
	ui->getEditorViewport()->saveState(lastConfig);
	ui->getEditorViewport()->saveState(recoveryConfig);

	//TODO: Possibly send some message inidicating everything has been saved successfully
	if (http_server_thread) {
        disableHttpServer();
    }
    
	setMenuBar(0);

#if JUCE_MAC
	MenuBarModel::setMacMainMenu(0);
#endif

}

void MainWindow::enableHttpServer() {
    http_server_thread->start();
}

void MainWindow::disableHttpServer() {
    http_server_thread->stop();
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

	if (CoreServices::getAcquisitionStatus())
		processorGraph->stopAcquisition();

}

void MainWindow::handleCrash(void* input)
{

	String backtrace = SystemStats::getStackBacktrace();
    LOGD("\n", backtrace);
    std::flush(std::cout);
    
	File crashLogDir = CoreServices::getSavedStateDirectory();
	if(!crashLogDir.getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
		crashLogDir = crashLogDir.getChildFile("configs-api" + String(PLUGIN_API_VER));

    File activityLog = crashLogDir.getChildFile("activity.log");
	String dt = AccessClass::getControlPanel()->generateDatetimeFromFormat("MM-DD-YYYY_HH_MM_SS");
	File crashLog = crashLogDir.getChildFile("activity_" + dt + ".log");
    
    if (activityLog.exists())
    {
        
        activityLog.copyFileTo(File(crashLog));
        activityLog.deleteFile();
    }

	String recoveryFileLocation = crashLogDir.getChildFile("recoveryConfig.xml").getFullPathName();

	AlertWindow::showMessageBox(AlertWindow::NoIcon,
		"Open Ephys has stopped working",
		"To help fix the problem, please email the following files to support@open-ephys.org: \n\n"
		+ recoveryFileLocation
		+ "\n\n"
		+ crashLog.getFullPathName()
	);
    
}

void MainWindow::saveWindowBounds()
{
	LOGD("Saving window bounds.");

	File file = configsDir.getChildFile("windowState.xml");

	XmlElement* xml = new XmlElement("MAINWINDOW");

	xml->setAttribute("version", JUCEApplication::getInstance()->getApplicationVersion());
	xml->setAttribute("shouldReloadOnStartup", shouldReloadOnStartup);
	xml->setAttribute("shouldEnableHttpServer", shouldEnableHttpServer);

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

	XmlElement* signalChainLocked = new XmlElement("SIGNALCHAIN");
	signalChainLocked->setAttribute("locked", ui->getEditorViewport()->isSignalChainLocked());

	xml->addChildElement(signalChainLocked);

	String error;

	if (! xml->writeTo(file))
		error = "Couldn't write to file";

	delete xml;
}

void MainWindow::loadWindowBounds()
{
    
	File file = configsDir.getChildFile("windowState.xml");

	if(!file.exists())
		openDefaultConfigWindow = true;

	XmlDocument doc(file);
	std::unique_ptr<XmlElement> xml = doc.getDocumentElement();

	if (xml == 0 || ! xml->hasTagName("MAINWINDOW"))
	{

		LOGDD("File not found.");
		centreWithSize(1200, 800);

	}
	else
	{

		String description;

		shouldReloadOnStartup = xml->getBoolAttribute("shouldReloadOnStartup", false);
		shouldEnableHttpServer = xml->getBoolAttribute("shouldEnableHttpServer", false);

		for (auto* e : xml->getChildIterator())
		{

			if (e->hasTagName("BOUNDS"))
			{

				int x = e->getIntAttribute("x");
				int y = e->getIntAttribute("y");
				int w = e->getIntAttribute("w");
				int h = e->getIntAttribute("h");

				// bool fs = e->getBoolAttribute("fullscreen");

				// without the correction, you get drift over time
#ifdef _WIN32
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

				for (auto* directory : e->getChildIterator())
				{

					if (directory->hasTagName("DIRECTORY"))
					{
						filenames.add(directory->getStringAttribute("name"));
					}
				}

				UIComponent* ui = (UIComponent*) getContentComponent();
				ui->setRecentlyUsedFilenames(filenames);

			}
			else if (e->hasTagName("SIGNALCHAIN"))
			{
				UIComponent* ui = (UIComponent*)getContentComponent();
				ui->getEditorViewport()->lockSignalChain(e->getBoolAttribute("locked", false));
			}

		}

	}
}


bool MainWindow::compareConfigFiles(File file1, File file2)
{
	XmlDocument lcDoc(file1);
	XmlDocument rcDoc(file2);
	
	std::unique_ptr<XmlElement> lcXml (lcDoc.getDocumentElement());
	std::unique_ptr<XmlElement> rcXml (rcDoc.getDocumentElement());

	if(rcXml == 0 || ! rcXml->hasTagName("SETTINGS"))
	{
		LOGD("Recovery config is invalid. Loading lastConfig.xml");
		return true;
	}

	if (lcXml == 0 || !lcXml->hasTagName("SETTINGS"))
	{
		LOGD("Last config is invalid. Loading recoveryConfig.xml");
		return false;
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
