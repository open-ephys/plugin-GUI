/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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
#include "PulsePalOutputCanvas.h"

#include "PulsePalOutput.h"
#include "serial/PulsePal.h"

PulsePalOutputEditor::PulsePalOutputEditor(GenericProcessor* parentNode, PulsePal* pp, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors), pulsePal(pp)

{
    tabText = "PulsePal";
    desiredWidth = 460;

    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        ChannelTriggerInterface* cti = new ChannelTriggerInterface(pp, (PulsePalOutput*) getProcessor(), i);
        channelTriggerInterfaces.add(cti);
        cti->setBounds(5+115*(i), 30, 100, 90);
        addAndMakeVisible(cti);
    }
}

Visualizer* PulsePalOutputEditor::createNewCanvas()
{
    PulsePalOutput* processor = (PulsePalOutput*) getProcessor();
    return new PulsePalOutputCanvas(processor);
}

PulsePalOutputEditor::~PulsePalOutputEditor()
{
}

void PulsePalOutputEditor::updateSettings()
{
    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        channelTriggerInterfaces[i]->updateSources();
    }
}

void PulsePalOutputEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "PulsePalOutputEditor");

    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        XmlElement* outputXml = xml->createNewChildElement("OUTPUTCHANNEL");
        outputXml->setAttribute("Number", i);
        outputXml->setAttribute("Trigger",channelTriggerInterfaces[i]->getTriggerChannel());
        outputXml->setAttribute("Gate",channelTriggerInterfaces[i]->getGateChannel());
    }


}

void PulsePalOutputEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("OUTPUTCHANNEL"))
        {

            int chNum = xmlNode->getIntAttribute("Number");

            channelTriggerInterfaces[chNum]->setTriggerChannel(xmlNode->getIntAttribute("Trigger"));
            channelTriggerInterfaces[chNum]->setGateChannel(xmlNode->getIntAttribute("Gate"));

        }
    }
}


//-----------------------------------------------

ChannelTriggerInterface::ChannelTriggerInterface(PulsePal* pp, PulsePalOutput* ppo, int chan)
    : pulsePal(pp), processor(ppo), isEnabled(true), channelNumber(chan), name(String(chan))
    , m_triggerSelected(1)
    , m_gateSelected(1)
{

    triggerButton = new UtilityButton("trigger", Font("Small Text", 10, Font::plain));
    triggerButton->addListener(this);
    triggerButton->setRadius(3.0f);
    triggerButton->setBounds(5, 5, 90, 20);
    addAndMakeVisible(triggerButton);

    triggerSelector = new ComboBox();
    triggerSelector->setBounds(5, 30, 90, 20);
    triggerSelector->addListener(this);
    triggerSelector->addItem("Trig",1);

    gateSelector = new ComboBox();
    gateSelector->setBounds(5, 55, 90,20);
    gateSelector->addListener(this);
    gateSelector->addItem("Gate",1);

    updateSources();

    triggerSelector->setSelectedId(m_triggerSelected, dontSendNotification);
    addAndMakeVisible(triggerSelector);
    gateSelector->setSelectedId(m_gateSelected, dontSendNotification);
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
    pulsePal->triggerChannel(channelNumber + 1);
}

void ChannelTriggerInterface::updateSources()
{
    EventSources s;
    String name;
    processor->clearEventSources();
    triggerSelector->clear();
    gateSelector->clear();
    triggerSelector->addItem("Trigger", 1);
    gateSelector->addItem("Gate", 1);
    int nextItemTrig = 2;
    int nextItemGate = 2;
    int nEvents = processor->getTotalEventChannels();
    for (int i = 0; i < nEvents; i++)
    {
        const EventChannel* event = processor->getEventChannel(i);
        if (event->getChannelType() == EventChannel::TTL)
        {
            s.eventIndex = event->getSourceIndex();
            s.sourceId = event->getSourceNodeID();
            int nChans = event->getNumChannels();
            for (int c = 0; c < nChans; c++)
            {
                s.channel = c;
                name = event->getSourceName() + " " + String(event->getSourceIndex() + 1) + " (TTL" + String(c+1) + ")";
                processor->addEventSource(s);
                triggerSelector->addItem(name, nextItemTrig++);
                gateSelector->addItem(name, nextItemGate++);
            }
        }
    }

    if (m_triggerSelected > triggerSelector->getNumItems())
    {
        m_triggerSelected = triggerSelector->getNumItems();
        triggerSelector->setSelectedId(m_triggerSelected);
    }    
    else
        triggerSelector->setSelectedId(m_triggerSelected);
    if (m_gateSelected > triggerSelector->getNumItems())
    {
        m_gateSelected = gateSelector->getNumItems();
        gateSelector->setSelectedId(m_gateSelected);
    }
    else
        gateSelector->setSelectedId(m_gateSelected);
}

void ChannelTriggerInterface::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == triggerSelector)
    {
        processor->setParameter(0, channelNumber);
        processor->setParameter(1, (float) comboBoxThatHasChanged->getSelectedId() - 2);
        if (comboBoxThatHasChanged->getSelectedId() - 1 > 0)
            m_triggerSelected = comboBoxThatHasChanged->getSelectedId() - 1;
        else
            m_triggerSelected = 1;
    }
    else if (comboBoxThatHasChanged == gateSelector)
    {
        processor->setParameter(0, channelNumber);
        processor->setParameter(2, (float) comboBoxThatHasChanged->getSelectedId() - 2);
        if (comboBoxThatHasChanged->getSelectedId() - 1 > 0)
            m_gateSelected = comboBoxThatHasChanged->getSelectedId() - 1;
        else
            m_gateSelected = 1;    }
}

int ChannelTriggerInterface::getTriggerChannel()
{
    return triggerSelector->getSelectedId();
}


int ChannelTriggerInterface::getGateChannel()
{
    return gateSelector->getSelectedId();
}

void ChannelTriggerInterface::setTriggerChannel(int chan)
{
    m_triggerSelected = chan;
    triggerSelector->setSelectedId(chan);
}


void ChannelTriggerInterface::setGateChannel(int chan)
{
    m_gateSelected = chan;
    return gateSelector->setSelectedId(chan);
}
