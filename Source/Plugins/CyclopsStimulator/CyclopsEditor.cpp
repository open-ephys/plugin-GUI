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

#include "CyclopsEditor.h"

CyclopsEditor::CyclopsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : VisualizerEditor   (parentNode, 240, useDefaultParameterEditors)
    //, progress(0, 1.0, 1000)
    , progress(0)
    , in_a_test(false)
{
    node = (CyclopsProcessor*)parentNode;
    tabText = "Cyclops";
    // Add "port" list
    portList = new ComboBox();
    portList->setBounds(desiredWidth-13-60, 39, 60, 18);
    portList->addListener(this);
    portList->setTooltip("Select the serial port connected to Cyclops.");
    portList->addItemList(node->getDevices(), 1);
    addAndMakeVisible(portList);

    // Add baudrate list
    baudrateList = new ComboBox();
    baudrateList->setBounds(desiredWidth-13-60, 63, 60, 18);
    baudrateList->addListener(this);
    baudrateList->setTooltip("Set the baud rate (115200 recommended).");

    Array<int> baudrates(node->getBaudrates());
    for (int i = 0; i < baudrates.size(); i++)
    {
        baudrateList->addItem(String(baudrates[i]), baudrates[i]);
    }
    addAndMakeVisible(baudrateList);

    // Add refresh button
    refreshButton = new UtilityButton("R", Font("Default", 9, Font::plain));
    refreshButton->setRadius(3.0f);
    refreshButton->setBounds(145, 51, 20, 20);
    refreshButton->addListener(this);
    addAndMakeVisible(refreshButton);

    // Add TEST buttons
    for (int i=0; i < 4; i++){
        testButtons.add(new UtilityButton(String("Test") + String(i), Font("Default", 10, Font::bold)));
        testButtons[i]->setBounds(7+(58*i), 104, 40, 20);
        testButtons[i]->addListener(this);
        addAndMakeVisible(testButtons[i]);
    }

    //progressBar = new ProgressBar(progress.var);
    progressBar = new ProgressBar(progress);
    progressBar->setPercentageDisplay(false);
    progressBar->setBounds(2, 106, 236, 16);
    addChildComponent(progressBar);

    pstep = 0.01;
}

CyclopsEditor::~CyclopsEditor()
{
    ;
}


Visualizer* CyclopsEditor::createNewCanvas()
{
    //GenericProcessor* audio_processor = (GenericProcessor*) getProcessor();
    return new CyclopsCanvas(node);
}

/**
The listener methods that reacts to the button click. The same method is called for all buttons
on the editor, so the button variable, which cointains a pointer to the button that called the method
has to be checked to know which function to perform.
*/
void CyclopsEditor::buttonCallback(Button* button)
{
    if (button == refreshButton)
    {
        // Refresh list of devices
        portList->clear();
        portList->addItemList(node->getDevices(), 1);
        GenericEditor::repaint();
    }

    int test_index = -1;
    for (int i=0; i < 4; i++){
        if (button == testButtons[i]){
            test_index = i;
            break;
        }
    }
    if (test_index >= 0){
        disableAllInputWidgets();
        std::cout << "Testing LED channel " << test_index << "\n";
        in_a_test = true;
        //node->testChannel(test_index);
        progressBar->setVisible(true);
        startTimer(20);
        test_index = -1;
    }
}

void CyclopsEditor::comboBoxChanged(ComboBox* comboBox)
{
    // Push new selection to parent node
    if (comboBox == portList)
    {
        node->setDevice(comboBox->getText().toStdString());
    }
    else if (comboBox == baudrateList)
    {
        node->setBaudrate(comboBox->getSelectedId());
    }
}

void CyclopsEditor::timerCallback()
{
    if (in_a_test){
        progress += pstep;
        if (progress >= 1.0){
            progressBar->setVisible(false);
            progress = 0;
            in_a_test = false;
            stopTimer();
            enableAllInputWidgets();
        }
    }
}

void CyclopsEditor::paint(Graphics& g)
{
    GenericEditor::paint(g);
    g.setColour(Colour(193, 208, 69));
    g.fillEllipse(170, 7, 10, 10);
    g.setColour(Colour(0, 0, 0));
    g.drawEllipse(169, 6, 12, 12, 1);

    g.setColour(Colour(193, 208, 69));
    g.fillEllipse(184, 7, 10, 10);
    g.setColour(Colour(0, 0, 0));
    g.drawEllipse(183, 6, 12, 12, 1);
}

void CyclopsEditor::disableAllInputWidgets()
{
    // Disable the whole gui
    portList->setEnabled(false);
    baudrateList->setEnabled(false);
    refreshButton->setEnabled(false);
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(false);
}

void CyclopsEditor::enableAllInputWidgets()
{
    // Reenable the whole gui
    portList->setEnabled(true);
    baudrateList->setEnabled(true);
    refreshButton->setEnabled(true);
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(true);
}

void CyclopsEditor::startAcquisition()
{
    disableAllInputWidgets();
    GenericEditor::startAcquisition();
}

void CyclopsEditor::stopAcquisition()
{
    enableAllInputWidgets();
    GenericEditor::stopAcquisition();
}

void CyclopsEditor::updateSettings()
{
    ;
}

void CyclopsEditor::saveEditorParameters(XmlElement* xmlNode)
{
    XmlElement* parameters = xmlNode->createNewChildElement("PARAMETERS");

    parameters->setAttribute("device", portList->getText().toStdString());
    parameters->setAttribute("baudrate", baudrateList->getSelectedId());
}

void CyclopsEditor::loadEditorParameters(XmlElement* xmlNode)
{
    forEachXmlChildElement(*xmlNode, subNode)
    {
        if (subNode->hasTagName("PARAMETERS"))
        {
            portList->setText(subNode->getStringAttribute("device", ""));
            baudrateList->setSelectedId(subNode->getIntAttribute("baudrate"));
        }
    }
}
