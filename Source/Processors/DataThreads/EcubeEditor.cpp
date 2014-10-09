/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys
Copyright (C) 2014 Michael Borisov

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

#include "EcubeEditor.h"

#include "EcubeThread.h"

#include <stdio.h>

EcubeEditor::EcubeEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
: GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 180;

    ipLabel = new Label("IP address label", "Address");
    ipLabel->setBounds(35, 80, 180, 20);
    ipLabel->setFont(Font("Small Text", 12, Font::plain));
    addAndMakeVisible(ipLabel);

    ipValue = new Label("IP address value", "127.0.0.1");
    ipValue->setBounds(40, 50, 60, 20);
    ipValue->setFont(Font("Default", 15, Font::plain));
    ipValue->setColour(Label::textColourId, Colours::white);
    ipValue->setColour(Label::backgroundColourId, Colours::grey);
    ipValue->setEditable(true);
    ipValue->addListener(this);
    addAndMakeVisible(ipValue);

}

EcubeEditor::~EcubeEditor()
{

}

void EcubeEditor::labelTextChanged(Label* label)
{/*
    FilterNode* fn = (FilterNode*)getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000)
    {
        sendActionMessage("Value out of range.");

        if (label == highCutValue)
        {
            label->setText(lastHighCutString, dontSendNotification);
            lastHighCutString = label->getText();
        }
        else
        {
            label->setText(lastLowCutString, dontSendNotification);
            lastLowCutString = label->getText();
        }

        return;
    }

    Array<int> chans = getActiveChannels();

    // This needs to change, since there's not enough feedback about whether
    // or not individual channel settings were altered:

    for (int n = 0; n < chans.size(); n++)
    {

        if (label == highCutValue)
        {
            double minVal = fn->getLowCutValueForChannel(n);

            if (requestedValue > minVal)
            {
                fn->setCurrentChannel(n);
                fn->setParameter(1, requestedValue);
            }

            lastHighCutString = label->getText();

        }
        else
        {
            double maxVal = fn->getHighCutValueForChannel(n);

            if (requestedValue < maxVal)
            {
                fn->setCurrentChannel(n);
                fn->setParameter(0, requestedValue);
            }

            lastLowCutString = label->getText();
        }

    }
*/
}


void EcubeEditor::buttonEvent(Button* button)
{

    if (!acquisitionIsActive)
    {

/*        if (button == fileButton)
        {
            //std::cout << "Button clicked." << std::endl;
            FileChooser chooseFileReaderFile("Please select the file you want to load...",
                lastFilePath,
                "*");

            if (chooseFileReaderFile.browseForFileToOpen())
            {
                // Use the selected file
                setFile(chooseFileReaderFile.getResult().getFullPathName());

                // lastFilePath = fileToRead.getParentDirectory();

                // thread->setFile(fileToRead.getFullPathName());

                // fileNameLabel->setText(fileToRead.getFileName(),false);
            }
        }
        */
    }
}

void EcubeEditor::saveEditorParameters(XmlElement* xml)
{

    // XmlElement* fileName = xml->createNewChildElement("FILENAME");
    // fileName->addTextElement(lastFilePath.getFullPathName());

}

void EcubeEditor::loadEditorParameters(XmlElement* xml)
{

    // forEachXmlChildElement(*xml, xmlNode)
    //    {
    //       if (xmlNode->hasTagName("FILENAME"))
    //       {

    //           lastFilePath = File(xmlNode->getText());
    //           thread->setFile(lastFilePath.getFullPathName());
    //           fileNameLabel->setText(lastFilePath.getFullPathName(),false);
    //       }
    //   }

}
