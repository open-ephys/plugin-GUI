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

#include <stdio.h>
#include "PulsePalOutputEditor.h"

#include "../PulsePalOutput.h"
#include "../Serial/PulsePal.h"

PulsePalOutputEditor::PulsePalOutputEditor(GenericProcessor* parentNode, PulsePal* pp, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), pulsePal(pp)

{

    desiredWidth = 315;

    for (int i = 1; i < 5; i++)
    {
        ChannelTriggerInterface* cti = new ChannelTriggerInterface(pp, (PulsePalOutput*) getProcessor(), i);

        channelTriggerInterfaces.add(cti);

        cti->setBounds(10+75*(i-1),30,65,90);

        addAndMakeVisible(cti);

    }


}

PulsePalOutputEditor::~PulsePalOutputEditor()
{


}

//-----------------------------------------------

ChannelTriggerInterface::ChannelTriggerInterface(PulsePal* pp, PulsePalOutput* ppo, int chan)
    : pulsePal(pp), processor(ppo), channelNumber(chan), isEnabled(true), name(String(chan))
{

    triggerButton = new UtilityButton("trigger", Font("Small Text", 10, Font::plain));
    triggerButton->addListener(this);
    triggerButton->setRadius(3.0f);
    triggerButton->setBounds(5,5,55,20);
    addAndMakeVisible(triggerButton);

    comboBox = new ComboBox();
    comboBox->setBounds(5,30,55,20);
    comboBox->addListener(this);
    comboBox->addItem("N/A",1);

    for (int i = 0; i < 10; i++)
        comboBox->addItem(String(i),i+2);

    comboBox->setSelectedId(1, false);
    addAndMakeVisible(comboBox);

}

ChannelTriggerInterface::~ChannelTriggerInterface()
{

}

void ChannelTriggerInterface::paint(Graphics& g)
{
    g.setColour(Colours::lightgrey);

    g.fillRoundedRectangle(0,0,getWidth(),getHeight(),4.0f);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(Font("Small Text", 25, Font::plain));

    g.drawText(name, 25, 55, 200, 25, Justification::left, false);
}

void ChannelTriggerInterface::buttonClicked(Button* button)
{
    pulsePal->triggerChannel(channelNumber);
}

void ChannelTriggerInterface::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    //std::cout << "Combo box changed to " << comboBoxThatHasChanged->getSelectedId() << std::endl;
   
    processor->setParameter(channelNumber, (float) comboBoxThatHasChanged->getSelectedId() - 2);

}
