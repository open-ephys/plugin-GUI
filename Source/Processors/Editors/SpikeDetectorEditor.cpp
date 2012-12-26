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
#include "ChannelSelector.h"
#include "../../UI/EditorViewport.h"
#include <stdio.h>



SpikeDetectorEditor::SpikeDetectorEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode), isPlural(true)

{

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);

	desiredWidth = 300;

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

    numElectrodes = new Label("Number of Electrodes","1");
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
    plusButton->setRadius(3.0f);
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

    thresholdSlider = new ThresholdSlider(font);
    thresholdSlider->setBounds(200,35,75,75);
    addAndMakeVisible(thresholdSlider);
    thresholdSlider->addListener(this);
    thresholdSlider->setActive(false);
    Array<double> v;
    thresholdSlider->setValues(v);

    thresholdLabel = new Label("Name","Threshold");
    font.setHeight(10);
    thresholdLabel->setFont(font);
    thresholdLabel->setBounds(202, 105, 95, 15);
    thresholdLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(thresholdLabel);

    // create a custom channel selector
    deleteAndZero(channelSelector);

    channelSelector = new ChannelSelector(false, font);
    addChildComponent(channelSelector);
    channelSelector->setVisible(false);

  //  Array<int> a;

    channelSelector->inactivateButtons();
    channelSelector->paramButtonsToggledByDefault(false);
  //  channelSelector->paramButtonsActiveByDefault(false);

}

SpikeDetectorEditor::~SpikeDetectorEditor()
{

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        removeChildComponent(electrodeButtons[i]);
    }

	deleteAllChildren();	

}

void SpikeDetectorEditor::sliderEvent(Slider* slider)
{
    int electrodeNum = -1;

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeNum = i; //electrodeButtons[i]->getChannelNum()-1;
            break;
        }
    }

 //   std::cout << "Slider value changed." << std::endl;
    if (electrodeNum > -1) {
        SpikeDetector* processor = (SpikeDetector*) getProcessor();
        processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),
                                       electrodeNum,
                                       slider->getValue());
    }

}

void SpikeDetectorEditor::buttonEvent(Button* button)
{

    if (electrodeEditorButtons[0]->getToggleState()) // EDIT is active
    {

        std::cout << "Editing active." << std::endl;

        if (electrodeButtons.contains((ElectrodeButton*) button))
        {
            ElectrodeButton* eb = (ElectrodeButton*) button;
            int electrodeNum = eb->getChannelNum()-1;

            std::cout << "Channel number: " << electrodeNum << std::endl;
            Array<int> a;
            a.add(electrodeNum);
            channelSelector->setActiveChannels(a);

            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            thresholdSlider->setActive(true);
            thresholdSlider->setValue(processor->getChannelThreshold(electrodeList->getSelectedItemIndex(), 
                                                                    electrodeButtons.indexOf((ElectrodeButton*) button)));
        }
    }

    int num = numElectrodes->getText().getIntValue();

    if (button == upButton)
    {
        numElectrodes->setText(String(++num), true);

        return;

    } else if (button == downButton)
    {

        if (num > 1)
            numElectrodes->setText(String(--num), true);

        return;

    } else if (button == plusButton)
    {
       // std::cout << "Plus button pressed!" << std::endl;

        int type = electrodeTypes->getSelectedId();
        std::cout << type << std::endl;
        int nChans;

        switch (type)
        {
            case 1:
                nChans = 1; break;
            case 2:
                nChans = 2; break;
            case 3:
                nChans = 4; break;
            default:
                nChans = 1;
        }
       
        for (int n = 0; n < num; n++)
        {
            if (!addElectrode(nChans))
            {
                sendActionMessage("Not enough channels to add electrode.");
            } 
        }

        refreshElectrodeList();

        electrodeList->setSelectedId(electrodeList->getNumItems(), true);
        electrodeList->setText(electrodeList->getItemText(electrodeList->getNumItems()-1));
        lastId = electrodeList->getNumItems();
        electrodeList->setEditableText(true);

        drawElectrodeButtons(electrodeList->getNumItems()-1);

        getEditorViewport()->makeEditorVisible(this, true, true);
        return;

    } else if (button == electrodeEditorButtons[0]) // EDIT
    {

        Array<int> activeChannels;

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (button->getToggleState())
            {
                electrodeButtons[i]->setToggleState(false, false);
                electrodeButtons[i]->setRadioGroupId(299);
                channelSelector->activateButtons();
                channelSelector->setRadioStatus(true);
            } else {
                electrodeButtons[i]->setToggleState(true, false);
                electrodeButtons[i]->setRadioGroupId(0);
                channelSelector->inactivateButtons();
                channelSelector->setRadioStatus(false);
                activeChannels.add(electrodeButtons[i]->getChannelNum()-1);
            }
        }


        if (!button->getToggleState())
        {
            thresholdSlider->setActive(false);

            // This will be -1 with nothing selected
            int selectedItemIndex = electrodeList->getSelectedItemIndex();
            if (selectedItemIndex != -1)
            {
                drawElectrodeButtons(selectedItemIndex);
            } else {
                electrodeButtons.clear();
            }
        }

      //   channelSelector->setActiveChannels(activeChannels);

        return;

    } else if (button == electrodeEditorButtons[1]) // MONITOR
    {
        return;
    } else if (button == electrodeEditorButtons[2]) // DELETE
    {

        removeElectrode(electrodeList->getSelectedItemIndex());

        getEditorViewport()->makeEditorVisible(this, true, true);

        return;
    }



}

void SpikeDetectorEditor::channelChanged(int chan)
{
    //std::cout << "New channel: " << chan << std::endl;

     for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (electrodeButtons[i]->getToggleState())
            {
                electrodeButtons[i]->setChannelNum(chan);
                electrodeButtons[i]->repaint();

                SpikeDetector* processor = (SpikeDetector*) getProcessor();
                processor->setChannel(electrodeList->getSelectedItemIndex(),
                                      i,
                                      chan-1);
            }
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

bool SpikeDetectorEditor::addElectrode(int nChans)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    return processor->addElectrode(nChans);
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

            processor->setElectrodeName(lastId, comboBox->getText());
            refreshElectrodeList();

        } else {

            lastId = ID;

            drawElectrodeButtons(ID-1);

        }
    }

    thresholdSlider->setActive(false);
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

    Array<int> activeChannels;
    Array<double> thresholds;

    for (int i = 0; i < numChannels; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(processor->getChannel(ID,i)+1);
        electrodeButtons.add(button);

        thresholds.add(processor->getChannelThreshold(ID,i));

        if (electrodeEditorButtons[0]->getToggleState())
        {
            button->setToggleState(false, false);
            button->setRadioGroupId(299);
        } else {
            activeChannels.add(processor->getChannel(ID,i));
        }
        
        if (numChannels < 3)
            button->setBounds(145+(column++)*width, 78+row*height, width, 15);
        else 
            button->setBounds(145+(column++)*width, 70+row*height, width, 15);

        addAndMakeVisible(button);
        button->addListener(this);

        if (column%2 == 0)
        {
            column = 0;
            row++;
        }

    }

    channelSelector->setActiveChannels(activeChannels);
    thresholdSlider->setValues(thresholds);
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


ThresholdSlider::ThresholdSlider(Font f) : Slider("name"), font(f)
{

    setSliderStyle(Slider::Rotary);
    setRange(25.0f,400.0f,25.0f);
    setValue(75.0f);
    setTextBoxStyle(Slider::NoTextBox, false, 40, 20);

}

void ThresholdSlider::paint(Graphics& g)
{

    ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,
                                  Colour(80, 80, 80), 0.0, 40.0f, false);

    Path p;
    p.addPieSegment(3, 3, getWidth()-6, getHeight()-6, 5*double_Pi/4-0.2, 5*double_Pi/4+3*double_Pi/2+0.2, 0.5);

    g.setGradientFill(grad);
    g.fillPath(p);

    String valueString;

    if (isActive) {
        p = makeRotaryPath(getMinimum(), getMaximum(), getValue());
        g.setColour(Colour(240,179,12));
        g.fillPath(p);

        valueString = String( (int) getValue());
    } else {

        valueString = "";

        for (int i = 0; i < valueArray.size(); i++)
        {
            p = makeRotaryPath(getMinimum(), getMaximum(), valueArray[i]);
            g.setColour(Colours::lightgrey.withAlpha(0.4f));
            g.fillPath(p);
            valueString = String((int) valueArray.getLast());
        }
        
    }
    
    font.setHeight(9.0);
        g.setFont(font);
    int stringWidth = font.getStringWidth(valueString);

    g.setFont(font);

    g.setColour(Colours::darkgrey);
    g.drawSingleLineText(valueString, getWidth()/2 - stringWidth/2, getHeight()/2+3);

}

Path ThresholdSlider::makeRotaryPath(double min, double max, double val)
{

    Path p;

    double start = 5*double_Pi/4 - 0.11;

    double range = (val-min)/(max - min)*1.5*double_Pi + start + 0.22;

    p.addPieSegment(6,6, getWidth()-12, getHeight()-12, start, range, 0.65);

    return p;

}

void ThresholdSlider::setActive(bool t)
{
    isActive = t;
    repaint();
}

void ThresholdSlider::setValues(Array<double> v)
{
    valueArray = v;
}