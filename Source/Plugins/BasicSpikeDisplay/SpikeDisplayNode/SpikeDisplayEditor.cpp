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

#include "SpikeDisplayEditor.h"

#include <string>

SpikeDisplayEditor::SpikeDisplayEditor(GenericProcessor* parentNode)
    : VisualizerEditor(parentNode,200)

{
    // // Get the number of sub channels from the parentNode
    // // Assume all plots have the same number of subChannels
    // // Otherwise we'll have to track the number of subChannels
    // nSubChannels = 4;

    // for (int i=0; i<nSubChannels; i++)
    //     subChanSelected[i] = true;

    // initializeButtons();

    tabText = "Spikes";



}

SpikeDisplayEditor::~SpikeDisplayEditor()
{
    deleteAllChildren();
}

void SpikeDisplayEditor::initializeButtons()
{
    int w = 18;
    int h = 18;
    int xPad = 5;
    int yPad = 6;

    int xInitial = 10;
    int yInitial = 25;
    int x = xInitial;
    int y = yInitial;

    panLabel = new Label("PanLabel", "Pan:");
    panLabel->setBounds(x-xPad, y, w*2 + xPad, h);
    panLabel->setJustificationType(Justification::centredLeft);
    x+= 2*w+2*xPad;

    zoomLabel = new Label("ZoomLabel", "Zoom:");
    zoomLabel->setBounds(x-xPad,y,w*3+xPad, h);
    zoomLabel->setJustificationType(Justification::centredLeft);
    x = xInitial;
    y += h + yPad/2;


    panDownBtn = new UtilityButton("-", titleFont);
    panDownBtn->setCorners(true, false, true, false);
    panDownBtn->setBounds(x, y, w, h);
    panDownBtn->setClickingTogglesState(false);
    panDownBtn->addListener(this);
    x+= w;//+xPad;

    panUpBtn = new UtilityButton("+", titleFont);
    panUpBtn->setCorners(false, true, false, true);
    panUpBtn->setBounds(x, y, w, h);
    panUpBtn->setClickingTogglesState(false);
    panUpBtn->addListener(this);
    x+= w+xPad*2;


    zoomOutBtn = new UtilityButton("-", titleFont);
    zoomOutBtn->setCorners(true, false, true, false);
    zoomOutBtn->setBounds(x,y,w,h);
    zoomOutBtn->setClickingTogglesState(false);
    zoomOutBtn->addListener(this);
    x += w;// + xPad;

    zoomInBtn = new UtilityButton("+", titleFont);
    zoomInBtn->setCorners(false, true, false, true);
    zoomInBtn->setBounds(x,y,w,h);
    zoomInBtn->setClickingTogglesState(false);
    zoomInBtn->addListener(this);
    x += w + xPad*3;


    clearBtn = new UtilityButton("Clear", titleFont);
    clearBtn->setBounds(x, y, w*2 + xPad, h);
    clearBtn->setClickingTogglesState(false);
    clearBtn->addListener(this);
    //x += (w + xPad) *2;

    /*
    	x = xInitial;
    	y += h + yPad;

    	//panLabel->setFont(titleFont);

    	saveImgBtn = new UtilityButton("Save", titleFont);
    	saveImgBtn->setBounds(x,y,w*2 + xPad, h);
    	saveImgBtn->setClickingTogglesState(false);
    	saveImgBtn->addListener(this);
    	x += (w + xPad) * 2;

    	*/



    //zoomLabel->setFont(titleFont);
    x = xInitial;
    y += h + yPad;
    // Button *zoomOutBtn = new EditorButton("-");

    subChanLabel = new Label("SubChan", "Sub Channel:");
    subChanLabel->setBounds(x - xPad,y,w*8, h);
    subChanLabel->setJustificationType(Justification::centredLeft);
    y += h + yPad/2;
    //x += w/2;

    allSubChansBtn = new UtilityButton("All", titleFont);
    allSubChansBtn->setBounds(x,y,w*2+xPad,h);
    allSubChansBtn->addListener(this);
    allSubChansBtn->setToggleState(true, dontSendNotification);
    x += (w+xPad) * 2;

    for (int i = 0; i < nSubChannels; i++)
    {
        String s = "";
        s += i;

        subChanBtn[i] = new UtilityButton(s, titleFont);
        subChanBtn[i]->setBounds(x,y,w,h);
        subChanBtn[i]->addListener(this);
        subChanBtn[i]->setToggleState(true, dontSendNotification);
        x += w + xPad;
    }


    addAndMakeVisible(panDownBtn);
    addAndMakeVisible(panUpBtn);
    addAndMakeVisible(panLabel);

    addAndMakeVisible(zoomOutBtn);
    addAndMakeVisible(zoomInBtn);
    addAndMakeVisible(zoomLabel);

    addAndMakeVisible(clearBtn);
    //addAndMakeVisible(saveImgBtn);

    addAndMakeVisible(subChanLabel);
    addAndMakeVisible(allSubChansBtn);
    for (int i=0; i<nSubChannels; i++)
        addAndMakeVisible(subChanBtn[i]);

}

Visualizer* SpikeDisplayEditor::createNewCanvas()
{

    SpikeDisplayNode* processor = (SpikeDisplayNode*) getProcessor();
    return new SpikeDisplayCanvas(processor);

}

void SpikeDisplayEditor::startRecording()
{

    SpikeDisplayCanvas* sdc = (SpikeDisplayCanvas*) canvas.get();

    sdc->startRecording();

}

void SpikeDisplayEditor::stopRecording()
{

    SpikeDisplayCanvas* sdc = (SpikeDisplayCanvas*) canvas.get();

    sdc->stopRecording();

}

// void SpikeDisplayEditor::updateSettings()
// {
// 	// called by base class

// }

// void SpikeDisplayEditor::updateVisualizer()
// {

// }

void SpikeDisplayEditor::buttonCallback(Button* button)
{
    //std::cout<<"Got event from component:"<<button<<std::endl;

    // int pIdx = 0;
    // if (button == panUpBtn)
    // {
    //     for (int i=0; i<nSubChannels; i++)
    //         if (subChanSelected[i])
    //             canvas->setParameter(SPIKE_CMD_PAN_AXES, pIdx, i, 1);
    // }
    // else if (button == panDownBtn)
    // {
    //     for (int i=0; i<nSubChannels; i++)
    //         if (subChanSelected[i])
    //             canvas->setParameter(SPIKE_CMD_PAN_AXES, pIdx, i, -1);
    // }
    // else if (button == zoomInBtn)
    // {
    //     for (int i=0; i<nSubChannels; i++)
    //         if (subChanSelected[i])
    //             canvas->setParameter(SPIKE_CMD_ZOOM_AXES, pIdx, i, -1);
    // }
    // else if (button == zoomOutBtn)
    // {
    //     for (int i=0; i<nSubChannels; i++)
    //         if (subChanSelected[i])
    //             canvas->setParameter(SPIKE_CMD_ZOOM_AXES, pIdx, i, 1);
    // }

    // else if (button == clearBtn)
    // {
    //     std::cout<<"Clear!"<<std::endl;
    //     canvas->setParameter(SPIKE_CMD_CLEAR_ALL, 0);
    // }
    // else if (button == saveImgBtn)
    //     std::cout<<"Save!"<<std::endl;

    // // toggle all sub channel buttons
    // else if (button == allSubChansBtn)
    // {
    //     bool b = allSubChansBtn->getToggleState();
    //     for (int i=0; i<nSubChannels; i++)
    //         subChanBtn[i]->setToggleState(b, sendNotification);

    // }
    // // Check the sub Channel selection buttons one by one
    // else
    // {
    //     // If the user has clicked a sub channel button then the all channels button should be untoggled if toggled
    //     allSubChansBtn->setToggleState(false, dontSendNotification);
    //     for (int i=0; i<nSubChannels; i++)
    //         if (button == subChanBtn[i])
    //         {
    //             std::cout<<"SubChannel:"<<i<< " set to:";
    //             subChanSelected[i] = ((UtilityButton*) button)->getToggleState();
    //             std::cout<< subChanSelected[i]<<std::endl;
    //         }

    //     // If the user has toggled all of the sub channels on, then set AllChans to on
    //     bool allChansToggled = true;
    //     for (int i=0; i<nSubChannels; i++)
    //     {
    //         if (subChanBtn[i]->getToggleState()!=allChansToggled)
    //         {
    //             allChansToggled = false;
    //             break;
    //         }
    //     }
    //     allSubChansBtn->setToggleState(allChansToggled, dontSendNotification);

    // }
}
