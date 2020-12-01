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
#include "../../Utils/Utils.h"


static const int FONT_SIZE = 10;


ParameterEditor::ParameterEditor (GenericProcessor* processor, Parameter* parameter, Font labelFont)
    : m_activationState     (true)
    , m_parameter           (parameter)
    , m_processor           (processor)
{
    shouldDeactivateDuringAcquisition = parameter->shouldDeactivateDuringAcquisition;

    const bool isParameterHasCustomBounds = parameter->hasCustomEditorBounds();

    // Create label for parameter
    Label* label = new Label (parameter->getName(), parameter->getName());
    labelFont.setHeight (FONT_SIZE);
    label->setColour (Label::textColourId, Colours::darkgrey);
    label->setFont (labelFont);
    m_labelsArray.add (label);
    addAndMakeVisible (label);

    if (parameter->isBoolean())
    {
        LOGD("Boolean parameter-> Creating checkbox.");

        // create checkbox
        ParameterCheckbox* pc = new ParameterCheckbox ((bool) parameter->getDefaultValue());
        pc->setComponentID (String (parameter->getID()));
        pc->setName (parameter->getName());
        pc->addListener (this);
        m_checkboxArray.add (pc);
        addAndMakeVisible (pc);

        if (isParameterHasCustomBounds)
        {
            const auto desiredBounds = getDesiredBounds();
            const int checkBoxSize = desiredBounds.getHeight();
            pc->setBounds       (0, 0, checkBoxSize, checkBoxSize);
            label->setBounds    (checkBoxSize, 0, desiredBounds.getWidth() - checkBoxSize, checkBoxSize);
        }
        else
        {
            pc->setBounds       (0, 0, 12, 12);
            label->setBounds    (10, 1, 100, FONT_SIZE);
            desiredWidth = 120;
            desiredHeight = 25;
        }
    }
    else if (parameter->isContinuous())
    {
        LOGD("Continuous parameter-> Creating slider.");

        // create slider
        Array<var> possibleValues = parameter->getPossibleValues();
        ParameterSlider* ps = new ParameterSlider ((float) possibleValues[0],
                                                   (float) possibleValues[1],
                                                   (float) parameter->getDefaultValue(),
                                                   labelFont);

        ps->setComponentID (String (parameter->getID()));
        ps->setName (parameter->getName());
        ps->addListener (this);
        addAndMakeVisible (ps);
        m_sliderArray.add (ps);

        const int labelWidth = labelFont.getStringWidth (parameter->getName());
        if (isParameterHasCustomBounds)
        {
            const auto desiredBounds = getDesiredBounds();
            ps->setBounds       (0, 0, desiredBounds.getWidth(), desiredBounds.getHeight());
            label->setBounds    ( (desiredBounds.getWidth() - labelWidth) / 2, desiredBounds.getHeight() - FONT_SIZE,
                                  labelWidth, FONT_SIZE);
        }
        else
        {
            ps->setBounds (0, 0, 80, 80);

            label->setBounds ((80 - labelWidth) / 2 - 5, 70, 100, FONT_SIZE);

            desiredWidth = 80;
            desiredHeight = 80;
        }
    }
    else if (parameter->isDiscrete())
    {
        LOGD("Discrete parameter-> Creating buttons.");


        Array<var> possibleValues = parameter->getPossibleValues();

        const int numButtons  = possibleValues.size();
        const int buttonWidth = isParameterHasCustomBounds ? (m_parameter->getEditorDesiredBounds().getWidth() / numButtons)
                                                           : 35;

        LOGD("Button width: ", buttonWidth);
        LOGD("Default value: ", (int) parameter->getDefaultValue());

        for (int i = 0; i < numButtons; ++i)
        {
            LOGD("Creating button ", i);

            int buttonType = MIDDLE;
            if (i == 0)
                buttonType = LEFT;
            else if (i == possibleValues.size() - 1)
                buttonType = RIGHT;

            // create buttons
            ParameterButton* pb = new ParameterButton (possibleValues[i], buttonType, labelFont);
            pb->setComponentID (String (parameter->getID()));
            pb->setName (parameter->getName());
            pb->addListener (this);
            m_buttonArray.add (pb);

            if (isParameterHasCustomBounds)
            {
                pb->setBounds (buttonWidth * i, 12, buttonWidth, getDesiredBounds().getHeight() - FONT_SIZE);
            }
            else
            {
                pb->setBounds (buttonWidth * i, 12, buttonWidth, 18);
            }

            if (i == (int) parameter->getDefaultValue())
                pb->setToggleState (true, dontSendNotification);

            addAndMakeVisible (pb);
        }

        if (isParameterHasCustomBounds)
        {
            label->setBounds (0, 0, getDesiredBounds().getWidth(), FONT_SIZE);
        }
        else
        {
            label->setBounds (0, 0, 100, FONT_SIZE);

            desiredWidth = buttonWidth * numButtons;
            desiredHeight = 30;
        }
    }
    else if (parameter->isNumerical())
    {
        LOGD("Numerical parameter-> Creating slider.");

        // create slider
        Array<var> possibleValues = parameter->getPossibleValues();
        ParameterLabel* pl = new ParameterLabel (parameter->getName(),
                                                 (double) possibleValues[0],
                                                 (double) possibleValues[1],
                                                 (double) parameter->getDefaultValue());

        pl->setComponentID (String (parameter->getID()));
        pl->setName (parameter->getName());
        pl->addListener (this);
        addAndMakeVisible (pl);
        m_parameterLabelsArray.add (pl);

        //const int labelWidth = labelFont.getStringWidth (parameter->getName());
        if (isParameterHasCustomBounds)
        {
            const auto desiredBounds = getDesiredBounds();
            pl->setBounds       (0, 0, desiredBounds.getWidth(), desiredBounds.getHeight());
            //label->setBounds    ( (desiredBounds.getWidth() - labelWidth) / 2, desiredBounds.getHeight() - FONT_SIZE,
            //                      labelWidth, FONT_SIZE);
        }
        else
        {
            pl->setBounds (5, 10, 60, 40);

            //label->setBounds ((80 - labelWidth) / 2 - 5, 70, 100, FONT_SIZE);

            desiredWidth = 60;
            desiredHeight = 60;
        }
    }
}


void ParameterEditor::parentHierarchyChanged()
{
}


void ParameterEditor::setChannelSelector (ChannelSelector* channelSelector)
{
    m_channelSelector = channelSelector;
}


void ParameterEditor::setEnabled (bool isEnabled)
{
    LOGD("Changing editor state!");

    if (shouldDeactivateDuringAcquisition)
    {
        for (int i = 0; i < m_sliderArray.size(); ++i)
        {
            m_sliderArray[i]->isEnabled = isEnabled;
            m_sliderArray[i]->setInterceptsMouseClicks (isEnabled, isEnabled);
            m_sliderArray[i]->repaint();
        }

        for (int i = 0; i < m_buttonArray.size(); ++i)
        {
            m_buttonArray[i]->isEnabled = isEnabled;
            m_buttonArray[i]->setInterceptsMouseClicks (isEnabled, isEnabled);
            m_buttonArray[i]->repaint();
        }

        for (int i = 0; i < m_checkboxArray.size(); ++i)
        {
            m_checkboxArray[i]->isEnabled = isEnabled;
            m_checkboxArray[i]->setInterceptsMouseClicks (isEnabled, isEnabled);
            m_checkboxArray[i]->repaint();
        }

        for (int i = 0; i < m_parameterLabelsArray.size(); ++i)
        {
            m_parameterLabelsArray[i]->isEnabled = isEnabled;
            m_parameterLabelsArray[i]->setInterceptsMouseClicks (isEnabled, isEnabled);
            m_parameterLabelsArray[i]->repaint();
        }
    }
}


void ParameterEditor::buttonClicked (Button* buttonThatWasClicked)
{
    LOGD("Button ID: ", buttonThatWasClicked->getComponentID());
    LOGD("Button name: ", buttonThatWasClicked->getName());
    LOGD("Button value: ", buttonThatWasClicked->getButtonText());

    ParameterButton* b = (ParameterButton*) buttonThatWasClicked;

    if (b->isEnabled)
    {
        Array<int> activeChannels = m_channelSelector->getActiveChannels();
        {
            for (int i = 0; i < activeChannels.size(); ++i)
            {
                m_processor->setCurrentChannel (activeChannels[i]);
                m_processor->setParameter (buttonThatWasClicked->getComponentID().getIntValue(),
                                           buttonThatWasClicked->getButtonText().getFloatValue());
            }
        }
    }
}


void ParameterEditor::sliderValueChanged (Slider* sliderWhichValueHasChanged)
{
    ParameterSlider* s = (ParameterSlider*) sliderWhichValueHasChanged;

    if (s->isEnabled)
    {
        Array<int> activeChannels = m_channelSelector->getActiveChannels();
        {
            for (int i = 0; i < activeChannels.size(); ++i)
            {
                m_processor->setCurrentChannel (activeChannels[i]);
                m_processor->setParameter (sliderWhichValueHasChanged->getComponentID().getIntValue(),
                                           sliderWhichValueHasChanged->getValue());
            }
        }
    }
}


void ParameterEditor::parameterLabelValueChanged (ParameterLabel* parameterLabelWhichValueHasChanged)
{
    if (parameterLabelWhichValueHasChanged->isEnabled)
    {
        Array<int> activeChannels = m_channelSelector->getActiveChannels();
        {
            for (int i = 0; i < activeChannels.size(); ++i)
            {
                m_processor->setCurrentChannel (activeChannels[i]);
                m_processor->setParameter (parameterLabelWhichValueHasChanged->getComponentID().getIntValue(),
                                           parameterLabelWhichValueHasChanged->getValue());
            }
        }
    }
}


/// ============= PARAMETER BUTTON ==================
ParameterButton::ParameterButton (var value, int buttonType, Font labelFont) 
    : Button        ("parameter")
    , isEnabled     (true)
    , type          (buttonType)
    , valueString   (value.toString())
    , font          (labelFont)
{
    setButtonText (valueString);
    setRadioGroupId (1999);
    setClickingTogglesState (true);

    selectedGrad            = ColourGradient (Colour (240, 179, 12),  0.0, 0.0,
                                              Colour (207, 160, 33),  0.0, 20.0f,
                                              false);
    selectedOverGrad        = ColourGradient (Colour (209, 162, 33),  0.0, 5.0f,
                                              Colour (190, 150, 25),  0.0, 0.0f,
                                              false);
    usedByNonActiveGrad     = ColourGradient (Colour (200, 100, 0),   0.0, 0.0,
                                              Colour (158, 95,  32),  0.0, 20.0f,
                                              false);
    usedByNonActiveOverGrad = ColourGradient (Colour (158, 95,  32),  0.0, 5.0f,
                                              Colour (128, 70,  13),  0.0, 0.0f,
                                              false);
    neutralGrad             = ColourGradient (Colour (220, 220, 220), 0.0, 0.0,
                                              Colour (170, 170, 170), 0.0, 20.0f,
                                              false);
    neutralOverGrad         = ColourGradient (Colour (180, 180, 180), 0.0, 5.0f,
                                              Colour (150, 150, 150), 0.0, 0.0,
                                              false);
    deactivatedGrad         = ColourGradient (Colour (120, 120, 120), 0.0, 5.0f,
                                              Colour (100, 100, 100), 0.0, 0.0,
                                              false);
}


void ParameterButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (Colours::grey);
    g.fillPath (outlinePath);

    if (colorState == 1)
        g.setGradientFill (isMouseOver ? selectedOverGrad : selectedGrad);
    else if (colorState == 2)
        g.setGradientFill (isMouseOver ? usedByNonActiveOverGrad : usedByNonActiveGrad);
    else
        g.setGradientFill (isMouseOver ? neutralOverGrad : neutralGrad);

    if (! isEnabled)
        g.setGradientFill (deactivatedGrad);

    AffineTransform a = AffineTransform::scale (0.98f, 0.94f, float (getWidth()) / 2.0f,
                                                float (getHeight()) / 2.0f);
    g.fillPath (outlinePath, a);

    font.setHeight (12.0f);
    int stringWidth = font.getStringWidth (valueString);

    g.setFont (font);

    g.setColour (Colours::darkgrey);
    g.drawSingleLineText (valueString, getWidth() / 2 - stringWidth / 2, 12);
};


void ParameterButton::resized()
{
    float radius = 5.0f;

    if (type == LEFT)
    {
        outlinePath.startNewSubPath (0, radius);
        outlinePath.addArc (0, 0, radius * 2, radius * 2, 1.5 * double_Pi, 2.0 * double_Pi);

        outlinePath.lineTo (getWidth(), 0);
        outlinePath.lineTo (getWidth(), getHeight());
        outlinePath.lineTo (radius, getHeight());

        outlinePath.addArc (0, getHeight() - radius * 2, radius * 2, radius * 2, double_Pi, 1.5 * double_Pi);
        outlinePath.closeSubPath();
    }
    else if (type == RIGHT)
    {
        outlinePath.startNewSubPath (0, 0);

        outlinePath.lineTo (getWidth() - radius, 0);

        outlinePath.addArc (getWidth() - radius * 2, 0, radius * 2, radius * 2, 0, 0.5 * double_Pi);

        outlinePath.lineTo (getWidth(), getHeight() - radius);

        outlinePath.addArc (getWidth() - radius * 2, getHeight() - radius * 2, radius * 2, radius * 2, 0.5 * double_Pi, double_Pi);

        outlinePath.lineTo (0, getHeight());
        outlinePath.closeSubPath();
    }
    else if (type == MIDDLE)
    {
        outlinePath.addRectangle (0, 0, getWidth(), getHeight());
    }
}


// ==== PARAMETER CHECKBOX =======================
ParameterCheckbox::ParameterCheckbox (bool defaultState) 
    : Button    ("name")
    , isEnabled (true)
{
    setToggleState (defaultState, dontSendNotification);
    setClickingTogglesState (true);

    selectedGrad        = ColourGradient (Colour (240, 179, 12),  0.0, 0.0,
                                          Colour (207, 160, 33),  0.0, 20.0f,
                                          true);
    selectedOverGrad    = ColourGradient (Colour (209, 162, 33),  0.0, 5.0f,
                                          Colour (190, 150, 25),  0.0, 0.0f,
                                          true);
    neutralGrad         = ColourGradient (Colour (220, 220, 220), 0.0, 0.0,
                                          Colour (170, 170, 170), 0.0, 20.0f,
                                          true);
    neutralOverGrad     = ColourGradient (Colour (180, 180, 180), 0.0, 5.0f,
                                          Colour (150, 150, 150), 0.0, 0.0,
                                          true);
    deactivatedGrad     = ColourGradient (Colour (120, 120, 120), 0.0, 5.0f,
                                          Colour (100, 100, 100), 0.0, 0.0,
                                          false);
}


void ParameterCheckbox::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (Colours::grey);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 2.0f);

    if (getToggleState())
        g.setGradientFill (isMouseOver ? selectedOverGrad : selectedGrad);
    else
        g.setGradientFill (isMouseOver ? neutralOverGrad : neutralGrad);

    if (! isEnabled)
        g.setGradientFill (deactivatedGrad);

    g.fillRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 2.0f);
}


void ParameterCheckbox::clicked()
{
    Button::clicked();

    setButtonText (String (getToggleState()));
}


// ========== PARAMETER SLIDER ====================
ParameterSlider::ParameterSlider (float minValue, float maxValue, float def, Font labelFont) 
    : Slider    ("name")
    , isEnabled (true)
    , font      (labelFont)
{
    setSliderStyle (Slider::Rotary);
    setRange (minValue, maxValue, 1.0f);
    setValue (def);
    setTextBoxStyle (Slider::NoTextBox, false, 40, 20);

    setColour (Slider::rotarySliderFillColourId, Colour (240, 179, 12));
}


void ParameterSlider::paint (Graphics& g)
{
    ColourGradient grad = ColourGradient (Colour (40, 40, 40), 0.0f, 0.0f,
                                          Colour (80, 80, 80), 0.0f, 40.0f,
                                          false);

    Path p;
    p.addPieSegment (3, 3,
                     getWidth() - 6, getHeight() - 6,
                     5 * double_Pi / 4 - 0.2, 5 * double_Pi / 4 + 3 * double_Pi / 2 + 0.2,
                     0.5);

    g.setGradientFill (grad);
    g.fillPath (p);

    p = makeRotaryPath (getMinimum(), getMaximum(), getValue());

    if (isEnabled)
        g.setColour (findColour (Slider::rotarySliderFillColourId));
    else
        g.setColour (Colour (75, 75, 75));

    g.fillPath (p);

    font.setHeight (9.0);
    g.setFont (font);

    String valueString = String ((int) getValue());

    const int stringWidth = font.getStringWidth (valueString);

    g.setFont (font);

    g.setColour (Colours::darkgrey);
    g.drawSingleLineText (valueString, getWidth() / 2 - stringWidth / 2, getHeight() / 2 + 3);
}


Path ParameterSlider::makeRotaryPath (double minValue, double maxValue, double value)
{
    Path p;

    const double start = 5 * double_Pi / 4 - 0.11;
    const double range = (value - minValue) / (maxValue - minValue) *1.5 * double_Pi + start + 0.22;

    p.addPieSegment (6, 6,
                     getWidth() - 12, getHeight() - 12,
                     start, range,
                     0.65);

    return p;
}


// ParameterLabel
// ============================================================================
ParameterLabel::ParameterLabel (const String& labelName, double minValue, double maxValue, double defaultValue)
    : isEnabled      (true)
    , m_minValue     (minValue)
    , m_maxValue     (maxValue)
    , m_defaultValue (defaultValue)
    , m_infoLabel    ("Info label", labelName)
    , m_valueLabel   ("Value label", String (defaultValue))
    , m_infoFont     ("Arial", 13, Font::plain)
    , m_valueFont    ("Arial", 15, Font::plain)
{
    m_infoLabel.setColour (Label::textColourId, Colours::darkgrey);
    m_infoLabel.setFont (m_infoFont);
    addAndMakeVisible (&m_infoLabel);

    m_valueLabel.setColour (Label::textColourId,        Colours::white);
    m_valueLabel.setColour (Label::backgroundColourId,  Colours::grey);
    m_valueLabel.setText (String (defaultValue), dontSendNotification);
    //m_valueLabel.setJustificationType (Justification::topLeft);
    m_valueLabel.setJustificationType (Justification::centredLeft);
    m_valueLabel.setFont (m_valueFont);
    m_valueLabel.setEditable (true);
    m_valueLabel.addListener (this);
    addAndMakeVisible (&m_valueLabel);
}


void ParameterLabel::resized()
{
    const int margin = 3;
    auto localBounds = getLocalBounds();

    m_infoLabel.setBounds  (localBounds.removeFromTop (m_infoFont.getHeight() + margin));
    m_valueLabel.setBounds (localBounds);
}


void ParameterLabel::labelTextChanged (Label* label)
{
    if (label == &m_valueLabel)
    {
        Value val = label->getTextValue();
        double requestedValue = double (val.getValue());
        setValue (requestedValue, sendNotificationAsync);
    }
}


double ParameterLabel::getValue() const noexcept
{
    return m_valueLabel.getText().getDoubleValue();
}


void ParameterLabel::setValue (double value, NotificationType notificationType)
{
    if (value < m_minValue || value > m_maxValue)
    {
        CoreServices::sendStatusMessage ("Value out of range.");

        m_valueLabel.setText (String (m_defaultValue), dontSendNotification);
    }
    else
    {
        m_valueLabel.setText (String (value), notificationType);

        if (notificationType != dontSendNotification)
            m_listeners.call (&ParameterLabel::Listener::parameterLabelValueChanged, this);
    }
}


void ParameterLabel::setInfoFont (Font font)
{
    m_infoFont = font;
    m_infoLabel.setFont (font);
}


void ParameterLabel::setValueFont (Font font)
{
    m_valueFont = font;
    m_valueLabel.setFont (font);
}


void ParameterLabel::addListener (Listener* listener)
{
    m_listeners.add (listener);
}


void ParameterLabel::removeListener (Listener* listener)
{
    m_listeners.remove (listener);
}
// ============================================================================


void ParameterEditor::updateChannelSelectionUI()
{
    const int numChannels = m_channelSelector->getNumChannels();
    if (m_parameter->isBoolean())
    {
        m_checkboxArray[0]->setToggleState (m_parameter->getValue (m_processor->getCurrentChannel()), dontSendNotification);
    }
    else if (m_parameter->isContinuous())
    {
        m_sliderArray[0]->setValue (m_parameter->getValue (m_processor->getCurrentChannel()), dontSendNotification);
    }
    else if (m_parameter->isNumerical())
    {
        m_parameterLabelsArray[0]->setValue (m_parameter->getValue (m_processor->getCurrentChannel()), dontSendNotification);
    }
    else if (m_parameter->isDiscrete())
    {
        LOGD("Calculating colors for discrete buttons");
        Array<var> possibleValues = m_parameter->getPossibleValues();

        for (int i = 0; i < m_buttonArray.size(); ++i)
        {
            m_buttonArray[i]->colorState = 0;

            for (int j = 0; j < numChannels; ++j)
            {
                if (possibleValues[i] == m_parameter->getValue (j))
                {
                    if (m_channelSelector->getParamStatus (j))
                    {
                        /* Set button as usedbyactive */
                        m_buttonArray[i]->colorState = 1;
                    }
                    else if (m_buttonArray[i]->colorState == 0)
                    {
                        // Set button as used by non-selected
                        m_buttonArray[i]->colorState = 2;
                    }
                }
            }

            m_buttonArray[i]->repaint();
        }
    }
}


bool ParameterEditor::hasCustomBounds() const noexcept
{
    return m_parameter->hasCustomEditorBounds();
}


const Rectangle<int>& ParameterEditor::getDesiredBounds() const noexcept
{
    return m_parameter->getEditorDesiredBounds();
}