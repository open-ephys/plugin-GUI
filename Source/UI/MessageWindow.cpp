/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2024 Open Ephys

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

#include "MessageWindow.h"
#include <stdio.h>

#include "../AccessClass.h"
#include "../CoreServices.h"
#include "../Processors/MessageCenter/MessageCenter.h"

#include "UIComponent.h"

//-----------------------------------------------------------------------

MessageWindow::MessageWindow()
{
    launch();
}

MessageWindow::~MessageWindow()
{
    if (messageWindow != nullptr)
    {
        messageWindow->exitModalState (0);
    }
}

void MessageWindow::launch()
{
    DialogWindow::LaunchOptions options;

    messageWindowComponent = new MessageWindowComponent();
    options.content.setOwned (messageWindowComponent);

    juce::Rectangle<int> area (0, 0, 450, 120);
    options.content->setSize (area.getWidth(), area.getHeight());

    options.dialogTitle = "Save Message";
    options.dialogBackgroundColour = messageWindowComponent->findColour (ThemeColors::componentBackground);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;

    messageWindow = options.launchAsync();
}

MessageWindowComponent::MessageWindowComponent()
{
    timestampLabel = std::make_unique<Label> ("Timestamp");
    String labelText;
    int64 ts = CoreServices::getGlobalTimestamp();
    labelText << String(ts) << " <-- message time:" << newLine;
    timestampLabel->setFont (FontOptions { "Inter", "Regular", 18.0f });
    timestampLabel->setText (labelText, dontSendNotification);
    timestampLabel->setJustificationType (Justification::centred);
    addAndMakeVisible (timestampLabel.get());

    messageLabel = std::make_unique<Label> ("Message");
    messageLabel->setFont (FontOptions { "Inter", "Regular", 18.0f });
    messageLabel->setColour (Label::backgroundColourId, findColour (ThemeColors::componentParentBackground));
    messageLabel->setJustificationType (Justification::centred);
    messageLabel->setEditable (true);
    addAndMakeVisible (messageLabel.get());
    
    sendMessageButton = std::make_unique<TextButton> ("Default Config Selector - Load Button");
    sendMessageButton->setButtonText ("Save");
    sendMessageButton->setColour (TextButton::buttonColourId, findColour (ThemeColors::highlightedFill));
    sendMessageButton->addListener (this);
    addAndMakeVisible (sendMessageButton.get());
}

MessageWindowComponent::~MessageWindowComponent()
{
}

void MessageWindowComponent::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColors::componentParentBackground));
    g.setColour (findColour (ThemeColors::componentBackground));
    g.fillRect (10, 0, getWidth() - 20, getHeight() - 10);

}

void MessageWindowComponent::resized()
{
    timestampLabel->setBounds (10, 20, getWidth() - 20, 20);

    messageLabel->setBounds (10, 50, getWidth() - 20, 20);

    sendMessageButton->setBounds (10, 80, getWidth() - 20, 20);
}

void MessageWindowComponent::buttonClicked (Button* button)
{
    if (button == sendMessageButton.get())
    {
        AccessClass::getMessageCenter()->broadcastMessage (messageLabel->getText());
    }
        
}
