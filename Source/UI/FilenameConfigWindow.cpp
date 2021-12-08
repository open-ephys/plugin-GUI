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

#include "FilenameConfigWindow.h"
#include <stdio.h>

FilenameFieldComponent::FilenameFieldComponent(int type_, int state_, String value_) 
        : type(static_cast<Type>(type_)), state(static_cast<State>(state_)), value(value_)
{

    typeLabel = std::make_unique<Label>("Type", types[type_] + ":");
    //typeLabel->setFont(Font(1.5*typeLabel->getFont().getHeight()));
    typeLabel->setColour(Label::textColourId, Colours::white);
    typeLabel->setBounds(0, 0, 80, 32);
    addAndMakeVisible(typeLabel.get());

    stateButton = std::make_unique<TextButton>("State");
    stateButton->setButtonText(states[state_]);
    stateButton->setBounds(85, 0, 80, 32);
    stateButton->addListener(this);
    addAndMakeVisible(stateButton.get());

    valueLabel = std::make_unique<Label>("Value", value_);
    valueLabel->setEditable(true);
    valueLabel->setBounds(170, 0, 230, 32);
    valueLabel->addListener(this);
    valueLabel->setColour(
        Label::ColourIds::backgroundColourId, 
        state == FilenameFieldComponent::State::CUSTOM ? Colours::white : Colours::grey);
    addAndMakeVisible(valueLabel.get());

}

void FilenameFieldComponent::labelTextChanged(Label* label)
{
    //TODO: Do some basic string validation here
    value = label->getText();
}

void FilenameFieldComponent::buttonClicked(Button* button)
{
    state = static_cast<State>(int(state + 1) % states.size());
    stateButton->setButtonText(states[state]);

    valueLabel->setEditable(false);
    valueLabel->setColour(Label::ColourIds::backgroundColourId, Colours::grey);

    if (state == FilenameFieldComponent::State::NONE)
        valueLabel->setText("", sendNotification);
    else if (state == FilenameFieldComponent::State::CUSTOM)
    {
        valueLabel->setEditable(true);
        valueLabel->setColour(Label::ColourIds::backgroundColourId, Colours::white);
    }
    else //AUTO
    {
        switch (type)
        {
          case FilenameFieldComponent::Type::PREPEND:

            valueLabel->setText("001_", sendNotification);
            break;

          case FilenameFieldComponent::Type::MAIN:

            valueLabel->setText("MM-DD-YYYY_HH-MM-SS", sendNotification);
            break;

          case FilenameFieldComponent::Type::APPEND:

            valueLabel->setText("_001", sendNotification);
            break;

          default:
            break;
        }
    }

}

void FilenameConfigWindow::saveStateToXml(XmlElement* xml)
{

    XmlElement* state = xml->createNewChildElement ("FILENAMECONFIG");
    for (auto field : fields)
    {
        XmlElement* currentField = state->createNewChildElement(String(field->types[field->type]).toUpperCase());
        currentField->setAttribute ("state", field->state);
        currentField->setAttribute ("value", field->value);
    }

}

/** Load settings. */
void FilenameConfigWindow::loadStateFromXml(XmlElement* xml)
{

    forEachXmlChildElement (*xml, xmlNode)
    {
        if (xmlNode->hasTagName ("FILENAMECONFIG"))
        {

            forEachXmlChildElement (*xmlNode, fieldNode)
            {
                FilenameFieldComponent::Type type;

                if (fieldNode->hasTagName ("PREPEND"))
                    type = FilenameFieldComponent::Type::PREPEND;
                else if (fieldNode->hasTagName ("MAIN"))
                    type = FilenameFieldComponent::Type::MAIN;
                else 
                    type = FilenameFieldComponent::Type::APPEND;

                fields[type]->state = static_cast<FilenameFieldComponent::State>(fieldNode->getIntAttribute("state",0));
                fields[type]->stateButton->setButtonText(fields[type]->states[fields[type]->state]);
                fields[type]->value = fieldNode->getStringAttribute("value","");
                fields[type]->valueLabel->setText(fields[type]->value, sendNotification);
                if (fields[type]->state == FilenameFieldComponent::State::CUSTOM)
                {
                    fields[type]->valueLabel->setEditable(true, sendNotification);
                    fields[type]->valueLabel->setColour(Label::ColourIds::backgroundColourId, Colours::white);
                }

            }

        }

    }

}