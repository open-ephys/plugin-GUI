/*
  ==============================================================================

    Splitter.cpp
    Created: 17 Aug 2011 1:02:57am
    Author:  jsiegle

  ==============================================================================
*/


#include "Splitter.h"
#include "../Editors/SplitterEditor.h"

Splitter::Splitter()
	: GenericProcessor("Splitter"), 
		destNodeA(0), destNodeB(0), activePath(0)
	{
		
	}

Splitter::~Splitter()
{
	
}

AudioProcessorEditor* Splitter::createEditor()
{
	SplitterEditor* editor = new SplitterEditor(this, viewport);
	setEditor(editor);
	
	std::cout << "Creating editor." << std::endl;
	return editor;
}

void Splitter::setDestNode(GenericProcessor* dn)
{

	destNode = dn;

	if (activePath == 0) {
		std::cout << "Setting destination node A." << std::endl;
		destNodeA = dn;
	} else {
		destNodeB = dn;
		std::cout << "Setting destination node B." << std::endl;

	}
}

void Splitter::switchDest(int destNum) {
	
	activePath = destNum;

	if (destNum == 0) 
	{
		setDestNode(destNodeA);
	} else 
	{
		setDestNode(destNodeB);
	}

	//viewport->setActiveEditor((GenericEditor*) getEditor());
	//viewport->updateVisibleEditors();
}