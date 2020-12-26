/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "LfpDisplayEditor.h"

using namespace LfpViewer;


LfpDisplayEditor::LfpDisplayEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
, hasNoInputs(true)
{
    lfpProcessor = (LfpDisplayNode*) parentNode;
    tabText = "LFP";

    desiredWidth = 180;

	defaultSubprocessor = 0;
}

LfpDisplayEditor::~LfpDisplayEditor()
{
}

void LfpDisplayEditor::startAcquisition()
{
	//subprocessorSelection->setEnabled(false);
}

void LfpDisplayEditor::stopAcquisition()
{
	//subprocessorSelection->setEnabled(true);
}

Visualizer* LfpDisplayEditor::createNewCanvas()
{
    canvas = new LfpDisplayCanvas(lfpProcessor);
    return canvas;
}

// not really being used (yet)...
void LfpDisplayEditor::buttonEvent(Button* button)
{


}


void LfpDisplayEditor::saveVisualizerParameters(XmlElement* xml)
{

	// xml->setAttribute("Type", "LfpDisplayEditor");

	// int subprocessorItemId = subprocessorSelection->getSelectedId();

	// XmlElement* values = xml->createNewChildElement("VALUES");
	// values->setAttribute("SubprocessorId", subprocessorItemId - 1);
}

void LfpDisplayEditor::loadVisualizerParameters(XmlElement* xml)
{

	// forEachXmlChildElement(*xml, xmlNode)
	// {
	// 	if (xmlNode->hasTagName("VALUES"))
	// 	{
	// 		std::cout << "LfpDisplay found " << xmlNode->getIntAttribute("SubprocessorId") << std::endl;
	// 		defaultSubprocessor = xmlNode->getIntAttribute("SubprocessorId");
	// 		subprocessorSelection->setSelectedItemIndex(defaultSubprocessor, sendNotification);

	// 	}
	// }

}