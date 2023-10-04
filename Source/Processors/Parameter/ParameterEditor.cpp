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

void ParameterEditor::setLayout(Layout newLayout)
{
    layout = newLayout;
    updateBounds();
}

void ParameterEditor::updateBounds()
{
    if(label == nullptr || editor == nullptr)
        return;
        
    Rectangle<int> bounds = getBounds();
    int finalWidth = bounds.getWidth();
    int finalHeight = bounds.getHeight();

    switch (layout)
    {
        case nameOnTop:
            label->setBounds(0, 0, finalWidth, finalHeight / 2);
            label->setJustificationType(Justification::centredLeft);
            label->setVisible(true);
            editor->setBounds(0, finalHeight /2, finalWidth, finalHeight /2);
            break;
        case nameOnBottom:
            label->setBounds(0, finalHeight / 2, finalWidth, finalHeight / 2);
            label->setJustificationType(Justification::centredLeft);
            label->setVisible(true);
            editor->setBounds(0, 0, finalWidth, finalHeight / 2);
            break;
        case nameOnLeft:
            label->setBounds(0, 0, (finalWidth / 2) - 5, finalHeight);
            label->setJustificationType(Justification::centredRight);
            label->setVisible(true);
            editor->setBounds((finalWidth / 2)+ 5, 0, (finalWidth/ 2) - 5, finalHeight);
            break;
        case nameOnRight:
            label->setBounds((finalWidth / 2) + 5 , 0, (finalWidth / 2) - 5, finalHeight);
            label->setJustificationType(Justification::centredLeft);
            label->setVisible(true);
            editor->setBounds(0, 0, (finalWidth / 2) - 5, finalHeight);
            break;
        case nameHidden:
            label->setVisible(false);
            editor->setBounds(0, 0, finalWidth, finalHeight);
            break;

    }
}


/********* CustomTextBox *********/
TextEditor* CustomTextBox::createEditorComponent()
{
    TextEditor* const ed = Label::createEditorComponent();
    ed->setInputRestrictions(0, allowedChars);
    return ed;
}

TextBoxParameterEditor::TextBoxParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels)
    : ParameterEditor(param)
{
    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM
        || param->getType() == Parameter::STRING_PARAM);

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = Font("Arial", "Regular", int(0.75*rowHeightPixels));
    int width = rowWidthPixels;
    label->setFont(labelFont);
    label->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(label.get());

    if(param->getType() == Parameter::FLOAT_PARAM)
        valueTextBox = std::make_unique<CustomTextBox>("Parameter value", String(float(param->getValue())), "0123456789.");
    else if(param->getType() == Parameter::INT_PARAM)
        valueTextBox = std::make_unique<CustomTextBox>("Parameter value", String(int(param->getValue())), "0123456789");
    else
        valueTextBox = std::make_unique<CustomTextBox>("Parameter value", param->getValue().toString(), "");

    valueTextBox->setFont(Font("CP Mono", "Plain", int(0.75*rowHeightPixels)));
    valueTextBox->setName(param->getKey());
    valueTextBox->setColour(Label::textColourId, Colours::black);
    valueTextBox->setColour(Label::backgroundColourId, Colours::lightgrey);
    valueTextBox->setJustificationType(Justification::centred);
    valueTextBox->setEditable(true);
    valueTextBox->addListener(this);
    valueTextBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueTextBox.get());

    setBounds(0, 0, width, rowHeightPixels);
    label->setBounds(width / 2, 0, getWidth() - 50, rowHeightPixels);
    valueTextBox->setBounds(0, 0, width/2, rowHeightPixels);

    editor = (Component*)valueTextBox.get();
}

void TextBoxParameterEditor::labelTextChanged(Label* label)
{
    if(param->getType() == Parameter::FLOAT_PARAM)
        param->setNextValue(label->getText().getFloatValue());
    else if(param->getType() == Parameter::INT_PARAM)
        param->setNextValue(label->getText().getIntValue());
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
    updateBounds();
}


void CustomToggleButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    // Set the color based on the button state
    if (getToggleState())
    {
        g.setColour(juce::Colours::orange);
    }
    else
    {
        g.setColour(juce::Colours::grey);
    }

    // Draw a rounded rectangle
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 3.0f);

    // Set the text color
    g.setColour(juce::Colours::black);

    // Draw a rounded rectangle border
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 3.0f, 1.0f);

    // Set the text font
    g.setFont(Font("Fira Sans", "Regular", int(0.75*getHeight())));

    // Set the text based on the button state
    if (getToggleState())
    {
        g.drawText("ON", getLocalBounds(), juce::Justification::centred);
    }
    else
    {
        g.drawText("OFF", getLocalBounds(), juce::Justification::centred);
    }
}

ToggleParameterEditor:: ToggleParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::BOOLEAN_PARAM);

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    label->setFont(Font("Arial", "Regular", int(0.75*rowHeightPixels)));
    label->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(label.get());

    toggleButton = std::make_unique<CustomToggleButton>();
    toggleButton->setName(param->getKey());
    toggleButton->setToggleState(bool(param->getValue()), dontSendNotification);
    toggleButton->addListener(this);
    toggleButton->setTooltip(param->getDescription());
    addAndMakeVisible(toggleButton.get());

    int width = rowWidthPixels;

    label->setBounds(width / 2, 0, width / 2, rowHeightPixels);
    toggleButton->setBounds(0, 0, width / 2, rowHeightPixels);
    setBounds(0, 0, width, rowHeightPixels);

    editor = (Component*)toggleButton.get();
}

void ToggleParameterEditor::buttonClicked(Button* button)
{
    if (param != nullptr)
        param->setNextValue(button->getToggleState());
}

void ToggleParameterEditor::updateView()
{
    if (param != nullptr)
        toggleButton->setToggleState(param->getValue(), dontSendNotification);
    
    
    repaint();
}

void ToggleParameterEditor::resized()
{
    updateBounds();
}


ComboBoxParameterEditor::ComboBoxParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::CATEGORICAL_PARAM
        || param->getType() == Parameter::INT_PARAM);

    label = std::make_unique<Label>("Parameter name", param->getDisplayName()); // == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    label->setFont(Font("Arial", "Regular", int(0.75*rowHeightPixels)));
    addAndMakeVisible(label.get());

    valueComboBox = std::make_unique<ComboBox>();
    valueComboBox->setName(param->getKey());
    valueComboBox->setJustificationType(Justification::centred);
    valueComboBox->addListener(this);
    valueComboBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueComboBox.get());

    if (param->getType() == Parameter::CATEGORICAL_PARAM)
    {
        CategoricalParameter* p = (CategoricalParameter*)param;

        offset = 1;

        const Array<String>& categories = p->getCategories();

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

    int width = rowWidthPixels;

    setBounds(0, 0, width, rowHeightPixels);
    label->setBounds(width / 2, 0, width/2, rowHeightPixels);
    valueComboBox->setBounds(0, 0, width/2, rowHeightPixels);

    editor = (Component*)valueComboBox.get();
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
    updateBounds();
}


CustomSlider::CustomSlider() : isEnabled(true)
{
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


SliderParameterEditor::SliderParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM);

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    label->setFont(Font("Silkscreen", "Regular", 12));
    addAndMakeVisible(label.get());

    slider = std::make_unique<CustomSlider>();
    slider->setName(param->getKey());
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

    setBounds(0, 0, rowWidthPixels, 65);

    editor = (Component*)slider.get();
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

            //slider->setValue(p->getFloatValue(), dontSendNotification);
        }
        else {
            IntParameter* p = (IntParameter*)param;

            //slider->setValue(p->getIntValue(), dontSendNotification);
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
    updateBounds();
}


TextEditor* BoundedValueEditor::createEditorComponent()
{
    auto* editor = new juce::TextEditor(getComponentID());

    editor->setJustification(juce::Justification::centred);
    editor->setInputRestrictions(0, "0123456789.");

    return editor;
}

void BoundedValueEditor::paint(juce::Graphics& g)
{
    // Get the label's value as a percentage
    float value = getText().getFloatValue();

    // Calculate the width of the colored area based on the value
    int coloredWidth = static_cast<int>(getWidth() * (value - minValue) / (maxValue - minValue));

    // Fill the colored area
    g.setColour(getLookAndFeel().findColour(ProcessorColor::IDs::FILTER_COLOR));
    g.fillRect(1, 1, coloredWidth > 3 ? coloredWidth - 2 : 2, getHeight() - 2);

    // Fill the rest of the background with another color
    g.setColour(getLookAndFeel().findColour(TextEditor::backgroundColourId));
    g.fillRect(coloredWidth > 0 ? coloredWidth : 1, 1, getWidth() - coloredWidth > 1 ? getWidth() - coloredWidth - 1 : 1, getHeight() - 2);

    // Draw a rounded border
    g.setColour(Colours::black);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f, 0.5f), 3.0f, 1.0f);

    // Call the base class method to ensure the text is drawn
    juce::Label::paint(g);
}

void BoundedValueEditor::mouseDrag(const MouseEvent& event)
{
    // Calculate the new value based on the mouse position
    double newValue = static_cast<float>(event.position.x) / getWidth() * (maxValue - minValue) + minValue;

    // Clamp the new value to the range [minValue, maxValue]
    newValue = jlimit(minValue, maxValue, newValue);

    float multiplier = std::pow(10.0f, -std::log10(stepSize));
    newValue = std::round(newValue * multiplier) / multiplier;

    setText(String(newValue) + " " + units, juce::dontSendNotification);

    mouseWasDragged = true;

    // Redraw the component
    repaint();
}

void BoundedValueEditor::mouseUp (const MouseEvent& event)
{
    if (mouseWasDragged)
    {
        callChangeListeners();
        mouseWasDragged = false;
    }
    else
    {
        juce::Label::mouseUp(event);
    }
}


BoundedValueParameterEditor::BoundedValueParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{
    
    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM);

    /* TODO: Unit should be displayed next to actual value while not editing
    String unit = "";
    if (param->getType() == Parameter::FLOAT_PARAM) unit = ((FloatParameter*)param)->getUnit();
    unit = (unit != "" ? " [" + ((FloatParameter*)param)->getUnit() + "]" : "");
    */

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = Font("Arial", "Regular", int(0.75*rowHeightPixels));
    label->setFont(labelFont);
    addAndMakeVisible(label.get());

    if (param->getType() == Parameter::FLOAT_PARAM)
    {
        FloatParameter* p = (FloatParameter*)param;
        valueEditor = std::make_unique<BoundedValueEditor>(p->getMinValue(), p->getMaxValue(), p->getStepSize(), p->getUnit());
        valueEditor->setText(String(p->getFloatValue()), dontSendNotification);
    }
    else {
        IntParameter* p = (IntParameter*)param;
        valueEditor = std::make_unique<BoundedValueEditor>(p->getMinValue(), p->getMaxValue(), 1);
        valueEditor->setText(String(p->getIntValue()), dontSendNotification);
    }
    valueEditor->setName(param->getKey());
    valueEditor->setFont(Font("Fira Sans", "Regular", int(0.75*rowHeightPixels)));
    valueEditor->setJustificationType(Justification::centred);
    valueEditor->addListener(this);
    valueEditor->setTooltip(param->getDescription());
    labelTextChanged(valueEditor.get());
    addAndMakeVisible(valueEditor.get());

    label->setBounds(rowWidthPixels / 2, 0, rowWidthPixels / 2, rowHeightPixels);
    valueEditor->setBounds(0, 0, rowWidthPixels/2, rowHeightPixels);

    setBounds(0, 0, rowWidthPixels, rowHeightPixels);

    editor = (Component*)valueEditor.get();
    
}

void BoundedValueParameterEditor::labelTextChanged(Label* label)
{
    if (param != nullptr)
    {
        if (param->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*)param;
            if (label->getText().getFloatValue() < p->getMinValue())
                label->setText(String(p->getMinValue()), dontSendNotification);
            else if (label->getText().getFloatValue() > p->getMaxValue())
                label->setText(String(p->getMaxValue()), dontSendNotification);

            param->setNextValue(label->getText().getFloatValue());
        }
        else 
        {
            IntParameter* p = (IntParameter*)param;
            if (label->getText().getIntValue() < p->getMinValue())
                label->setText(String(p->getMinValue()), dontSendNotification);
            else if (label->getText().getIntValue() > p->getMaxValue())
                label->setText(String(p->getMaxValue()), dontSendNotification);

            param->setNextValue(int(label->getText().getFloatValue()));
        }

    }
}

void BoundedValueParameterEditor::updateView()
{
    if (param != nullptr)
    {

        if (param->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*)param;
            valueEditor->setText(String(p->getFloatValue()), dontSendNotification);
        }
        else {
            IntParameter* p = (IntParameter*)param;
            valueEditor->setText(String(p->getIntValue()), dontSendNotification);
        }
    }

    repaint();
}

void BoundedValueParameterEditor::resized()
{
    updateBounds();
}


SelectedChannelsParameterEditor::SelectedChannelsParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{
    int selectedChannels = ((SelectedChannelsParameter*)param)->getArrayValue().size();
    int numChannels = ((SelectedChannelsParameter*)param)->getChannelStates().size();

    button = std::make_unique<TextButton>(String(selectedChannels) + "/" + String(numChannels));
    button->setName(param->getKey());
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip(param->getDescription());
    addAndMakeVisible(button.get());

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = Font("Arial", "Regular", int(0.75*rowHeightPixels));
    label->setFont(labelFont);
    label->setJustificationType(Justification::left);
    addAndMakeVisible(label.get());

    setBounds(0, 0, rowWidthPixels, rowHeightPixels);
    label->setBounds(rowWidthPixels / 2, 0, rowWidthPixels / 2, rowHeightPixels);
    button->setBounds(0, 0, rowWidthPixels/2, rowHeightPixels);

    editor = (Component*)button.get();
}

void SelectedChannelsParameterEditor::channelStateChanged(Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add(newChannels[i]);
    
    param->setNextValue(newArray);

    updateView();

}

void SelectedChannelsParameterEditor::buttonClicked(Button* button_)
{
    if (param == nullptr)
        return;

    SelectedChannelsParameter* p = (SelectedChannelsParameter*)param;

    auto* channelSelector = new PopupChannelSelector(this, p->getChannelStates());

    channelSelector->setChannelButtonColour(param->getColor());
    
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
    {
        button->setEnabled(true);
        int numChannels = ((SelectedChannelsParameter*)param)->getChannelStates().size();
        int selected = 0;
        for (auto chan : ((SelectedChannelsParameter*)param)->getChannelStates())
            if (chan) selected++;
        button->setButtonText(String(selected) + "/" + String(numChannels));
    }
}

void SelectedChannelsParameterEditor::resized()
{
    updateBounds();
}


MaskChannelsParameterEditor::MaskChannelsParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{

    int numChannels = ((MaskChannelsParameter*)param)->getChannelStates().size();
    int selected = 0;
    for (auto chan : ((MaskChannelsParameter*)param)->getChannelStates())
        if (chan)
            selected++;

    button = std::make_unique<TextButton>(String(selected) + "/" + String(numChannels));
    button->setName(param->getKey());
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip("Mask channels to filter within this stream");
    addAndMakeVisible(button.get());

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = Font("Arial", "Regular", int(0.75*rowHeightPixels));
    label->setFont(labelFont);
    label->setJustificationType(Justification::left);
    addAndMakeVisible(label.get());

    int width = rowWidthPixels;

    setBounds(0, 0, width, rowHeightPixels);
    label->setBounds(width / 2, 0, getWidth() - 50, rowHeightPixels);
    button->setBounds(0, 0, width/2, rowHeightPixels);

    editor = (Component*)button.get();
}

void MaskChannelsParameterEditor::channelStateChanged(Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add(newChannels[i]);
    
    param->setNextValue(newArray);
    
    updateView();

}

void MaskChannelsParameterEditor::buttonClicked(Button* button_)
{

    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*)param;
    
    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector(this, channelStates);

    channelSelector->setChannelButtonColour(param->getColor());

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
    {
        button->setEnabled(true);
        int numChannels = ((MaskChannelsParameter*)param)->getChannelStates().size();
        int selected = 0;
        for (auto chan : ((MaskChannelsParameter*)param)->getChannelStates())
            if (chan) selected++;
        button->setButtonText(String(selected) + "/" + String(numChannels));
    }
}

void MaskChannelsParameterEditor::resized()
{
    updateBounds();
}


PathParameterEditor::PathParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{
    setBounds(0, 0, rowWidthPixels, rowHeightPixels);

    button = std::make_unique<TextButton>("Browse");
    button->setName(param->getKey());
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip(param->getValueAsString());
    addAndMakeVisible(button.get());

    label = std::make_unique<Label>("Parameter name", param->getDisplayName());// == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = Font("Arial", "Regular", int(0.75*rowHeightPixels));
    label->setFont(labelFont);
    label->setJustificationType(Justification::left);
    addAndMakeVisible(label.get());

    int width = rowWidthPixels;

    label->setBounds(width / 2, 0, getWidth() - 50, rowHeightPixels);
    button->setBounds(0, 0, width/2, rowHeightPixels);

    editor = (Component*)button.get();
}

void PathParameterEditor::buttonClicked(Button* button_)
{
    String dialogBoxTitle = "Select a ";
    bool isDirectory = ((PathParameter*)param)->getIsDirectory();
    String validFilePatterns;

    if (isDirectory)
    {
        dialogBoxTitle += "directory...";
    }
    else
    {
        dialogBoxTitle += "file...";
        validFilePatterns = "*." + ((PathParameter*)param)->getValidFilePatterns().joinIntoString(";.");
    }

    FileChooser chooser(dialogBoxTitle, File(), validFilePatterns);

    bool success = isDirectory ? chooser.browseForDirectory() : chooser.browseForFileToOpen();
    if (success)
    {
        File file = chooser.getResult();
        param->setNextValue(file.getFullPathName());
        button->setTooltip(file.getFullPathName());
    }
}

void PathParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled(false);
    else
        button->setEnabled(true);

    if (param)
        button->setButtonText(param->getValueAsString());
    //button->setButtonText(File(param->getValueAsString()).getFileName());
}

void PathParameterEditor::resized()
{
    updateBounds();
}


SelectedStreamParameterEditor::SelectedStreamParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::SELECTED_STREAM_PARAM
        || param->getType() == Parameter::INT_PARAM);

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    label->setFont(Font("Arial", "Regular", int(0.75*rowHeightPixels)));
    addAndMakeVisible(label.get());

    valueComboBox = std::make_unique<ComboBox>();
    valueComboBox->setName(param->getKey());
    valueComboBox->setJustificationType(Justification::centred);
    valueComboBox->addListener(this);
    valueComboBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueComboBox.get());

    if (param->getType() == Parameter::SELECTED_STREAM_PARAM)
    {
        SelectedStreamParameter* p = (SelectedStreamParameter*)param;

        Array<String>& streams = p->getStreamNames();

        for (int i = 0; i < streams.size(); i++)
            valueComboBox->addItem(streams[i], i + 1);

        valueComboBox->setSelectedId(p->getSelectedIndex() + 1, dontSendNotification);

    }
    else {
        IntParameter* p = (IntParameter*)param;

        for (int i = p->getMinValue(); i <= p->getMaxValue(); i++)
        {
            valueComboBox->addItem(String(i), i + 1);
        }

        valueComboBox->setSelectedId(p->getIntValue() + 1, dontSendNotification);
    }

    int width = rowWidthPixels;

    setBounds(0, 0, width, rowHeightPixels);
    label->setBounds(width / 2, 0, width / 2, rowHeightPixels);
    valueComboBox->setBounds(0, 0, width / 2, rowHeightPixels);

    editor = (Component*)valueComboBox.get();

}

void SelectedStreamParameterEditor::comboBoxChanged(ComboBox* comboBox)
{
    if (param != nullptr)
        param->setNextValue(comboBox->getSelectedId() - 1);
}

void SelectedStreamParameterEditor::updateView()
{
    if (param == nullptr)
        valueComboBox->setEnabled(false);

    else
    {
        if (param->getType() == Parameter::SELECTED_STREAM_PARAM)
        {
            valueComboBox->setEnabled(true);
            valueComboBox->clear(dontSendNotification);

            SelectedStreamParameter* p = (SelectedStreamParameter*)param;
            Array<String>& streams = p->getStreamNames();

            for (int i = 0; i < streams.size(); i++)
            {
                valueComboBox->addItem(streams[i], i + 1);
            }

            valueComboBox->setSelectedId(p->getSelectedIndex() + 1, dontSendNotification);
        }
    }
    // repaint();
}

void SelectedStreamParameterEditor::resized()
{
    updateBounds();
}


TimeParameterEditor::TimeParameterEditor(Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::TIME_PARAM);

    label = std::make_unique<Label>("Parameter name", param->getDisplayName() == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    label->setFont(Font("Arial", "Regular", int(0.75*rowHeightPixels)));
    addAndMakeVisible(label.get());

    button = std::make_unique<TextButton>(param->getValueAsString());
    button->setName(param->getKey());
    button->addListener(this);
    button->setClickingTogglesState(false);
    button->setTooltip("Set a time...");
    addAndMakeVisible(button.get());

    int width = rowWidthPixels;

    setBounds(0, 0, width, rowHeightPixels);
    label->setBounds(width / 2, 0, width / 2, rowHeightPixels);
    button->setBounds(0, 0, width / 2, rowHeightPixels);

    editor = (Component*)button.get();

    startTimer(200);
}

void TimeParameterEditor::buttonClicked(Button* button)
{
    if (param == nullptr)
        return;

    TimeParameter* p = (TimeParameter*)param;

    auto* timeEditor = new PopupTimeEditor(p);

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(timeEditor),
            button->getScreenBounds(),
            nullptr);
}

void TimeParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled(false);
    else
        button->setEnabled(true);
}

void TimeParameterEditor::resized()
{
    updateBounds();
}

void TimeParameterEditor::timerCallback()
{
    if (param != nullptr)
        button->setButtonText(((TimeParameter*)param)->getTimeValue()->toString());
    else
        button->setButtonText("00:00:00.000");
}
