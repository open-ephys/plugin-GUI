/*
  ==============================================================================

    GenericEditor.cpp
    Created: 7 Jun 2011 11:37:12am
    Author:  jsiegle

  ==============================================================================
*/

#include "GenericEditor.h"

GenericEditor::GenericEditor (GenericProcessor* owner, FilterViewport* vp) 
	: AudioProcessorEditor (owner), isSelected(false), viewport(vp),
	  desiredWidth(150), tNum(-1), isEnabled(true)

{
	name = getAudioProcessor()->getName();

	nodeId = owner->getNodeId();

	Random r = Random(99);
	r.setSeedRandomly();

	//titleFont = new Font(14.0, Font::plain);

	//titleFont->setTypefaceName(T("Miso"));

	backgroundColor = Colour(3, 143, 255);

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

//void GenericEditor::setTabbedComponent(TabbedComponent* tc) {
	
//	tabComponent = tc;

//}

bool GenericEditor::keyPressed (const KeyPress& key)
{
	//std::cout << name << " received " << key.getKeyCode() << std::endl;

	if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey) {
		
		//std::cout << name << " should be deleted." << std::endl;
		viewport->deleteNode(this);

	} else if (key.getKeyCode() == key.leftKey || key.getKeyCode() == key.rightKey) {

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
	return isEnabled;
}

void GenericEditor::setEnabledState(bool t)
{
	isEnabled = t;
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	p->enabledState(isEnabled);
}

void GenericEditor::paint (Graphics& g)
{
	int offset = 0;

	GenericProcessor* p = (GenericProcessor*) getProcessor();

	g.setColour(Colour(127,137,147));
	g.fillAll();

	if (isSelected)
		g.setColour(Colours::yellow);
	else
		g.setColour(Colours::darkgrey);
	
	
	g.fillRoundedRectangle(0,0,getWidth()-offset,getHeight(),7.0);
	

	if (isEnabled)
		g.setColour(backgroundColor);
	else 
		g.setColour(Colours::lightgrey);

	// if (p->isSource()) {
	// 	g.setColour(Colours::red);
	// } else if (p->isSink()) {
	// 	g.setColour(Colours::blue);
	// } else if (p->isSplitter() || p->isMerger())
	// 	{
	// 	g.setColour(Colours::darkgrey);
	// } else {
	// 	g.setColour(Colours::red);
	// }

	g.fillRoundedRectangle(1,1,getWidth()-(2+offset),getHeight()-2,6.0);

	g.setColour(Colours::grey);
	g.fillRoundedRectangle(4,15,getWidth()-(8+offset), getHeight()-19,5.0);
	g.fillRect(4,15,getWidth()-(8+offset), 20);

	Font titleFont = Font(14.0, Font::plain);

	//titleFont.setTypefaceName(T("Miso"));

	g.setFont(titleFont);

	if (isEnabled) 
	{
		g.setColour(Colours::black);		
	} else {
		g.setColour(Colours::grey);
	}

	g.drawText(name, 8, 4, 100, 7, Justification::left, false);

}
