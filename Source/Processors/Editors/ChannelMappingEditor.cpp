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


#include "ChannelMappingEditor.h"
#include "../ChannelMappingNode.h"
#include "../../UI/EditorViewport.h"
#include "ChannelSelector.h"
#include <stdio.h>


ChannelMappingEditor::ChannelMappingEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
	: GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(0)

{
	desiredWidth = 340;

	selectAllButton = new ElectrodeEditorButton("Select All",Font("Small Text",14,Font::plain));
	selectAllButton->addListener(this);
	addAndMakeVisible(selectAllButton);
	selectAllButton->setBounds(140,110,110,10);
	selectAllButton->setToggleState(false, false);

	modifyButton = new ElectrodeEditorButton("Remap",Font("Small Text",14,Font::plain));
	modifyButton->addListener(this);
	addAndMakeVisible(modifyButton);
	modifyButton->setBounds(250,110,60,10);
	modifyButton->setToggleState(false, false);
	modifyButton->setClickingTogglesState(true);

	//    channelSelector->setRadioStatus(true);

	for (int i = 0 ; i < NUM_REFERENCES; i++)
	{
		ElectrodeButton *button = new ElectrodeButton(i+1);
		referenceButtons.add(button);

		button->setBounds(10+i*30,110,20,15);
		button->setToggleState(false,false);
		button->setRadioGroupId(900);

		addAndMakeVisible(button);
		button->addListener(this);
		button->setButtonText(String::charToString('A'+i));

	}

	reorderActive = false;
	isDragging = false;

}

ChannelMappingEditor::~ChannelMappingEditor()
{

}

void ChannelMappingEditor::updateSettings()
{

	if (getProcessor()->getNumInputs() != previousChannelCount)
	{

		createElectrodeButtons(getProcessor()->getNumInputs());

		previousChannelCount = getProcessor()->getNumInputs();

	}

}

void ChannelMappingEditor::createElectrodeButtons(int numNeeded)
{

	electrodeButtons.clear();

	referenceArray.clear();
	channelArray.clear();
	referenceChannels.clear();
	enabledChannelArray.clear();

	int width = 20;
	int height = 15;

	int row = 0;
	int column = 0;

	for (int i = 0; i < numNeeded; i++)
	{
		ElectrodeButton* button = new ElectrodeButton(i+1);
		electrodeButtons.add(button);

		button->setBounds(10+(column++)*(width), 30+row*(height), width, 15);
		button->setToggleState(false,false);
		button->setRadioGroupId(0);

		addAndMakeVisible(button);
		button->addListener(this);

		referenceArray.add(-1);

		getProcessor()->setCurrentChannel(i);
		getProcessor()->setParameter(0,i); // set channel mapping to standard channel
		getProcessor()->setParameter(1,-1); // set reference to none
		getProcessor()->setParameter(3,1); //enable channel

		channelArray.add(i+1);
		enabledChannelArray.add(true);

		if (column % 16 == 0)
		{
			column = 0;
			row++;
		}

	}
	getProcessor()->setCurrentChannel(-1);
	for (int i = 0; i < NUM_REFERENCES; i++)
	{

		getProcessor()->setParameter(2,i); //Clear reference
		referenceChannels.add(-1);
	}

	referenceButtons[0]->setToggleState(true,false);
	selectedReference = 0;

	modifyButton->setToggleState(false,false);
	reorderActive = false;
	previousClickedChan = -1;
	previousShiftClickedChan = -1;


	channelSelector->setRadioStatus(true);
}


void ChannelMappingEditor::buttonEvent(Button* button)
{

	if (button == selectAllButton)
	{
		for (int i = 0; i < electrodeButtons.size(); i++)
		{
			electrodeButtons[i]->setToggleState(true,false);
			setChannelReference(electrodeButtons[i]);
		}
		previousClickedChan = -1;
	}
	else if (button == modifyButton)
	{
		if (reorderActive)
		{
			channelSelector->activateButtons();
			reorderActive = false;
			selectedReference = 0;
			for (int i=0; i < referenceButtons.size(); i++)
			{
				referenceButtons[i]->setEnabled(true);
			}
			referenceButtons[0]->setToggleState(true,false);
			Array<int> a;

			if (referenceChannels[selectedReference] >= 0 )
			{
				a.add(referenceChannels[selectedReference]);
			}
			channelSelector->setActiveChannels(a);

			for (int i = 0; i < electrodeButtons.size(); i++)
			{
				electrodeButtons[i]->setClickingTogglesState(true);
				electrodeButtons[i]->removeMouseListener(this);
				if (referenceArray[electrodeButtons[i]->getChannelNum()-1] == selectedReference)
				{
					electrodeButtons[i]->setToggleState(true,false);
				}
				else
				{
					electrodeButtons[i]->setToggleState(false,false);
				}
				if (enabledChannelArray[electrodeButtons[i]->getChannelNum()-1])
				{
					electrodeButtons[i]->setEnabled(true);
				}
				else
				{
					electrodeButtons[i]->setEnabled(false);
				}
			}
			selectAllButton->setEnabled(true);
		}
		else
		{
			reorderActive = true;
			channelSelector->inactivateButtons();

			for (int i=0; i < referenceButtons.size(); i++)
			{
				referenceButtons[i]->setEnabled(false);
				referenceButtons[i]->setToggleState(false,false);
			}

			for (int i = 0; i < electrodeButtons.size(); i++)
			{
				electrodeButtons[i]->setClickingTogglesState(false);
				electrodeButtons[i]->setEnabled(true);
				if (enabledChannelArray[electrodeButtons[i]->getChannelNum()-1])
				{
					electrodeButtons[i]->setToggleState(true,false);
				}
				else
				{
					electrodeButtons[i]->setToggleState(false,false);
				}
				electrodeButtons[i]->addMouseListener(this,false);
				
			}
			selectAllButton->setEnabled(false);
		}
	}
	else if (referenceButtons.contains((ElectrodeButton*)button))
	{
		selectedReference = ((ElectrodeButton*)button)->getChannelNum()-1;

		Array<int> a;

		if (referenceChannels[selectedReference] >= 0 )
		{
			a.add(referenceChannels[selectedReference]);
		}
		channelSelector->setActiveChannels(a);

		for (int i = 0; i < electrodeButtons.size(); i++)
		{
			if (referenceArray[electrodeButtons[i]->getChannelNum()-1] == selectedReference)
			{
				electrodeButtons[i]->setToggleState(true,false);
			}
			else
			{
				electrodeButtons[i]->setToggleState(false,false);
			}
		}
		previousClickedChan = -1;

	}
	else if (electrodeButtons.contains((ElectrodeButton*)button))
	{
		if (!reorderActive)
		{
			int clickedChan = electrodeButtons.indexOf((ElectrodeButton*)button);

			if (ModifierKeys::getCurrentModifiers().isShiftDown() && (previousClickedChan >= 0))
			{
				int toChanA,toChanD,fromChanA,fromChanD;

				if (previousShiftClickedChan < 0)
				{
					previousShiftClickedChan = clickedChan;
					if (clickedChan > previousClickedChan)
					{
						toChanA = clickedChan;
						fromChanA = previousClickedChan;
					}
					else
					{
						toChanA = previousClickedChan;
						fromChanA = clickedChan;
					}
					for (int i = fromChanA; i <= toChanA; i++)
					{
						electrodeButtons[i]->setToggleState(previousClickedState,false);
						setChannelReference(electrodeButtons[i]);
					}
				}
				else 
				{
					if ((clickedChan > previousClickedChan) && (clickedChan > previousShiftClickedChan))
					{
						fromChanA = previousShiftClickedChan+1;
						toChanA = clickedChan;
						if (previousShiftClickedChan < previousClickedChan)
						{
							fromChanD = previousShiftClickedChan;
							toChanD = previousClickedChan-1;
						}
						else
						{
							fromChanD = -1;
						}
					}
					else if ((clickedChan > previousClickedChan) && (clickedChan < previousShiftClickedChan))
					{
						fromChanA = -1;
						fromChanD = clickedChan+1;
						toChanD = previousShiftClickedChan;
						button->setToggleState(previousClickedState,false); // Do not toggle this button;
					}
					else if ((clickedChan < previousClickedChan) && (clickedChan < previousShiftClickedChan))
					{
						fromChanA = clickedChan;
						toChanA = previousShiftClickedChan-1;
						if (previousShiftClickedChan > previousClickedChan)
						{
							fromChanD = previousClickedChan+1;
							toChanD = previousShiftClickedChan;
						}
						else
						{
							fromChanD = -1;
						}
					}
					else if ((clickedChan < previousClickedChan) && (clickedChan > previousShiftClickedChan))
					{
						fromChanA = -1;
						fromChanD = previousShiftClickedChan;
						toChanD = clickedChan - 1;
						button->setToggleState(previousClickedState,false); // Do not toggle this button;
					}
					else if (clickedChan == previousShiftClickedChan)
					{
						fromChanA = -1;
						fromChanD = -1;
						button->setToggleState(previousClickedState,false); // Do not toggle this button;
					}
					else
					{
						fromChanA = -1;
						button->setToggleState(previousClickedState,false); // Do not toggle this button;
						if (previousShiftClickedChan < previousClickedChan)
						{
							fromChanD = previousShiftClickedChan;
							toChanD = previousClickedChan - 1;
						}
						else if (previousShiftClickedChan > previousClickedChan)
						{
							fromChanD = previousClickedChan + 1;
							toChanD = previousShiftClickedChan;
						}
						else
						{
							fromChanD = -1;
						}
					}

					if (fromChanA >= 0)
					{
						for (int i = fromChanA; i <= toChanA; i++)
						{
							electrodeButtons[i]->setToggleState(previousClickedState,false);
							setChannelReference(electrodeButtons[i]);
						}
					}
					if (fromChanD >= 0)
					{
						for (int i = fromChanD; i <= toChanD; i++)
						{
							electrodeButtons[i]->setToggleState(!previousClickedState,false);
							setChannelReference(electrodeButtons[i]);
						}
					}
				}

				previousShiftClickedChan = clickedChan;
			}
			else
			{
				previousClickedChan = clickedChan;
				previousShiftClickedChan = -1;
				setChannelReference((ElectrodeButton*)button);
				previousClickedState = button->getToggleState();
			}


		}
	}
}

void ChannelMappingEditor::setChannelReference(ElectrodeButton *button)
{
	int chan = button->getChannelNum()-1;
	getProcessor()->setCurrentChannel(chan);

	if (button->getToggleState())
	{
		referenceArray.set(chan,selectedReference);
		getProcessor()->setParameter(1,selectedReference);				
	}
	else
	{
		referenceArray.set(chan,-1);
		getProcessor()->setParameter(1,-1);				
	}

}

void ChannelMappingEditor::channelChanged(int chan)
{
	if (!reorderActive)
	{
		getProcessor()->setCurrentChannel(chan-1);
		getProcessor()->setParameter(2,selectedReference);
		referenceChannels.set(selectedReference,chan-1);
	}
}

void ChannelMappingEditor::saveEditorParameters(XmlElement* xml)
{
	xml->setAttribute("Type", "ChannelMappingEditor");

	for (int i = 0; i < channelArray.size(); i++)
	{
		XmlElement* channelXml = xml->createNewChildElement("CHANNEL");
		channelXml->setAttribute("Number", i);
		channelXml->setAttribute("Mapping", channelArray[i]);
		channelXml->setAttribute("Reference", referenceArray[channelArray[i]-1]);
		channelXml->setAttribute("Enabled",enabledChannelArray[channelArray[i]-1]);
	}
	for (int i = 0; i< referenceChannels.size(); i++)
	{
		XmlElement* referenceXml = xml->createNewChildElement("REFERENCE");
		referenceXml->setAttribute("Number", i);
		referenceXml->setAttribute("Channel",referenceChannels[i]);
	}

}

void ChannelMappingEditor::loadEditorParameters(XmlElement* xml)
{

	forEachXmlChildElementWithTagName(*xml, channelXml, "CHANNEL")
	{
		int i = channelXml->getIntAttribute("Number");

		if (i < channelArray.size())
		{

			int mapping = channelXml->getIntAttribute("Mapping");
			int reference = channelXml->getIntAttribute("Reference");
			bool enabled = channelXml->getBoolAttribute("Enabled");

			channelArray.set(i, mapping);
			referenceArray.set(mapping-1, reference);
			enabledChannelArray.set(mapping-1,enabled);

			electrodeButtons[i]->setChannelNum(mapping);
			electrodeButtons[i]->setEnabled(enabled);
			electrodeButtons[i]->repaint();
			

			getProcessor()->setCurrentChannel(i);

			getProcessor()->setParameter(0, mapping-1); // set mapping

			getProcessor()->setCurrentChannel(mapping-1);

			getProcessor()->setParameter(1, reference); // set reference			

			getProcessor()->setParameter(3,enabled ? 1 : 0); //set enabled
		}

	}

	forEachXmlChildElementWithTagName(*xml, referenceXml, "REFERENCE")
	{
		int i = referenceXml->getIntAttribute("Number");

		if ( i < referenceChannels.size() )
		{
			int channel = referenceXml->getIntAttribute("Channel");

			referenceChannels.set(i,channel);

			getProcessor()->setCurrentChannel(channel);

			getProcessor()->setParameter(2,i);
		}
	}

	for (int i = 0; i < electrodeButtons.size(); i++)
		{
			if (referenceArray[electrodeButtons[i]->getChannelNum()-1] == selectedReference)
			{
				electrodeButtons[i]->setToggleState(true,false);
			}
			else
			{
				electrodeButtons[i]->setToggleState(false,false);
			}
		}

}

void ChannelMappingEditor::mouseDrag(const MouseEvent &e)
{
	if (reorderActive)
	{
		if ((!isDragging) && (electrodeButtons.contains((ElectrodeButton*) e.originalComponent) ))
		{
			ElectrodeButton* button = (ElectrodeButton*)e.originalComponent;
			isDragging = true;

			String desc = "EditorDrag/MAP/";
			desc += button->getChannelNum();

			const String dragDescription = desc;

			Image dragImage(Image::ARGB,20,15,true);

			Graphics g(dragImage);
			if (button->getToggleState())
			{
				g.setColour(Colours::orange);
			}
			else
			{
				g.setColour(Colours::darkgrey);
			}
			g.fillAll();
			g.setColour(Colours::black);
			g.drawText(String(button->getChannelNum()),0,0,20,15,Justification::centred,true);

			dragImage.multiplyAllAlphas(0.6f);

			startDragging(dragDescription,this,dragImage,false);
			button->setVisible(false);
			initialDraggedButton = electrodeButtons.indexOf(button);
			lastHoverButton = initialDraggedButton;
			draggingChannel = button->getChannelNum();
		}
		else if (isDragging)
		{
			MouseEvent ev = e.getEventRelativeTo(this);

			int col = ((ev.x-5) / 20);
			if (col < 0) col = 0;
			else if (col > 16) col = 16;

			int row = ((ev.y-30) / 15);
			if (row < 0) row = 0;

			int hoverButton = row*16+col;

			if (hoverButton >= electrodeButtons.size())
			{
				hoverButton = electrodeButtons.size() -1;
			}

			if (hoverButton != lastHoverButton)
			{
				
				electrodeButtons[lastHoverButton]->setVisible(true);
				electrodeButtons[hoverButton]->setVisible(false);

				if (lastHoverButton > hoverButton)
				{
					for (int i = lastHoverButton; i > hoverButton; i--)
					{
						electrodeButtons[i]->setChannelNum(electrodeButtons[i-1]->getChannelNum());
						if (enabledChannelArray[electrodeButtons[i]->getChannelNum()-1]) //Could be more compact, but definitely less legible
						{
							electrodeButtons[i]->setToggleState(true,false);
						}
						else
						{
							electrodeButtons[i]->setToggleState(false,false);
						}
					}
				}
				else
				{
					for (int i = lastHoverButton; i < hoverButton; i++)
					{
						electrodeButtons[i]->setChannelNum(electrodeButtons[i+1]->getChannelNum());
						if (enabledChannelArray[electrodeButtons[i]->getChannelNum()-1])
						{
							electrodeButtons[i]->setToggleState(true,false);
						}
						else
						{
							electrodeButtons[i]->setToggleState(false,false);
						}
					}
				}
				electrodeButtons[hoverButton]->setChannelNum(draggingChannel);
				electrodeButtons[hoverButton]->setToggleState(enabledChannelArray[draggingChannel-1],false);
				
				lastHoverButton = hoverButton;
				repaint();
			}

		}

	}
}

void ChannelMappingEditor::mouseUp(const MouseEvent &e)
{
	if (isDragging)
	{
		isDragging = false;
		electrodeButtons[lastHoverButton]->setVisible(true);
		int from, to;
		if (lastHoverButton < initialDraggedButton)
		{
			from = lastHoverButton;
			to = initialDraggedButton;
		}
		else
		{
			from = initialDraggedButton;
			to = lastHoverButton;
		}

		for (int i=from; i <= to; i++)
		{
			setChannelPosition(i,electrodeButtons[i]->getChannelNum());
		}
	}
}

void ChannelMappingEditor::setChannelPosition(int position, int channel)
{
	getProcessor()->setCurrentChannel(position);
	getProcessor()->setParameter(0,channel-1);
	channelArray.set(position,channel);
}

void ChannelMappingEditor::mouseDoubleClick(const MouseEvent &e)
{
	if ((reorderActive) && electrodeButtons.contains((ElectrodeButton*)e.originalComponent))
	{
		ElectrodeButton *button = (ElectrodeButton*)e.originalComponent;

		if (button->getToggleState())
		{
			button->setToggleState(false,false);
			enabledChannelArray.set(button->getChannelNum()-1,false);
			getProcessor()->setCurrentChannel(button->getChannelNum()-1);
			getProcessor()->setParameter(3,0);
		}
		else
		{
			button->setToggleState(true,false);
			enabledChannelArray.set(button->getChannelNum()-1,true);
			getProcessor()->setCurrentChannel(button->getChannelNum()-1);
			getProcessor()->setParameter(3,1);
		}
		getEditorViewport()->makeEditorVisible(this, false, true);
	}
}