#include "SpikeDisplayEditor.h"
#include <string>



SpikeDisplayEditor::SpikeDisplayEditor (GenericProcessor* parentNode) 
	: VisualizerEditor(parentNode,200)

{
	// Get the number of sub channels from the parentNode
	// Assume all plots have the same number of subChannels
	// Otherwise we'll have to track the number of subChannels
	nSubChannels = 4;

	for (int i=0; i<nSubChannels; i++)
		subChanSelected[i] = true;

	initializeButtons();

}

SpikeDisplayEditor::~SpikeDisplayEditor()
{
	deleteAllChildren();
}

void SpikeDisplayEditor::initializeButtons(){
	int w = 18;
	int h = 18;
	int xPad = 5;
	int yPad = 6;

	int xInitial = 10;
	int yInitial = 25;
	int x = xInitial;
	int y = yInitial;

	panLabel = new Label("PanLabel", "Pan:");	
	panLabel->setBounds(x-xPad, y, w*2 + xPad, h);
	panLabel->setJustificationType(Justification::centredLeft );
	x+= 2*w+3*xPad;

	zoomLabel = new Label("ZoomLabel", "Zoom:");
	zoomLabel->setBounds(x-xPad,y,w*3+xPad, h);
	zoomLabel->setJustificationType(Justification::centredLeft);
	x = xInitial;
	y += h + yPad/2;

	panUpBtn = new ChannelSelectorButton("+", titleFont);
	panUpBtn->setBounds(x, y, w, h); 
	panUpBtn->setClickingTogglesState(false);
	panUpBtn->addListener(this);
	x+= w+xPad;

	panDownBtn = new ChannelSelectorButton("-", titleFont);
	panDownBtn->setBounds(x, y, w, h);
	panDownBtn->setClickingTogglesState(false);
	panDownBtn->addListener(this);
	x+= w+xPad*2;

	zoomInBtn = new ChannelSelectorButton("+", titleFont);
	zoomInBtn->setBounds(x,y,w,h);
	zoomInBtn->setClickingTogglesState(false);
	zoomInBtn->addListener(this);
	x += w + xPad;

	zoomOutBtn = new ChannelSelectorButton("-", titleFont);
	zoomOutBtn->setBounds(x,y,w,h);
	zoomOutBtn->setClickingTogglesState(false);
	zoomOutBtn->addListener(this);
	x += w + xPad*3;


	clearBtn = new ChannelSelectorButton("Clear", titleFont);
	clearBtn->setBounds(x, y, w*2 + xPad, h);
	clearBtn->setClickingTogglesState(false);
	clearBtn->addListener(this);
	x += (w + xPad) *2;
	
	
	
/*
	x = xInitial;
	y += h + yPad;

	//panLabel->setFont(titleFont);

	saveImgBtn = new ChannelSelectorButton("Save", titleFont);
	saveImgBtn->setBounds(x,y,w*2 + xPad, h);
	saveImgBtn->setClickingTogglesState(false);
	saveImgBtn->addListener(this);
	x += (w + xPad) * 2;

	*/

	

	//zoomLabel->setFont(titleFont);
	x = xInitial;
	y += h + yPad;
	// Button *zoomOutBtn = new EditorButton("-");

	subChanLabel = new Label("SubChan", "Sub Channel:");
	subChanLabel->setBounds(x - xPad,y,w*8, h);
	subChanLabel->setJustificationType(Justification::centredLeft);
	y += h + yPad/2;
	//x += w/2;

	allSubChansBtn = new ChannelSelectorButton("All", titleFont);
	allSubChansBtn->setBounds(x,y,w*2+xPad,h);
	allSubChansBtn->addListener(this);
	allSubChansBtn->setToggleState(true, false);
	x += (w+xPad) * 2;
	
	for (int i=0; i<nSubChannels; i++)
	{
		String s = "";
		s += i;

		subChanBtn[i] = new ChannelSelectorButton(s, titleFont);
		subChanBtn[i]->setBounds(x,y,w,h);
		subChanBtn[i]->addListener(this);
		subChanBtn[i]->setToggleState(true, false);
		x += w + xPad;
	}



	addAndMakeVisible(panUpBtn);
	addAndMakeVisible(panDownBtn);
	addAndMakeVisible(panLabel);


	addAndMakeVisible(zoomInBtn);
	addAndMakeVisible(zoomOutBtn);
	addAndMakeVisible(zoomLabel);
	addAndMakeVisible(clearBtn);
	//addAndMakeVisible(saveImgBtn);

	addAndMakeVisible(subChanLabel);
	addAndMakeVisible(allSubChansBtn);
	for (int i=0; i<nSubChannels; i++)
		addAndMakeVisible(subChanBtn[i]);

}

Visualizer* SpikeDisplayEditor::createNewCanvas()
{

	SpikeDisplayNode* processor = (SpikeDisplayNode*) getProcessor();
	return new SpikeDisplayCanvas(processor);

}

void SpikeDisplayEditor::buttonCallback(Button* button)
{
	//std::cout<<"Got event from component:"<<button<<std::endl;

	int pIdx = 0;
	if (button == panUpBtn){
		for (int i=0; i<nSubChannels; i++)
			if (subChanSelected[i])
				canvas->setParameter(SPIKE_CMD_PAN_AXES, pIdx, i, 1);
	}
	else if (button == panDownBtn){
		for (int i=0; i<nSubChannels; i++)
			if (subChanSelected[i])
				canvas->setParameter(SPIKE_CMD_PAN_AXES, pIdx, i, -1);
	}
	else if (button == zoomInBtn){
		for (int i=0; i<nSubChannels; i++)
			if (subChanSelected[i])
				canvas->setParameter(SPIKE_CMD_ZOOM_AXES, pIdx, i, -1);
	}
	else if (button == zoomOutBtn)
	{
		for (int i=0; i<nSubChannels; i++)
			if (subChanSelected[i])
				canvas->setParameter(SPIKE_CMD_ZOOM_AXES, pIdx, i, 1);
	}

	else if (button == clearBtn){
		std::cout<<"Clear!"<<std::endl;
		canvas->setParameter(SPIKE_CMD_CLEAR_ALL, 0);
	}
	else if (button == saveImgBtn)
		std::cout<<"Save!"<<std::endl;
	
	// toggle all sub channel buttons
	else if (button == allSubChansBtn)
	{
		bool b = allSubChansBtn->getToggleState();
		for (int i=0; i<nSubChannels; i++)
			subChanBtn[i]->setToggleState(b, true);

	}
	// Check the sub Channel selection buttons one by one
	else{ 
		// If the user has clicked a sub channel button then the all channels button should be untoggled if toggled
		allSubChansBtn->setToggleState(false, false);
		for (int i=0; i<nSubChannels; i++)
			if(button == subChanBtn[i])
			{
				std::cout<<"SubChannel:"<<i<< " set to:";
				subChanSelected[i] = ((ChannelSelectorButton*) button)->getToggleState();
				std::cout<< subChanSelected[i]<<std::endl;
			}

		// If the user has toggled all of the sub channels on, then set AllChans to on
		bool allChansToggled = true;
		for (int i=0; i<nSubChannels; i++)
		{
			if (subChanBtn[i]->getToggleState()!=allChansToggled){
				allChansToggled = false;
				break;
			}
		}
		allSubChansBtn->setToggleState(allChansToggled, false);

	}
}
