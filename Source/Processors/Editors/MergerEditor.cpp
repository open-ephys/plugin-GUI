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

#include "MergerEditor.h"
#include "../Utilities/Merger.h"

// PipelineSelectorButton::PipelineSelectorButton()
// 	: DrawableButton ("Selector", DrawableButton::ImageFitted)
// {
// 	DrawablePath normal, over, down;

// 	    Path p;
//         p.addTriangle (0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
//         normal.setPath (p);
//         normal.setFill (Colours::lightgrey);
//         normal.setStrokeThickness (0.0f);

//         over.setPath (p);
//         over.setFill (Colours::black);
//         over.setStrokeFill (Colours::black);
//         over.setStrokeThickness (5.0f);

//         setImages (&normal, &over, &over);
//         setBackgroundColours(Colours::darkgrey, Colours::purple);
//         setClickingTogglesState (true);
//         setTooltip ("Toggle a state.");

// }

// PipelineSelectorButton::~PipelineSelectorButton()
// {
// }

MergerEditor::MergerEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode)

{
	desiredWidth = 90;

	pipelineSelectorA = new ImageButton("Pipeline A");

	Image normalImageA = ImageCache::getFromMemory (BinaryData::MergerB01_png, BinaryData::MergerB01_pngSize);
	Image downImageA = ImageCache::getFromMemory (BinaryData::MergerA01_png, BinaryData::MergerA01_pngSize);
	Image normalImageB = ImageCache::getFromMemory (BinaryData::MergerA02_png, BinaryData::MergerA02_pngSize);
	Image downImageB = ImageCache::getFromMemory (BinaryData::MergerB02_png, BinaryData::MergerB02_pngSize);

	pipelineSelectorA->setImages(true, true, true,
								normalImageA, 1.0f, Colours::white.withAlpha(0.0f),
								normalImageA, 1.0f, Colours::black.withAlpha(0.0f),
								downImageA, 1.0f, Colours::white.withAlpha(0.0f));


	pipelineSelectorA->addListener(this);
	pipelineSelectorA->setBounds(-10,25,95,50);
	pipelineSelectorA->setToggleState(true,false);
	addAndMakeVisible(pipelineSelectorA);

	pipelineSelectorB = new ImageButton("Pipeline B");

	pipelineSelectorB->setImages(true, true, true,
								normalImageB, 1.0f, Colours::white.withAlpha(0.0f),
								normalImageB, 1.0f, Colours::black.withAlpha(0.0f),
								downImageB, 1.0f, Colours::white.withAlpha(0.0f));

	pipelineSelectorB->addListener(this);
	pipelineSelectorB->setBounds(-10,75,95,50);
	pipelineSelectorB->setToggleState(false,false);
	addAndMakeVisible(pipelineSelectorB);

}

MergerEditor::~MergerEditor()
{
	deleteAllChildren();
}

void MergerEditor::buttonEvent(Button* button)
{
	if (button == pipelineSelectorA)
	{
		pipelineSelectorA->setToggleState(true,false);
		pipelineSelectorB->setToggleState(false,false);
		Merger* processor = (Merger*) getProcessor();
		processor->switchIO(0);

	} else if (button == pipelineSelectorB) 
	{
		pipelineSelectorB->setToggleState(true,false);
		pipelineSelectorA->setToggleState(false,false);
		Merger* processor = (Merger*) getProcessor();
		processor->switchIO(1);
		
	}
}

void MergerEditor::switchSource(int source)
{
	if (source == 0)
	{
		pipelineSelectorA->setToggleState(true,false);
		pipelineSelectorB->setToggleState(false,false);
		Merger* processor = (Merger*) getProcessor();
		processor->switchIO(0);

	} else if (source == 1)
	{
		pipelineSelectorB->setToggleState(true,false);
		pipelineSelectorA->setToggleState(false,false);
		Merger* processor = (Merger*) getProcessor();
		processor->switchIO(1);
		
	}
}


void MergerEditor::switchIO(int source)
{
	switchSource(source);

	select();
}

void MergerEditor::switchSource()
{
	
	bool isBOn = pipelineSelectorB->getToggleState();
	bool isAOn = pipelineSelectorA->getToggleState();

	pipelineSelectorB->setToggleState(!isBOn,false);
	pipelineSelectorA->setToggleState(!isAOn,false);

	Merger* processor = (Merger*) getProcessor();
	processor->switchIO();

}
