#include "SpikeDisplayEditor.h"



SpikeDisplayEditor::SpikeDisplayEditor (GenericProcessor* parentNode) 
	: VisualizerEditor(parentNode)

{

	desiredWidth = 250;

	StringArray timeBaseValues;
	timeBaseValues.add("1");
	timeBaseValues.add("2");
	timeBaseValues.add("5");
	timeBaseValues.add("10");

	createRadioButtons(35, 50, 160, timeBaseValues, "Display width (s)");

	StringArray displayGainValues;
	displayGainValues.add("1");
	displayGainValues.add("2");
	displayGainValues.add("4");
	displayGainValues.add("8");

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
