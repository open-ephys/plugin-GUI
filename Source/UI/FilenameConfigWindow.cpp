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

#include "FilenameConfigWindow.h"
#include "LookAndFeel/CustomLookAndFeel.h"
#include <stdio.h>

FilenameFieldComponent::FilenameFieldComponent (int type_, int state_, String value_)
    : type (static_cast<Type> (type_)),
      state (static_cast<State> (state_)),
      value (value_),
      index (0),
      newDirectoryNeeded (true)
{
    typeLabel = std::make_unique<Label> ("Type", types[type_] + ":");
    typeLabel->setFont (FontOptions ("Inter", "Medium", 18.0f));
    typeLabel->setBounds (2, 0, 80, 32);
    addAndMakeVisible (typeLabel.get());

    stateButton = std::make_unique<TextButton> ("State");
    stateButton->setButtonText (states[state_]);
    stateButton->setBounds (85, 1, 80, 30);
    stateButton->addListener (this);
    addAndMakeVisible (stateButton.get());

    valueLabel = std::make_unique<Label> ("Value", value_);
    if (state != FilenameFieldComponent::State::AUTO)
    {
        valueLabel->setEditable (true);
    }
    else
    {
        valueLabel->setEditable (false);
    }

    valueLabel->setBounds (170, 1, 188, 30);
    valueLabel->addListener (this);
    valueLabel->setColour (
        Label::outlineColourId,
        state == FilenameFieldComponent::State::CUSTOM ? findColour (ThemeColours::outline) : findColour (ThemeColours::outline).withAlpha (0.25f));
    addAndMakeVisible (valueLabel.get());

    if (type == FilenameFieldComponent::Type::MAIN)
    {
        savedValue = "directory_name";
    }
    else if (type == FilenameFieldComponent::Type::PREPEND)
    {
        savedValue = "prepend_";
    }
    else
    {
        savedValue = "_append";
    }
}

/* Returns an empty string if the candidate is valid, else returns the error as a string */
String FilenameFieldComponent::validate (String candidate)
{
    String errorStr = "";

    if (candidate.length() == 0 && state != State::NONE)
        errorStr = "File name must have at least 1 character.";

    if (candidate.contains (File::getSeparatorString()))
        errorStr = "File name cannot contain slashes.";

    if (candidate.contains ("."))
        errorStr = "File name cannot contain periods.";

    return errorStr;
}

void FilenameFieldComponent::labelTextChanged (Label* label)
{
    String candidateValue = label->getText();

    String errorStr = validate (candidateValue);

    if (errorStr.length())
    {
        label->setText (value, dontSendNotification);
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Invalid file name...", errorStr);
        return;
    }
    value = candidateValue;
}

void FilenameFieldComponent::incrementDirectoryIndex()
{
    index++;
    newDirectoryNeeded = true;
}

String FilenameFieldComponent::getNextValue (bool usePlaceholderText)
{
    if (newDirectoryNeeded)
    {
        //std::cout << "New directory needed." << std::endl;

        newDirectoryNeeded = false;

        stateButton->setButtonText (states[state]);

        if (state == FilenameFieldComponent::State::CUSTOM)
        {
            valueLabel->setEditable (true);
            valueLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline));
            valueLabel->setText (value, dontSendNotification);

            return value;
        }

        if (state == FilenameFieldComponent::State::NONE)
        {
            valueLabel->setEditable (false);
            valueLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline).withAlpha (0.25f));
            valueLabel->setText ("", dontSendNotification);

            value = "";

            return value;
        }

        if (state == FilenameFieldComponent::State::AUTO)
        {
            valueLabel->setEditable (false);
            valueLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline).withAlpha (0.25f));

            if (type == FilenameFieldComponent::Type::MAIN)
            {
                if (usePlaceholderText)
                {
                    value = "YYYY-MM-DD_HH-MM-SS";
                    valueLabel->setText (value, dontSendNotification);
                    return value;
                }
                else
                {
                    value = Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S");
                    valueLabel->setText (value, dontSendNotification);
                    return value;
                }
            }
            else
            {
                value = "";

                if (type == FilenameFieldComponent::Type::APPEND)
                    value += "_";

                if (index < 100)
                    value += "0";

                if (index < 10)
                    value += "0";

                value += String (index);

                if (type == FilenameFieldComponent::Type::PREPEND)
                    value += "_";

                valueLabel->setText (value, dontSendNotification);
                return value;
            }
        }
    }
    return value;
}

void FilenameFieldComponent::buttonClicked (Button* button)
{
    state = static_cast<State> (int (state + 1) % states.size());

    if (type == FilenameFieldComponent::Type::MAIN
        && state == FilenameFieldComponent::State::NONE) // MAIN string cannot be empty
    {
        state = FilenameFieldComponent::State::AUTO;
    }

    stateButton->setButtonText (states[state]);

    valueLabel->setEditable (false);
    valueLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline).withAlpha (0.25f));

    if (state == FilenameFieldComponent::State::NONE)
    {
        savedValue = value;
        value = "";
        valueLabel->setText ("", dontSendNotification);
    }
    else if (state == FilenameFieldComponent::State::CUSTOM)
    {
        value = savedValue;
        valueLabel->setEditable (true);
        valueLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline));
        valueLabel->setText (value, sendNotification);
    }
    else //AUTO
    {
        newDirectoryNeeded = true;
        getNextValue (true);
    }
}

void FilenameConfigWindow::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::componentBackground));
}

void FilenameConfigWindow::saveStateToXml (XmlElement* xml)
{
    XmlElement* state = xml->createNewChildElement ("FILENAMECONFIG");
    for (auto field : fields)
    {
        XmlElement* currentField = state->createNewChildElement (String (field->types[field->type]).toUpperCase());
        currentField->setAttribute ("state", field->state);
        currentField->setAttribute ("value", field->value);
    }
}

/** Load settings. */
void FilenameConfigWindow::loadStateFromXml (XmlElement* xml)
{
    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName ("FILENAMECONFIG"))
        {
            for (auto* fieldNode : xmlNode->getChildIterator())
            {
                FilenameFieldComponent::Type type;

                if (fieldNode->hasTagName ("PREPEND"))
                    type = FilenameFieldComponent::Type::PREPEND;
                else if (fieldNode->hasTagName ("MAIN"))
                    type = FilenameFieldComponent::Type::MAIN;
                else
                    type = FilenameFieldComponent::Type::APPEND;

                fields[type]->state = static_cast<FilenameFieldComponent::State> (fieldNode->getIntAttribute ("state", 0));
                fields[type]->stateButton->setButtonText (fields[type]->states[fields[type]->state]);

                if (fields[type]->state != FilenameFieldComponent::State::AUTO)
                {
                    fields[type]->value = fieldNode->getStringAttribute ("value", "");
                    fields[type]->valueLabel->setText (fields[type]->value, sendNotification);
                    fields[type]->valueLabel->setEditable (false);
                }
                else
                {
                    fields[type]->newDirectoryNeeded = true;
                    fields[type]->getNextValue (true);
                }

                if (fields[type]->state == FilenameFieldComponent::State::CUSTOM)
                {
                    fields[type]->valueLabel->setEditable (true, sendNotification);
                    fields[type]->valueLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline));
                }
            }
        }
    }
}
