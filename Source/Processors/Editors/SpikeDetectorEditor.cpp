/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#include "SpikeDetectorEditor.h"
#include "../SpikeDetector.h"
#include "../../UI/EditorViewport.h"
#include <stdio.h>


SpikeDetectorEditor::SpikeDetectorEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode), isPlural(true)

{

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);

	desiredWidth = 370;

    electrodeTypes = new ComboBox("Electrode Types");

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    for (int i = 0; i < processor->electrodeTypes.size(); i++)
    {
        String type = processor->electrodeTypes[i];
        electrodeTypes->addItem (type += "s", i+1); 
    }

    electrodeTypes->setEditableText(false);
    electrodeTypes->setJustificationType (Justification::centredLeft);
    electrodeTypes->addListener(this);
    electrodeTypes->setBounds(65,40,110,20);
    electrodeTypes->setSelectedId(2);
    addAndMakeVisible(electrodeTypes);

    electrodeList = new ComboBox("Electrode List");
    electrodeList->setEditableText(false);
    electrodeList->setJustificationType (Justification::centredLeft);
    electrodeList->addListener(this);
    electrodeList->setBounds(15,75,115,20);
    addAndMakeVisible(electrodeList);

    numElectrodes = new Label("Number of Electrodes","2");
    numElectrodes->setEditable(true);
    numElectrodes->addListener(this);
    numElectrodes->setBounds(30,40,25,20);
    //labelTextChanged(numElectrodes);
    addAndMakeVisible(numElectrodes);

    upButton = new TriangleButton(1);
    upButton->addListener(this);
    upButton->setBounds(50,40,10,8);
    addAndMakeVisible(upButton);

    downButton = new TriangleButton(2);
    downButton->addListener(this);
    downButton->setBounds(50,50,10,8);
    addAndMakeVisible(downButton);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setBounds(15,42,14,14);
    addAndMakeVisible(plusButton);

    ElectrodeEditorButton* e1 = new ElectrodeEditorButton("EDIT",font);
    e1->addListener(this);
    addAndMakeVisible(e1);
    e1->setBounds(15,110,40,10);
    electrodeEditorButtons.add(e1);

    ElectrodeEditorButton* e2 = new ElectrodeEditorButton("MONITOR",font);
    e2->addListener(this);
    addAndMakeVisible(e2);
    e2->setBounds(55,110,70,10);
    electrodeEditorButtons.add(e2);

    ElectrodeEditorButton* e3 = new ElectrodeEditorButton("DELETE",font);
    e3->addListener(this);
    addAndMakeVisible(e3);
    e3->setBounds(130,110,70,10);
    electrodeEditorButtons.add(e3);


}

SpikeDetectorEditor::~SpikeDetectorEditor()
{

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        removeChildComponent(electrodeButtons[i]);
    }

	deleteAllChildren();	

}

void SpikeDetectorEditor::buttonEvent(Button* button)
{

    int num = numElectrodes->getText().getIntValue();

    if (button == upButton)
    {
        numElectrodes->setText(String(++num), true);

    } else if (button == downButton)
    {

        if (num > 1)
            numElectrodes->setText(String(--num), true);

    } else if (button == plusButton)
    {
       // std::cout << "Plus button pressed!" << std::endl;
       
        for (int n = 0; n < num; n++)
        {
            addElectrode(electrodeTypes->getSelectedId()); 
        }

        refreshElectrodeList();

        electrodeList->setSelectedId(electrodeList->getNumItems(), true);
        electrodeList->setText(electrodeList->getItemText(electrodeList->getNumItems()-1));
        lastId = electrodeList->getNumItems();
        electrodeList->setEditableText(true);

        drawElectrodeButtons(electrodeList->getNumItems()-1);

        getEditorViewport()->makeEditorVisible(this);

    } else if (button == electrodeEditorButtons[0]) // EDIT
    {

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (button->getToggleState())
            {
                electrodeButtons[i]->setToggleState(false, false);
                electrodeButtons[i]->setRadioGroupId(299);
            } else {
                electrodeButtons[i]->setToggleState(true, false);
                electrodeButtons[i]->setRadioGroupId(0);
            }
        }


    } else if (button == electrodeEditorButtons[1]) // MONITOR
    {

    } else if (button == electrodeEditorButtons[2]) // DELETE
    {

        removeElectrode(electrodeList->getSelectedItemIndex());
    }
}

void SpikeDetectorEditor::refreshElectrodeList()
{
    electrodeList->clear();

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    StringArray electrodeNames = processor->getElectrodeNames();

    for (int i = 0; i < electrodeNames.size(); i++)
    {
        electrodeList->addItem (electrodeNames[i], electrodeList->getNumItems()+1);
    }
}

void SpikeDetectorEditor::addElectrode(int nChans)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    processor->addElectrode(nChans);
}


void SpikeDetectorEditor::removeElectrode(int index)
{
    std::cout << "Deleting electrode number " << index << std::endl;
    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    processor->removeElectrode(index);
    refreshElectrodeList();

    int newIndex = jmin(index, electrodeList->getNumItems()-1);
    newIndex = jmax(newIndex, 0);

    electrodeList->setSelectedId(newIndex, true);
    electrodeList->setText(electrodeList->getItemText(newIndex));

    if (electrodeList->getNumItems() == 0)
    {
        electrodeButtons.clear();
        electrodeList->setEditableText(false);
    }
}

void SpikeDetectorEditor::labelTextChanged(Label* label)
{
    if (label->getText().equalsIgnoreCase("1") && isPlural)
    {
        for (int n = 1; n < electrodeTypes->getNumItems()+1; n++)
        {
            electrodeTypes->changeItemText(n,
                    electrodeTypes->getItemText(n-1).trimCharactersAtEnd("s"));
        }

        isPlural = false;

        String currentText = electrodeTypes->getText();
        electrodeTypes->setText(currentText.trimCharactersAtEnd("s"));

    } else if (!label->getText().equalsIgnoreCase("1") && !isPlural)
    {
        const String s = "s";
        size_t one = 1;

        for (int n = 1; n < electrodeTypes->getNumItems()+1; n++)
        {
            String currentString = electrodeTypes->getItemText(n-1);
            currentString += "s";

            electrodeTypes->changeItemText(n,currentString);
        }
        isPlural = true;

        String currentText = electrodeTypes->getText();
        electrodeTypes->setText(currentText += "s");
    }

}

void SpikeDetectorEditor::comboBoxChanged(ComboBox* comboBox)
{
    
    if (comboBox == electrodeList)
    {
        int ID = comboBox->getSelectedId();

        if (ID == 0)
        {
            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            processor->setName(lastId, comboBox->getText());
            refreshElectrodeList();

        } else {

            lastId = ID;

            drawElectrodeButtons(ID-1);

        }

    }
}

void SpikeDetectorEditor::drawElectrodeButtons(int ID)
{

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    electrodeButtons.clear();

    int width = 20;
    int height = 15;

    int numChannels = processor->getNumChannels(ID);
    int row = 0;
    int column = 0;

    for (int i = 0; i < numChannels; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(processor->getChannel(ID,i));
        electrodeButtons.add(button);
        
        button->setBounds(150+(column++)*width, 80+row*height, width, 15);
        addAndMakeVisible(button);

        if (column%5 == 0)
        {
            column = 0;
            row++;
        }

    }

}



   
void ElectrodeButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else 
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    g.fillRect(0,0,getWidth(),getHeight());

   // g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    g.drawText(String(chan),0,0,getWidth(),getHeight(),Justification::centred,true);
}

   
void ElectrodeEditorButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::darkgrey);
    else 
        g.setColour(Colours::lightgrey);

    g.setFont(font);

    g.drawText(name,0,0,getWidth(),getHeight(),Justification::left,true);
}

