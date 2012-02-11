/*
  ==============================================================================

    SplitterEditor.cpp
    Created: 5 Sep 2011 2:18:10pm
    Author:  jsiegle

  ==============================================================================
*/

#include "SplitterEditor.h"
#include "../Utilities/Splitter.h"

// PipelineSelectorButton::PipelineSelectorButton()
// 	: DrawableButton (T("Selector"), DrawableButton::ImageFitted)
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

SplitterEditor::SplitterEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp)

{
	desiredWidth = 90;

	pipelineSelectorA = new ImageButton("Pipeline A");

	Image normalImageA = ImageCache::getFromMemory (BinaryData::PipelineB01_png, BinaryData::PipelineB01_pngSize);
	Image downImageA = ImageCache::getFromMemory (BinaryData::PipelineA01_png, BinaryData::PipelineA01_pngSize);
	Image normalImageB = ImageCache::getFromMemory (BinaryData::PipelineA02_png, BinaryData::PipelineA02_pngSize);
	Image downImageB = ImageCache::getFromMemory (BinaryData::PipelineB02_png, BinaryData::PipelineB02_pngSize);

	pipelineSelectorA->setImages(true, true, true,
								normalImageA, 1.0f, Colours::white.withAlpha(0.0f),
								normalImageA, 1.0f, Colours::black.withAlpha(0.0f),
								downImageA, 1.0f, Colours::white.withAlpha(0.0f));


	pipelineSelectorA->addListener(this);
	pipelineSelectorA->setBounds(-10,15,95,50);
	pipelineSelectorA->setToggleState(true,false);
	addAndMakeVisible(pipelineSelectorA);

	pipelineSelectorB = new ImageButton("Pipeline B");

	pipelineSelectorB->setImages(true, true, true,
								normalImageB, 1.0f, Colours::white.withAlpha(0.0f),
								normalImageB, 1.0f, Colours::black.withAlpha(0.0f),
								downImageB, 1.0f, Colours::white.withAlpha(0.0f));

	pipelineSelectorB->addListener(this);
	pipelineSelectorB->setBounds(-10,65,95,50);
	pipelineSelectorB->setToggleState(false,false);
	addAndMakeVisible(pipelineSelectorB);

}

SplitterEditor::~SplitterEditor()
{
	deleteAllChildren();
}

void SplitterEditor::buttonClicked(Button* button)
{
	if (button == pipelineSelectorA)
	{
		pipelineSelectorA->setToggleState(true,false);
		pipelineSelectorB->setToggleState(false,false);
		Splitter* processor = (Splitter*) getProcessor();
		processor->switchDest(0);

	} else if (button == pipelineSelectorB) 
	{
		pipelineSelectorB->setToggleState(true,false);
		pipelineSelectorA->setToggleState(false,false);
		Splitter* processor = (Splitter*) getProcessor();
		processor->switchDest(1);
		
	}
}