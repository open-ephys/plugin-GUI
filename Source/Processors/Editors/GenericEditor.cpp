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
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	return p->enabledState();
}

void GenericEditor::setEnabledState(bool t)
{
	
	GenericProcessor* p = (GenericProcessor*) getProcessor();
	p->enabledState(t);
	isEnabled = p->enabledState();
}

void GenericEditor::paint (Graphics& g)
{
	int offset = 0;

	GenericProcessor* p = (GenericProcessor*) getProcessor();

	if (isSelected) {
		g.setColour(Colours::yellow);
		g.fillRect(0,0,getWidth()-offset,getHeight());
	}

	if (isEnabled)
		g.setColour(backgroundColor);
	else 
		g.setColour(Colours::lightgrey);

	g.fillRect(1,1,getWidth()-(2+offset),getHeight()-2);

	g.setColour(Colour(192, 205, 209));
	g.fillRect(1,22,getWidth()-2, getHeight()-26);

	g.setFont(titleFont);
	g.setFont(14);

	if (isEnabled) 
	{
		g.setColour(Colours::white);		
	} else {
		g.setColour(Colours::grey);
	}

	g.drawText(name, 6, 5, 500, 15, Justification::left, false);

}
