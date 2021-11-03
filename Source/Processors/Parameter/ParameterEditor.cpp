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

#include "../Editors/GenericEditor.h"

TextBoxParameterEditor::TextBoxParameterEditor(Parameter* param) : ParameterEditor(param)
{
    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM
        || param->getType() == Parameter::STRING_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    parameterNameLabel->setFont(Font("Small Text", 12, Font::plain));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    valueTextBox = std::make_unique<Label>("Parameter value", String(int(param->getValue())));
    valueTextBox->setFont(Font("Default", 15, Font::plain));
    valueTextBox->setColour(Label::textColourId, Colours::white);
    valueTextBox->setColour(Label::backgroundColourId, Colours::grey);
    valueTextBox->setEditable(true);
    valueTextBox->addListener(this);
    valueTextBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueTextBox.get());

    setBounds(0, 0, 80, 42);
}

void TextBoxParameterEditor::labelTextChanged(Label* label)
{
    std::cout << "Label text: " << label->getText() << std::endl;

    param->setNextValue(label->getText().getFloatValue());
}

void TextBoxParameterEditor::updateView()
{
    
    if (param != nullptr)
    {
        std::cout << "Updating view for " << param->getName() << std::endl;
        std::cout << "Value is int?: " << param->getValue().isInt() << std::endl;
        std::cout << "Value: " << int(param->getValue()) << std::endl;

        valueTextBox->setText(String(float(param->getValue())), dontSendNotification);
    }

}

void TextBoxParameterEditor::resized()
{
    parameterNameLabel->setBounds(0, 0, 80, 20);
    valueTextBox->setBounds(0, 22, getWidth(), 18);
}


CheckBoxParameterEditor::CheckBoxParameterEditor(Parameter* param) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::BOOLEAN_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    parameterNameLabel->setFont(Font("Small Text", 12, Font::plain));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    valueCheckBox = std::make_unique<ToggleButton>("Parameter value");
    valueCheckBox->setToggleState(bool(param->getValue()), dontSendNotification);
    valueCheckBox->addListener(this);
    valueCheckBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueCheckBox.get());

    setBounds(0, 0, 80, 42);

}

void CheckBoxParameterEditor::buttonClicked(Button* button)
{
    param->setNextValue(button->getToggleState());
}

void CheckBoxParameterEditor::updateView()
{
    if (param != nullptr)
        valueCheckBox->setToggleState(param->getValue(), dontSendNotification);
    
    repaint();
}

void CheckBoxParameterEditor::resized()
{

    parameterNameLabel->setBounds(0, 0, 80, 20);
    valueCheckBox->setBounds(0, 22, 60, 18);
}


ComboBoxParameterEditor::ComboBoxParameterEditor(Parameter* param) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::CATEGORICAL_PARAM
        || param->getType() == Parameter::INT_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    parameterNameLabel->setFont(Font("Small Text", 12, Font::plain));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    valueComboBox = std::make_unique<ComboBox>(param->getName());
    valueComboBox->addListener(this);
    valueComboBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueComboBox.get());

    if (param->getType() == Parameter::CATEGORICAL_PARAM)
    {
        CategoricalParameter* p = (CategoricalParameter*)param;

        offset = 1;

        const StringArray& categories = p->getCategories();

        for (int i = 0; i < categories.size(); i++)
        {
            valueComboBox->addItem(categories[i], i + offset);
        }

        valueComboBox->setSelectedId(p->getSelectedIndex() + offset, dontSendNotification);

    }
    else {
        IntParameter* p = (IntParameter*)param;

        offset = -(p->getMinValue()) + 1;

        for (int i = p->getMinValue(); i <= p->getMaxValue(); i++)
        {
            valueComboBox->addItem(String(i), i + offset);
        }

        valueComboBox->setSelectedId(p->getIntValue() + offset, dontSendNotification);
    }

    setBounds(0, 0, 80, 42);
}


void ComboBoxParameterEditor::comboBoxChanged(ComboBox* comboBox)
{
    param->setNextValue(comboBox->getSelectedId() - offset);
}

void ComboBoxParameterEditor::updateView()
{
    if (param->getType() == Parameter::CATEGORICAL_PARAM)
    {
        CategoricalParameter* p = (CategoricalParameter*)param;

        valueComboBox->setSelectedId(p->getSelectedIndex() + offset, dontSendNotification);

    }
    else {
        IntParameter* p = (IntParameter*)param;

        valueComboBox->setSelectedId(p->getIntValue() + offset, dontSendNotification);
    }
    
    repaint();

}

void ComboBoxParameterEditor::resized()
{

    parameterNameLabel->setBounds(0, 0, 80, 20);
    valueComboBox->setBounds(0, 22, 80, 18);
}


SliderParameterEditor::SliderParameterEditor(Parameter* param) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    parameterNameLabel->setFont(Font("Small Text", 12, Font::plain));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    valueSlider = std::make_unique<Slider>("Parameter value");
    valueSlider->addListener(this);
    valueSlider->setTooltip(param->getDescription());
    addAndMakeVisible(valueSlider.get());

    if (param->getType() == Parameter::FLOAT_PARAM)
    {
        FloatParameter* p = (FloatParameter*)param;

        valueSlider->setRange(p->getMinValue(), p->getMaxValue(), p->getStepSize());
        valueSlider->setValue(p->getFloatValue(), dontSendNotification);
    }
    else {
        IntParameter* p = (IntParameter*)param;

        valueSlider->setRange(p->getMinValue(), p->getMaxValue(), 1);
        valueSlider->setValue(p->getIntValue(), dontSendNotification);
    }

    setBounds(0, 0, 100, 42);

}

void SliderParameterEditor::sliderValueChanged(Slider* slider)
{
    param->setNextValue(slider->getValue());
}

void SliderParameterEditor::updateView()
{
    if (param != nullptr)
        valueSlider->setValue(param->getValue(), dontSendNotification);
    
    repaint();
}

void SliderParameterEditor::resized()
{

    parameterNameLabel->setBounds(0, 0, 80, 20);
    valueSlider->setBounds(0, 22, 100, 18);
}


SelectedChannelsParameterEditor::SelectedChannelsParameterEditor(Parameter* param) : ParameterEditor(param)
{

    button = std::make_unique<UtilityButton>("Channels", Font("Default", 10, Font::plain));
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip("Select channels to filter within this stream");
    addAndMakeVisible(button.get());

    setBounds(0, 0, 80, 42);
}

void SelectedChannelsParameterEditor::channelStateChanged(Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add(newChannels[i]);
    
    param->setNextValue(newArray);

}

void SelectedChannelsParameterEditor::buttonClicked(Button* button_)
{

    SelectedChannelsParameter* p = (SelectedChannelsParameter*)param;

    auto* channelSelector = new PopupChannelSelector(this, p->getChannelStates());

    channelSelector->setChannelButtonColour(Colour(0, 174, 239));
    channelSelector->setMaximumSelectableChannels(p->getMaxSelectableChannels());

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            button->getScreenBounds(),
            nullptr);

    //myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
}

void SelectedChannelsParameterEditor::updateView()
{
}

void SelectedChannelsParameterEditor::resized()
{
    button->setBounds(0, 0, 80, 20);
}
