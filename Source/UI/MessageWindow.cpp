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

    juce::Rectangle<int> area (0, 0, 450, 193);
    options.content->setSize (area.getWidth(), area.getHeight());

    options.dialogTitle = "Message Window";
    options.dialogBackgroundColour = messageWindowComponent->findColour (ThemeColours::componentBackground);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;

    auto* window = options.launchAsync();
    window->setAlwaysOnTop (true);
    messageWindow = window;
}

MessageWindowComponent::MessageWindowComponent()
{
    timestampLabel = std::make_unique<Label> ("Timestamp");
    timestampLabel->setFont (FontOptions { "Inter", "Regular", 18.0f });

    resetTime();

    timestampLabel->setJustificationType (Justification::left);
    addAndMakeVisible (timestampLabel.get());

    timestampResetButton = std::make_unique<TextButton> ("Reset Timestamp Button");
    timestampResetButton->setButtonText ("Reset");
    timestampResetButton->setColour (TextButton::buttonColourId, findColour (ThemeColours::highlightedFill));
    timestampResetButton->addListener (this);
    if (CoreServices::getRecordingStatus())
        addAndMakeVisible (timestampResetButton.get());

    messageLabel = std::make_unique<Label> ("Message");
    messageLabel->setFont (FontOptions { "Inter", "Regular", 18.0f });
    messageLabel->setColour (Label::backgroundColourId, findColour (ThemeColours::widgetBackground));
    messageLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline));
    messageLabel->setColour (Label::outlineWhenEditingColourId, findColour (ThemeColours::menuHighlightBackground));
    messageLabel->setJustificationType (Justification::left);
    messageLabel->setEditable (true);
    messageLabel->addListener (this);
    addAndMakeVisible (messageLabel.get());

    sendMessageButton = std::make_unique<TextButton> ("Send Message Button");
    sendMessageButton->setButtonText ("Send");
    sendMessageButton->setColour (TextButton::buttonColourId, findColour (ThemeColours::highlightedFill));
    sendMessageButton->setColour (TextButton::textColourOnId, Colours::black);
    sendMessageButton->addListener (this);
    sendMessageButton->setEnabled (CoreServices::getAcquisitionStatus());
    addAndMakeVisible (sendMessageButton.get());

    Array<String>& savedMessages = AccessClass::getMessageCenter()->getSavedMessages();

    savedMessageSelector = std::make_unique<ComboBox> ("Saved Messages");

    for (auto message : savedMessages)
    {
        savedMessageSelector->addItem (message, savedMessageSelector->getNumItems() + 1);
    }
    savedMessageSelector->addListener (this);
    addAndMakeVisible (savedMessageSelector.get());

    clearSavedMessagesButton = std::make_unique<TextButton> ("Clear Saved Messages");
    clearSavedMessagesButton->setButtonText ("Clear");
    clearSavedMessagesButton->addListener (this);
    addAndMakeVisible (clearSavedMessagesButton.get());
}

MessageWindowComponent::~MessageWindowComponent()
{
}

void MessageWindowComponent::resetTime()
{
    if (! CoreServices::getRecordingStatus())
    {
        timestampLabel->setText ("Start recording to save a message.", dontSendNotification);
        timestampLabel->setColour (Label::textColourId, Colours::red);
    }
    else
    {
        timestampLabel->setText (createTimeString (CoreServices::getRecordingTime()), dontSendNotification);
        timestampLabel->setColour (Label::textColourId, findColour (ThemeColours::defaultText));
    }

    messageTimeMillis = CoreServices::getSystemTime();
}

String MessageWindowComponent::createTimeString (float milliseconds)
{
    int h = floor (milliseconds / 3600000.0);
    int m = floor (milliseconds / 60000.0);
    int s = floor ((milliseconds - m * 60000.0) / 1000.0);
    int ms = milliseconds - m * 60000 - s * 1000;

    String timeString = "Time of message: ";

    if (h < 10)
        timeString += "0";
    timeString += h;
    timeString += ":";

    int minutes = m - h * 60;

    if (minutes < 10)
        timeString += "0";
    timeString += minutes;
    timeString += ":";

    if (s < 10)
        timeString += "0";
    timeString += s;

    timeString += ".";

    if (ms < 100)
        timeString += "0";

    if (ms < 10)
        timeString += "0";

    timeString += ms;

    return timeString;
}

void MessageWindowComponent::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::componentParentBackground));
    g.setColour (findColour (ThemeColours::componentBackground));
    g.fillRect (10, 0, getWidth() - 20, getHeight() - 10);

    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (FontOptions { "Inter", "Regular", 14.0f });
    g.drawMultiLineText ("This window is used to broadcast messages "
                         "to all processors in the signal chain. "
                         "Messages will only be saved if recording is active.",
                         20,
                         20,
                         getWidth() - 40);

    g.drawSingleLineText ("Enter message here: ",
                          20,
                          93);

    g.drawSingleLineText ("Load a saved message: ",
                          20,
                          143);
}

void MessageWindowComponent::resized()
{
    int yoffset = 47;

    timestampLabel->setBounds (15, yoffset, getWidth() - 95, 20);
    timestampResetButton->setBounds (getWidth() - 65, yoffset, 45, 20);

    yoffset += 56;

    messageLabel->setBounds (20, yoffset, getWidth() - 95, 20);
    sendMessageButton->setBounds (getWidth() - 65, yoffset, 45, 20);

    yoffset += 50;

    savedMessageSelector->setBounds (20, yoffset, getWidth() - 95, 20);
    clearSavedMessagesButton->setBounds (getWidth() - 65, yoffset, 45, 20);
}

void MessageWindowComponent::buttonClicked (Button* button)
{
    if (button == sendMessageButton.get())
    {
        if (messageLabel->getText().length() > 0)
        {
            AccessClass::getMessageCenter()->addOutgoingMessage (messageLabel->getText(), messageTimeMillis);
        }
    }
    else if (button == timestampResetButton.get())
    {
        resetTime();
    }
    else if (button == clearSavedMessagesButton.get())
    {
        AccessClass::getMessageCenter()->clearSavedMessages();
        savedMessageSelector->clear();
    }
}

void MessageWindowComponent::editorShown (Label*, TextEditor& editor)
{
    editor.setInputRestrictions (490);
}

void MessageWindowComponent::comboBoxChanged (ComboBox* comboBox)
{
    if (comboBox == savedMessageSelector.get())
    {
        messageLabel->setText (savedMessageSelector->getText(), dontSendNotification);
    }
}
