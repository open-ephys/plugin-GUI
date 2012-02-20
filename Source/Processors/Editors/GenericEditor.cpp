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

GenericEditor::GenericEditor (GenericProcessor* owner, FilterViewport* vp) 
	: AudioProcessorEditor (owner), isSelected(false), viewport(vp),
	  desiredWidth(150), tNum(-1), isEnabled(true), radioGroupId(1),
	  accumulator(0.0), isFading(false)

{
	name = getAudioProcessor()->getName();

	nodeId = owner->getNodeId();

	Random r = Random(99);
	r.setSeedRandomly();

	MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    titleFont = Font(typeface);

	//titleFont = new Font(14.0, Font::plain);

	//titleFont->setTypefaceName(T("Miso"));

	if (owner->isSource())
		backgroundColor = Colour(255, 0, 0);//Colour(int(0.9*255.0f),int(0.019*255.0f),int(0.16*255.0f));
	else if (owner->isSink())
		backgroundColor = Colour(255, 149, 0);//Colour(int(0.06*255.0f),int(0.46*255.0f),int(0.9*255.0f));
	else if (owner->isSplitter() || owner->isMerger())
		backgroundColor = Colour(80, 80, 80);//Colour(int(0.7*255.0f),int(0.7*255.0f),int(0.7*255.0f));
	else
		backgroundColor = Colour(255, 89, 0);//Colour(int(1.0*255.0f),int(0.5*255.0f),int(0.0*255.0f));


	fadeIn();
}

GenericEditor::~GenericEditor()
{
	//std::cout << "  Generic editor for " << getName() << " being deleted with " << getNumChildComponents() << " children. " << std::endl;
	deleteAllChildren();
	//delete titleFont;
}

void GenericEditor::setViewport(FilterViewport* vp) {
	
	viewport = vp;

}


bool GenericEditor::keyPressed (const KeyPress& key)
{
	//std::cout << name << " received " << key.getKeyCode() << std::endl;

	if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey) {
		
		//std::cout << name << " should be deleted." << std::endl;
		if (getSelectionState())
			viewport->deleteNode(this);

	} else if (key.getKeyCode() == key.leftKey || key.getKeyCode() == key.rightKey) {

		if (getSelectionState())
			viewport->moveSelection(key);

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
	g.setColour(Colour(140, 140, 140));
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

	if (isFading)
	{
		g.setColour(Colours::black.withAlpha((float) (15.0-accumulator)/15.0f));
		g.fillAll();
	}

}

void GenericEditor::timerCallback()
{
	accumulator++;

	repaint();

	if (accumulator > 15.0)
	{
		stopTimer();
		isFading = false;
	}
}

void GenericEditor::createRadioButtons(int x, int y, int w, StringArray values, const String& groupName)
{
	int numButtons = values.size();
	int width = w / numButtons;

	for (int i = 0; i < numButtons; i++)
	{

		RadioButton* b = new RadioButton(values[i], radioGroupId);
		addAndMakeVisible(b);
		b->setBounds(x+width*i,y,width,15);
	//	b->addListener(this);
		

		// if (i == numButtons-1)
		// {
		// 	b->setToggleState(true, true);
		// }
	}

	Label* l = new Label("Label",groupName);
	addChildComponent(l);
	l->setBounds(x,y-15,200,10);
	titleFont.setHeight(10);
	l->setFont(titleFont);

	radioGroupId++;
}


RadioButton::RadioButton(const String& name, int groupId) : Button(name) 
{

    setRadioGroupId(groupId);
    setClickingTogglesState(true);

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    buttonFont = Font(typeface);
    buttonFont.setHeight(10);
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


