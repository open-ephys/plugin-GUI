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

#include "SpikeSorterEditor.h"
#include "SpikeSorterCanvas.h"
#include "SpikeSorter.h"

#include <stdio.h>



SpikeSorterEditor::SpikeSorterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, 300, useDefaultParameterEditors), spikeSorterCanvas(nullptr), isPlural(true)

{
    tabText = "Spike Detector";

	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
    MemoryInputStream mis(silk, silksize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);

    desiredWidth = 300;

    //SpikeSorter* processor = (SpikeSorter*) getProcessor();

    advancerList = new ComboBox("Advancers");
    advancerList->addListener(this);
    advancerList->setBounds(10,95,130,20);
    addAndMakeVisible(advancerList);

    depthOffsetLabel = new Label("Depth Offset","Depth Offset");
    depthOffsetLabel->setFont(Font("Default", 10, Font::plain));
    depthOffsetLabel->setEditable(false);
    depthOffsetLabel->setBounds(125,115,80,20);
    depthOffsetLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(depthOffsetLabel);

    advancerLabel = new Label("Depth Offset","ADVANCER:");
    advancerLabel->setFont(Font("Default", 10, Font::plain));
    advancerLabel->setEditable(false);
    advancerLabel->setBounds(10,80,80,20);
    advancerLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(advancerLabel);

    depthOffsetEdit = new Label("Depth Offset","0.0");
    depthOffsetEdit->setFont(Font("Default", 10, Font::plain));
    depthOffsetEdit->setEditable(true);
    depthOffsetEdit->setBounds(145,95,40,20);
    depthOffsetEdit->addListener(this);
    depthOffsetEdit->setColour(Label::textColourId, Colours::white);
    depthOffsetEdit->setColour(Label::backgroundColourId, Colours::grey);

    addAndMakeVisible(depthOffsetEdit);

    electrodeList = new ComboBox("Electrode List");
    electrodeList->setEditableText(false);
    electrodeList->setJustificationType(Justification::centredLeft);
    electrodeList->addListener(this);
    //electrodeList->setBounds(65,30,130,20);
    electrodeList->setBounds(65,30,130,20);
    addAndMakeVisible(electrodeList);

    numElectrodes = new Label("Number of Electrodes","1");
    numElectrodes->setEditable(true);
    numElectrodes->addListener(this);
    numElectrodes->setBounds(30,30,25,20);
    addAndMakeVisible(numElectrodes);

    upButton = new TriangleButton(1);
    upButton->addListener(this);
    upButton->setBounds(50,30,10,8);
    addAndMakeVisible(upButton);

    downButton = new TriangleButton(2);
    downButton->addListener(this);
    downButton->setBounds(50,40,10,8);
    addAndMakeVisible(downButton);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(15,27,14,14);
    addAndMakeVisible(plusButton);

    audioMonitorButton = new UtilityButton("MONITOR", Font("Default", 12, Font::plain));
    audioMonitorButton->addListener(this);
    audioMonitorButton->setRadius(3.0f);
    audioMonitorButton->setBounds(80,65,65,15);
    audioMonitorButton->setClickingTogglesState(true);
    addAndMakeVisible(audioMonitorButton);

    removeElectrodeButton = new UtilityButton("-",font);
    removeElectrodeButton->addListener(this);
    removeElectrodeButton->setBounds(15,45,14,14);
    addAndMakeVisible(removeElectrodeButton);


    configButton = new UtilityButton("CONFIG",Font("Default", 12, Font::plain));
    configButton->addListener(this);
    configButton->setBounds(10,65,60,15);
    addAndMakeVisible(configButton);

    thresholdSlider = new ThresholdSlider(font);
    thresholdSlider->setBounds(210,25,65,65);
    addAndMakeVisible(thresholdSlider);
    thresholdSlider->addListener(this);
    thresholdSlider->setActive(false);
    Array<double> v;
    thresholdSlider->setValues(v);

    thresholdLabel = new Label("Name","Threshold");
    font.setHeight(10);
    thresholdLabel->setFont(font);
    thresholdLabel->setBounds(208, 85, 95, 15);
    thresholdLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(thresholdLabel);

    // create a custom channel selector
    deleteAndZero(channelSelector);

    channelSelector = new ChannelSelector(true, font);
    addChildComponent(channelSelector);
    channelSelector->setVisible(false);

    channelSelector->activateButtons();
    channelSelector->setRadioStatus(true);
    channelSelector->paramButtonsToggledByDefault(false);
    //	updateAdvancerList();

 /*   dacAssignmentLabel= new Label("DAC output","DAC output");
    dacAssignmentLabel->setFont(Font("Default", 10, Font::plain));
    dacAssignmentLabel->setEditable(false);
    dacAssignmentLabel->setBounds(210,115,80,20);
    dacAssignmentLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(dacAssignmentLabel);

    dacCombo = new ComboBox("DAC Assignment");
    dacCombo->addListener(this);
    dacCombo->setBounds(205,100,70,18);
    dacCombo->addItem("-",1);
    for (int k=0; k<8; k++)
    {
        dacCombo->addItem("DAC"+String(k+1),k+2);
    }
    dacCombo->setSelectedId(1);
    addAndMakeVisible(dacCombo);*/

}

Visualizer* SpikeSorterEditor::createNewCanvas()
{

    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    spikeSorterCanvas = new SpikeSorterCanvas(processor);
    //ActionListener* listener = (ActionListener*) SpikeSorterCanvas;
    //getUIComponent()->registerAnimatedComponent(listener);
    return spikeSorterCanvas;
}


SpikeSorterEditor::~SpikeSorterEditor()
{

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        removeChildComponent(electrodeButtons[i]);
    }

}

void SpikeSorterEditor::sliderEvent(Slider* slider)
{
    int electrodeNum = -1;

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeNum = i;
            break;
        }
    }

    if (electrodeNum > -1)
    {
        // new
        SpikeSorter* processor = (SpikeSorter*) getProcessor();
        if (processor->getEditAllState()){
            int numElectrodes = processor->getNumElectrodes();
            for (int electrodeIt = 0 ; electrodeIt < numElectrodes ; electrodeIt++){
                //processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),i,slider->getValue());
                for (int channelIt = 0 ; channelIt < processor->getNumChannels(electrodeIt) ; channelIt++){
                    processor->setChannelThreshold(electrodeIt,channelIt,slider->getValue());
                }
            }
        }
        else{
        processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),
                                       electrodeNum,
                                       slider->getValue());
        }


     /*   //Array<int> dacChannels = processor->getDACassignments;
        int dacChannel = dacCombo->getSelectedId()-2;
        if (dacChannel >= 0)
        {
            // update dac threshold.
            processor->updateDACthreshold(dacChannel, slider->getValue());
        }*/

    }
    repaint();
    if (canvas!= nullptr)
        canvas->repaint();

}


void SpikeSorterEditor::buttonEvent(Button* button)
{
 
    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    
    if (electrodeButtons.contains((ElectrodeButton*) button))
    {

        {
            for (int k=0; k<electrodeButtons.size(); k++)
            {
                if (electrodeButtons[k] != button)
                    electrodeButtons[k]->setToggleState(false,dontSendNotification);
            }
            if (electrodeButtons.size() == 1)
                electrodeButtons[0]->setToggleState(true,dontSendNotification);

            ElectrodeButton* eb = (ElectrodeButton*) button;
            int channelNum = eb->getChannelNum()-1;

            std::cout << "Channel number: " << channelNum << std::endl;
            Array<int> a;
            a.add(channelNum);
            channelSelector->setActiveChannels(a);

            SpikeSorter* processor = (SpikeSorter*) getProcessor();

            thresholdSlider->setActive(true);
            thresholdSlider->setValue(processor->getChannelThreshold(electrodeList->getSelectedItemIndex(),
                                                                     electrodeButtons.indexOf((ElectrodeButton*) button)));


        /*    if (processor->getAutoDacAssignmentStatus())
            {
                processor->assignDACtoChannel(0, channelNum);
                processor->assignDACtoChannel(1, channelNum);
            }
            Array<int> dacAssignmentToChannels = processor->getDACassignments();
            // search for channel[0]. If found, set the combo box accordingly...
            dacCombo->setSelectedId(1, sendNotification);
            for (int i=0; i<dacAssignmentToChannels.size(); i++)
            {
                if (dacAssignmentToChannels[i] == channelNum)
                {
                    dacCombo->setSelectedId(i+2, sendNotification);
                    break;
                }
            }*/

        }
    }


    int num = numElectrodes->getText().getIntValue();

    if (button == upButton)
    {
        numElectrodes->setText(String(++num), sendNotification);

        return;

    }
    else if (button == downButton)
    {

        if (num > 1)
            numElectrodes->setText(String(--num), sendNotification);

        return;

    }
    else if (button == configButton)
    {
        PopupMenu configMenu;
        PopupMenu waveSizeMenu;
        PopupMenu waveSizePreMenu;
        PopupMenu waveSizePostMenu;
		bool allowSampleChange = !CoreServices::getAcquisitionStatus();

        waveSizePreMenu.addItem(1,"8",allowSampleChange,processor->getNumPreSamples() == 8);
		waveSizePreMenu.addItem(2, "16", allowSampleChange, processor->getNumPreSamples() == 16);
		waveSizePostMenu.addItem(3, "32", allowSampleChange, processor->getNumPostSamples() == 32);
		waveSizePostMenu.addItem(4, "64", allowSampleChange, processor->getNumPostSamples() == 64);

        waveSizeMenu.addSubMenu("Pre samples",waveSizePreMenu);
        waveSizeMenu.addSubMenu("Post samples",waveSizePostMenu);
        waveSizeMenu.addItem(7,"Flip Signal",true,processor->getFlipSignalState());
        configMenu.addSubMenu("Waveform",waveSizeMenu,true);
        configMenu.addItem(5,"Current Channel => Audio",true,processor->getAutoDacAssignmentStatus());
        configMenu.addItem(6,"Threshold => All channels",true,processor->getThresholdSyncStatus());

        const int result = configMenu.show();
        switch (result)
        {
            case 1:
                processor->setNumPreSamples(8);
				CoreServices::updateSignalChain(this);
                break;
            case 2:
                processor->setNumPreSamples(16);
				CoreServices::updateSignalChain(this);
                break;
            case 3:
                processor->setNumPostSamples(32);
				CoreServices::updateSignalChain(this);
                break;
            case 4:
                processor->setNumPostSamples(64);
				CoreServices::updateSignalChain(this);
                break;
            case 5:
                processor->seteAutoDacAssignment(!processor->getAutoDacAssignmentStatus());
                refreshElectrodeList();
                break;
            case 6:
                processor->setThresholdSyncStatus(!processor->getThresholdSyncStatus());
                break;
            case 7:
                processor->setFlipSignalState(!processor->getFlipSignalState());
                break;
        }

    }
    else if (button == plusButton)
    {
        // std::cout << "Plus button pressed!" << std::endl;
        if (acquisitionIsActive)
        {
            CoreServices::sendStatusMessage("Stop acquisition before adding electrodes.");
            return;
        }

        //updateAdvancerList();
        PopupMenu probeMenu;
        probeMenu.addItem(1,"Single Electrode");
        probeMenu.addItem(2,"Stereotrode");
        probeMenu.addItem(3,"Tetrode");
        PopupMenu depthprobeMenu;
        depthprobeMenu.addItem(4,"8 ch, 125um");
        depthprobeMenu.addItem(5,"16 ch, 125um");
        depthprobeMenu.addItem(6,"24 ch, 125um");
        depthprobeMenu.addItem(7,"32 ch, 50um");
        depthprobeMenu.addItem(8,"32 ch, 25um");
        probeMenu.addSubMenu("Depth probe", depthprobeMenu,true);

        const int result = probeMenu.show();
        int nChansPerElectrode = 0;
        int nElectrodes = 0;
        double interelectrodeDistance = 0;
        double firstElectrodeOffset = 0;
        int numProbes = numElectrodes->getText().getIntValue();
        String ProbeType;

        switch (result)
        {
            case 0:
                return;
            case 1:
                ProbeType = "Single Electrode";
                nChansPerElectrode = 1;
                nElectrodes = 1;
                firstElectrodeOffset=0;
                break;
            case 2:
                ProbeType = "Stereotrode";
                nChansPerElectrode = 2;
                nElectrodes = 1;
                firstElectrodeOffset = 0;
                break;
            case 3:
                ProbeType = "Tetrode";
                nChansPerElectrode = 4;
                nElectrodes = 1;
                firstElectrodeOffset = 0;
                break;
            case 4:
                ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
                nElectrodes = 8;
                interelectrodeDistance = 0.125;
                firstElectrodeOffset= -0.5;
                break;
            case 5:
                ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
                nElectrodes = 16;
                interelectrodeDistance = 0.125;
                firstElectrodeOffset= -0.5;
                break;
            case 6:
                ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
                nElectrodes = 24;
                interelectrodeDistance = 0.125;
                firstElectrodeOffset= -0.5;
                break;
            case 7:
                ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
                nElectrodes = 32;
                interelectrodeDistance = 0.050;
                firstElectrodeOffset= -0.5;
                break;
            case 8:
                ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
                nElectrodes = 32;
                interelectrodeDistance = 0.025;
                firstElectrodeOffset= -0.075;
                break;
        }

        processor->addProbes(ProbeType,numProbes, nElectrodes,nChansPerElectrode, firstElectrodeOffset,interelectrodeDistance);
        refreshElectrodeList();

        CoreServices::updateSignalChain(this);
        
        return;

    }
    else if (button == removeElectrodeButton)   // DELETE
    {
        if (acquisitionIsActive)
        {
            CoreServices::sendStatusMessage("Stop acquisition before deleting electrodes.");
            return;
        }
        removeElectrode(electrodeList->getSelectedItemIndex());

        CoreServices::updateSignalChain(this);

        return;
    }
    else if (button == audioMonitorButton)
    {

        channelSelector->clearAudio();

        SpikeSorter* processor = (SpikeSorter*) getProcessor();

        const OwnedArray<Electrode>& electrodes = processor->getElectrodes();
		int nElectrodes = electrodes.size();
		if (nElectrodes <= 0)
		{
			audioMonitorButton->setToggleState(false, dontSendNotification);
			return;
		}

        for (int i = 0; i < nElectrodes; i++)
        {
            Electrode* e = electrodes[i];
            e->isMonitored = false;
        }

        Electrode* e = processor->getActiveElectrode();
        e->isMonitored = audioMonitorButton->getToggleState();

        for (int i = 0; i < e->numChannels; i++)
        {
            int channelNum = e->channels[i];
            channelSelector->setAudioStatus(channelNum, audioMonitorButton->getToggleState());

        }

    }



}

void SpikeSorterEditor::setThresholdValue(int channel, double threshold)
{
    thresholdSlider->setActive(true);
    thresholdSlider->setValue(threshold);
    repaint();
}

void SpikeSorterEditor::channelChanged (int channel, bool newState)
{
    //std::cout << "New channel: " << chan << std::endl;
    if (channel <= 0)
        return;

    const int numElectrodeButtons = electrodeButtons.size();
    for (int i = 0; i < numElectrodeButtons; ++i)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum (channel);
            electrodeButtons[i]->repaint();

            Array<int> a;
            a.add (channel - 1);
            channelSelector->setActiveChannels (a);

            SpikeSorter* processor = (SpikeSorter*) getProcessor();
            processor->setChannel(electrodeList->getSelectedItemIndex(),
                                  i,
                                  channel - 1);

         /*   // if DAC is selected, update the mapping.
            int dacchannel = dacCombo->getSelectedId() - 2;
            if (dacchannel >=0)
            {
                processor->assignDACtoChannel (dacchannel, channel - 1);
            }
            if (processor->getAutoDacAssignmentStatus())
            {
                processor->assignDACtoChannel (0, channel - 1);
                processor->assignDACtoChannel (1, channel - 1);
                break;
            }*/
        }
    }
}

int SpikeSorterEditor::getSelectedElectrode()
{
    return electrodeList->getSelectedId();
}

void SpikeSorterEditor::setSelectedElectrode(int i)
{
    electrodeList->setSelectedId(i);
}

void SpikeSorterEditor::refreshElectrodeList(int selected)
{
    electrodeList->clear();

    SpikeSorter* processor = (SpikeSorter*) getProcessor();

    StringArray electrodeNames = processor->getElectrodeNames();

    for (int i = 0; i < electrodeNames.size(); i++)
    {
        electrodeList->addItem(electrodeNames[i], electrodeList->getNumItems()+1);
    }

    if (electrodeList->getNumItems() > 0)
    {
        if (selected == 0)
            selected = electrodeList->getNumItems();

        electrodeList->setSelectedId(selected);
        //        electrodeList->setText(electrodeList->getItemText(electrodeList->getNumItems()-1));
        lastId = electrodeList->getNumItems();
        electrodeList->setEditableText(true);

        drawElectrodeButtons(selected-1);
        Electrode* e = processor->getElectrode(selected - 1);

        int advancerIndex = 0;
        for (int k=0; k<advancerIDs.size(); k++)
        {
            if (advancerIDs[k] == e->advancerID)
            {
                advancerIndex = 1+k;
                break;
            }
        }

        advancerList->setSelectedId(advancerIndex);
        depthOffsetEdit->setText(String(e->depthOffsetMM,4),dontSendNotification);

        if (processor->getAutoDacAssignmentStatus())
        {
            processor->assignDACtoChannel(0, e->channels[0]);
            processor->assignDACtoChannel(1, e->channels[0]);
        }
     /*   Array<int> dacAssignmentToChannels = processor->getDACassignments();
        // search for channel[0]. If found, set the combo box accordingly...
        dacCombo->setSelectedId(1, sendNotification);
        for (int i=0; i<dacAssignmentToChannels.size(); i++)
        {
            if (dacAssignmentToChannels[i] == e->channels[0])
            {
                dacCombo->setSelectedId(i+2, sendNotification);
                processor->updateDACthreshold(i+2, e->thresholds[0]);
                break;
            }
        }*/




    }
    if (spikeSorterCanvas != nullptr)
        spikeSorterCanvas->update();
}


void SpikeSorterEditor::removeElectrode(int index)
{
    std::cout << "Deleting electrode number " << index << std::endl;
    SpikeSorter* processor = (SpikeSorter*) getProcessor();
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

void SpikeSorterEditor::labelTextChanged(Label* label)
{
    if (label == depthOffsetEdit)
    {
        // update electrode depth offset.
        //Value v = depthOffsetEdit->getTextValue();
        //double offset = v.getValue();

        //int electrodeIndex = electrodeList->getSelectedId()-1;
        //SpikeSorter* processor = (SpikeSorter*) getProcessor();
        //if (electrodeIndex >= 0)
        //	processor->setElectrodeAdvancerOffset(electrodeIndex, offset);

        if (spikeSorterCanvas != nullptr)
            spikeSorterCanvas->update();

    }
}

void SpikeSorterEditor::setElectrodeComboBox(int direction)
{
    int N = electrodeList->getNumItems();
    int C = electrodeList->getSelectedId();
    C+=direction;
    if (C <= 0)
        C = N;
    if (C > N)
        C = 1;
    electrodeList->setSelectedId(C, sendNotification);
}

void SpikeSorterEditor::comboBoxChanged(ComboBox* comboBox)
{
    SpikeSorter* processor = (SpikeSorter*) getProcessor();

   /* if (comboBox == dacCombo)
    {
        int selection = dacCombo->getSelectedId();
        // modify the dac channel assignment...
        if (selection > 1)
        {
            int selectedSubChannel = -1;
            for (int i = 0; i < electrodeButtons.size(); i++)
            {
                if (electrodeButtons[i]->getToggleState())
                {
                    selectedSubChannel = i;
                    break;
                }
            }
            Electrode* e = processor->getActiveElectrode();
            if (e != nullptr)
            {
                int dacchannel = selection-2;
                processor->assignDACtoChannel(dacchannel, e->channels[selectedSubChannel]);
            }
        }

    }
    else*/ if (comboBox == electrodeList)
    {
        int ID = comboBox->getSelectedId();

        if (ID == 0)
        {
            // modify electrode name
            processor->setElectrodeName(lastId, comboBox->getText());
            refreshElectrodeList();

        }
        else
        {
            // switch to a new electrode.
            SpikeSorter* processor = (SpikeSorter*) getProcessor();
            lastId = ID;
            Electrode* e= processor->setCurrentElectrodeIndex(ID-1);
            drawElectrodeButtons(ID-1);
            int advancerIndex = 0;

            audioMonitorButton->setToggleState(e->isMonitored, dontSendNotification);

            for (int k=0; k<advancerIDs.size(); k++)
            {
                if (advancerIDs[k] == e->advancerID)
                {
                    advancerIndex = 1+k;
                    break;
                }
            }
            advancerList->setSelectedId(advancerIndex, dontSendNotification);
            depthOffsetEdit->setText(String(e->depthOffsetMM,4),dontSendNotification);

            if (processor->getAutoDacAssignmentStatus())
            {
                processor->assignDACtoChannel(0, e->channels[0]);
                processor->assignDACtoChannel(1, e->channels[0]);
            }
         /*   Array<int> dacAssignmentToChannels = processor->getDACassignments();
            // search for channel[0]. If found, set the combo box accordingly...
            dacCombo->setSelectedId(1, sendNotification);
            for (int i=0; i<dacAssignmentToChannels.size(); i++)
            {
                if (dacAssignmentToChannels[i] == e->channels[0])
                {
                    dacCombo->setSelectedId(i+2, sendNotification);
                    break;
                }
            }*/

        }



    }
    else if (comboBox == advancerList)
    {
        // attach advancer to electrode.
        // int electrodeIndex = electrodeList->getSelectedId()-1;
        // SpikeSorter* processor = (SpikeSorter*) getProcessor();
        // int selectedAdvancer = advancerList->getSelectedId() ;
        // if (electrodeIndex >= 0 && selectedAdvancer > 0)
        // 	processor->setElectrodeAdvancer(electrodeIndex,advancerIDs[advancerList->getSelectedId()-1]);
        // else
        // 	advancerList->setSelectedId(0,dontSendNotification);
    }

}

void SpikeSorterEditor::checkSettings()
{
    electrodeList->setSelectedItemIndex(0);
}

void SpikeSorterEditor::drawElectrodeButtons(int ID)
{

    SpikeSorter* processor = (SpikeSorter*) getProcessor();

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

        if (i == 0)
        {
            activeChannels.add(processor->getChannel(ID,i));
            thresholds.add(processor->getChannelThreshold(ID,i));
        }

        button->setToggleState(i == 0, dontSendNotification);
        button->setBounds(155+(column++)*width, 60+row*height, width, 15);
        addAndMakeVisible(button);
        button->addListener(this);

        if (column % 2 == 0)
        {
            column = 0;
            row++;
        }

    }

    channelSelector->setActiveChannels(activeChannels);

    thresholdSlider->setValues(thresholds);
    thresholdSlider->setActive(true);
    thresholdSlider->setEnabled(true);
    thresholdSlider->setValue(processor->getChannelThreshold(ID,0),dontSendNotification);
    repaint();
    if (spikeSorterCanvas != nullptr)
        spikeSorterCanvas->update();
}


// void SpikeSorterEditor::updateAdvancerList()
// {

// 	ProcessorGraph *g = getProcessor()->getProcessorGraph();
// 	Array<GenericProcessor*> p = g->getListOfProcessors();
// 	for (int k=0;k<p.size();k++)
// 	{
// 		if (p[k]->getName() == "Advancers")
// 		{
// 			AdvancerNode *node = (AdvancerNode *)p[k];
// 			if (node != nullptr)
// 			{
// 				advancerNames = node->getAdvancerNames();
// 				advancerIDs =  node->getAdvancerIDs();

// 				advancerList->clear(dontSendNotification);
// 				for (int i=0;i<advancerNames.size();i++)
// 				{
// 					advancerList->addItem(advancerNames[i],1+i);
// 				}
// 			}
// 		}
// 	}


//         int selectedElectrode = electrodeList->getSelectedId();
// 		if (selectedElectrode > 0) {
// 			SpikeSorter* processor = (SpikeSorter*) getProcessor();
// 			Electrode *e = processor->getElectrode( selectedElectrode-1);
// 			int advancerIndex = 0;
// 			for (int k=0;k<advancerIDs.size();k++)
// 			{
// 				if (advancerIDs[k] == e->advancerID)
// 				{
// 						advancerIndex = 1+k;
// 					break;
// 				}
// 			}
// 			advancerList->setSelectedId(advancerIndex);
// 			}
// 	repaint();
// }
