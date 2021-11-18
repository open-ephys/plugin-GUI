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

CustomSlider::CustomSlider()
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
            setNumDecimalPlacesToDisplay (2);
        else if (10 <= getValue() && getValue() < 100)
            setNumDecimalPlacesToDisplay (1);
        else
            setNumDecimalPlacesToDisplay (0);
    };
}

void CustomSlider::mouseDown (const MouseEvent& event)
{
    Slider::mouseDown (event);

    setMouseCursor (MouseCursor::NoCursor);
}

void CustomSlider::mouseUp (const MouseEvent& event)
{
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
    auto arcRadius = radius - lineW * 1.6f;

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

    auto thumbWidth = lineW * 2.0f;

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
    parameterNameLabel->setFont(Font("Small Text", 12, Font::plain));
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    slider = std::make_unique<CustomSlider>();
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

    setBounds(0, 0, 120, 82);

}

void SliderParameterEditor::sliderValueChanged(Slider* slider)
{
    param->setNextValue(slider->getValue());
}

void SliderParameterEditor::updateView()
{
    if (param != nullptr)
    {
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
        
    repaint();
}

void SliderParameterEditor::resized()
{

    parameterNameLabel->setBounds(0, 0, 80, 20);
    slider->setBounds(0, 22, 60, 60);
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
    
    std::cout << "Setting max selectable channels to " << p->getMaxSelectableChannels() << std::endl;
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



MaskChannelsParameterEditor::MaskChannelsParameterEditor(Parameter* param) : ParameterEditor(param)
{

    button = std::make_unique<UtilityButton>("Channels", Font("Default", 10, Font::plain));
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

    MaskChannelsParameter* p = (MaskChannelsParameter*)param;
    
    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector(this, channelStates);

    channelSelector->setChannelButtonColour(Colour(0, 174, 239));

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            button->getScreenBounds(),
            nullptr);

    //myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
}

void MaskChannelsParameterEditor::updateView()
{
}

void MaskChannelsParameterEditor::resized()
{
    button->setBounds(0, 0, 80, 20);
}
