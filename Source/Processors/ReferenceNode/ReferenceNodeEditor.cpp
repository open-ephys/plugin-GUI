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


#include "ReferenceNodeEditor.h"
#include "../ReferenceNode.h"
#include <stdio.h>


ReferenceNodeEditor::ReferenceNodeEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(0)

{
    desiredWidth = 180;

    referenceSelector = new ComboBox();
    referenceSelector->setBounds(15,50,150,25);
    referenceSelector->addListener(this);
    referenceSelector->addItem("None", 1);
    referenceSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(referenceSelector);

}

ReferenceNodeEditor::~ReferenceNodeEditor()
{

}

void ReferenceNodeEditor::updateSettings()
{

    if (getProcessor()->getNumInputs() != previousChannelCount)
    {
        referenceSelector->clear();

        referenceSelector->addItem("None", 1);

        for (int i = 0; i < getProcessor()->getNumInputs(); i++)
        {
            referenceSelector->addItem("Channel " + String(i+1), i+2);

        }

        previousChannelCount = getProcessor()->getNumInputs();

    }

    referenceSelector->setSelectedId(1, dontSendNotification);

    getProcessor()->setParameter(1,-1.0f);
}

void ReferenceNodeEditor::comboBoxChanged(ComboBox* c)
{
    float channel;

    int id = c->getSelectedId();

    if (id == 1)
    {
        channel = -1.0f;
    }
    else
    {
        channel = float(id) - 2.0f;
    }

    getProcessor()->setParameter(1,channel);

}

void ReferenceNodeEditor::buttonEvent(Button* button)
{


}

void ReferenceNodeEditor::saveEditorParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "ReferenceNodeEditor");

    XmlElement* selectedChannel = xml->createNewChildElement("SELECTEDID");

    selectedChannel->setAttribute("ID",referenceSelector->getSelectedId());

}

void ReferenceNodeEditor::loadEditorParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("SELECTEDID"))
        {

            int id = xmlNode->getIntAttribute("ID");

            referenceSelector->setSelectedId(id);

        }
    }
}