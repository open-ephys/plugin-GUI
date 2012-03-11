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

#include "../../UI/EditorViewport.h"

Merger::Merger()
	: GenericProcessor("Merger"), 
		sourceNodeA(0), sourceNodeB(0), activePath(0)//, tabA(-1), tabB(-1)
{
	
}

Merger::~Merger()
{
	
}

AudioProcessorEditor* Merger::createEditor()
{
	editor = new MergerEditor(this);
	//tEditor(editor);
	
	//std::cout << "Creating editor." << std::endl;
	return editor;
}

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

void Merger::switchIO(int sourceNum) {

	//std::cout << "Switching to source number " << sourceNum << std::endl;
	
	activePath = sourceNum;

	if (sourceNum == 0) 
	{
		sourceNode = sourceNodeA;
		//std::cout << "Source node: " << getSourceNode() << std::endl;
	} else 
	{
		sourceNode = sourceNodeB;
		//std::cout << "Source node: " << getSourceNode() << std::endl;
	}

	getEditorViewport()->makeEditorVisible((GenericEditor*) getEditor());

}

bool Merger::stillHasSource()
{
	if (sourceNodeA == 0 || sourceNodeB == 0)
	{
		return false;
	} else {
		return true;
	}

}

void Merger::switchIO()
{

	std::cout << "Merger switching source." << std::endl;

	if (activePath == 0) {
		activePath = 1;
		sourceNode = sourceNodeB;
	}
	else {
	    activePath = 0;
	    sourceNode = sourceNodeA;
	}

}

void Merger::addSettingsFromSourceNode(GenericProcessor* sn)
{

	settings.numInputs += sn->getNumOutputs();
	settings.inputChannelNames.addArray(sn->settings.inputChannelNames);
	settings.eventChannelIds.addArray(sn->settings.eventChannelIds);
	settings.eventChannelNames.addArray(sn->settings.eventChannelNames);
	settings.bitVolts.addArray(sn->settings.bitVolts);

	settings.originalSource = sn->settings.originalSource;
	settings.sampleRate = sn->settings.sampleRate;

	settings.numOutputs = settings.numInputs;
	settings.outputChannelNames = settings.inputChannelNames;

}

void Merger::updateSettings()
{

	// default is to get everything from sourceNodeA,
	// but this might not be ideal
	clearSettings();

	if (sourceNodeA != 0)
	{
		std::cout << "   Merger source A found." << std::endl;
		addSettingsFromSourceNode(sourceNodeA);
	}

	if (sourceNodeB != 0)
	{
		std::cout << "   Merger source B found." << std::endl;
		addSettingsFromSourceNode(sourceNodeB);
	}	

	if (sourceNodeA == 0 && sourceNodeB == 0) {

		settings.sampleRate = getDefaultSampleRate();
		settings.numOutputs = getDefaultNumOutputs();

		for (int i = 0; i < getNumOutputs(); i++)
			settings.bitVolts.add(getDefaultBitVolts());

		generateDefaultChannelNames(settings.outputChannelNames);
	}

	std::cout << "Number of merger outputs: " << getNumInputs() << std::endl;

}

// void Merger::setNumOutputs(int /*outputs*/)
// {
// 	numOutputs = 0;

// 	if (sourceNodeA != 0)
// 	{
// 		std::cout << "   Merger source A found." << std::endl;
// 		numOutputs += sourceNodeA->getNumOutputs();
// 	}
// 	if (sourceNodeB != 0)
// 	{
// 		std::cout << "   Merger source B found." << std::endl;
// 		numOutputs += sourceNodeB->getNumOutputs();
// 	}

// 	std::cout << "Number of merger outputs: " << getNumOutputs() << std::endl;

// }

// void Merger::tabNumber(int t)
// {
// 	if (tabA == -1)
// 		tabA = t;
// 	else
// 		tabB = t;

// }