#include "PSTHConditionList.h"

ConditionList::ConditionList(PSTHProcessor* n, Viewport *p, PSTHCanvas*c) :
processor(n), viewport(p), canvas(c)
{

	titleButton = new ColorButton("CONDITIONS LIST", Font("Default", 24, Font::plain));
	titleButton->setBounds(0, 0, 200, 25);
	titleButton->addListener(this);
	addAndMakeVisible(titleButton);

	allButton = new ColorButton("All", Font("Default", 20, Font::plain));
	allButton->setBounds(0, 25, 100, 20);
	allButton->addListener(this);
	addAndMakeVisible(allButton);

	noneButton = new ColorButton("None", Font("Default", 20, Font::plain));
	noneButton->setBounds(100, 25, 100, 20);
	noneButton->addListener(this);
	addAndMakeVisible(noneButton);

	updateConditionButtons();

}


void ConditionList::updateConditionButtons()
{
	if (processor->trialCircularBuffer != nullptr)
	{
		const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);
		//processor->trialCircularBuffer->lockConditions();
		conditionButtons.clear();
		for (int k = 0; k<processor->trialCircularBuffer->getNumConditions(); k++)
		{
			Condition cond = processor->trialCircularBuffer->getCondition(k);
			ColorButton* conditionButton = new ColorButton(cond.name, Font("Default", 20, Font::plain));
			conditionButton->setBounds(0, 50 + k * 20, 200, 20);
			conditionButton->setColors(Colours::white,
				juce::Colour::fromRGB(cond.colorRGB[0],
				cond.colorRGB[1],
				cond.colorRGB[2]));
			conditionButton->setEnabledState(cond.visible);
			conditionButton->setUserDefinedData(cond.conditionID);
			conditionButton->setShowEnabled(true);
			conditionButton->addListener(this);
			addAndMakeVisible(conditionButton);
			conditionButtons.add(conditionButton);
		}

		//processor->trialCircularBuffer->unlockConditions();
	}
}

ConditionList::~ConditionList()
{

	for (int i = 0; i < conditionButtons.size(); i++)
	{
		removeChildComponent(conditionButtons[i]);
	}
}

void ConditionList::paint(Graphics& g)
{
	g.fillAll(juce::Colours::grey);
	//g.drawText
}

void ConditionList::buttonClicked(Button *btn)
{
	ColorButton *cbtn = (ColorButton *)btn;
	// also inform trial circular buffer about visibility change.
	if (btn == titleButton)
	{
		int x = 5;
	}
	else if (btn == noneButton)
	{
		if (processor->trialCircularBuffer != nullptr)
		{
			//processor->trialCircularBuffer->lockConditions();
			const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);
			for (int k = 0; k<processor->trialCircularBuffer->getNumConditions(); k++)
			{
				processor->trialCircularBuffer->modifyConditionVisibility(k, false);
				conditionButtons[k]->setEnabledState(false);
			}
			//processor->trialCircularBuffer->unlockConditions();
		}

	}
	else if (btn == allButton)
	{
		if (processor->trialCircularBuffer != nullptr)
		{
			const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);
			//processor->trialCircularBuffer->lockConditions();
			for (int k = 0; k<processor->trialCircularBuffer->getNumConditions(); k++)
			{
				processor->trialCircularBuffer->modifyConditionVisibility(k, true);
				conditionButtons[k]->setEnabledState(true);
			}
			//processor->trialCircularBuffer->unlockConditions();
		}

	}
	else
	{
		// probably a condition button
		int conditionID = cbtn->getUserDefinedData();
		cbtn->setEnabledState(!cbtn->getEnabledState());
		processor->trialCircularBuffer->modifyConditionVisibilityusingConditionID(conditionID, cbtn->getEnabledState());
	}

	repaint();
}