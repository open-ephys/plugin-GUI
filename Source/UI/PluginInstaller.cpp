/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2019 Open Ephys

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

#include "PluginInstaller.h"
#include <stdio.h>
//-----------------------------------------------------------------------

static inline File getPluginsLocationDirectory() {
#if defined(__APPLE__)
    File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
    if (!dir.isDirectory()) {
        dir.createDirectory();
    }
    return std::move(dir);
#else
    return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
#endif
}

PluginInstaller::PluginInstaller(MainWindow* mainWindow)
: DocumentWindow(WINDOW_TITLE,
		Colour(Colours::black),
		DocumentWindow::closeButton)
{

	parent = (DocumentWindow*)mainWindow;

	setResizable(
        false,  // isResizable
		false); // useBottomCornerRisizer -- doesn't work very well

    //TODO: Add command manager for hot-key functionality later...

    /*
	commandManager.registerAllCommandsForTarget(ui);
	commandManager.registerAllCommandsForTarget(JUCEApplication::getInstance());
	ui->setApplicationCommandManagerToWatch(&commandManager);
	addKeyListener(commandManager.getKeyMappings());
    */

	setUsingNativeTitleBar(true);
	Component::addToDesktop(getDesktopWindowStyleFlags());  // prevents the maximize
	// button from randomly disappearing
	setVisible(true);
	
	int x = parent->getX();
	int y = parent->getY();
	int w = parent->getWidth();
	int h = parent->getHeight();

	setBounds(x+0.1*w, y+0.25*h, 0.8*w, 0.5*h);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(500, 500, 10000, 10000);

}

PluginInstaller::~PluginInstaller()
{
	setMenuBar(0);

#if JUCE_MAC
	MenuBarModel::setMacMainMenu(0);
#endif

}

void PluginInstaller::closeButtonPressed()
{
	delete this;
}



bool PluginInstaller::pluginSelected()
{

	//Get selected plugin from pull-down menu

	//Download from binTray

	//Unzip plugin and install in plugins directory

	//Prompt user to restart to see plugin in ProcessorList

	return true;

}