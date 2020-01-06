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
	
	/* Get list of plugins uploaded to bintray */
	RestRequest::Response response = request.get("https://api.bintray.com/repos/open-ephys-gui-plugins")
										 .execute();

	var jsonReply = JSON::parse(response.bodyAsString);

	for (int i = 0; i < jsonReply.size(); i++)
	{
		plugins.add(jsonReply[i].getProperty("name", var()).toString());
	}

	pluginSelected(plugins.size()-1);

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

bool PluginInstaller::pluginSelected(int index)
{

	//Get selected plugin from pull-down menu
	juce::String plugin = plugins[index];

	//Download from binTray
	Array<juce::String> packages;
	juce::String url = "https://api.bintray.com/repos/open-ephys-gui-plugins/";
	url+=plugin;
	url+="/packages";

	RestRequest::Response response = request.get(url).execute();

	var jsonReply = JSON::parse(response.bodyAsString);

	for (int i = 0; i < jsonReply.size(); i++)
	{
		packages.add(jsonReply[i].getProperty("name", var()).toString());
		std::cout << packages[packages.size()-1] << std::endl;
	}

	juce::SystemStats::OperatingSystemType os = juce::SystemStats::getOperatingSystemType();
	String os_name;

	if (os & juce::SystemStats::OperatingSystemType::Windows)
	{
		os_name = "windows";
	}
	else if (os & juce::SystemStats::OperatingSystemType::MacOSX)
	{
		os_name = "mac";
	}
	else if (os == juce::SystemStats::OperatingSystemType::Linux)
	{
		os_name = "linux";
	}
	else
	{
		std::cout << "Unsupported OS type..." << std::endl;
		return false;
	}

	String package;
	for (int i = 0; i < packages.size(); i++)
	{
		if (packages[i].contains(os_name))
		{
			package = packages[i];
		}
	}

	std::cout << "Installing package: " << package << std::endl;

	//Get latest version
	String version_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	version_url+=plugin;
	version_url+="/";
	version_url+=package;
	version_url+="/versions/_latest";

	std::cout << version_url << std::endl;

	RestRequest::Response version_response = request.get(version_url).execute();

	std::cout << version_response.bodyAsString << std::endl;

	var version_reply = JSON::parse(version_response.bodyAsString);

	String version = version_reply.getProperty("name", "NULL");
	String created = version_reply.getProperty("created", "NULL");
	String updated = version_reply.getProperty("updated", "NULL");
	String released = version_reply.getProperty("released", "NULL");

	std::cout << "Latest version: " << version << " released: " << released << std::endl;

	//TODO: Build download filename...
	String filename = ""; 

	//Get avaialble filenames:
	//https: //api.bintray.com/packages/$bintrayUser/$REPO/$PACKAGE/versions/$VERSION/files

	String files_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	files_url += plugin;
	files_url += "/";
	files_url += package;
	files_url += "/versions/";
	files_url += version;
	files_url += "/files";

	RestRequest::Response files_response = request.get(files_url).execute();

	var files_reply = JSON::parse(files_response.bodyAsString);

	if (files_reply.size())
	{
		std::cout << files_reply.size() << " files available" << std::endl;
		for (int i = 0; i < files_reply.size(); i++)
		{
			std::cout << i << "th file: " << files_reply[i].getProperty("name", "NULL").toString() << std::endl;
			filename = files_reply[i].getProperty("name", "NULL").toString();
		}
	}

	//Unzip plugin and install in plugins directory
	//curl -L https://dl.bintray.com/$bintrayUser/$repo/$filename
	String fileDownloadURL = "https://dl.bintray.com/open-ephys-gui-plugins/";
	fileDownloadURL+=plugin;
	fileDownloadURL+="/";
	fileDownloadURL+=filename;

	std::cout << "Download URL: " << fileDownloadURL << std::endl;

	URL fileUrl(fileDownloadURL);

	/*
	ScopedPointer<InputStream> fileStream = fileUrl.createInputStream(false);

	//Get path to plugins directory
	File pluginsPath = getPluginsLocationDirectory();

	//Construct path for downloaded zip file
	String pluginFilePath = pluginsPath.getFullPathName();
	pluginFilePath+='/';
	pluginFilePath+=filename;
	pluginFilePath+='.7z';

	std::cout << "Plugin zip file destination path: " << pluginFilePath << std::endl;
	
	//Create local file
	File localFile(pluginFilePath);
	localFile.deleteFile();
	MemoryBlock mem(1024);
	fileStream->readIntoMemoryBlock(mem);
	FileOutputStream out(localFile);
	out.write(mem.getData(), mem.getSize());
	*/

	//TODO: Prompt user to restart to see plugin in ProcessorList

	return true;
}