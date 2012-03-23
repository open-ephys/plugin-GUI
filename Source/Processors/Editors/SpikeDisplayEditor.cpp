#include "SpikeDisplayEditor.h"



SpikeDisplayEditor::SpikeDisplayEditor (GenericProcessor* parentNode) 
	: VisualizerEditor(parentNode)

{

	desiredWidth = 250;
	int w = 18;
	int h = 18;
	int xPad = 5;
	int yPad = 6;

	int xInitial = 10;
	int yInitial = 30;
	int x = xInitial;
	int y = yInitial;

	panUpBtn = new ChannelSelectorButton("+", titleFont);
	panUpBtn->setBounds(x, y, w, h); 
	panUpBtn->setClickingTogglesState(false);
	panUpBtn->addListener(this);
	x+= w+xPad;

	panDownBtn = new ChannelSelectorButton("-", titleFont);
	panDownBtn->setBounds(x, y, w, h);
	panDownBtn->setClickingTogglesState(false);
	panDownBtn->addListener(this);
	x+= w+xPad;

	panLabel = new Label("PanLabel", "Pan");	
	panLabel->setBounds(x, y, w*4, h);
	//panLabel->setFont(titleFont);
	x = xInitial;
	y += h + yPad;

	zoomInBtn = new ChannelSelectorButton("+", titleFont);
	zoomInBtn->setBounds(x,y,w,h);
	zoomInBtn->setClickingTogglesState(false);
	zoomInBtn->addListener(this);
	x += w + xPad;

	zoomOutBtn = new ChannelSelectorButton("-", titleFont);
	zoomOutBtn->setBounds(x,y,w,h);
	zoomOutBtn->setClickingTogglesState(false);
	zoomOutBtn->addListener(this);
	x += w + xPad;

	zoomLabel = new Label("ZoomLabel", "Zoom");
	zoomLabel->setBounds(x,y,w*4, h);
	//zoomLabel->setFont(titleFont);
	x = xInitial;
	y += h + yPad;
	// Button *zoomOutBtn = new EditorButton("-");
	
	clearBtn = new ChannelSelectorButton("Clear", titleFont);
	clearBtn->setBounds(x, y, w*2 + xPad, h);
	clearBtn->setClickingTogglesState(false);
	clearBtn->addListener(this);
	y += h + yPad;

	saveImgBtn = new ChannelSelectorButton("Save", titleFont);
	saveImgBtn->setBounds(x,y,w*2 + xPad, h);
	saveImgBtn->setClickingTogglesState(false);
	saveImgBtn->addListener(this);


	addAndMakeVisible(panUpBtn);
	addAndMakeVisible(panDownBtn);
	addAndMakeVisible(panLabel);


	addAndMakeVisible(zoomInBtn);
	addAndMakeVisible(zoomOutBtn);
	addAndMakeVisible(zoomLabel);
	addAndMakeVisible(clearBtn);
	addAndMakeVisible(saveImgBtn);



/*
	StringArray timeBaseValues;
	timeBaseValues.add("100");
	timeBaseValues.add("200");
	timeBaseValues.add("500");
	timeBaseValues.add("1000");

	createRadioButtons(35, 50, 160, timeBaseValues, "Thresholds (s)");

	StringArray displayGainValues;
	displayGainValues.add("100");
	displayGainValues.add("200");
	displayGainValues.add("400");
	displayGainValues.add("800");

	createRadioButtons(35, 90, 160, displayGainValues, "Display Gain");
*/


}

SpikeDisplayEditor::~SpikeDisplayEditor()
{
	deleteAllChildren();
}


Visualizer* SpikeDisplayEditor::createNewCanvas()
{

	SpikeDisplayNode* processor = (SpikeDisplayNode*) getProcessor();
	return new SpikeDisplayCanvas(processor);

}

void SpikeDisplayEditor::buttonCallback(Button* button)
{
	std::cout<<"Got event from component:"<<button<<std::endl;
	if (button == panUpBtn)
		std::cout<<"Pan Up"<<std::endl;
	else if (button == panDownBtn)
		std::cout<<"Pan Down"<<std::endl;
	else if (button == zoomInBtn)
		std::cout<<"Zoom In"<<std::endl;
	else if (button == zoomOutBtn)
		std::cout<<"Zoom Out"<<std::endl;
	else if (button == clearBtn){
		std::cout<<"Clear!"<<std::endl;
		canvas->setParameter(SPIKE_CMD_CLEAR_ALL, 0);
	}
	else if (button == saveImgBtn)
		std::cout<<"Save!"<<std::endl;
}
