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

#include "GenericEditor.h"

#include "ParameterEditor.h"
#include "ChannelSelector.h"
#include "../ProcessorGraph.h"
#include "../RecordNode.h"
#include "../../UI/ProcessorList.h"

#include "../../UI/EditorViewport.h"

#include <math.h>

GenericEditor::GenericEditor (GenericProcessor* owner) 
	: AudioProcessorEditor (owner), isSelected(false),
	  desiredWidth(150), tNum(-1), isEnabled(true),
	  accumulator(0.0), isFading(false), drawerButton(0),
	  channelSelector(0)

{
	name = getAudioProcessor()->getName();

	nodeId = owner->getNodeId();

	MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    titleFont = Font(typeface);

    if (!owner->isMerger() && !owner->isSplitter())
    {
    	drawerButton = new DrawerButton("name");
    	drawerButton->addListener(this);
    	addAndMakeVisible(drawerButton);

    	if (!owner->isSink())
    	{
    		channelSelector = new ChannelSelector(true, titleFont);
    	} else {
    		channelSelector = new ChannelSelector(false, titleFont);
    	}

    	addChildComponent(channelSelector);
    	channelSelector->setVisible(false);


	}

	backgroundGradient = ColourGradient(Colour(190, 190, 190), 0.0f, 0.0f, 
										 Colour(185, 185, 185), 0.0f, 120.0f, false);
	backgroundGradient.addColour(0.2f, Colour(155, 155, 155));

	addParameterEditors();

	backgroundColor = Colour(10,10,10);

	//fadeIn();
}

GenericEditor::~GenericEditor()
{
	deleteAllChildren();
}

void GenericEditor::addParameterEditors()
{
	
	int maxX = 15;
	int maxY = 30;

	std::cout << "Adding parameter editors." << std::endl;

	for (int i = 0; i < getProcessor()->getNumParameters(); i++)
	{
		ParameterEditor* p = new ParameterEditor(getProcessor(), getProcessor()->getParameterReference(i), titleFont);												
		
		p->setChannelSelector(channelSelector);
		int dWidth = p->desiredWidth;
		int dHeight = p->desiredHeight;

		p->setBounds(maxX, maxY, dWidth, dHeight);
		addAndMakeVisible(p);
		parameterEditors.add(p);

		maxY += dHeight;
		maxY += 10;
	}
}


void GenericEditor::refreshColors()
{

	//std::cout << getName() << " refreshing colors." << std::endl;

	enum {
		PROCESSOR_COLOR = 801,
		FILTER_COLOR = 802,
		SINK_COLOR = 803,
		SOURCE_COLOR = 804,
		UTILITY_COLOR = 805,
	};

	if (getProcessor()->isSource())
		backgroundColor = getProcessorList()->findColour(SOURCE_COLOR);// Colour(255, 0, 0);//Colour(int(0.9*255.0f),int(0.019*255.0f),int(0.16*255.0f));
	else if (getProcessor()->isSink())
		backgroundColor = getProcessorList()->findColour(SINK_COLOR);//Colour(255, 149, 0);//Colour(int(0.06*255.0f),int(0.46*255.0f),int(0.9*255.0f));
	else if (getProcessor()->isSplitter() || getProcessor()->isMerger())
		backgroundColor =  getProcessorList()->findColour(UTILITY_COLOR);//Colour(40, 40, 40);//Colour(int(0.7*255.0f),int(0.7*255.0f),int(0.7*255.0f));
	else
		backgroundColor =  getProcessorList()->findColour(FILTER_COLOR);//Colour(255, 89, 0);//Colour(int(1.0*255.0f),int(0.5*255.0f),int(0.0*255.0f));

	repaint();

}


void GenericEditor::resized()
{
	if (drawerButton != 0)
		drawerButton->setBounds(getWidth()-14, 40, 10, getHeight()-60);
	
	if (channelSelector != 0)
	 	channelSelector->setBounds(desiredWidth - drawerWidth, 30, channelSelector->getDesiredWidth(), getHeight()-45);
}


bool GenericEditor::keyPressed (const KeyPress& key)
{
	return false;
}

void GenericEditor::switchSelectedState() 
{
	//std::cout << "Switching selected state" << std::endl;
	isSelected = !isSelected;
	repaint();
}

void GenericEditor::select()
{
	isSelected = true;
	repaint();
	//setWantsKeyboardFocus(true);
	//grabKeyboardFocus();

	editorWasClicked();
}

void GenericEditor::highlight()
{
	isSelected = true;
	repaint();
}

bool GenericEditor::getSelectionState() {
	return isSelected;
}

void GenericEditor::deselect()
{
	isSelected = false;
	repaint();
	//setWantsKeyboardFocus(false);
}

void GenericEditor::enable() 
{
	isEnabled = true;
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	p->enabledState(true);
}

void GenericEditor::disable()
{
	isEnabled = false;
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	p->enabledState(false);
}

bool GenericEditor::getEnabledState()
{
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	return p->enabledState();
}

void GenericEditor::setEnabledState(bool t)
{
	
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	p->enabledState(t);
	isEnabled = p->enabledState();
}

void GenericEditor::startAcquisition()
{

	std::cout << "GenericEditor received message to start acquisition." << std::endl;

	if (channelSelector != 0)
		channelSelector->startAcquisition();

	for (int n = 0; n < parameterEditors.size(); n++)
	{

		if (parameterEditors[n]->shouldDeactivateDuringAcquisition)
			parameterEditors[n]->setEnabled(false);

	}

}

void GenericEditor::stopAcquisition()
{
	if (channelSelector != 0)
		channelSelector->stopAcquisition();

	for (int n = 0; n < parameterEditors.size(); n++)
	{

		if (parameterEditors[n]->shouldDeactivateDuringAcquisition)
			parameterEditors[n]->setEnabled(true);

	}


}

void GenericEditor::fadeIn()
{
	isFading = true;
	startTimer(10);
}

void GenericEditor::paint (Graphics& g)
{
	int offset = 0;

	GenericProcessor* p = (GenericProcessor*) getProcessor();

	if (isEnabled)
		g.setColour(backgroundColor);
	else 
		g.setColour(Colours::lightgrey);

    // draw colored background
	g.fillRect(1,1,getWidth()-(2+offset),getHeight()-2);

	// draw gray workspace
	g.setGradientFill(backgroundGradient);
	g.fillRect(1,22,getWidth()-2, getHeight()-29);

	g.setFont(titleFont);
	g.setFont(14);

	if (isEnabled) 
	{
		g.setColour(Colours::white);		
	} else {
		g.setColour(Colours::grey);
	}

	// draw title
	g.drawText(name, 6, 5, 500, 15, Justification::left, false);


	if (isSelected) {
		//g.setColour(Colours::yellow);
		//g.drawRect(0,0,getWidth(),getHeight(),1.0);
		g.setColour(Colours::yellow.withAlpha(0.5f));
		
	} else {
		g.setColour(Colours::black);
	}

	// draw highlight box
	g.drawRect(0,0,getWidth(),getHeight(),2.0);

	if (isFading)
	{
		g.setColour(Colours::black.withAlpha((float) (10.0-accumulator)/10.0f));
		if (getWidth() > 0 && getHeight() > 0)
			g.fillAll();
	}

}

void GenericEditor::timerCallback()
{
	accumulator++;

	repaint();

	if (accumulator > 10.0)
	{
		stopTimer();
		isFading = false;
	}
}

void GenericEditor::buttonClicked(Button* button)
{

	std::cout << "Button clicked." << std::endl;
	
	checkDrawerButton(button);

	buttonEvent(button); // needed to inform subclasses of 
						 // button event
}


bool GenericEditor::checkDrawerButton(Button* button)
{
	if (button == drawerButton)
	{
		if (drawerButton->getToggleState()) 
		{
			
			channelSelector->setVisible(true);

			drawerWidth = channelSelector->getDesiredWidth() + 20;

			desiredWidth += drawerWidth;

		} else {
			
			channelSelector->setVisible(false);

			desiredWidth -= drawerWidth;
		}

		getEditorViewport()->makeEditorVisible(this);

		deselect();

		return true;
	} else {
		return false;
	}

}

void GenericEditor::sliderValueChanged(Slider* slider)
{

	sliderEvent(slider);
}

void GenericEditor::update()
{

	std::cout << "Editor for ";

	GenericProcessor* p = (GenericProcessor*) getProcessor();

	std::cout << p->getName() << " updating settings." << std::endl;

	int numChannels;

	if (channelSelector != 0)
	{
		if (!p->isSink())
			numChannels = p->getNumOutputs();
		else
			numChannels = p->getNumInputs();

		channelSelector->setNumChannels(numChannels);
	}

	if (numChannels == 0)
	{
		if (drawerButton != 0)
			drawerButton->setVisible(false);
	} else {
		if (drawerButton != 0)
			drawerButton->setVisible(true);
	}

	updateSettings();

	updateVisualizer(); // does nothing unless this method
						// has been implemented

}

Channel* GenericEditor::getChannel(int chan)
{
	return getProcessor()->channels[chan];

}

Channel* GenericEditor::getEventChannel(int chan)
{
	return getProcessor()->eventChannels[chan];
}

Array<int> GenericEditor::getActiveChannels()
{
	Array<int> a = channelSelector->getActiveChannels();
	return a;
}

bool GenericEditor::getRecordStatus(int chan)
{
	return channelSelector->getRecordStatus(chan);
}

bool GenericEditor::getAudioStatus(int chan)
{
	return channelSelector->getAudioStatus(chan);
}



/////////////////////// BUTTONS ///////////////////////////////

DrawerButton::DrawerButton(const String& name) : Button(name)
{
	 setClickingTogglesState(true);
}

void DrawerButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
	if (isMouseOver)
		g.setColour(Colour(210,210,210));
	else
		g.setColour(Colour(110, 110, 110));
	
	g.drawVerticalLine(3, 0.0f, getHeight());
	g.drawVerticalLine(5, 0.0f, getHeight());
	g.drawVerticalLine(7, 0.0f, getHeight());	

}

UtilityButton::UtilityButton(const String& label_, Font font_) :
    	 Button(label_), label(label_), font(font_)
 {
 
    roundUL = true;
	roundUR = true;
	roundLL = true;
	roundLR = true;

	radius = 5.0f;

	font.setHeight(12.0f);

	setEnabledState(true);

 }

void UtilityButton::setCorners(bool UL, bool UR, bool LL, bool LR)
{
	roundUL = UL;
	roundUR = UR;
	roundLL = LL;
	roundLR = LR;
}

void UtilityButton::setEnabledState(bool state)
{

	isEnabled = state;

	if (state)
	{
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
		fontColor = Colours::darkgrey;

	} else {

		selectedGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
											 Colour(200,200,200),0.0, 20.0f,
											 false);
		selectedOverGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
											 Colour(200,200,200),0.0, 20.0f,
											 false);
		neutralGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
											 Colour(200,200,200),0.0, 20.0f,
											 false);
		neutralOverGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
											 Colour(200,200,200),0.0, 20.0f,
											 false);
		fontColor = Colours::white;
	}

	repaint();
}

void UtilityButton::setRadius(float r)
{
	radius = r;
}

void UtilityButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
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

	
	//int stringWidth = font.getStringWidth(getName());

	g.setFont(font);

	g.setColour(fontColor);
	g.drawText(getName(),0,0,getWidth(),getHeight(),Justification::centred,true);

	//g.drawSingleLineText(getName(), getWidth()/2 - stringWidth/2, 12);

 	// if (getToggleState() == true)
  //       g.setColour(Colours::orange);
  //   else
  //       g.setColour(Colours::darkgrey);

  //   if (isMouseOver)
  //       g.setColour(Colours::white);

  //   g.fillRect(0,0,getWidth(),getHeight());

  //   font.setHeight(10);
  //   g.setFont(font);
  //   g.setColour(Colours::black);

  //   g.drawRect(0,0,getWidth(),getHeight(),1.0);

    //g.drawText(getName(),0,0,getWidth(),getHeight(),Justification::centred,true);
    // if (isButtonDown)
    // {
    //     g.setColour(Colours::white);
    // }

    // int thickness = 1;
    // int offset = 3;

    // g.fillRect(getWidth()/2-thickness,
    //            offset, 
    //            thickness*2,
    //            getHeight()-offset*2);

    // g.fillRect(offset,
    //            getHeight()/2-thickness,
    //            getWidth()-offset*2,
    //            thickness*2);
}
   
void UtilityButton::resized()
{

	outlinePath.clear();

	if (roundUL)
	{
		outlinePath.startNewSubPath(radius, 0);
	} else {
		outlinePath.startNewSubPath(0, 0);
	}

	if (roundUR)
	{
		outlinePath.lineTo(getWidth()-radius, 0);
		outlinePath.addArc(getWidth()-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);
	} else {
		outlinePath.lineTo(getWidth(), 0);
	}

	if (roundLR)
	{
		outlinePath.lineTo(getWidth(), getHeight()-radius);
		outlinePath.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);
	} else {
		outlinePath.lineTo(getWidth(), getHeight());
	}

	if (roundLL)
	{
		outlinePath.lineTo(radius, getHeight());
		outlinePath.addArc(0, getHeight()-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);
	} else {
		outlinePath.lineTo(0, getHeight());
	}

	if (roundUL)
	{
		outlinePath.lineTo(0, radius);
		outlinePath.addArc(0, 0, radius*2, radius*2, 1.5*double_Pi, 2.0*double_Pi);
	} 
	
	outlinePath.closeSubPath();

}

void TriangleButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    //  g.fillAll(Colours::orange);
    // g.setColour(Colours::black);
    // g.drawRect(0,0,getWidth(),getHeight(),1.0);

    if (isMouseOver)
    {
        g.setColour(Colours::grey);
    } else {
        g.setColour(Colours::black);
    }

    if (isButtonDown)
    {
        g.setColour(Colours::white);
    }

    int inset = 1;
    int x1, y1, x2, y2, x3;

    x1 = inset;
    x2 = getWidth()/2;
    x3 = getWidth()-inset;

    if (direction == 1)
    {
        y1 = getHeight()-inset;
        y2 = inset;

    } else {
        y1 = inset;
        y2 = getHeight()-inset;
    }

    g.drawLine(x1, y1, x2, y2);
    g.drawLine(x2, y2, x3, y1);
    g.drawLine(x3, y1, x1, y1);


}

