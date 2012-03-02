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

#include "Merger.h"
#include "../Editors/MergerEditor.h"

Merger::Merger()
	: GenericProcessor("Merger"), 
		sourceNodeA(0), sourceNodeB(0), activePath(0)
{
	setNumOutputs(0);
	setNumInputs(0);

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);
}

Merger::~Merger()
{
	
}

// AudioProcessorEditor* Merger::createEditor()
// {
// 	MergerEditor* editor = new MergerEditor(this);
// 	setEditor(editor);
	
// 	std::cout << "Creating editor." << std::endl;
// 	return editor;
// }

void Merger::setMergerSourceNode(GenericProcessor* sn)
{

	sourceNode = sn;

	if (activePath == 0) {
		std::cout << "Setting source node A." << std::endl;
		sourceNodeA = sn;
	} else {
		sourceNodeB = sn;
		std::cout << "Setting source node B." << std::endl;

	}
}

void Merger::switchSource(int sourceNum) {
	
	activePath = sourceNum;

	if (sourceNum == 0) 
	{
		setDestNode(sourceNodeA);
	} else 
	{
		setDestNode(sourceNodeB);
	}

	//viewport->setActiveEditor((GenericEditor*) getEditor());
	//viewport->updateVisibleEditors();
}