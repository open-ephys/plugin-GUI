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

    triggerSelector = new ComboBox();
    triggerSelector->setBounds(5,30,55,20);
    triggerSelector->addListener(this);
    triggerSelector->addItem("Trig",1);

    for (int i = 0; i < 10; i++)
        triggerSelector->addItem(String(i),i+2);

    triggerSelector->setSelectedId(1, false);
    addAndMakeVisible(triggerSelector);

    gateSelector = new ComboBox();
    gateSelector->setBounds(5,55,55,20);
    gateSelector->addListener(this);
    gateSelector->addItem("Gate",1);

    for (int i = 0; i < 10; i++)
        gateSelector->addItem(String(i),i+2);

    gateSelector->setSelectedId(1, false);
    addAndMakeVisible(gateSelector);

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

    g.setFont(Font("Small Text", 10, Font::plain));

    g.drawText(name, 5, 80, 200, 10, Justification::left, false);
}

void ChannelTriggerInterface::buttonClicked(Button* button)
{
    pulsePal->triggerChannel(channelNumber);
}

void ChannelTriggerInterface::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    //std::cout << "Combo box changed to " << comboBoxThatHasChanged->getSelectedId() << std::endl;
   
    if (comboBoxThatHasChanged == triggerSelector)
    {
        processor->setParameter(0, channelNumber);
        processor->setParameter(1, (float) comboBoxThatHasChanged->getSelectedId() - 2);
    } else if (comboBoxThatHasChanged == gateSelector)
    {
        processor->setParameter(0, channelNumber);
        processor->setParameter(2, (float) comboBoxThatHasChanged->getSelectedId() - 2);
    }

    
}
