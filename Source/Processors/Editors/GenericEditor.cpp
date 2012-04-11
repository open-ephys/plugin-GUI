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
#include "../ProcessorGraph.h"
#include "../RecordNode.h"
#include "../../UI/ProcessorList.h"

#include "../../UI/EditorViewport.h"

#include <math.h>

GenericEditor::GenericEditor (GenericProcessor* owner)//, FilterViewport* vp) 
	: AudioProcessorEditor (owner), isSelected(false),
	  desiredWidth(150), tNum(-1), isEnabled(true), radioGroupId(1),
	  accumulator(0.0), isFading(false), drawerButton(0), audioButton(0),
	  recordButton(0), paramsButton(0), allButton(0), noneButton(0),
	  numChannels(-1)

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
    		audioButton = new EditorButton("A", titleFont);
    		audioButton->addListener(this);
    		addChildComponent(audioButton);
    		audioButton->setVisible(false);

    		recordButton = new EditorButton("R", titleFont);
    		recordButton->addListener(this);
    		addChildComponent(recordButton);
    		recordButton->setVisible(false);

    		paramsButton = new EditorButton("P", titleFont);
    		paramsButton->addListener(this);
    		paramsButton->setVisible(false);
    		addChildComponent(paramsButton);
    		
    		paramsButton->setToggleState(true, true);
    	}

	}

	paramsChannels.clear();
	audioChannels.clear();
	recordChannels.clear();

	backgroundGradient = ColourGradient(Colour(190, 190, 190), 0.0f, 0.0f, 
										 Colour(185, 185, 185), 0.0f, 120.0f, false);

	//backgroundGradient.addColour(0.05f, Colour(255, 255, 255));
	backgroundGradient.addColour(0.2f, Colour(155, 155, 155));
	//grad.addColour(0.5, Colours::lightgrey);
	//grad.addColour(1.0f, Colours::grey);

	int maxX = 20;
	int maxY = 30;

	for (int i = 0; i < owner->getNumParameters(); i++)
	{
		ParameterEditor* p = new ParameterEditor(owner->getParameterReference(i), titleFont);												
		
		int dWidth = p->desiredWidth;
		int dHeight = p->desiredHeight;

		p->setBounds(maxX, maxY, dWidth, dHeight);
		addAndMakeVisible(p);

		maxY += dHeight;
		maxY += 10;

	}

	backgroundColor = Colour(10,10,10);

	//refreshColors();

	fadeIn();
}

GenericEditor::~GenericEditor()
{
	//std::cout << "  Generic editor for " << getName() << " being deleted with " << getNumChildComponents() << " children. " << std::endl;
	deleteAllChildren();
	//delete titleFont;
}

void GenericEditor::refreshColors()
{

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

}


void GenericEditor::resized()
{
	if (drawerButton != 0)
		drawerButton->setBounds(getWidth()-14, 40, 10, getHeight()-60);
	
	if (audioButton != 0)
		audioButton->setBounds(getWidth()-60, 4, 15, 15);
	
	if (recordButton != 0)
		recordButton->setBounds(getWidth()-40, 4, 15, 15);
	
	if (paramsButton != 0)
		paramsButton->setBounds(getWidth()-20, 4, 15, 15);

}


bool GenericEditor::keyPressed (const KeyPress& key)
{
	//std::cout << name << " received " << key.getKeyCode() << std::endl;

	if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey) {
		
		//std::cout << name << " should be deleted." << std::endl;
		if (getSelectionState())
			getEditorViewport()->deleteNode(this);

	} else if (key.getKeyCode() == key.leftKey || key.getKeyCode() == key.rightKey) {

		if (getSelectionState())
			getEditorViewport()->moveSelection(key);

	}
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
	setWantsKeyboardFocus(true);
	grabKeyboardFocus();
}

bool GenericEditor::getSelectionState() {
	return isSelected;
}

void GenericEditor::deselect()
{
	isSelected = false;
	repaint();
	setWantsKeyboardFocus(false);
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
	//g.setColour(Colour(140, 140, 140));
	

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
		g.setColour(Colours::yellow);
		
	} else {
		g.setColour(Colours::black);
	}

	// draw highlight box
	g.drawRect(0,0,getWidth(),getHeight(),2.0);

	// if (isFading)
	// {
	// 	g.setColour(Colours::black.withAlpha((float) (10.0-accumulator)/10.0f));
	// 	if (getWidth() > 0 && getHeight() > 0)
	// 		g.fillAll();
	// }

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
	
	checkDrawerButton(button);
	checkChannelSelectors(button);

	buttonEvent(button); // needed to inform subclasses of 
						 // button event
}

bool GenericEditor::checkDrawerButton(Button* button)
{
	if (button == drawerButton)
	{
		if (drawerButton->getToggleState()) 
		{
			

			if (recordButton != 0)
				recordButton->setVisible(true);
			if (audioButton != 0)
				audioButton->setVisible(true);
			if (paramsButton != 0)
				paramsButton->setVisible(true);

			drawerWidth = createChannelSelectors();

			desiredWidth += drawerWidth;

		} else {
			

			if (recordButton != 0)
				recordButton->setVisible(false);
			if (audioButton != 0)
				audioButton->setVisible(false);
			if (paramsButton != 0)
				paramsButton->setVisible(false);

			removeChannelSelectors();

			desiredWidth -= drawerWidth;
		}

		getEditorViewport()->makeEditorVisible(this);

		return true;
	} else {
		return false;
	}

}

bool GenericEditor::checkChannelSelectors(Button* button)
{



	for (int n = 0; n < channelSelectorButtons.size(); n++)
	{
		if (button == channelSelectorButtons[n])
		{

		//	String type;

			if (audioButton->getToggleState())
			{
				audioChannels.set(n,button->getToggleState());
				//type = "Audio ";
			}
			else if (recordButton->getToggleState())
			{
				recordChannels.set(n,button->getToggleState());
				int id = getProcessor()->getNodeId();

				RecordNode* rn = getProcessorGraph()->getRecordNode();

				std::cout << "Button " << n << " was pressed." << std::endl;
				rn->setChannel(id, n);

				if (button->getToggleState())
					rn->setParameter(2, 1.0f);
				else
					rn->setParameter(2, 0.0f);
			}		
			else if (paramsButton->getToggleState())
			{
				paramsChannels.set(n,button->getToggleState());
				//type = "Params ";	
			}
			
			//std::cout << type << "button " << n << " clicked." << std::endl;
			return true;
			
		}
	}

	Array<bool> arr;

	if (button == audioButton)
		{arr = audioChannels; allButton->setVisible(false);}// std::cout << "AUDIO" << std::endl;}
	else if (button == paramsButton)
		{arr = paramsChannels; if (allButton != 0) {allButton->setVisible(true);}}// std::cout << "PARAMS" << std::endl;}
	else if (button == recordButton)
		{arr = recordChannels; allButton->setVisible(true);}// std::cout << "RECORD" << std::endl;}

	if (arr.size() > 0)
	{
		for (int n = 0; n < channelSelectorButtons.size(); n++)
		{
			channelSelectorButtons[n]->setToggleState(arr[n],false);
		}

		allButton->setToggleState(false,false);

		return true;
	}

	if (button == noneButton)
	{
		for (int n = 0; n < channelSelectorButtons.size(); n++) 
		{

			channelSelectorButtons[n]->setToggleState(false,false);

			if (audioButton->getToggleState())
				audioChannels.set(n,false);
			else if (recordButton->getToggleState())
				recordChannels.set(n,false);			
			else if (paramsButton->getToggleState())
				paramsChannels.set(n,false);	

		}

		//allButton->setToggleState(false,false);

		return true;
	}

	if (button == allButton)
	{

		for (int n = 0; n < channelSelectorButtons.size(); n++) 
		{

			channelSelectorButtons[n]->setToggleState(true,false);

			if (audioButton->getToggleState())
				audioChannels.set(n,true);
			else if (recordButton->getToggleState())
				recordChannels.set(n,true);			
			else if (paramsButton->getToggleState())
				paramsChannels.set(n,true);	

		}

		return true;
	}

	return false;

}

void GenericEditor::selectChannels(Array<int> arr)
{
	for (int i = 0; i < channelSelectorButtons.size(); i++)
	{
		channelSelectorButtons[i]->setToggleState(false, false);
	}

	for (int i = 0; i < arr.size(); i++)
	{
		if (i > -1 && i < channelSelectorButtons.size())
		{
			channelSelectorButtons[i]->setToggleState(true,false);
		}
	}

}

void GenericEditor::update()
{

	std::cout << "Editor for ";

	GenericProcessor* p = (GenericProcessor*) getProcessor();

	std::cout << p->getName() << " updating settings." << std::endl;

	if (!p->isSink())
	{

		if (p->getNumOutputs() != numChannels)
		{
			destroyChannelSelectors();
		}

		numChannels = p->getNumOutputs();

	} else {

		if (p->getNumInputs() != numChannels)
		{
			destroyChannelSelectors();
		}

		numChannels = p->getNumInputs();
	}

	if (numChannels == 0)
	{
		if (drawerButton != 0)
			drawerButton->setVisible(false);
	} else {
		if (drawerButton != 0)
			drawerButton->setVisible(true);
	}

	updateVisualizer(); // does nothing unless this method
						// has been implemented

}

Array<int> GenericEditor::getActiveChannels()
{
	Array<int> chans;

	for (int n = 0; n < paramsChannels.size(); n++)
	{
		if (paramsChannels[n])
		{
			chans.add(n);
		}

	}

	return chans;
}

void GenericEditor::createRadioButtons(int x, int y, int w, StringArray values, const String& groupName)
{
	int numButtons = values.size();
	int width = w / numButtons;

	for (int i = 0; i < numButtons; i++)
	{

		RadioButton* b = new RadioButton(values[i], radioGroupId, titleFont);
		addAndMakeVisible(b);
		b->setBounds(x+width*i,y,width,15);
		b->addListener(this);
		

		// if (i == numButtons-1)
		// {
		// 	b->setToggleState(true, true);
		// }
	}

	Label* l = new Label("Label",groupName);
	addAndMakeVisible(l);
	l->setBounds(x,y-15,200,10);
	titleFont.setHeight(10);
	l->setFont(titleFont);

	radioGroupId++;
}


int GenericEditor::createChannelSelectors()
{

	GenericProcessor* p = getProcessor();

	if (channelSelectorButtons.size() == 0) {

		int width = 20;
		int height = 14;
		int numChannels;

		if (!p->isSink())
			numChannels = p->getNumOutputs();
		else
			numChannels = p->getNumInputs();

		int nColumns = jmax((int) ceil(numChannels/4),1);
		//std::cout << numChannels << " channels" << std::endl;
		//std::cout << nColumns << " columns" << std::endl;

		for (int n = 1; n < numChannels+1; n++)
		{
			String channelName = "";
			channelName += n;
			ChannelSelectorButton* b = new ChannelSelectorButton(channelName, titleFont);
			channelSelectorButtons.add(b);
			addAndMakeVisible(b);
			b->addListener(this);
			b->setBounds(desiredWidth+width*((n-1)%nColumns)-12,
			  floor((n-1)/nColumns)*height+40, width-1, height-1);
			  
			audioChannels.add(false);
			recordChannels.add(false);
			paramsChannels.add(false);

		}

		if (allButton == 0)
		{
			allButton = new ChannelSelectorButton("+",titleFont);
			addAndMakeVisible(allButton);
			allButton->addListener(this);
			allButton->setVisible(true);
			allButton->setClickingTogglesState(false);
			allButton->setBounds(desiredWidth-30,
				  40, height, height);
		}

		if (noneButton == 0)
		{
			noneButton = new ChannelSelectorButton("-",titleFont);
			addAndMakeVisible(noneButton);
			noneButton->addListener(this);
			noneButton->setVisible(true);
			noneButton->setClickingTogglesState(false);
			noneButton->setBounds(desiredWidth-30,
				  60, height, height);
		}

		 return nColumns*width+ 15;

	} else {

		for (int n = 0; n < channelSelectorButtons.size(); n++)
		{
			channelSelectorButtons[n]->setVisible(true);

			if (!p->isSink()) {

				if (audioButton->getToggleState())
					channelSelectorButtons[n]->setToggleState(audioChannels[n],false);
				else if (recordButton->getToggleState())
					channelSelectorButtons[n]->setToggleState(recordChannels[n],false);
				else if (paramsButton->getToggleState())
					channelSelectorButtons[n]->setToggleState(paramsChannels[n],false);
				
			}

		}

		allButton->setVisible(true);
		noneButton->setVisible(true);

		return drawerWidth;
	}

}

void GenericEditor::removeChannelSelectors()
{
	for (int n = 0; n < channelSelectorButtons.size(); n++)
	{
		channelSelectorButtons[n]->setVisible(false);
	}

	allButton->setVisible(false);
	noneButton->setVisible(false);

}

void GenericEditor::destroyChannelSelectors()
{
	for (int n = 0; n < channelSelectorButtons.size(); n++)
	{
		removeChildComponent(channelSelectorButtons[n]);
		ChannelSelectorButton* t = channelSelectorButtons.remove(n);
     	deleteAndZero(t);
	}

	if (allButton != 0)
		allButton->setVisible(false);
	
	if (allButton != 0)
		noneButton->setVisible(false);

	recordChannels.clear();
	audioChannels.clear();
	paramsChannels.clear();
	channelSelectorButtons.clear();
}

RadioButton::RadioButton(const String& name, int groupId, Font f) : Button(name) 
{

    setRadioGroupId(groupId);
    setClickingTogglesState(true);

    buttonFont = f;
    buttonFont.setHeight(10);

    // MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    // Typeface::Ptr typeface = new CustomTypeface(mis);
    // buttonFont = Font(typeface);
    // 
}


void RadioButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else 
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    g.fillRect(0,0,getWidth(),getHeight());

    g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    g.drawText(getName(),0,0,getWidth(),getHeight(),Justification::centred,true);
}


DrawerButton::DrawerButton(const String& name) : Button(name)
{
	 setClickingTogglesState(true);
}

void DrawerButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
	if (isMouseOver)
		g.setColour(Colours::white);
	else
		g.setColour(Colours::darkgrey);
	
	g.drawVerticalLine(3, 0.0f, getHeight());
	g.drawVerticalLine(5, 0.0f, getHeight());
	g.drawVerticalLine(7, 0.0f, getHeight());	

}


EditorButton::EditorButton(const String& name, Font f) : Button(name) 
{

	buttonFont = f;
	buttonFont.setHeight(10);

    setRadioGroupId(999);
    setClickingTogglesState(true);

}


void EditorButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else 
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    int b = 2;

    g.fillEllipse(b,b,getWidth()-2*b,getHeight()-2*b);

    g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawEllipse(b,b,getWidth()-2*b,getHeight()-2*b,1.0);

    g.drawText(getName(),0,0,getWidth(),getHeight(),Justification::centred,true);
}


ChannelSelectorButton::ChannelSelectorButton(const String& name, Font f) : Button(name) 
{
    setClickingTogglesState(true);

    buttonFont = f;
    buttonFont.setHeight(10);
}


void ChannelSelectorButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else 
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    g.fillRect(0,0,getWidth(),getHeight());

    g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    g.drawText(getName(),0,0,getWidth(),getHeight(),Justification::centred,true);
}

//// BUTTONS ////

void PlusButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.fillAll(Colours::orange);
    g.setColour(Colours::black);
    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    if (isMouseOver)
    {
        g.setColour(Colours::white);
    } else {
        g.setColour(Colours::black);
    }

    // if (isButtonDown)
    // {
    //     g.setColour(Colours::white);
    // }

    int thickness = 1;
    int offset = 3;

    g.fillRect(getWidth()/2-thickness,
               offset, 
               thickness*2,
               getHeight()-offset*2);

    g.fillRect(offset,
               getHeight()/2-thickness,
               getWidth()-offset*2,
               thickness*2);
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

