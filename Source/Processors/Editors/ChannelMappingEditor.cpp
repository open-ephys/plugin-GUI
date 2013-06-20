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

    mappingButton = new ElectrodeEditorButton("MAPPING",Font("Small Text",14,Font::plain));
    mappingButton->addListener(this);
    addAndMakeVisible(mappingButton);
    mappingButton->setBounds(15,110,80,10);
    mappingButton->setToggleState(true, false);

    referenceButton = new ElectrodeEditorButton("REF",Font("Small Text",14,Font::plain));
    referenceButton->addListener(this);
    addAndMakeVisible(referenceButton);
    referenceButton->setBounds(100,110,50,10);
    referenceButton->setToggleState(false, false);

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

        if (column % 16 == 0)
        {
            column = 0;
            row++;
        }

    }

    channelSelector->setRadioStatus(true);
}


void ChannelMappingEditor::buttonEvent(Button* button)
{

    if (button == mappingButton) // mapping
    {
        std::cout << "Mapping button clicked." << std::endl;

        referenceButton->setToggleState(false, false);
        mappingButton->setToggleState(true, false);

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            electrodeButtons[i]->setRadioGroupId(999);
            electrodeButtons[i]->setChannelNum(channelArray[i]);
            electrodeButtons[i]->repaint();
        }

    }
    else if (button == referenceButton) // reference
    {

        std::cout << "Reference button clicked." << std::endl;

        mappingButton->setToggleState(false, false);
        referenceButton->setToggleState(true, false);

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

    std::cout << "Reference button state: " << referenceButton->getToggleState() << std::endl;
    std::cout << "Mapping button state: " << mappingButton->getToggleState() << std::endl;

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

            if (referenceButton->getToggleState()) // reference
            {
                referenceArray.set(i,chan);

                getProcessor()->setParameter(1,chan-1); // set reference

            }
            else
            {
                channelArray.set(i,chan);

                getProcessor()->setParameter(0,chan-1); // set mapping
            }

        }
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
        channelXml->setAttribute("Reference", referenceArray[i]);
    }

    // String channelString = "";
    // String referenceString = "";

    // for (int i = 0; i < channelArray.size(); i++)
    // {
    //     channelString += String(channelArray[i]);
    //     referenceString += String(referenceArray[i]);
    // }

    // xml->setAttribute("Channels", channelString);
    // xml->setAttribute("References", referenceString);

    //  XmlElement* selectedChannel = xml->createNewChildElement("SELECTEDID");

    // selectedChannel->setAttribute("ID",referenceSelector->getSelectedId());

}

void ChannelMappingEditor::loadEditorParameters(XmlElement* xml)
{

   // String channelString = xml->getStringAttribute("Channels");
   // String referenceString = xml->getStringAttribute("References");

    forEachXmlChildElementWithTagName(*xml, channelXml, "CHANNEL")
    {
        int i = channelXml->getIntAttribute("Number");

        if (i < channelArray.size())
        {

            int mapping = channelXml->getIntAttribute("Mapping");
            int reference = channelXml->getIntAttribute("Reference");

            channelArray.set(i, mapping);
            referenceArray.set(i, reference);

            electrodeButtons[i]->setChannelNum(mapping);
            electrodeButtons[i]->repaint();

            getProcessor()->setCurrentChannel(i);

            getProcessor()->setParameter(1, reference-1); // set reference

            getProcessor()->setParameter(0, mapping-1); // set mapping
        }

    }

    // for (int i = 0; i < channelString.length(); i++)
    // {

    //     String ch = channelString.substring(i,i);
    //     String ref = referenceString.substring(i,i);

    //     if (i < channelArray.size())
    //     {
    //         int chNum = ch.getIntValue();
    //         int refNum = ref.getIntValue();

    //         channelArray.set(i, chNum);
    //         referenceArray.set(i, refNum);

    //         electrodeButtons[i]->setChannelNum(chNum);
    //         electrodeButtons[i]->repaint();

    //         getProcessor()->setCurrentChannel(i);

    //         getProcessor()->setParameter(1, refNum-1); // set reference

    //         getProcessor()->setParameter(0, chNum-1); // set mapping
            
    //     }

    // }

}