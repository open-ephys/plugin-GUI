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

#include "RHD2000Editor.h"
#include "../../UI/EditorViewport.h"

#include "../DataThreads/RHD2000Thread.h"

RHD2000Editor::RHD2000Editor(GenericProcessor* parentNode,
                             RHD2000Thread* board_,
                             bool useDefaultParameterEditors
                            )
    : GenericEditor(parentNode, useDefaultParameterEditors), board(board_)
{
    desiredWidth = 400;

    // add headstage-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 4; i++)
    {
        HeadstageOptionsInterface* hsOptions = new HeadstageOptionsInterface(board, this, i);
        headstageOptionsInterfaces.add(hsOptions);
        addAndMakeVisible(hsOptions);
        hsOptions->setBounds(3,28+i*20, 70, 18);
    }

    // add sample rate selection
    sampleRateInterface = new SampleRateInterface(board, this);
    addAndMakeVisible(sampleRateInterface);
    sampleRateInterface->setBounds(150,25,160, 50);

    // add Bandwidth selection
    bandwidthInterface = new BandwidthInterface(board, this);
    addAndMakeVisible(bandwidthInterface);
    bandwidthInterface->setBounds(150,65,160, 50);

    // add rescan button
    rescanButton = new UtilityButton("RESCAN", Font("Small Text", 13, Font::plain));
    rescanButton->setRadius(3.0f);
    rescanButton->setBounds(6,108,65,18);
    rescanButton->addListener(this);
    addAndMakeVisible(rescanButton);

}

RHD2000Editor::~RHD2000Editor()
{

}

void RHD2000Editor::buttonEvent(Button* button)
{

    if (button == rescanButton)
    {
        board->scanPorts();

        for (int i = 0; i < 4; i++)
        {
            headstageOptionsInterfaces[i]->checkEnabledState();
        }

    }

}

// Bandwidth Options --------------------------------------------------------------------

BandwidthInterface::BandwidthInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name = "Bandwidth";


    UpperBandwidthSelection = new Label("UpperBandwidth","7500 Hz"); // this is currently set in RHD2000Thread, the cleaner would be to set it here again
    UpperBandwidthSelection->setEditable(true,false,false);
    UpperBandwidthSelection->addListener(this);
    UpperBandwidthSelection->setBounds(0,10,300,30);
    addAndMakeVisible(UpperBandwidthSelection);


    LowerBandwidthSelection = new Label("LowerBandwidth","1 Hz");
    LowerBandwidthSelection->setEditable(true,false,false);
    LowerBandwidthSelection->addListener(this);
    LowerBandwidthSelection->setBounds(0,30,300,30);
    addAndMakeVisible(LowerBandwidthSelection);



}

BandwidthInterface::~BandwidthInterface()
{

}


void BandwidthInterface::labelTextChanged(Label* te)
{
    
    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (te == UpperBandwidthSelection)
        {
            double actualUpperBandwidth = board->setUpperBandwidth(te->getText().getDoubleValue());
           // cb->setText(cb->getItemText(te->getSelectedId()),true);
            std::cout << "Setting Upper Bandwidth to " << te->getText().getDoubleValue() << std::endl;
            std::cout << "Actual Upper Bandwidth:  " <<  actualUpperBandwidth  << std::endl;
            te->setText(String(actualUpperBandwidth)+" Hz",false);


            repaint();
        } else {
            double actualLowerBandwidth = board->setLowerBandwidth(te->getText().getDoubleValue());
           // cb->setText(cb->getItemText(te->getSelectedId()),true);
            std::cout << "Setting Lower Bandwidth to " << te->getText().getDoubleValue() << std::endl;
            std::cout << "Actual Lower Bandwidth:  " <<  actualLowerBandwidth  << std::endl;
            te->setText(String(actualLowerBandwidth)+" Hz",false);

            repaint(); 
        }
    }
}




void BandwidthInterface::paint(Graphics& g)
{
    //g.setColour(Colours::lightgrey);

    //g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);
  
   // g.setColour(Colours::grey);

    g.setFont(Font("Small Text",15,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

}

// Sample rate Options --------------------------------------------------------------------

SampleRateInterface::SampleRateInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name="SampleRate";

    sampleRateOptions.add("1.00 kS/s");
    sampleRateOptions.add("1.25 kS/s");
    sampleRateOptions.add("1.50 kS/s");
    sampleRateOptions.add("2.00 kS/s");
    sampleRateOptions.add("2.50 kS/s");
    sampleRateOptions.add("3.00 kS/s");
    sampleRateOptions.add("3.33 kS/s");
    sampleRateOptions.add("4.00 kS/s");
    sampleRateOptions.add("5.00 kS/s");
    sampleRateOptions.add("6.25 kS/s");
    sampleRateOptions.add("8.00 kS/s");
    sampleRateOptions.add("10.0 kS/s");
    sampleRateOptions.add("12.5 kS/s");
    sampleRateOptions.add("15.0 kS/s");
    sampleRateOptions.add("20.0 kS/s");
    sampleRateOptions.add("25.0 kS/s");
    sampleRateOptions.add("30.0 kS/s");


    rateSelection = new ComboBox("Sample Rate");
    rateSelection->addItemList(sampleRateOptions, 1);
    rateSelection->setSelectedId(17,false);
    rateSelection->addListener(this);

    rateSelection->setBounds(0,15,300,20);
    addAndMakeVisible(rateSelection);


}

SampleRateInterface::~SampleRateInterface()
{

}

void SampleRateInterface::comboBoxChanged(ComboBox* cb)
{
	if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (cb == rateSelection)
        {
            board->setSampleRate(cb->getSelectedId());
            //cb->setText(cb->getItemText(cb->getSelectedId()),true);
            std::cout << "Setting sample rate to index " << cb->getSelectedId() << std::endl;

            editor->getEditorViewport()->makeEditorVisible(editor, false, true);
            //repaint();
        }
    }
}




void SampleRateInterface::paint(Graphics& g)
{
    //g.setColour(Colours::lightgrey);

    //g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);
  
   // g.setColour(Colours::grey);

    g.setFont(Font("Small Text",15,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

}


// Headstage Options --------------------------------------------------------------------

HeadstageOptionsInterface::HeadstageOptionsInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_,
                                                     int hsNum) :
    isEnabled(false), board(board_), editor(editor_)
{

    switch (hsNum)
    {
        case 0 :
            name = "A";
            break;
        case 1:
            name = "B";
            break;
        case 2:
            name = "C";
            break;
        case 3:
            name = "D";
            break;
        default:
            name = "X";
    }

    hsNumber1 = hsNum*2; // data stream 1
    hsNumber2 = hsNumber1+1; // data stream 2

    channelsOnHs1 = 0;
    channelsOnHs2 = 0;

    

    hsButton1 = new UtilityButton(" ", Font("Small Text", 13, Font::plain));
    hsButton1->setRadius(3.0f);
    hsButton1->setBounds(23,1,20,17);
    hsButton1->setEnabledState(false);
    hsButton1->setCorners(true, false, true, false);
    hsButton1->addListener(this);
    addAndMakeVisible(hsButton1);

    hsButton2 = new UtilityButton(" ", Font("Small Text", 13, Font::plain));
    hsButton2->setRadius(3.0f);
    hsButton2->setBounds(43,1,20,17);
    hsButton2->setEnabledState(false);
    hsButton2->setCorners(false, true, false, true);
    hsButton2->addListener(this);
    addAndMakeVisible(hsButton2);

    checkEnabledState();
}

HeadstageOptionsInterface::~HeadstageOptionsInterface()
{

}

void HeadstageOptionsInterface::checkEnabledState()
{
    isEnabled = (board->isHeadstageEnabled(hsNumber1) || 
                 board->isHeadstageEnabled(hsNumber2));

    if (board->isHeadstageEnabled(hsNumber1))
    {
        channelsOnHs1 = 32;
        hsButton1->setLabel(String(channelsOnHs1));
        hsButton1->setEnabledState(true);
    } else {
        channelsOnHs1 = 0;
        hsButton1->setLabel(" ");
        hsButton1->setEnabledState(false);
    }

    if (board->isHeadstageEnabled(hsNumber2))
    {
        channelsOnHs2 = 32;
        hsButton2->setLabel(String(channelsOnHs2));
        hsButton2->setEnabledState(true);
    } else {
        channelsOnHs2 = 0;
        hsButton2->setLabel(" ");
        hsButton2->setEnabledState(false);
    }

    repaint();

}

void HeadstageOptionsInterface::buttonClicked(Button* button)
{

    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {

        //std::cout << "Acquisition is not active" << std::endl;
        if (button == hsButton1)
        {
            if (channelsOnHs1 == 32)
                channelsOnHs1 = 16;
            else
                channelsOnHs1 = 32;

            //std::cout << "HS1 has " << channelsOnHs1 << " channels." << std::endl;

            hsButton1->setLabel(String(channelsOnHs1));
            board->setNumChannels(hsNumber1, channelsOnHs1);

        } else if (button == hsButton2)
        {
            if (channelsOnHs2 == 32)
                channelsOnHs2 = 16;
            else
                channelsOnHs2 = 32;

            hsButton2->setLabel(String(channelsOnHs2));
            board->setNumChannels(hsNumber2, channelsOnHs2);
        }


        editor->getEditorViewport()->makeEditorVisible(editor, false, true);
    }

}


void HeadstageOptionsInterface::paint(Graphics& g)
{
    g.setColour(Colours::lightgrey);

    g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(Font("Small Text",15,Font::plain));

    g.drawText(name, 8, 2, 200, 15, Justification::left, false);

}