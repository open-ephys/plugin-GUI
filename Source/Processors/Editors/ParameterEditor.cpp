/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

ParameterEditor::ParameterEditor(Parameter& p, Font labelFont)
{

	if (p.isBoolean())
	{
		std::cout << "Boolean parameter. Creating checkbox." << std::endl;
		// create checkbox
		//ParameterCheckbox* pc = new ParameterCheckbox(p.getName());
		//addAndMakeVisible(pc);
		//pc->setBounds(0,0,getWidth(), getHeight());

	} else if (p.isContinuous())
	{
		std::cout << "Continuous parameter. Creating slider." << std::endl;
		// create slider
		//ParameterSlider* ps = new ParameterSlider(p.getName(), p.getPossibleValues());

	} else if (p.isDiscrete())
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

		for (int i = 0; i < possibleValues.size(); i++)
		{
			std::cout << "Creating button " << i << std::endl;
			int buttonType = MIDDLE;
			if (i == 0)
				buttonType = LEFT;
			else if (i == possibleValues.size()-1)
				buttonType = RIGHT;

			ParameterButton* pb = new ParameterButton(possibleValues[i], buttonType, labelFont);
			pb->setBounds(buttonWidth*i, 12, buttonWidth, 18);

			if (i == (int) p.getDefaultValue())
				pb->setToggleState(true, false);

			componentArray.add(pb);
			addAndMakeVisible(pb);

		}
	}
}

ParameterEditor::~ParameterEditor()
{
	deleteAllChildren();
}

void ParameterEditor::paint(Graphics& g)
{

	// Path p;
	// PathStrokeType pst = PathStrokeType(2.0f);

	// float radius = 10.0f;

	// p.startNewSubPath(0, radius);
	// p.addArc(0, 0, radius*2, radius*2, 1.5*double_Pi, 2.0*double_Pi );

	// p.lineTo(getWidth() - radius, 0);//getHeight());
	// p.addArc(getWidth()-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);

	// p.lineTo(getWidth(), getHeight()-radius);
	// p.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);

	// p.lineTo(radius, getHeight());
	// p.addArc(0, getHeight()-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);
	// p.closeSubPath();
	// //p.lineTo(0, radius);

	// g.setColour(Colours::grey);
	// //g.strokePath(p, pst);
	// g.fillPath(p);
	// //g.setColour(Colours::grey)
	// ColourGradient grad = ColourGradient(Colour(220,220,220),0.0,0.0,
	// 									 Colour(170,170,170),0.0, getHeight(),
	// 									 false);
	// //grad.addColour(0.5f, Colour(50,50,50));
	// g.setGradientFill(grad);

	// AffineTransform a = AffineTransform::scale(0.98f, 0.94f, float(getWidth())/2.0f,
	// 														float(getHeight())/2.0f);
	// //a.scaled(0.7, 0.7);
	// g.fillPath(p, a);



}

/// ============= PARAMETER BUTTON ==================

ParameterButton::ParameterButton(var value, int buttonType, Font labelFont) :
	Button("parameter"), type(buttonType), valueString(value.toString()),
	font(labelFont)
{

	setRadioGroupId(1999);
	setClickingTogglesState(true);


	selectedGrad = ColourGradient(Colour(240,179,12),0.0,0.0,
										 Colour(207,160,33),0.0, 20.0f,
										 false);
    selectedOverGrad = ColourGradient(Colour(209,162,33),0.0, 5.0f,
										 Colour(190,150,25),0.0, 0.0f,
										 false);
    neutralGrad = ColourGradient(Colour(220,220,220),0.0,0.0,
										 Colour(170,170,170),0.0, 20.0f,
										 false);
    neutralOverGrad = ColourGradient(Colour(180,180,180),0.0,5.0f,
										 Colour(150,150,150),0.0, 0.0,
										 false);


}

void ParameterButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
 	g.setColour(Colours::grey);
	g.fillPath(outlinePath);

	 if (getToggleState())
     {
     	if (isMouseOver)
     		g.setGradientFill(selectedOverGrad);
        else
        	g.setGradientFill(selectedGrad);
     } else {
         if (isMouseOver)
         	g.setGradientFill(neutralOverGrad);
        else
        	g.setGradientFill(neutralGrad);
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
		outlinePath.addArc(0, 0, radius*2, radius*2, 1.5*double_Pi, 2.0*double_Pi );

		outlinePath.lineTo(getWidth(), 0);//getHeight());

		outlinePath.lineTo(getWidth(), getHeight());

		outlinePath.lineTo(radius, getHeight());
		outlinePath.addArc(0, getHeight()-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);
		outlinePath.closeSubPath();

	} else if (type == RIGHT)
	{
		outlinePath.startNewSubPath(0, 0);

		outlinePath.lineTo(getWidth()-radius, 0);
	
		outlinePath.addArc(getWidth()-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);

		outlinePath.lineTo(getWidth(), getHeight()-radius);

		outlinePath.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);

		outlinePath.lineTo(0, getHeight());
		outlinePath.closeSubPath();


	} else if (type == MIDDLE)
	{
		outlinePath.addRectangle(0,0,getWidth(),getHeight());
	}
}

