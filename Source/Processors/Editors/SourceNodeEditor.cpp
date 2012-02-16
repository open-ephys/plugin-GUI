/*
  ==============================================================================

    SourceNodeEditor.cpp
    Created: 7 Sep 2011 5:08:31pm
    Author:  jsiegle

  ==============================================================================
*/


#include "SourceNodeEditor.h"
#include "../SourceNode.h"
#include <stdio.h>


SourceNodeEditor::SourceNodeEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp)

{
	desiredWidth = 170;

	Image im;

	std::cout << "I think my name is: " << getName() << std::endl;

	if (getName().equalsIgnoreCase("Intan Demo Board"))
	 {
	 	im = ImageCache::getFromMemory (BinaryData::IntanIcon_png, 
	 								    BinaryData::IntanIcon_pngSize);
	 } else if (getName().equalsIgnoreCase("File Reader")) {
	 	im = ImageCache::getFromMemory (BinaryData::FileReaderIcon_png, 
	 								    BinaryData::FileReaderIcon_pngSize);	
	} else {
	 	im = ImageCache::getFromMemory (BinaryData::DefaultDataSource_png, 
	 								    BinaryData::DefaultDataSource_pngSize);
	 }

	 icon = new ImageIcon(im);
	 addAndMakeVisible(icon);
	 icon->setBounds(50,40,70,70);

	//Array<int> values;
	//values.add(1); values.add(2), values.add(3);

	//createRadioButtons(10, 25, 100, values);

}

SourceNodeEditor::~SourceNodeEditor()
{
	deleteAllChildren();
}
