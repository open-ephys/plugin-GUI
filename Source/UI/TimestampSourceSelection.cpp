/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2016 Open Ephys

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

#include "TimestampSourceSelection.h"
#include "../AccessClass.h"
#include "EditorViewport.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"

TimestampSourceSelectionWindow::TimestampSourceSelectionWindow()
	: DocumentWindow("Global timestamp source selection", Colours::red,
	DocumentWindow::closeButton)
{
	centreWithSize(300, 200);
	setUsingNativeTitleBar(true);
	setResizable(false, false);
	m_selectorComponent = new TimestampSourceSelectionComponent();
	setContentNonOwned(m_selectorComponent, true);
	AccessClass::getProcessorGraph()->setTimestampWindow(this);
	m_selectorComponent->setAcquisitionState(CoreServices::getAcquisitionStatus());
}

TimestampSourceSelectionWindow::~TimestampSourceSelectionWindow()
{
	masterReference.clear();
}

void TimestampSourceSelectionWindow::updateProcessorList()
{
	m_selectorComponent->triggerAsyncUpdate();
}

void TimestampSourceSelectionWindow::setAcquisitionState(bool s)
{
	m_selectorComponent->setAcquisitionState(s);
}

void TimestampSourceSelectionWindow::closeButtonPressed()
{
	setVisible(false);
	delete this;
}

//Component
TimestampSourceSelectionComponent::TimestampSourceSelectionComponent()
{
	setSize(300, 200);
	m_selector = new ComboBox("Timestamp Sources");
	m_selector->setBounds(50, 150, 200, 30);
	m_selector->addListener(this);
	addAndMakeVisible(m_selector);
	updateProcessorList();
}

TimestampSourceSelectionComponent::~TimestampSourceSelectionComponent()
{}

void TimestampSourceSelectionComponent::handleAsyncUpdate()
{
	updateProcessorList();
}

void TimestampSourceSelectionComponent::updateProcessorList()
{
	int selectedIdx, selectedSubIdx;
	Array<const GenericProcessor*> processors;
	AccessClass::getProcessorGraph()->getTimestampSources(processors, selectedIdx, selectedSubIdx);
	m_sourcesArray.clear();
	m_selector->clear();
	m_selector->addItem("Software timer", 1);
	int selected = 1;

	int nProcessors = processors.size();
	int n = 1;
	for (int i = 0; i < nProcessors; i++)
	{
		const GenericProcessor* p = processors.getUnchecked(i);
		int numSubs = p->getNumSubProcessors();
		for (int j = 0; j < numSubs; j++)
		{
			String text = (numSubs > 1) ? p->getName() + "(" + String(j + 1) + ")" : p->getName();
			m_selector->addItem(text, ++n);
			SourceInfo s;
			s.processorIndex = i;
			s.subProcessorIndex = j;
			m_sourcesArray.add(s);

			if (selectedIdx == i && selectedSubIdx == j)
				selected = n;
		}
	}
	m_selector->setSelectedId(selected, dontSendNotification);
}

void TimestampSourceSelectionComponent::comboBoxChanged(ComboBox* c)
{
	int selected = c->getSelectedId() - 2;
	int sourceIdx, subIdx;
	if (selected < 0)
	{
		sourceIdx = -1;
		subIdx = 0;
	}
	else
	{
		sourceIdx = m_sourcesArray[selected].processorIndex;
		subIdx = m_sourcesArray[selected].subProcessorIndex;
	}
	AccessClass::getProcessorGraph()->setTimestampSource(sourceIdx, subIdx);
}

void TimestampSourceSelectionComponent::setAcquisitionState(bool s)
{
	m_selector->setEnabled(!s);
}

void TimestampSourceSelectionComponent::paint(Graphics& g)
{
	g.setColour(Colours::darkgrey);
	g.fillAll();
	g.setColour(Colours::black);
	g.drawMultiLineText("This field sets the global timestamp source.\n\n"
		"Processors that generate events not based on any existing data streams but do not generate their "
		"own timestamps will use both the timestamps and sample rate of the selected processor as reference.",
		10, 30, 280);
}