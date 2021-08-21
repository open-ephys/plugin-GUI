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

#include "SpikeDetectorEditor.h"
#include "SpikeDetector.h"
#include <stdio.h>



SpikeDetectorEditor::SpikeDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), isPlural(true)

{
	int silksize;
	//const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
   // MemoryInputStream mis(silk, silksize, false);
    //Typeface::Ptr typeface = new CustomTypeface(mis);
    //font = Font(typeface);

    desiredWidth = 300;

    electrodeTypes = std::make_unique<ComboBox>("Electrode Types");

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    for (int i = 0; i < processor->electrodeTypes.size(); i++)
    {
        String type = processor->electrodeTypes[i];
        electrodeTypes->addItem(type += "s", i+1);
    }

    electrodeTypes->setEditableText(false);
    electrodeTypes->setJustificationType(Justification::centredLeft);
    electrodeTypes->addListener(this);
    electrodeTypes->setBounds(65,40,110,20);
    electrodeTypes->setSelectedId(2);
    addAndMakeVisible(electrodeTypes.get());

    electrodeList = std::make_unique<ComboBox>("Electrode List");
    electrodeList->setEditableText(false);
    electrodeList->setJustificationType(Justification::centredLeft);
    electrodeList->addListener(this);
    electrodeList->setBounds(15,75,115,20);
    addAndMakeVisible(electrodeList.get());

    numElectrodes = std::make_unique<Label>("Number of Electrodes","1");
    numElectrodes->setEditable(true);
    numElectrodes->addListener(this);
    numElectrodes->setBounds(30,40,25,20);
    //labelTextChanged(numElectrodes);
    addAndMakeVisible(numElectrodes.get());

    upButton = std::make_unique<TriangleButton>(1);
    upButton->addListener(this);
    upButton->setBounds(50,40,10,8);
    addAndMakeVisible(upButton.get());

    downButton = std::make_unique<TriangleButton>(2);
    downButton->addListener(this);
    downButton->setBounds(50,50,10,8);
    addAndMakeVisible(downButton.get());

    plusButton = std::make_unique<UtilityButton>("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(15,42,14,14);
    addAndMakeVisible(plusButton.get());

    editorFont = Font("Small Text", 10, Font::FontStyleFlags::plain); 

    std::unique_ptr<ElectrodeEditorButton> e1 = std::make_unique<ElectrodeEditorButton>("EDIT", editorFont);
    e1->addListener(this);
    addAndMakeVisible(e1.get());
    e1->setBounds(15,110,40,10);
    electrodeEditorButtons.push_back(std::move(e1));

    std::unique_ptr<ElectrodeEditorButton> e2 = std::make_unique<ElectrodeEditorButton>("MONITOR", editorFont);
    e2->addListener(this);
    addAndMakeVisible(e2.get());
    e2->setBounds(55,110,70,10);
    electrodeEditorButtons.push_back(std::move(e2));

    std::unique_ptr<ElectrodeEditorButton> e3 = std::make_unique<ElectrodeEditorButton>("DELETE", editorFont);;
    e3->addListener(this);
    addAndMakeVisible(e3.get());
    e3->setBounds(130,110,70,10);
    electrodeEditorButtons.push_back(std::move(e3));

    thresholdSlider = std::make_unique<ThresholdSlider>(editorFont);
    thresholdSlider->setBounds(200,35,75,75);
    addAndMakeVisible(thresholdSlider.get());
    thresholdSlider->addListener(this);
    thresholdSlider->setActive(false);
    Array<double> v;
    thresholdSlider->setValues(v);

    thresholdLabel = std::make_unique<Label>("Name","Threshold");
    thresholdLabel->setFont(editorFont);
    thresholdLabel->setBounds(202, 105, 95, 15);
    thresholdLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(thresholdLabel.get());

    // create a custom channel selector
    //deleteAndZero(channelSelector);

    // channelSelector = new ChannelSelector(false, font);
    //  addChildComponent(channelSelector);
    // channelSelector->setVisible(false);
    //
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
    if (electrodeNum > -1)
    {
        SpikeDetector* processor = (SpikeDetector*) getProcessor();
        processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),
                                       electrodeNum,
                                       slider->getValue());
    }

}


void SpikeDetectorEditor::buttonEvent(Button* button)
{


    if (electrodeButtons.contains((ElectrodeButton*) button))
    {

        if (electrodeEditorButtons[0]->getToggleState()) // EDIT is active
        {
            ElectrodeButton* eb = (ElectrodeButton*) button;
            int electrodeNum = eb->getChannelNum()-1;

            // std::cout << "Channel number: " << electrodeNum << std::endl;
            Array<int> a;
            a.add(electrodeNum);
            channelSelector->setActiveChannels(a);

            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            thresholdSlider->setActive(true);
            thresholdSlider->setValue(processor->getChannelThreshold(electrodeList->getSelectedItemIndex(),
                                                                     electrodeButtons.indexOf((ElectrodeButton*) button)));
        }
        else
        {

            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            ElectrodeButton* eb = (ElectrodeButton*) button;
            int electrodeNum = electrodeList->getSelectedItemIndex();
            int channelNum = electrodeButtons.indexOf(eb);

            processor->setChannelActive(electrodeNum,
                                        channelNum,
                                        button->getToggleState());

            std::cout << "Disabling channel " << channelNum <<
                      " of electrode " << electrodeNum << std::endl;

        }


    }


    int num = numElectrodes->getText().getIntValue();

    if (button == upButton.get())
    {
        numElectrodes->setText(String(++num), sendNotification);

        return;

    }
    else if (button == downButton.get())
    {

        if (num > 1)
            numElectrodes->setText(String(--num), sendNotification);

        return;

    }
    else if (button == plusButton.get())
    {
        // std::cout << "Plus button pressed!" << std::endl;
        if (acquisitionIsActive)
        {
            CoreServices::sendStatusMessage("Stop acquisition before adding electrodes.");
            return;
        }

        int type = electrodeTypes->getSelectedId();
        // std::cout << type << std::endl;
        int nChans;

        switch (type)
        {
            case 1:
                nChans = 1;
                break;
            case 2:
                nChans = 2;
                break;
            case 3:
                nChans = 4;
                break;
            default:
                nChans = 1;
        }

        for (int n = 0; n < num; n++)
        {
            if (!addElectrode(nChans))
            {
                CoreServices::sendStatusMessage("Not enough channels to add electrode.");
            }
        }

        electrodeEditorButtons[1]->setToggleState(false, dontSendNotification);

		CoreServices::updateSignalChain(this);
		CoreServices::highlightEditor(this);
        return;

    }
    else if (button == electrodeEditorButtons[0].get())   // EDIT
    {

        Array<int> activeChannels;

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (button->getToggleState())
            {
                electrodeButtons[i]->setToggleState(false, dontSendNotification);
                electrodeButtons[i]->setRadioGroupId(299);
                channelSelector->activateButtons();
                channelSelector->setRadioStatus(true);
            }
            else
            {
                electrodeButtons[i]->setToggleState(true, dontSendNotification);
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
            }
            else
            {
                electrodeButtons.clear();
            }
        }

        //   channelSelector->setActiveChannels(activeChannels);

        return;

    }
    else if (button == electrodeEditorButtons[1].get())   // MONITOR
    {

       /*Button* audioMonitorButton = electrodeEditorButtons[1];

        //channelSelector->clearAudio();

        SpikeDetector* processor = (SpikeDetector*) getProcessor();

		Array<SimpleElectrode*> electrodes;
		processor->getElectrodes(electrodes);

        for (int i = 0; i < electrodes.size(); i++)
        {
            SimpleElectrode* e = electrodes[i];
            e->isMonitored = false;
        }

        SimpleElectrode* e = processor->getActiveElectrode();

        if (e != nullptr)
        {

            e->isMonitored = audioMonitorButton->getToggleState();

            for (int i = 0; i < e->numChannels; i++)
            {
                std::cout << "Channel " << e->channels[i] << std::endl;
                int channelNum = e->channels[i];
                channelSelector->setAudioStatus(channelNum, audioMonitorButton->getToggleState());

            }
        }
        else
        {
            audioMonitorButton->setToggleState(false, dontSendNotification);
        }*/

        return;
    }
    else if (button == electrodeEditorButtons[2].get())   // DELETE
    {
        if (acquisitionIsActive)
        {
            CoreServices::sendStatusMessage("Stop acquisition before deleting electrodes.");
            return;
        }

        removeElectrode(electrodeList->getSelectedItemIndex());

		CoreServices::updateSignalChain(this);
		CoreServices::highlightEditor(this);

        return;
    }



}

void SpikeDetectorEditor::channelChanged (int channel, bool /*newState*/)
{

    if (electrodeEditorButtons[0]->getToggleState()) // editing is active
    {
        //std::cout << "New channel: " << chan << std::endl;

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (electrodeButtons[i]->getToggleState())
            {
                electrodeButtons[i]->setChannelNum (channel);
                electrodeButtons[i]->repaint();

                SpikeDetector* processor = (SpikeDetector*) getProcessor();
                processor->setChannel(electrodeList->getSelectedItemIndex(),
                                      i,
                                      channel - 1);
            }
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
        electrodeList->addItem(electrodeNames[i], electrodeList->getNumItems()+1);
    }

    if (electrodeList->getNumItems() > 0)
    {
        electrodeList->setSelectedId(electrodeList->getNumItems(), sendNotification);
        electrodeList->setText(electrodeList->getItemText(electrodeList->getNumItems()-1));
        lastId = electrodeList->getNumItems();
        electrodeList->setEditableText(true);

        drawElectrodeButtons(electrodeList->getNumItems()-1);
    }
}

bool SpikeDetectorEditor::addElectrode(int nChans, int electrodeID)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    if (processor->addElectrode(nChans, electrodeID))
    {
        refreshElectrodeList();
        return true;
    }
    else
    {
        return false;
    }

}


void SpikeDetectorEditor::removeElectrode(int index)
{
    std::cout << "Deleting electrode number " << index << std::endl;
    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    processor->removeElectrode(index);
    refreshElectrodeList();

    int newIndex = jmin(index, electrodeList->getNumItems()-1);
    newIndex = jmax(newIndex, 0);

    electrodeList->setSelectedId(newIndex, sendNotification);
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

    }
    else if (!label->getText().equalsIgnoreCase("1") && !isPlural)
    {
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

	CoreServices::updateSignalChain(this);

}

void SpikeDetectorEditor::comboBoxChanged(ComboBox* comboBox)
{

    if (comboBox == electrodeList.get())
    {
        int ID = comboBox->getSelectedId();

        std::cout << "ID: " << ID << std::endl;

        if (ID == 0)
        {
            //SpikeDetector* processor = (SpikeDetector*) getProcessor();

            //processor->setElectrodeName(lastId, comboBox->getText());
            //comboBox->changeItemText(lastId, comboBox->getText());
            //electrodeList->setText(comboBox->getText());
            refreshElectrodeList();

        }
        else
        {

            lastId = ID;

            SpikeDetector* processor = (SpikeDetector*) getProcessor();
            SimpleElectrode* e = processor->setCurrentElectrodeIndex(ID-1);

            electrodeEditorButtons[1]->setToggleState(e->isMonitored, dontSendNotification);

            drawElectrodeButtons(ID-1);

        }
    }

    thresholdSlider->setActive(false);
}

void SpikeDetectorEditor::checkSettings()
{
    electrodeList->setSelectedId(0);
    drawElectrodeButtons(0);

	CoreServices::updateSignalChain(this);
	CoreServices::highlightEditor(this);

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
            button->setToggleState(false, dontSendNotification);
            button->setRadioGroupId(299);
        }
        else
        {
            activeChannels.add(processor->getChannel(ID,i));

            button->setToggleState(processor->isChannelActive(ID,i), dontSendNotification);
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
