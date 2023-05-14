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

DefaultConfigWindow::DefaultConfigWindow()
{	
	launchWindow();
}

DefaultConfigWindow::~DefaultConfigWindow()
{
	if(configWindow != nullptr)
	{
		configWindow->exitModalState (0);
	}
	
}

void DefaultConfigWindow::launchWindow()
{
	DialogWindow::LaunchOptions options;

	configComponent = new DefaultConfigComponent();
	options.content.setOwned (configComponent);

	juce::Rectangle<int> area (0, 0, 450, 300);
	options.content->setSize (area.getWidth(), area.getHeight());

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

	File appDir = File::getSpecialLocation(File::currentApplicationFile);
#ifdef __APPLE__
    File iconsDir = appDir.getChildFile("Contents/Resources").getChildFile("configs/icons");
#else
    File iconsDir = appDir.getParentDirectory().getChildFile("configs/icons");
#endif

	acqBoardButton = std::make_unique<ImageButton>("Default Config Selector - Acquisition Board");	
	File acqIconFile = iconsDir.getChildFile("acq_board_icon.png");
	Image acqBoardIcon = ImageFileFormat::loadFrom(acqIconFile);
	acqBoardButton->setImages(false, true, true, 
							  acqBoardIcon, 1.0f, Colour(), 
							  acqBoardIcon, 1.0f, Colours::grey.withAlpha(0.5f),
							  acqBoardIcon, 1.0f, Colours::aliceblue.withAlpha(0.5f));
	
	acqBoardButton->setClickingTogglesState(true);
	acqBoardButton->setTooltip("Acquire data from an Open Ephys Acquisition Board");
	acqBoardButton->addListener(this);
	acqBoardButton->setRadioGroupId(101, dontSendNotification);
	addAndMakeVisible(acqBoardButton.get());

	acqBoardLabel = std::make_unique<Label>("Acq. Board Label");
	acqBoardLabel->setColour(Label::textColourId, Colours::white);
	acqBoardLabel->setFont(configFont);
	acqBoardLabel->setText("Acq. Board", dontSendNotification);
	acqBoardLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(acqBoardLabel.get());


	fileReaderButton = std::make_unique<ImageButton>("Default Config Selector - File Reader");	
	File fRIconFile = iconsDir.getChildFile("file_reader_icon.png");
	Image fRIcon = ImageFileFormat::loadFrom(fRIconFile);
	fileReaderButton->setImages(false, true, true, 
							  fRIcon, 1.0f, Colour(), 
							  fRIcon, 1.0f, Colours::grey.withAlpha(0.5f),
							  fRIcon, 1.0f, Colours::aliceblue.withAlpha(0.5f));
	
	fileReaderButton->setClickingTogglesState(true);
	fileReaderButton->setTooltip("Read data from a file");
	fileReaderButton->addListener(this);
	fileReaderButton->setRadioGroupId(101, dontSendNotification);
	fileReaderButton->setToggleState(true, dontSendNotification);
	addAndMakeVisible(fileReaderButton.get());

	fileReaderLabel = std::make_unique<Label>("File Reader Label");
	fileReaderLabel->setColour(Label::textColourId, Colours::white);
	fileReaderLabel->setFont(configFont);
	fileReaderLabel->setText("File Reader", dontSendNotification);
	fileReaderLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(fileReaderLabel.get());



	neuropixelsButton = std::make_unique<ImageButton>("Default Config Selector - Neuropixels-PXI");	
	File npxIconFile = iconsDir.getChildFile("neuropixels_icon.png");
	Image npxIcon = ImageFileFormat::loadFrom(npxIconFile);
	neuropixelsButton->setImages(false, true, true, 
							  npxIcon, 1.0f, Colour(), 
							  npxIcon, 1.0f, Colours::grey.withAlpha(0.5f),
							  npxIcon, 1.0f, Colours::aliceblue.withAlpha(0.5f));
	
	neuropixelsButton->setClickingTogglesState(true);
	neuropixelsButton->addListener(this);
	neuropixelsButton->setRadioGroupId(101, dontSendNotification);
	addAndMakeVisible(neuropixelsButton.get());

	neuropixelsLabel = std::make_unique<Label>("Npx Label");
	neuropixelsLabel->setColour(Label::textColourId, Colours::white);
	neuropixelsLabel->setFont(configFont);
	neuropixelsLabel->setText("Neuropixels", dontSendNotification);
	neuropixelsLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(neuropixelsLabel.get());

#ifdef _WIN32
    neuropixelsButton->setTooltip("Acquire data from Neuropixels probes");
#else
    neuropixelsButton->setTooltip("Acquire data from Neuropixels probes (Windows only)");
	neuropixelsButton->setEnabled(false);
	neuropixelsLabel->setEnabled(false);
#endif

	goButton = std::make_unique<TextButton>("Default Config Selector - Load Button");
	goButton->setButtonText("Load");
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

	g.setColour(Colours::lightgreen);

	if(acqBoardButton->getToggleState())
		g.drawRoundedRectangle(acqBoardButton->getBounds().expanded(5).toFloat(), 3.0f, 1.0f);
	else if(fileReaderButton->getToggleState())
		g.drawRoundedRectangle(fileReaderButton->getBounds().expanded(5).toFloat(), 3.0f, 1.0f);
	else
		g.drawRoundedRectangle(neuropixelsButton->getBounds().expanded(5).toFloat(), 3.0f, 1.0f);
}

void DefaultConfigComponent::resized()
{
	configLabel->setBounds(10, 20, getWidth() - 20, 50);

	acqBoardButton->setBounds((getWidth() / 5) - 50, 90, 100, 100);
	acqBoardLabel->setBounds((getWidth() / 5) - 50, 205, 100, 20);

	fileReaderButton->setBounds((getWidth() / 2) - 50, 90, 100, 100);
	fileReaderLabel->setBounds((getWidth() / 2) - 50, 205, 100, 20);

	neuropixelsButton->setBounds(( 4 * getWidth() / 5) - 50, 90, 100, 100);
	neuropixelsLabel->setBounds(( 4 * getWidth() / 5) - 50, 205, 100, 20);
	
	goButton->setBounds( (getWidth()/2) - 35, getHeight() - 50, 70, 30);
}

void DefaultConfigComponent::buttonClicked(Button* button)
{
	if (button == goButton.get())
	{
		// Get selected config file name with full path
		String filePath;

		if (acqBoardButton->getToggleState())
		{
			int response = AlertWindow::showYesNoCancelBox(AlertWindow::QuestionIcon, "Select acquisition board type",
				"What type of FPGA does your acquisition board have? \n\n"
				"If it was delivered by Open Ephys Production Site after "
				"November 2022, it has a custom FPGA designed by Open Ephys. \n\n"
				"Older acquisition boards likely use an Opal Kelly FPGA.",
				"Open Ephys FPGA", "Opal Kelly FPGA", "Cancel");

			if (response == 1) // OE FPGA
			{
				LOGA("Selected Open Ephys FPGA");
				filePath = "configs" + File::getSeparatorString() + "oe_acq_board_config.xml";
			}
			else if (response == 2)
			{
				LOGA("Selected Opal Kelly FPGA");
				filePath = "configs" + File::getSeparatorString() + "acq_board_config.xml";
			}
			else {
				return;
			}
			
		}
		else if (fileReaderButton->getToggleState())
		{
			filePath = "configs" + File::getSeparatorString() + "file_reader_config.xml";
		}
			
		else
		{
			filePath = "configs" + File::getSeparatorString() + "neuropixels_pxi_config.xml";
		}
			

#ifdef __APPLE__
		File configFile = File::getSpecialLocation(File::currentApplicationFile)
            .getChildFile("Contents/Resources")
            .getChildFile(filePath);
#else
        File configFile = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile(filePath);
#endif
		
		// Hide the config window
		if(DialogWindow* dw = this->findParentComponentOfClass<DialogWindow>())
			dw->setVisible(false);
		
		// Load the config file
		AccessClass::getUIComponent()->getEditorViewport()->loadState(configFile);

		// Close config window after loading the config file
		if(DialogWindow* dw = this->findParentComponentOfClass<DialogWindow>())
			dw->exitModalState (0);

	}
	else if (button->getRadioGroupId() == 101)
	{
		this->repaint();
	}
}
