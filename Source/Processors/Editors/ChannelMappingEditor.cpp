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
#include "ChannelSelector.h"
#include <stdio.h>


ChannelMappingEditor::ChannelMappingEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(0)

{
    desiredWidth = 340;

    ElectrodeEditorButton* e1 = new ElectrodeEditorButton("MAPPING",Font("Small Text",14,Font::plain));
    e1->addListener(this);
    addAndMakeVisible(e1);
    e1->setBounds(15,110,80,10);
    e1->setToggleState(true, false);
    electrodeEditorButtons.add(e1);

    ElectrodeEditorButton* e2 = new ElectrodeEditorButton("REF",Font("Small Text",14,Font::plain));
    e2->addListener(this);
    addAndMakeVisible(e2);
    e2->setBounds(100,110,50,10);
    electrodeEditorButtons.add(e2);

    channelSelector->setRadioStatus(true);

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

    int width = 20;
    int height = 15;

    int row = 0;
    int column = 0;

	for (int i = 0; i < numNeeded; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(i+1);
        electrodeButtons.add(button);

        button->setBounds(10+(column++)*width, 30+row*height, width, 15);
        button->setToggleState(false,false);
        button->setRadioGroupId(999);

        addAndMakeVisible(button);
        button->addListener(this);

        referenceArray.add(-1);

        getProcessor()->setCurrentChannel(i);
        getProcessor()->setParameter(0,i); // set channel mapping to standard channel
        getProcessor()->setParameter(1,-1); // set reference to none

        channelArray.add(i+1);

        if (column%16 == 0)
        {
            column = 0;
            row++;
        }

    }

    channelSelector->setRadioStatus(true);
}


void ChannelMappingEditor::buttonEvent(Button* button)
{

	if (button == electrodeEditorButtons[0]) // mapping
	{

		electrodeEditorButtons[1]->setToggleState(false, false);

		for (int i = 0; i < electrodeButtons.size(); i++)
		{
			electrodeButtons[i]->setRadioGroupId(999);
			electrodeButtons[i]->setChannelNum(channelArray[i]);
			 electrodeButtons[i]->repaint();
		}

	}
	 else if (button == electrodeEditorButtons[1]) // reference
	{
		electrodeEditorButtons[0]->setToggleState(false, false);

		for (int i = 0; i < electrodeButtons.size(); i++)
		{
			electrodeButtons[i]->setRadioGroupId(0);
			electrodeButtons[i]->setChannelNum(referenceArray[i]);
			 electrodeButtons[i]->repaint();
		}


	} 
	else if (electrodeButtons.contains((ElectrodeButton*) button))
	{

		ElectrodeButton* eb = (ElectrodeButton*) button;
        int electrodeNum = eb->getChannelNum()-1;

        std::cout << "Channel number: " << electrodeNum << std::endl;
        Array<int> a;

        if (electrodeNum >= 0)
        	a.add(electrodeNum);

        channelSelector->setActiveChannels(a);

	}

}

void ChannelMappingEditor::channelChanged(int chan)
{
	for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum(chan);
            electrodeButtons[i]->repaint();

            getProcessor()->setCurrentChannel(i);

            if (electrodeEditorButtons[1]->getToggleState()) // reference
            {
            	referenceArray.set(i,chan);

        		getProcessor()->setParameter(1,chan-1); // set reference

            } else {
            	channelArray.set(i,chan);

            	getProcessor()->setParameter(0,chan-1); // set mapping
            }

        }
    }
}

void ChannelMappingEditor::saveEditorParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "ChannelMappingEditor");

  //  XmlElement* selectedChannel = xml->createNewChildElement("SELECTEDID");

   // selectedChannel->setAttribute("ID",referenceSelector->getSelectedId());

}

void ChannelMappingEditor::loadEditorParameters(XmlElement* xml)
{

     forEachXmlChildElement(*xml, xmlNode)
     {
        // if (xmlNode->hasTagName("SELECTEDID"))
      //   {
//
           // int id = xmlNode->getIntAttribute("ID");

          //  referenceSelector->setSelectedId(id);

      //  }
    }
}