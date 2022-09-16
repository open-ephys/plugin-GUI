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

#include "../GenericProcessor/GenericProcessor.h"
#include "../Editors/GenericEditor.h"

TextBoxParameterEditor::TextBoxParameterEditor(Parameter* param) : ParameterEditor(param)
{
    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM
        || param->getType() == Parameter::STRING_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    Font labelFont = Font("Silkscreen", "Regular", 12);
    int labelWidth = labelFont.getStringWidth(param->getName());
    parameterNameLabel->setFont(labelFont);
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    if(param->getType() == Parameter::FLOAT_PARAM)
        valueTextBox = std::make_unique<Label>("Parameter value", String(float(param->getValue())));
    else
        valueTextBox = std::make_unique<Label>("Parameter value", param->getValue().toString());

    valueTextBox->setFont(Font("CP Mono", "Plain", 15));
    valueTextBox->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
    valueTextBox->setColour(Label::textColourId, Colours::white);
    valueTextBox->setColour(Label::backgroundColourId, Colours::grey);
    valueTextBox->setEditable(true);
    valueTextBox->addListener(this);
    valueTextBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueTextBox.get());
    
    finalWidth = std::max(labelWidth, 80);

    setBounds(0, 0, finalWidth, 42);
}

void TextBoxParameterEditor::labelTextChanged(Label* label)
{
    if(param->getType() == Parameter::FLOAT_PARAM)
        param->setNextValue(label->getText().getFloatValue());
    else
        param->setNextValue(label->getText());
}

void TextBoxParameterEditor::updateView()
{
    
    if (param != nullptr)
    {
        valueTextBox->setEditable(true);

        if(param->getType() == Parameter::FLOAT_PARAM)
            valueTextBox->setText(String(float(param->getValue())), dontSendNotification);
        else
            valueTextBox->setText(param->getValue().toString(), dontSendNotification);
    }
    else {
        valueTextBox->setEditable(false);
    }

}

void TextBoxParameterEditor::resized()
{
    parameterNameLabel->setBounds(0, 0, finalWidth, 20);
    valueTextBox->setBounds(0, 20, getWidth(), 18);
}


CheckBoxParameterEditor::CheckBoxParameterEditor(Parameter* param) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::BOOLEAN_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    parameterNameLabel->setFont(Font("Silkscreen", "Regular", 12));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    valueCheckBox = std::make_unique<ToggleButton>("Parameter value");
    valueCheckBox->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
    valueCheckBox->setToggleState(bool(param->getValue()), dontSendNotification);
    valueCheckBox->addListener(this);
    valueCheckBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueCheckBox.get());

    setBounds(0, 0, 80, 42);

}

void CheckBoxParameterEditor::buttonClicked(Button* button)
{
    if (param != nullptr)
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
    parameterNameLabel->setFont(Font("Silkscreen", "Regular", 12));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    valueComboBox = std::make_unique<ComboBox>();
    valueComboBox->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
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
    if (param != nullptr)
        param->setNextValue(comboBox->getSelectedId() - offset);
}

void ComboBoxParameterEditor::updateView()
{   
    if (param == nullptr)
    {
        for (int i = 0; i < valueComboBox->getNumItems(); i++)
            valueComboBox->setItemEnabled(valueComboBox->getItemId(i), false);

        return;
    }
    else 
    {

        if (param->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*)param;

            const StringArray& categories = p->getCategories();
            valueComboBox->clear(dontSendNotification);

            for (int i = 0; i < categories.size(); i++)
            {
                valueComboBox->addItem(categories[i], i + offset);
            }
        }

        for (int i = 0; i < valueComboBox->getNumItems(); i++)
            valueComboBox->setItemEnabled(valueComboBox->getItemId(i), true);
    }


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
    valueComboBox->setBounds(0, 20, 80, 18);
}

CustomSlider::CustomSlider() : isEnabled(true)
{
    
    setColour (Slider::textBoxTextColourId, Colours::black);
    setColour (Slider::textBoxOutlineColourId, Colours::grey);
    setLookAndFeel (&sliderLookAndFeel);
    
    setSliderStyle (Slider::SliderStyle::RotaryVerticalDrag);
    setRotaryParameters (MathConstants<float>::pi * 1.25f,
                         MathConstants<float>::pi * 2.75f,
                         true);
    setVelocityBasedMode (true);
    setVelocityModeParameters (0.5, 1, 0.09, false);
    setRange (0.0, 100.0, 0.01);
    setValue (50.0);
    onValueChange = [&]()
    {
        if (getValue() < 10)
            setNumDecimalPlacesToDisplay (1);
        else if (10 <= getValue() && getValue() < 100)
            setNumDecimalPlacesToDisplay (0);
        else
            setNumDecimalPlacesToDisplay (0);
    };
}

void CustomSlider::mouseDown (const MouseEvent& event)
{
    if (!isEnabled)
        return;

    Slider::mouseDown (event);

    setMouseCursor (MouseCursor::NoCursor);
}

void CustomSlider::mouseDrag(const MouseEvent& event)
{
    if (!isEnabled)
        return;

    Slider::mouseDrag(event);
}

void CustomSlider::mouseUp (const MouseEvent& event)
{
    if (!isEnabled)
        return;

    Slider::mouseUp (event);

    Desktop::getInstance().getMainMouseSource().setScreenPosition (event.source.getLastMouseDownPosition());

    setMouseCursor (MouseCursor::NormalCursor);
}

CustomSlider::~CustomSlider()
{
    setLookAndFeel(nullptr);
}

Slider::SliderLayout SliderLookAndFeel::getSliderLayout (juce::Slider& slider)
{
    auto localBounds = slider.getLocalBounds();

    Slider::SliderLayout layout;

    layout.textBoxBounds = localBounds;
    layout.sliderBounds = localBounds;

    return layout;
}

void SliderLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    auto fill = slider.findColour (Slider::rotarySliderFillColourId);

    auto bounds = Rectangle<float> (x, y, width, height).reduced (2.0f);
    auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = radius * 0.085f;
    auto arcRadius = radius - lineW * 1.9f;

    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColour (blackGrey);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    Path valueArc;
    valueArc.addCentredArc (bounds.getCentreX(),
                            bounds.getCentreY(),
                            arcRadius,
                            arcRadius,
                            0.0f,
                            rotaryStartAngle,
                            toAngle,
                            true);

    g.setColour (fill);
    g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    auto thumbWidth = radius * 0.085f * 2.0f;

    Path thumb;
    thumb.addRectangle (-thumbWidth / 2, -thumbWidth / 2, thumbWidth, radius + lineW);

    g.setColour (offWhite);
    g.fillPath (thumb, AffineTransform::rotation (toAngle + 3.12f).translated (bounds.getCentre()));

    g.fillEllipse (bounds.reduced (radius * 0.28));
}

Label* SliderLookAndFeel::createSliderTextBox (Slider& slider)
{
    auto* l = new Label();

    l->setFont (17.0f);
    l->setJustificationType (Justification::centred);
    l->setColour (Label::textColourId, slider.findColour (Slider::textBoxTextColourId));
    l->setColour (Label::textWhenEditingColourId, slider.findColour (Slider::textBoxTextColourId));
    l->setColour (Label::outlineWhenEditingColourId, slider.findColour (Slider::textBoxOutlineColourId));
    l->setInterceptsMouseClicks (false, false);

    return l;
}

SliderParameterEditor::SliderParameterEditor(Parameter* param) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    parameterNameLabel->setFont(Font("Silkscreen", "Regular", 12));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    slider = std::make_unique<CustomSlider>();
    slider->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
    slider->addListener(this);
    slider->setTooltip(param->getDescription());
    
    slider->setColour (Slider::rotarySliderFillColourId,
                       Colour(0, 174, 239));
    addAndMakeVisible(slider.get());

    if (param->getType() == Parameter::FLOAT_PARAM)
    {
        FloatParameter* p = (FloatParameter*)param;

        slider->setRange(p->getMinValue(), p->getMaxValue(), p->getStepSize());
        slider->setValue(p->getFloatValue(), dontSendNotification);
    }
    else {
        IntParameter* p = (IntParameter*)param;

        slider->setRange(p->getMinValue(), p->getMaxValue(), 1);
        slider->setValue(p->getIntValue(), dontSendNotification);
    }

    setBounds(0, 0, 80, 65);

}

void SliderParameterEditor::sliderValueChanged(Slider* slider)
{

    if (param != nullptr)
        param->setNextValue(slider->getValue());
}

void SliderParameterEditor::updateView()
{
    if (param != nullptr)
    {
        slider->setColour(Slider::rotarySliderFillColourId,
            Colour(0, 174, 239));

        if (param->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*)param;

            slider->setValue(p->getFloatValue(), dontSendNotification);
        }
        else {
            IntParameter* p = (IntParameter*)param;

            slider->setValue(p->getIntValue(), dontSendNotification);
        }
    }
    else {
        slider->setColour(Slider::rotarySliderFillColourId,
            Colour(150, 150, 150));
    }
        
    repaint();
}

void SliderParameterEditor::resized()
{
    parameterNameLabel->setBounds(0, 0, 80, 15);
    slider->setBounds(0, 15, 50, 50);
}


SelectedChannelsParameterEditor::SelectedChannelsParameterEditor(Parameter* param) : ParameterEditor(param)
{

    button = std::make_unique<UtilityButton>(param->getName(), Font("CP Mono", "Plain", 10));
    button->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip(param->getDescription());
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
    if (param == nullptr)
        return;

    SelectedChannelsParameter* p = (SelectedChannelsParameter*)param;

    auto* channelSelector = new PopupChannelSelector(this, p->getChannelStates());

    channelSelector->setChannelButtonColour(Colour(0, 174, 239));
    
    channelSelector->setMaximumSelectableChannels(p->getMaxSelectableChannels());

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            button->getScreenBounds(),
            nullptr);
}

void SelectedChannelsParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled(false);

    else
        button->setEnabled(true);
}

void SelectedChannelsParameterEditor::resized()
{
    button->setBounds(0, 0, 80, 20);
}



MaskChannelsParameterEditor::MaskChannelsParameterEditor(Parameter* param) : ParameterEditor(param)
{

    button = std::make_unique<UtilityButton>(param->getName(), Font("CP Mono", "Plain", 10));
    button->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip("Mask channels to filter within this stream");
    addAndMakeVisible(button.get());

    setBounds(0, 0, 80, 42);
}

void MaskChannelsParameterEditor::channelStateChanged(Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add(newChannels[i]);
    
    param->setNextValue(newArray);

}

void MaskChannelsParameterEditor::buttonClicked(Button* button_)
{

    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*)param;
    
    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector(this, channelStates);

    channelSelector->setChannelButtonColour(Colour(0, 174, 239));

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            button->getScreenBounds(),
            nullptr);
}

void MaskChannelsParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled(false);

    else
        button->setEnabled(true);
}

void MaskChannelsParameterEditor::resized()
{
    button->setBounds(0, 0, 80, 20);
}
