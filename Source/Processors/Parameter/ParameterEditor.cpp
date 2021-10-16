/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "ParameterEditor.h"


IntParameterEditor::IntParameterEditor(IntParameter* param)
{
    name = new Label("Parameter name", param->getName());
    name->setFont(Font("Small Text", 12, Font::plain));
    name->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(name);

    value = new Label("Parameter value", String(int(param->getValue())));
    value->setFont(Font("Default", 15, Font::plain));
    value->setColour(Label::textColourId, Colours::white);
    value->setColour(Label::backgroundColourId, Colours::grey);
    value->setEditable(true);
    value->addListener(this);
    value->setTooltip(param->getDescription());
    addAndMakeVisible(value);

    setBounds(0, 0, 80, 42);
}

void IntParameterEditor::labelTextChanged(Label* label)
{
    std::cout << "Label text: " << label->getText() << std::endl;

    param->setValue(label->getText().getIntValue());
}

void IntParameterEditor::updateView()
{
    if (param != nullptr)
    {
        std::cout << "Updating view: " << std::endl;
        std::cout << "Value is int?: " << param->getValue().isInt() << std::endl;
        std::cout << "Value: " << int(param->getValue()) << std::endl;
        std::cout << "streamId: " << param->getStreamId() << std::endl;

        value->setText(String(int(param->getValue())), dontSendNotification);
    }

}

void IntParameterEditor::resized()
{
    name->setBounds(0, 0, 80, 20);
    value->setBounds(0, 22, 60, 18);
}


BooleanParameterEditor::BooleanParameterEditor(BooleanParameter* param)
{
    name = new Label("Parameter name", param->getName());
    name->setFont(Font("Small Text", 12, Font::plain));
    name->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(name);

    value = new ToggleButton("Parameter value");
    value->setToggleState(bool(param->getValue()), dontSendNotification);
    value->addListener(this);
    value->setTooltip(param->getDescription());
    addAndMakeVisible(value);

    setBounds(0, 0, 80, 42);

}

void BooleanParameterEditor::buttonClicked(Button* button)
{
    param->setValue(button->getToggleState());
}

void BooleanParameterEditor::updateView()
{
    if (param != nullptr)
        value->setToggleState(param->getValue(), dontSendNotification);
}

void BooleanParameterEditor::resized()
{

    name->setBounds(0, 0, 80, 20);
    value->setBounds(0, 22, 60, 18);
}
