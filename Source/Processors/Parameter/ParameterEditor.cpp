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

#include "ParameterEditor.h"

ParameterEditor::ParameterEditor(GenericProcessor* proc, Parameter& p, Font labelFont)
{

    activationState = true;

    processor = proc;

    parameter = &p;

    shouldDeactivateDuringAcquisition = p.shouldDeactivateDuringAcquisition;

    if (p.isBoolean())
    {
        std::cout << "Boolean parameter. Creating checkbox." << std::endl;

        // create checkbox
        ParameterCheckbox* pc = new ParameterCheckbox((bool) p.getDefaultValue());
        addAndMakeVisible(pc);
        pc->setBounds(0,0,12, 12);
        pc->setName(String(p.getID()));
        checkboxArray.add(pc);
        //buttonIdArray.add(p.getID());
        pc->addListener(this);

        Label* label = new Label(p.getName(), p.getName());
        labelFont.setHeight(10);
        label->setColour(Label::textColourId, Colours::darkgrey);
        label->setFont(labelFont);
        label->setBounds(10, 1, 100, 10);
        addAndMakeVisible(label);

        desiredWidth = 120;
        desiredHeight = 25;

    }
    else if (p.isContinuous())
    {
        std::cout << "Continuous parameter. Creating slider." << std::endl;
        // create slider
        Array<var> possibleValues = p.getPossibleValues();
        ParameterSlider* ps = new ParameterSlider((float) possibleValues[0],
                                                  (float) possibleValues[1],
                                                  (float) p.getDefaultValue(),
                                                  labelFont);

        ps->setBounds(0,0, 80, 80);
        ps->setName(String(p.getID()));
        addAndMakeVisible(ps);
        sliderArray.add(ps);
        //sliderIdArray.add(p.getID());
        ps->addListener(this);

        Label* label = new Label(p.getName(), p.getName());
        labelFont.setHeight(10);
        int width = labelFont.getStringWidth(p.getName());
        label->setColour(Label::textColourId, Colours::darkgrey);
        label->setFont(labelFont);
        label->setBounds((80-width)/2-5, 70, 100, 10);
        addAndMakeVisible(label);

        desiredWidth = 80;
        desiredHeight = 80;

    }
    else if (p.isDiscrete())
    {
        std::cout << "Discrete parameter. Creating buttons." << std::endl;
        // create buttons
        Label* label = new Label(p.getName(), p.getName());
        labelFont.setHeight(10);
        label->setColour(Label::textColourId, Colours::darkgrey);
        label->setFont(labelFont);
        label->setBounds(0, 0, 100, 10);
        addAndMakeVisible(label);

        Array<var> possibleValues = p.getPossibleValues();

        int buttonWidth = 35;

        std::cout << "Button width: " << buttonWidth << std::endl;

        std::cout << "Default value: " << (int) p.getDefaultValue() << std::endl;

        int i;

        for (i = 0; i < possibleValues.size(); i++)
        {
            std::cout << "Creating button " << i << std::endl;
            int buttonType = MIDDLE;
            if (i == 0)
                buttonType = LEFT;
            else if (i == possibleValues.size()-1)
                buttonType = RIGHT;

            ParameterButton* pb = new ParameterButton(possibleValues[i], buttonType, labelFont);
            pb->setBounds(buttonWidth*i, 12, buttonWidth, 18);
            pb->setName(String(p.getID()));
            buttonArray.add(pb);
            //buttonIdArray.add(p.getID());
            pb->addListener(this);

            if (i == (int) p.getDefaultValue())
                pb->setToggleState(true, dontSendNotification);

            addAndMakeVisible(pb);

        }

        desiredWidth = buttonWidth*i;
        desiredHeight = 30;
    }
}

ParameterEditor::~ParameterEditor()
{
    deleteAllChildren();
}

void ParameterEditor::parentHierarchyChanged()
{
    // std::cout << "Parent hierarchy changed." << std::endl;

    // // register all children with parent --> not currently working
    // if (getParentComponent() != 0) {

    // 	for (int i = 0; i < sliderArray.size(); i++)
    // 	{
    // 		sliderArray[i]->addListener((Slider::Listener*) getParentComponent());
    // 	}

    // 	for (int i = 0; i < buttonArray.size(); i++)
    // 	{
    // 		buttonArray[i]->addListener((Button::Listener*) getParentComponent());
    // 	}
    // }

}

void ParameterEditor::setChannelSelector(ChannelSelector* ch)
{
	channelSelector = ch;
}

void ParameterEditor::setEnabled(bool state)
{

    std::cout << "Changing editor state!" << std::endl;

    if (shouldDeactivateDuringAcquisition)
    {

        for (int i = 0; i < sliderArray.size(); i++)
        {
            sliderArray[i]->isEnabled = state;
            sliderArray[i]->setInterceptsMouseClicks(state, state);
            sliderArray[i]->repaint();
        }

        for (int i = 0; i < buttonArray.size(); i++)
        {
            buttonArray[i]->isEnabled = state;
            buttonArray[i]->setInterceptsMouseClicks(state, state);
            buttonArray[i]->repaint();
        }

        for (int i = 0; i < checkboxArray.size(); i++)
        {
            checkboxArray[i]->isEnabled = state;
            checkboxArray[i]->setInterceptsMouseClicks(state, state);
            checkboxArray[i]->repaint();
        }

    }

}

void ParameterEditor::buttonClicked(Button* button)
{
    std::cout << "Button name: " << button->getName() << std::endl;
    std::cout << "Button value: " << button->getButtonText() << std::endl;

    ParameterButton* b = (ParameterButton*) button;

    if (b->isEnabled)
    {

        Array<int> a = channelSelector->getActiveChannels();
        {
            for (int i = 0; i < a.size(); i++)
            {
                //std::cout << a[i] << " ";
                processor->setCurrentChannel(a[i]);
                processor->setParameter(button->getName().getIntValue(),
                                        button->getButtonText().getFloatValue());
                //processor->
            }
            //std::cout << std::endl;
        }
    }
    //processor->sliderValueChanged(slider);

}

void ParameterEditor::sliderValueChanged(Slider* slider)
{


    //std::cout << "Slider name: " << slider->getName() << std::endl;
    //std::cout << "Slider value: " << slider->getValue() << std::endl;

    ParameterSlider* s = (ParameterSlider*) slider;

    if (s->isEnabled)
    {
        Array<int> a = channelSelector->getActiveChannels();
        {
            for (int i = 0; i < a.size(); i++)
            {
                //std::cout << a[i] << " ";
                processor->setCurrentChannel(a[i]);
                processor->setParameter(slider->getName().getIntValue(),
                                        slider->getValue());
                //processor->
            }
            //std::cout << std::endl;
        }
    }
}



/// ============= PARAMETER BUTTON ==================

ParameterButton::ParameterButton(var value, int buttonType, Font labelFont) :
    Button("parameter"), isEnabled(true), type(buttonType),
    valueString(value.toString()), font(labelFont)
{

    setButtonText(valueString);
    setRadioGroupId(1999);
    setClickingTogglesState(true);


    selectedGrad = ColourGradient(Colour(240,179,12),0.0,0.0,
                                  Colour(207,160,33),0.0, 20.0f,
                                  false);
    selectedOverGrad = ColourGradient(Colour(209,162,33),0.0, 5.0f,
                                      Colour(190,150,25),0.0, 0.0f,
                                      false);
    usedByNonActiveGrad = ColourGradient(Colour(200,100,0),0.0,0.0,
                                         Colour(158,95,32),0.0, 20.0f,
                                         false);
    usedByNonActiveOverGrad = ColourGradient(Colour(158,95,32),0.0, 5.0f,
                                             Colour(128,70,13),0.0, 0.0f,
                                             false);
    neutralGrad = ColourGradient(Colour(220,220,220),0.0,0.0,
                                 Colour(170,170,170),0.0, 20.0f,
                                 false);
    neutralOverGrad = ColourGradient(Colour(180,180,180),0.0,5.0f,
                                     Colour(150,150,150),0.0, 0.0,
                                     false);
    deactivatedGrad = ColourGradient(Colour(120, 120, 120), 0.0, 5.0f,
                                     Colour(100, 100, 100), 0.0, 0.0,
                                     false);

}

ParameterButton::~ParameterButton() {}

void ParameterButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour(Colours::grey);
    g.fillPath(outlinePath);

    if (colorState==1)
    {
        if (isMouseOver)
            g.setGradientFill(selectedOverGrad);
        else
            g.setGradientFill(selectedGrad);
    }
    else if (colorState==2)
    {
        if (isMouseOver)
            g.setGradientFill(usedByNonActiveOverGrad);
        else
            g.setGradientFill(usedByNonActiveGrad);
    }
    else
    {
        if (isMouseOver)
            g.setGradientFill(neutralOverGrad);
        else
            g.setGradientFill(neutralGrad);
    }

    if (!isEnabled)
    {
        g.setGradientFill(deactivatedGrad);
    }

    AffineTransform a = AffineTransform::scale(0.98f, 0.94f, float(getWidth())/2.0f,
                                               float(getHeight())/2.0f);
    g.fillPath(outlinePath, a);

    font.setHeight(12.0f);
    int stringWidth = font.getStringWidth(valueString);

    g.setFont(font);

    g.setColour(Colours::darkgrey);
    g.drawSingleLineText(valueString, getWidth()/2 - stringWidth/2, 12);

};

void ParameterButton::resized()
{


    float radius = 5.0f;

    if (type == LEFT)
    {
        outlinePath.startNewSubPath(0, radius);
        outlinePath.addArc(0, 0, radius*2, radius*2, 1.5*double_Pi, 2.0*double_Pi);

        outlinePath.lineTo(getWidth(), 0);//getHeight());

        outlinePath.lineTo(getWidth(), getHeight());

        outlinePath.lineTo(radius, getHeight());
        outlinePath.addArc(0, getHeight()-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);
        outlinePath.closeSubPath();

    }
    else if (type == RIGHT)
    {
        outlinePath.startNewSubPath(0, 0);

        outlinePath.lineTo(getWidth()-radius, 0);

        outlinePath.addArc(getWidth()-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);

        outlinePath.lineTo(getWidth(), getHeight()-radius);

        outlinePath.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);

        outlinePath.lineTo(0, getHeight());
        outlinePath.closeSubPath();


    }
    else if (type == MIDDLE)
    {
        outlinePath.addRectangle(0,0,getWidth(),getHeight());
    }
}


// ==== PARAMETER CHECKBOX =======================


ParameterCheckbox::ParameterCheckbox(bool defaultState) : Button("name"), isEnabled(true)
{
    setToggleState(defaultState, dontSendNotification);
    setClickingTogglesState(true);

    selectedGrad = ColourGradient(Colour(240,179,12),0.0,0.0,
                                  Colour(207,160,33),0.0, 20.0f,
                                  true);
    selectedOverGrad = ColourGradient(Colour(209,162,33),0.0, 5.0f,
                                      Colour(190,150,25),0.0, 0.0f,
                                      true);
    neutralGrad = ColourGradient(Colour(220,220,220),0.0,0.0,
                                 Colour(170,170,170),0.0, 20.0f,
                                 true);
    neutralOverGrad = ColourGradient(Colour(180,180,180),0.0,5.0f,
                                     Colour(150,150,150),0.0, 0.0,
                                     true);
    deactivatedGrad = ColourGradient(Colour(120, 120, 120), 0.0, 5.0f,
                                     Colour(100, 100, 100), 0.0, 0.0,
                                     false);
}

ParameterCheckbox::~ParameterCheckbox() {}

void ParameterCheckbox::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    g.setColour(Colours::grey);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 2.0f);

    if (getToggleState())
    {
        if (isMouseOver)
            g.setGradientFill(selectedOverGrad);
        else
            g.setGradientFill(selectedGrad);
    }
    else
    {
        if (isMouseOver)
            g.setGradientFill(neutralOverGrad);
        else
            g.setGradientFill(neutralGrad);
    }

    if (!isEnabled)
    {
        g.setGradientFill(deactivatedGrad);
    }

    // AffineTransform a = AffineTransform::scale(0.98f, 0.94f, float(getWidth())/2.0f,
    //                                           float(getHeight())/2.0f);

    g.fillRoundedRectangle(1, 1, getWidth()-2, getHeight()-2, 2.0f);

}

// ========== PARAMETER SLIDER ====================

ParameterSlider::ParameterSlider(float min, float max, float def, Font labelFont) :
    Slider("name"), isEnabled(true), font(labelFont)
{
    setSliderStyle(Slider::Rotary);
    setRange(min,max,1.0f);
    setValue(def);
    setTextBoxStyle(Slider::NoTextBox, false, 40, 20);

    setColour (Slider::rotarySliderFillColourId, Colour (240, 179, 12));
}

ParameterSlider::~ParameterSlider() {}

void ParameterSlider::paint(Graphics& g)
{

    ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,

                                         Colour(80, 80, 80), 0.0, 40.0f, false);

    Path p;
    p.addPieSegment(3, 3, getWidth()-6, getHeight()-6, 5*double_Pi/4-0.2, 5*double_Pi/4+3*double_Pi/2+0.2, 0.5);

    g.setGradientFill(grad);
    g.fillPath(p);
    //g.fillEllipse(3, 3, getWidth()-6, getHeight()-6);

    //g.setColour(Colours::lightgrey);
    //g.fillEllipse(12, 12, getWidth()-24, getHeight()-24);

    p = makeRotaryPath(getMinimum(), getMaximum(), getValue());

    if (isEnabled)
        g.setColour (findColour (Slider::rotarySliderFillColourId));
    else
        g.setColour(Colour(75,75,75));

    g.fillPath(p);

    //g.setColour(Colours::darkgrey);
    font.setHeight(9.0);
    g.setFont(font);


    String valueString = String((int) getValue());

    int stringWidth = font.getStringWidth(valueString);

    g.setFont(font);

    g.setColour(Colours::darkgrey);
    g.drawSingleLineText(valueString, getWidth()/2 - stringWidth/2, getHeight()/2+3);

}

Path ParameterSlider::makeRotaryPath(double min, double max, double val)
{
    Path p;

    double start = 5*double_Pi/4 - 0.11;

    double range = (val-min)/(max - min)*1.5*double_Pi + start + 0.22;

    p.addPieSegment(6,6, getWidth()-12, getHeight()-12, start, range, 0.65);

    // p.startNewSubPath(5, getHeight()-5);
    // p.addArc(5, 5, getWidth()-10, getWidth()-10, 5/4*double_Pi, range);
    // //p.addArc(getWidth()-5, getHeight()-5, getWidth()-16, getWidth()-16, 5/4*double_Pi, range);
    // p.closeSubPath();

    return p;
}

void ParameterEditor::channelSelectionUI()
{

    int numChannels=channelSelector->getNumChannels();
    if (parameter->isBoolean())
    {
    }
    else if (parameter->isContinuous())
    {

    }
    else if (parameter->isDiscrete())
    {
        std::cout << "Calculating colors for discrete buttons" << std::endl;
        Array<var> possibleValues=parameter->getPossibleValues();

        for (int i = 0; i < buttonArray.size(); i++)
        {
            buttonArray[i]->colorState=0;

            for (int j = 0; j < numChannels; j++)
            {

                if (possibleValues[i]==parameter->getValue(j))
                {

                    if (channelSelector->getParamStatus(j))
                    {
                        /* Set button as usedbyactive */
                        buttonArray[i]->colorState=1;

                    }
                    else if (buttonArray[i]->colorState==0)
                    {
                        // Set button as used by non-selected
                        buttonArray[i]->colorState=2;
                    }

                }
            }
            buttonArray[i]->repaint();


        }
    }
}
