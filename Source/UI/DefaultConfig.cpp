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

#include "DefaultConfig.h"
#include <stdio.h>

#include "../CoreServices.h"
#include "../AccessClass.h"
#include "UIComponent.h"
#include "EditorViewport.h"

//-----------------------------------------------------------------------

DefaultConfigWindow::DefaultConfigWindow(MainWindow* mainWindow)
{
	parent = (DocumentWindow*)mainWindow;
	
	launchWindow();
}

DefaultConfigWindow::~DefaultConfigWindow()
{
	if(configWindow != nullptr)
	{
		configWindow->exitModalState (0);
	}
	
	masterReference.clear();
}

void DefaultConfigWindow::launchWindow()
{
	DialogWindow::LaunchOptions options;

	// auto* label = new Label();
	// label->setText ("m", dontSendNotification);
	// label->setColour (Label::textColourId, Colours::whitesmoke);
	configComponent = new DefaultConfigComponent();
	options.content.setOwned (configComponent);

	Rectangle<int> area (0, 0, 400, 200);
	options.content->setSize (area.getWidth(), area.getHeight());

	options.componentToCentreAround 	  = parent;
	options.dialogTitle                   = "Load a Default Configuration";
    options.dialogBackgroundColour        = Colours::darkgrey;
	options.escapeKeyTriggersCloseButton  = true;
	options.useNativeTitleBar             = false;
	options.resizable 					  = false;

	configWindow = options.launchAsync();

}

//-----------------------------------------------------------------------

DefaultConfigComponent::DefaultConfigComponent()
{
	configFont = Font("FiraSans", 18, Font::plain);

	configLabel = std::make_unique<Label>("Configurations");
	String labelText;
	labelText << "Please select a default configuration from the list below to get started:" << newLine;
	configLabel->setColour(Label::textColourId, Colours::white);
	configLabel->setFont(configFont);
	configLabel->setText(labelText, dontSendNotification);
	configLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(configLabel.get());

	File configsDir = CoreServices::getSavedStateDirectory().getChildFile("configs");
	auto configFiles = configsDir.findChildFiles(File::findFiles, true, "*.xml");

	configSelector = std::make_unique<ComboBox>("Config Selector");
	configSelector->setJustificationType(Justification::centredLeft);
	
	int id = 1;

	for(auto file : configFiles)
	{
		configSelector->addItem(file.getFileNameWithoutExtension(), id);
		id++;
	}
	configSelector->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(configSelector.get());

	goButton = std::make_unique<TextButton>("Go");
	goButton->setButtonText("Go!");
	goButton->setColour(TextButton::buttonColourId, Colours::lightgreen);
	goButton->addListener(this);
	addAndMakeVisible(goButton.get());

}

DefaultConfigComponent::~DefaultConfigComponent()
{

}

void DefaultConfigComponent::paint(Graphics& g)
{
	g.fillAll (Colours::darkgrey);
	g.setColour(Colour::fromRGB(110, 110, 110));
	g.fillRect(10, 0, getWidth() - 20, getHeight() - 10);
}

void DefaultConfigComponent::resized()
{
	configLabel->setBounds(10, 20, 380, 50);
	configSelector->setBounds((getWidth() / 2) - 100, 80, 200, 30);
	goButton->setBounds( (getWidth()/2) - 25, getHeight() - 60, 50, 30);
}

void DefaultConfigComponent::buttonClicked(Button* button)
{
	if(button == goButton.get())
	{
		// Get selected config file name with full path
		String fileName = configSelector->getItemText(configSelector->getSelectedItemIndex());
		String filePath = "configs" + File::getSeparatorString() + fileName + ".xml";
		File configFile = CoreServices::getSavedStateDirectory().getChildFile(filePath);

		// Load the config file
		AccessClass::getUIComponent()->getEditorViewport()->loadState(configFile);

		// Close config window after loading the config file
		if (DialogWindow* dw = this->findParentComponentOfClass<DialogWindow>())
    		dw->exitModalState (0);

	}
}
