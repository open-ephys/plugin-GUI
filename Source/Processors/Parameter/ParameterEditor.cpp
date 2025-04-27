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

#include "ParameterEditor.h"

#include "../Editors/GenericEditor.h"
#include "../GenericProcessor/GenericProcessor.h"

void ParameterEditor::setLayout (Layout newLayout)
{
    layout = newLayout;
    updateBounds();
}

void ParameterEditor::updateBounds()
{
    if (label == nullptr || editor == nullptr)
        return;

    Rectangle<int> bounds = getBounds();
    int finalWidth = bounds.getWidth();
    int finalHeight = bounds.getHeight();
    int gap = finalWidth < 180 ? 2 : 4;

    switch (layout)
    {
        case nameOnTop:
            label->setBounds (0, 0, finalWidth, finalHeight / 2);
            label->setJustificationType (Justification::centredLeft);
            label->setVisible (true);
            editor->setBounds (0, finalHeight / 2, finalWidth, finalHeight / 2);
            break;
        case nameOnBottom:
            label->setBounds (0, finalHeight / 2, finalWidth, finalHeight / 2);
            label->setJustificationType (Justification::centredLeft);
            label->setVisible (true);
            editor->setBounds (0, 0, finalWidth, finalHeight / 2);
            break;
        case nameOnLeft:
            label->setBounds (0, 0, (finalWidth / 2) - gap, finalHeight);
            label->setJustificationType (Justification::centredRight);
            label->setVisible (true);
            editor->setBounds ((finalWidth / 2) + gap, 0, (finalWidth / 2) - gap, finalHeight);
            break;
        case nameOnRight:
            label->setBounds ((finalWidth / 2) + gap, 0, (finalWidth / 2) - gap, finalHeight);
            label->setJustificationType (Justification::centredLeft);
            label->setVisible (true);
            editor->setBounds (0, 0, (finalWidth / 2) - gap, finalHeight);
            break;
        case nameHidden:
            label->setVisible (false);
            editor->setBounds (0, 0, finalWidth, finalHeight);
            break;
    }
}

/********* CustomTextBox *********/
TextEditor* CustomTextBox::createEditorComponent()
{
    TextEditor* const ed = Label::createEditorComponent();
    ed->setInputRestrictions (0, allowedChars);
    return ed;
}

void CustomTextBox::paint (juce::Graphics& g)
{
    // Fill the background with a rounded rectangle
    g.setColour (findColour (ThemeColours::widgetBackground));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f, 0.5f), 3.0f);

    // Draw the text with units
    if (! isBeingEdited())
    {
        auto alpha = isEnabled() ? 1.0f : 0.5f;
        const Font font (getFont());

        g.setColour (findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getBorderSize().subtractedFrom (getLocalBounds());

        String valueWithUnits = units.isEmpty() ? getText() : getText() + " " + units;
        g.drawFittedText (valueWithUnits, textArea, getJustificationType(), jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())), getMinimumHorizontalScale());

        g.setColour (findColour (ThemeColours::outline).withMultipliedAlpha (alpha));
    }
    else if (isEnabled())
    {
        g.setColour (findColour (ThemeColours::outline));
    }

    // Draw a rounded rectangle border
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f, 0.5f), 3.0f, 1.0f);
}

TextBoxParameterEditor::TextBoxParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels)
    : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::FLOAT_PARAM
             || param->getType() == Parameter::INT_PARAM
             || param->getType() == Parameter::STRING_PARAM);

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
    int width = rowWidthPixels;
    label->setFont (labelFont);
    addAndMakeVisible (label.get());

    if (param->getType() == Parameter::FLOAT_PARAM) {
        if (((FloatParameter*) param)->getMinValue() < 0) {
            valueTextBox = std::make_unique<CustomTextBox> (param->getKey(), String (float (param->getValue())), "-0123456789.");
        } else {
            valueTextBox = std::make_unique<CustomTextBox> (param->getKey(), String (float (param->getValue())), "0123456789.");
        }
    }
    else if (param->getType() == Parameter::INT_PARAM) {
        if (((IntParameter*) param)->getMinValue() < 0) {
            valueTextBox = std::make_unique<CustomTextBox> (param->getKey(), String (int (param->getValue())), "-0123456789");
        } else {
            valueTextBox = std::make_unique<CustomTextBox> (param->getKey(), String (int (param->getValue())), "0123456789");
        }
    }
    else
        valueTextBox = std::make_unique<CustomTextBox> (param->getKey(), param->getValue().toString(), "");

    valueTextBox->setFont (FontOptions ("CP Mono", "Plain", int (0.75 * rowHeightPixels)));
    valueTextBox->setName (param->getKey());
    // valueTextBox->setColour(Label::textColourId, Colours::black);
    // valueTextBox->setColour(Label::backgroundColourId, Colours::lightgrey);
    valueTextBox->setJustificationType (Justification::centred);
    valueTextBox->setEditable (true);
    valueTextBox->addListener (this);
    valueTextBox->setTooltip (param->getDescription());
    addAndMakeVisible (valueTextBox.get());

    setBounds (0, 0, width, rowHeightPixels);

    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    valueTextBox->setBounds (0, 0, width / 2, rowHeightPixels);

    editor = (Component*) valueTextBox.get();
}

void TextBoxParameterEditor::labelTextChanged (Label* label)
{
    if (param->getType() == Parameter::FLOAT_PARAM)
        param->setNextValue (label->getText().getFloatValue());
    else if (param->getType() == Parameter::INT_PARAM)
        param->setNextValue (label->getText().getIntValue());
    else
        param->setNextValue (label->getText());
}

void TextBoxParameterEditor::updateView()
{
    if (param != nullptr)
    {
        valueTextBox->setEditable (true);

        if (param->getType() == Parameter::FLOAT_PARAM)
            valueTextBox->setText (String (float (param->getValue())), dontSendNotification);
        else
            valueTextBox->setText (param->getValue().toString(), dontSendNotification);
    }
    else
    {
        valueTextBox->setEditable (false);
    }
}

void TextBoxParameterEditor::resized()
{
    updateBounds();
}

void CustomToggleButton::paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    Colour backgroundColour = findColour (ThemeColours::widgetBackground);

    if (getToggleState())
    {
        backgroundColour = findColour (ThemeColours::highlightedFill);
    }

    auto baseColour = backgroundColour.withMultipliedSaturation (hasKeyboardFocus (true) ? 1.3f : 1.0f)
                          .withMultipliedAlpha (isEnabled() ? 1.0f : 0.5f);

    if (isButtonDown || isMouseOver)
        baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.05f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 3.0f);

    g.setColour (findColour (ThemeColours::outline).withAlpha (isEnabled() ? 1.0f : 0.5f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 3.0f, 1.0f);

    g.setFont (FontOptions ("Inter", "Regular", int (0.75 * getHeight())));

    if (! isEnabled() || isButtonDown)
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.4f));
    else
        g.setColour (findColour (ThemeColours::defaultText));

    // Set the text based on the button state
    if (getToggleState())
    {
        g.drawText ("ON", getLocalBounds(), juce::Justification::centred);
    }
    else
    {
        g.drawText ("OFF", getLocalBounds(), juce::Justification::centred);
    }
}

ToggleParameterEditor::ToggleParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::BOOLEAN_PARAM);

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    label->setFont (FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels)));
    addAndMakeVisible (label.get());

    toggleButton = std::make_unique<CustomToggleButton>();
    toggleButton->setName (param->getKey());
    toggleButton->setToggleState (bool (param->getValue()), dontSendNotification);
    toggleButton->addListener (this);
    toggleButton->setTooltip (param->getDescription());
    addAndMakeVisible (toggleButton.get());

    int width = rowWidthPixels;

    setBounds (0, 0, width, rowHeightPixels);
    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    toggleButton->setBounds (0, 0, width / 2, rowHeightPixels);

    editor = (Component*) toggleButton.get();
}

void ToggleParameterEditor::buttonClicked (Button* button)
{
    if (param != nullptr)
        param->setNextValue (button->getToggleState());
}

void ToggleParameterEditor::updateView()
{
    if (param != nullptr)
        toggleButton->setToggleState (param->getValue(), dontSendNotification);

    repaint();
}

void ToggleParameterEditor::resized()
{
    updateBounds();
}

ComboBoxParameterEditor::ComboBoxParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::CATEGORICAL_PARAM
             || param->getType() == Parameter::INT_PARAM);

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName()); // == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    label->setFont (FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels)));
    addAndMakeVisible (label.get());

    valueComboBox = std::make_unique<ComboBox>();
    valueComboBox->setName (param->getKey());
    valueComboBox->setJustificationType (Justification::centred);
    valueComboBox->addListener (this);
    valueComboBox->setTooltip (param->getDescription());
    addAndMakeVisible (valueComboBox.get());

    if (param->getType() == Parameter::CATEGORICAL_PARAM)
    {
        CategoricalParameter* p = (CategoricalParameter*) param;

        offset = 1;

        const Array<String>& categories = p->getCategories();

        for (int i = 0; i < categories.size(); i++)
        {
            valueComboBox->addItem (categories[i], i + offset);
        }

        valueComboBox->setSelectedId (p->getSelectedIndex() + offset, dontSendNotification);
    }
    else
    {
        IntParameter* p = (IntParameter*) param;

        offset = -(p->getMinValue()) + 1;

        for (int i = p->getMinValue(); i <= p->getMaxValue(); i++)
        {
            valueComboBox->addItem (String (i), i + offset);
        }

        valueComboBox->setSelectedId (p->getIntValue() + offset, dontSendNotification);
    }

    int width = rowWidthPixels;

    setBounds (0, 0, width, rowHeightPixels);

    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    valueComboBox->setBounds (0, 0, width / 2, rowHeightPixels);

    editor = (Component*) valueComboBox.get();
}

void ComboBoxParameterEditor::comboBoxChanged (ComboBox* comboBox)
{
    if (param != nullptr)
        param->setNextValue (comboBox->getSelectedId() - offset);
}

void ComboBoxParameterEditor::updateView()
{
    if (param == nullptr)
    {
        for (int i = 0; i < valueComboBox->getNumItems(); i++)
            valueComboBox->setItemEnabled (valueComboBox->getItemId (i), false);

        return;
    }
    else
    {
        if (param->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) param;

            const StringArray& categories = p->getCategories();
            valueComboBox->clear (dontSendNotification);

            for (int i = 0; i < categories.size(); i++)
            {
                valueComboBox->addItem (categories[i], i + offset);
            }
        }

        for (int i = 0; i < valueComboBox->getNumItems(); i++)
            valueComboBox->setItemEnabled (valueComboBox->getItemId (i), true);
    }

    if (param->getType() == Parameter::CATEGORICAL_PARAM)
    {
        CategoricalParameter* p = (CategoricalParameter*) param;

        valueComboBox->setSelectedId (p->getSelectedIndex() + offset, dontSendNotification);
    }
    else
    {
        IntParameter* p = (IntParameter*) param;

        valueComboBox->setSelectedId (p->getIntValue() + offset, dontSendNotification);
    }

    repaint();
}

void ComboBoxParameterEditor::resized()
{
    updateBounds();
}

TextEditor* BoundedValueEditor::createEditorComponent()
{
    auto* editor = new juce::TextEditor (getComponentID());

    editor->setJustification (juce::Justification::centred);
    editor->setInputRestrictions (0, "0123456789.");

    return editor;
}

void BoundedValueEditor::paint (juce::Graphics& g)
{
    // Get the label's value as a percentage
    float value = getText().getFloatValue();

    // Calculate the width of the coloured area based on the value
    int colouredWidth = static_cast<int> (getWidth() * (value - minValue) / (maxValue - minValue));

    // Fill the coloured area
    g.setColour (getLookAndFeel().findColour (ProcessorColour::IDs::FILTER_COLOUR));
    g.fillRect (1, 1, colouredWidth > 3 ? colouredWidth - 2 : 2, getHeight() - 2);

    // Fill the rest of the background with another colour
    g.setColour (getLookAndFeel().findColour (TextEditor::backgroundColourId));
    g.fillRect (colouredWidth > 0 ? colouredWidth : 1, 1, getWidth() - colouredWidth > 1 ? getWidth() - colouredWidth - 1 : 1, getHeight() - 2);

    // Draw the text with units
    if (! isBeingEdited())
    {
        auto alpha = isEnabled() ? 1.0f : 0.5f;
        const Font font (getFont());

        g.setColour (findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getBorderSize().subtractedFrom (getLocalBounds());

        String valueWithUnits = units.isEmpty() ? getText() : getText() + " " + units;
        g.drawFittedText (valueWithUnits, textArea, getJustificationType(), jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())), getMinimumHorizontalScale());

        g.setColour (findColour (ThemeColours::outline).withMultipliedAlpha (alpha));
    }
    else if (isEnabled())
    {
        g.setColour (findColour (ThemeColours::outline));
    }

    // Draw a rounded rectangle border
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f, 0.5f), 3.0f, 1.0f);
}

void BoundedValueEditor::mouseDrag (const MouseEvent& event)
{
    if (! isEnabled())
        return;

    // Calculate the new value based on the mouse position
    double newValue = static_cast<float> (event.position.x) / getWidth() * (maxValue - minValue) + minValue;

    // Clamp the new value to the range [minValue, maxValue]
    newValue = jlimit (minValue, maxValue, newValue);

    float multiplier = std::pow (10.0f, -std::log10 (stepSize));
    newValue = std::round (newValue * multiplier) / multiplier;

    setText (String (newValue), juce::dontSendNotification);

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
        juce::Label::mouseUp (event);
    }
}

BoundedValueParameterEditor::BoundedValueParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::FLOAT_PARAM
             || param->getType() == Parameter::INT_PARAM);

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
    label->setFont (labelFont);
    addAndMakeVisible (label.get());

    if (param->getType() == Parameter::FLOAT_PARAM)
    {
        FloatParameter* p = (FloatParameter*) param;
        valueEditor = std::make_unique<BoundedValueEditor> (p->getMinValue(), p->getMaxValue(), p->getStepSize(), p->getUnit());
        valueEditor->setText (String (p->getFloatValue()), dontSendNotification);
    }
    else
    {
        IntParameter* p = (IntParameter*) param;
        valueEditor = std::make_unique<BoundedValueEditor> (p->getMinValue(), p->getMaxValue(), 1);
        valueEditor->setText (String (p->getIntValue()), dontSendNotification);
    }
    valueEditor->setName (param->getKey());
    valueEditor->setFont (FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels)));
    valueEditor->setJustificationType (Justification::centred);
    valueEditor->addListener (this);
    valueEditor->setTooltip (param->getDescription());
    labelTextChanged (valueEditor.get());
    addAndMakeVisible (valueEditor.get());

    label->setBounds (rowWidthPixels / 2 + 4, 0, rowWidthPixels / 2, rowHeightPixels);
    valueEditor->setBounds (0, 0, rowWidthPixels / 2, rowHeightPixels);

    setBounds (0, 0, rowWidthPixels, rowHeightPixels);

    editor = (Component*) valueEditor.get();
}

void BoundedValueParameterEditor::labelTextChanged (Label* label)
{
    if (param != nullptr)
    {
        if (param->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) param;
            if (label->getText().getFloatValue() < p->getMinValue())
                label->setText (String (p->getMinValue()), dontSendNotification);
            else if (label->getText().getFloatValue() > p->getMaxValue())
                label->setText (String (p->getMaxValue()), dontSendNotification);

            param->setNextValue (label->getText().getFloatValue());
        }
        else
        {
            IntParameter* p = (IntParameter*) param;
            if (label->getText().getIntValue() < p->getMinValue())
                label->setText (String (p->getMinValue()), dontSendNotification);
            else if (label->getText().getIntValue() > p->getMaxValue())
                label->setText (String (p->getMaxValue()), dontSendNotification);

            param->setNextValue (int (label->getText().getFloatValue()));
        }
    }
}

void BoundedValueParameterEditor::updateView()
{
    if (param != nullptr)
    {
        valueEditor->setEnabled (true);

        if (param->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) param;
            valueEditor->setText (String (p->getFloatValue()), dontSendNotification);
        }
        else
        {
            IntParameter* p = (IntParameter*) param;
            valueEditor->setText (String (p->getIntValue()), dontSendNotification);
        }
    }
    else
    {
        valueEditor->setEnabled (false);
    }

    repaint();
}

void BoundedValueParameterEditor::resized()
{
    updateBounds();
}

SelectedChannelsParameterEditor::SelectedChannelsParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::SELECTED_CHANNELS_PARAM);

    int selectedChannels = ((SelectedChannelsParameter*) param)->getArrayValue().size();
    size_t numChannels = ((SelectedChannelsParameter*) param)->getChannelStates().size();

    button = std::make_unique<TextButton> (String (selectedChannels) + "/" + String (numChannels));
    button->setName (param->getKey());
    button->addListener (this);
    button->setClickingTogglesState (false);
    button->setTooltip (param->getDescription());
    addAndMakeVisible (button.get());

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
    label->setFont (labelFont);
    label->setJustificationType (Justification::left);
    addAndMakeVisible (label.get());

    setBounds (0, 0, rowWidthPixels, rowHeightPixels);
    label->setBounds (rowWidthPixels / 2 + 4, 0, rowWidthPixels / 2, rowHeightPixels);
    button->setBounds (0, 0, rowWidthPixels / 2, rowHeightPixels);

    editor = (Component*) button.get();

    updateView();
}

Array<int> SelectedChannelsParameterEditor::getSelectedChannels()
{
    return ((SelectedChannelsParameter*) param)->getArrayValue();
}

int SelectedChannelsParameterEditor::getChannelCount()
{
    return ((SelectedChannelsParameter*) param)->getChannelCount();
}

void SelectedChannelsParameterEditor::channelStateChanged (Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add (newChannels[i]);

    param->setNextValue (newArray);

    updateView();
}

void SelectedChannelsParameterEditor::buttonClicked (Button* button_)
{
    if (param == nullptr)
        return;

    SelectedChannelsParameter* p = (SelectedChannelsParameter*) param;

    Array<String> channelNames;
    String popupTitle;

    if (p->getOwner()->getType() == ParameterOwner::DATASTREAM)
    {
        DataStream* stream = (DataStream*) p->getOwner();
        popupTitle = String (stream->getSourceNodeId()) + " " + stream->getSourceNodeName() + " - " + stream->getName();

        for (auto channel : stream->getContinuousChannels())
            channelNames.add (channel->getName());
    }

    auto* channelSelector = new PopupChannelSelector (button.get(), this, p->getChannelStates(), channelNames, popupTitle);

    channelSelector->setChannelButtonColour (param->getColour());

    channelSelector->setMaximumSelectableChannels (p->getMaxSelectableChannels());

    CoreServices::getPopupManager()->showPopup (std::unique_ptr<Component> (channelSelector), button.get());

    /*
    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            button->getScreenBounds(),
            nullptr);
    */
}

void SelectedChannelsParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled (false);
    else
    {
        button->setEnabled (true);

        int numChannels = int (((SelectedChannelsParameter*) param)->getChannelStates().size());

        int numSelected = 0;
        for (auto chan : ((SelectedChannelsParameter*) param)->getChannelStates())
            if (chan)
                numSelected++;

        if (numSelected == 0)
        {
            button->setButtonText ("None");
        }
        else if (numSelected > 4)
        {
            button->setButtonText (String (numSelected) + "/" + String (numChannels));
        }
        else
        {
            String selectedChannelsString;

            for (int i = 0; i < numSelected; i++)
            {
                selectedChannelsString += String (int (param->currentValue[i]) + 1);

                if (i < numSelected - 1)
                    selectedChannelsString += ", ";
            }
            button->setButtonText (selectedChannelsString);
        }
    }
}

void SelectedChannelsParameterEditor::resized()
{
    updateBounds();
}

MaskChannelsParameterEditor::MaskChannelsParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::MASK_CHANNELS_PARAM);

    int numChannels = int (((MaskChannelsParameter*) param)->getChannelStates().size());
    int selected = 0;
    for (auto chan : ((MaskChannelsParameter*) param)->getChannelStates())
        if (chan)
            selected++;

    button = std::make_unique<TextButton> (String (selected) + "/" + String (numChannels));
    button->setComponentID (param->getKey());
    button->addListener (this);
    button->setClickingTogglesState (false);
    button->setTooltip ("Mask channels to filter within this stream");
    addAndMakeVisible (button.get());

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
    label->setFont (labelFont);
    label->setJustificationType (Justification::left);
    addAndMakeVisible (label.get());

    int width = rowWidthPixels;

    setBounds (0, 0, width, rowHeightPixels);
    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    button->setBounds (0, 0, width / 2, rowHeightPixels);

    editor = (Component*) button.get();
}

Array<int> MaskChannelsParameterEditor::getSelectedChannels()
{
    return ((MaskChannelsParameter*) param)->getArrayValue();
}

int MaskChannelsParameterEditor::getChannelCount()
{
    return ((MaskChannelsParameter*) param)->getChannelCount();
}

void MaskChannelsParameterEditor::channelStateChanged (Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add (newChannels[i]);

    param->setNextValue (newArray);

    updateView();
}

void MaskChannelsParameterEditor::buttonClicked (Button* button_)
{
    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*) param;

    std::vector<bool> channelStates = p->getChannelStates();

    Array<String> channelNames;
    String popupTitle;

    if (p->getOwner()->getType() == ParameterOwner::DATASTREAM)
    {
        DataStream* stream = (DataStream*) p->getOwner();
        popupTitle = String (stream->getSourceNodeId()) + " " + stream->getSourceNodeName() + " - " + stream->getName();

        for (auto channel : stream->getContinuousChannels())
            channelNames.add (channel->getName());
    }

    auto* channelSelector = new PopupChannelSelector (button.get(), this, channelStates, channelNames, popupTitle);

    channelSelector->setChannelButtonColour (param->getColour());

    CoreServices::getPopupManager()->showPopup (std::unique_ptr<Component> (channelSelector), button.get());

    /*
    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            button->getScreenBounds(),
            nullptr);
    */
}

void MaskChannelsParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled (false);

    else
    {
        button->setEnabled (true);
        int numChannels = int (((MaskChannelsParameter*) param)->getChannelStates().size());
        int selected = 0;
        for (auto chan : ((MaskChannelsParameter*) param)->getChannelStates())
            if (chan)
                selected++;
        button->setButtonText (String (selected) + "/" + String (numChannels));
    }
}

void MaskChannelsParameterEditor::resized()
{
    updateBounds();
}

SyncControlButton::SyncControlButton (SynchronizingProcessor* node_,
                                      const String& name,
                                      String streamKey_,
                                      int ttlLineCount_)
    : Button (name),
      streamKey (streamKey_),
      node (node_),
      ttlLineCount (ttlLineCount_)
{
    isPrimary = node->isMainDataStream (streamKey);
    LOGD ("SyncControlButton::Constructor; Stream: ", streamKey, " is main stream: ", isPrimary);
    startTimer (250);

    setTooltip ("Configure synchronization settings for " + streamKey);
}

SyncControlButton::~SyncControlButton() {}

void SyncControlButton::timerCallback()
{
    repaint();
}

void SyncControlButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (Colours::grey);
    g.fillRoundedRectangle (0, 0, getWidth(), getHeight(), 4);

    switch (node->synchronizer.getStatus (streamKey))
    {
        case SyncStatus::OFF:
        {
            if (isMouseOver)
            {
                //LIGHT GREY
                g.setColour (Colour (150, 150, 150));
            }
            else
            {
                //DARK GREY
                g.setColour (Colour (110, 110, 110));
            }
            break;
        }
        case SyncStatus::SYNCING:
        {
            if (isMouseOver)
            {
                //LIGHT ORANGE
                g.setColour (Colour (255, 216, 177));
            }
            else
            {
                //DARK ORAN
                g.setColour (Colour (255, 165, 0));
            }
            break;
        }
        case SyncStatus::SYNCED:
        {
            if (isMouseOver)
            {
                //LIGHT GREEN
                g.setColour (Colour (25, 255, 25));
            }
            else
            {
                //DARK GREEN
                g.setColour (Colour (25, 255, 25).darker (0.5f));
            }
            break;
        }
    }

    g.fillRoundedRectangle (2, 2, getWidth() - 4, getHeight() - 4, 2);

    if (node->isMainDataStream (streamKey))
    {
        g.setColour (Colour (255, 255, 255));
        g.setFont (FontOptions ("Inter", "Regular", 12.0f));
        g.drawText ("M", getLocalBounds().reduced (3), juce::Justification::centred);
    }
}

TtlLineParameterEditor::TtlLineParameterEditor (Parameter* param,
                                                Parameter* syncParam_,
                                                int rowHeightPixels,
                                                int rowWidthPixels)
    : ParameterEditor (param),
      syncParam (syncParam_)
{
    jassert (param->getType() == Parameter::TTL_LINE_PARAM);

    TtlLineParameter* ttlParam = ((TtlLineParameter*) param);
    DataStream* paramStream = ((DataStream*) param->getOwner());

    if (syncParam != nullptr)
    {
        jassert (syncParam->getType() == Parameter::SELECTED_STREAM_PARAM);
        jassert (syncParam->getScope() == Parameter::ParameterScope::PROCESSOR_SCOPE);

        GenericProcessor* syncProcessor = ((GenericProcessor*) syncParam->getOwner());

        syncControlButton = std::make_unique<SyncControlButton> (dynamic_cast<SynchronizingProcessor*> (syncProcessor),
                                                                 syncParam->getDisplayName(),
                                                                 paramStream->getKey(),
                                                                 ttlParam->getMaxAvailableLines());

        syncControlButton->addListener (this);
        syncControlButton->setBounds (0, 0, 15, 15);
        addAndMakeVisible (syncControlButton.get());

        setBounds (0, 0, 15, 15);
        editor = (Component*) syncControlButton.get();
    }
    else
    {
        int selectedLine = ((TtlLineParameter*) param)->getSelectedLine();
        textButton = std::make_unique<TextButton> ("Line " + String (selectedLine + 1), "Selected TTL Line");
        textButton->setName (param->getKey());
        textButton->addListener (this);
        textButton->setClickingTogglesState (false);
        textButton->setTooltip (param->getDescription());
        addAndMakeVisible (textButton.get());

        label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
        Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
        label->setFont (labelFont);
        label->setJustificationType (Justification::left);
        addAndMakeVisible (label.get());

        setBounds (0, 0, rowWidthPixels, rowHeightPixels);
        label->setBounds (rowWidthPixels / 2 + 4, 0, rowWidthPixels / 2, rowHeightPixels);
        textButton->setBounds (0, 0, rowWidthPixels / 2, rowHeightPixels);

        editor = (Component*) textButton.get();
    }
}

void TtlLineParameterEditor::selectedLineChanged (int newLine)
{
    param->setNextValue (newLine);
    updateView();
}

int TtlLineParameterEditor::getSelectedLine()
{
    if (param == nullptr)
        return -1;
    else
        return ((TtlLineParameter*) param)->getSelectedLine();
}

void TtlLineParameterEditor::primaryStreamChanged()
{
    if (syncParam == nullptr || syncParam->getType() != Parameter::SELECTED_STREAM_PARAM)
        return;

    Array<String> streamNames = ((SelectedStreamParameter*) syncParam)->getStreamNames();

    DataStream* paramStream = (DataStream*) param->getOwner();
    int streamIndex = -1;

    LOGA (param->getKey(), "::primaryStreamChanged; Stream: ", paramStream->getKey());

    for (int i = 0; i < streamNames.size(); i++)
    {
        if (streamNames[i] == paramStream->getKey())
        {
            streamIndex = i;
            break;
        }
    }

    if (streamIndex >= 0)
        syncParam->setNextValue (streamIndex);
}

void TtlLineParameterEditor::buttonClicked (Button* button_)
{
    if (param == nullptr)
        return;

    TtlLineParameter* p = (TtlLineParameter*) param;

    if (p->syncModeEnabled() && syncParam != nullptr)
    {
        DataStream* paramStream = (DataStream*) p->getOwner();

        auto* syncSelector = new SyncLineSelector (button_,
                                                   this,
                                                   p->getMaxAvailableLines(),
                                                   p->getSelectedLine(),
                                                   paramStream->getKey() == syncParam->getValueAsString());

        CoreServices::getPopupManager()->showPopup (std::unique_ptr<Component> (syncSelector), editor);
    }
    else
    {
        auto* lineSelector = new SyncLineSelector (button_,
                                                   this,
                                                   p->getMaxAvailableLines(),
                                                   p->getSelectedLine(),
                                                   true,
                                                   p->canSelectNone());

        CoreServices::getPopupManager()->showPopup (std::unique_ptr<Component> (lineSelector), editor);
    }
}

void TtlLineParameterEditor::updateView()
{
    if (param == nullptr)
        editor->setEnabled (false);
    else
    {
        editor->setEnabled (true);

        if (textButton != nullptr)
        {
            int selected = ((TtlLineParameter*) param)->getSelectedLine();
            String btnText = selected == -1 ? "None" : "Line " + String (selected + 1);
            textButton->setButtonText (btnText);
        }
    }
}

void TtlLineParameterEditor::resized()
{
    if (textButton != nullptr)
        updateBounds();
}

PathParameterEditor::PathParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::PATH_PARAM);

    setBounds (0, 0, rowWidthPixels, rowHeightPixels);
    
    button = std::make_unique<TextButton> ("Browse");
    button->setName (param->getKey());
    button->addListener (this);
    button->setClickingTogglesState (false);
    button->addMouseListener (this, true);
    addAndMakeVisible (button.get());

    clearButton = std::make_unique<TextButton> ("Clear");
    clearButton->setButtonText("x");
    clearButton->addListener (this);
    clearButton->addMouseListener (this, true);
    addAndMakeVisible (clearButton.get());
    clearButton->setVisible (false);
    clearButton->setTooltip ("Clear custom value");

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName()); // == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
    label->setFont (labelFont);
    label->setJustificationType (Justification::left);
    addAndMakeVisible (label.get());

    int width = rowWidthPixels;
    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    button->setBounds (0, 0, width / 2, rowHeightPixels);
    clearButton->setBounds (width / 2 - rowHeightPixels, 0, rowHeightPixels, rowHeightPixels);

    editor = (Component*) button.get();

}

void PathParameterEditor::buttonClicked (Button* button_)
{
    if (button_ == clearButton.get())
    {
        param->setNextValue ("None");
        clearButton->setVisible (false);
        updateView();
    } else {
        bool isDirectory = ((PathParameter*) param)->getIsDirectory();
        String dialogBoxTitle;
        String validFilePatterns;

        if (param->getDescription().isEmpty())
            dialogBoxTitle = "Select a " + String (isDirectory ? "directory" : "file");
        else
            dialogBoxTitle = param->getDescription();

        if (! isDirectory)
            validFilePatterns = "*." + ((PathParameter*) param)->getValidFilePatterns().joinIntoString (";*.");

        FileChooser chooser (dialogBoxTitle, File(), validFilePatterns);

        bool success = isDirectory ? chooser.browseForDirectory() : chooser.browseForFileToOpen();
        if (success)
        {
            File file = chooser.getResult();
            param->setNextValue (file.getFullPathName());
            updateView();
        }
    }
}

void PathParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled (false);
    else
        button->setEnabled (true);

    if (param)
    {
        if (param->getValue().toString() == "None")
        {
            button->setButtonText ("[ default path ]");
            button->setTooltip ("Override default...");
        }
        else
        {
            button->setButtonText (param->getValueAsString());
            button->setTooltip (param->getValueAsString());
        }
        if (! ((PathParameter*) param)->isValid())
        {
            button->setColour (TextButton::textColourOnId, Colours::red);
            button->setColour (TextButton::textColourOffId, Colours::red);
        }
        else
        {
            // Remove the red colour if it was previously set
            button->removeColour (TextButton::textColourOnId);
            button->removeColour (TextButton::textColourOffId);
        }
    }
}

void PathParameterEditor::resized()
{
    updateBounds();
    if (button != nullptr){
        auto buttonBounds = button->getBounds();
        auto clearButtonBounds = clearButton->getBounds();
        clearButton->setBounds(buttonBounds.getX() + buttonBounds.getWidth() - clearButtonBounds.getWidth(), buttonBounds.getY(), clearButtonBounds.getWidth(), clearButtonBounds.getHeight());
    }
}

void PathParameterEditor::mouseEnter(const juce::MouseEvent& _)
{
    if (param->getValue().toString() != "None" && !reinterpret_cast<PathParameter*>(param)->getIsRequired())
        clearButton->setVisible (true);
}

void PathParameterEditor::mouseExit(const juce::MouseEvent& _)
{
    clearButton->setVisible (false);
}

SelectedStreamParameterEditor::SelectedStreamParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::SELECTED_STREAM_PARAM
             || param->getType() == Parameter::INT_PARAM);

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    label->setFont (FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels)));
    addAndMakeVisible (label.get());

    valueComboBox = std::make_unique<ComboBox>();
    valueComboBox->setName (param->getKey());
    valueComboBox->setJustificationType (Justification::centred);
    valueComboBox->addListener (this);
    valueComboBox->setTooltip (param->getDescription());
    addAndMakeVisible (valueComboBox.get());

    if (param->getType() == Parameter::SELECTED_STREAM_PARAM)
    {
        SelectedStreamParameter* p = (SelectedStreamParameter*) param;

        Array<String>& streams = p->getStreamNames();

        for (int i = 0; i < streams.size(); i++)
            valueComboBox->addItem (streams[i], i + 1);

        valueComboBox->setSelectedId (p->getSelectedIndex() + 1, dontSendNotification);
    }
    else
    {
        IntParameter* p = (IntParameter*) param;

        for (int i = p->getMinValue(); i <= p->getMaxValue(); i++)
        {
            valueComboBox->addItem (String (i), i + 1);
        }

        valueComboBox->setSelectedId (p->getIntValue() + 1, dontSendNotification);
    }

    int width = rowWidthPixels;

    setBounds (0, 0, width, rowHeightPixels);
    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    valueComboBox->setBounds (0, 0, width / 2, rowHeightPixels);

    editor = (Component*) valueComboBox.get();
}

void SelectedStreamParameterEditor::comboBoxChanged (ComboBox* comboBox)
{
    if (param != nullptr)
        param->setNextValue (comboBox->getSelectedId() - 1);
}

void SelectedStreamParameterEditor::updateView()
{
    if (param == nullptr)
        valueComboBox->setEnabled (false);

    else
    {
        if (param->getType() == Parameter::SELECTED_STREAM_PARAM)
        {
            valueComboBox->setEnabled (true);
            valueComboBox->clear (dontSendNotification);

            SelectedStreamParameter* p = (SelectedStreamParameter*) param;
            Array<String>& streams = p->getStreamNames();

            for (int i = 0; i < streams.size(); i++)
            {
                valueComboBox->addItem (streams[i], i + 1);
            }

            valueComboBox->setSelectedId (p->getSelectedIndex() + 1, dontSendNotification);
        }
    }
    // repaint();
}

void SelectedStreamParameterEditor::resized()
{
    updateBounds();
}

TimeParameterEditor::TimeParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::TIME_PARAM);

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName() == "" ? param->getName().replace ("_", " ") : param->getDisplayName());
    label->setFont (FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels)));
    addAndMakeVisible (label.get());

    button = std::make_unique<TextButton> (param->getValueAsString());
    button->setName (param->getKey());
    button->addListener (this);
    button->setClickingTogglesState (false);
    button->setTooltip ("Uninitialized");
    addAndMakeVisible (button.get());

    int width = rowWidthPixels;

    setBounds (0, 0, width, rowHeightPixels);
    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    button->setBounds (0, 0, width / 2, rowHeightPixels);

    editor = (Component*) button.get();

    startTimer (200);
}

void TimeParameterEditor::buttonClicked (Button* button)
{
    if (param == nullptr)
        return;

    TimeParameter* p = (TimeParameter*) param;

    auto* timeEditor = new PopupTimeEditor (p);

    CallOutBox& myBox = CallOutBox::launchAsynchronously (std::unique_ptr<Component> (timeEditor),
                                                          button->getScreenBounds(),
                                                          nullptr);
}

void TimeParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled (false);
    else
        button->setEnabled (true);
}

void TimeParameterEditor::resized()
{
    updateBounds();
}

void TimeParameterEditor::timerCallback()
{
    if (param != nullptr)
    {
        button->setButtonText (((TimeParameter*) param)->getTimeValue()->toString());
        button->setTooltip ("Max time: " + String (((TimeParameter*) param)->getTimeValue()->getMaxTimeInMilliseconds()) + " ms");
    }
    else
        button->setButtonText ("00:00:00.000");
}
