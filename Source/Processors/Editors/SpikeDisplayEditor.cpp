#include "SpikeDisplayEditor.h"



SpikeDisplayEditor::SpikeDisplayEditor (GenericProcessor* parentNode) 
	: VisualizerEditor(parentNode)

{

	desiredWidth = 250;

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


}

SpikeDisplayEditor::~SpikeDisplayEditor()
{
}


Visualizer* SpikeDisplayEditor::createNewCanvas()
{

	SpikeDisplayNode* processor = (SpikeDisplayNode*) getProcessor();
	return new SpikeDisplayCanvas(processor);

}

void SpikeDisplayEditor::buttonCallback(Button* button)
{

	int gId = button->getRadioGroupId();

	if (gId > 0) {
		if (canvas != 0)
		{
			canvas->setParameter(gId-1, button->getName().getFloatValue());
		}

	} 

}
