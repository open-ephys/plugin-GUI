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
                             bool useDefaultParameterEditors=true
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
        hsOptions->setBounds(80,25+i*23, 60, 22);
    }

    // add sample rate selection
    SampleRateInterface* rateOptions = new SampleRateInterface(board, this);
    addAndMakeVisible(rateOptions);
    rateOptions->setBounds(150,25,160, 50);

    // add Bandwidth selection
    BandwidthInterface* bandwidthOptions = new BandwidthInterface(board, this);
    addAndMakeVisible(bandwidthOptions);
    bandwidthOptions->setBounds(150,65,160, 50);

}

RHD2000Editor::~RHD2000Editor()
{
}

// Bandwidth Options --------------------------------------------------------------------

BandwidthInterface::BandwidthInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name="Bandwidth";


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

            repaint();
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
    hsNumber(hsNum), isEnabled(false), board(board_), editor(editor_)
{

    switch (hsNumber)
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

    isEnabled = board->isHeadstageEnabled(hsNumber);

    enabledButton = new UtilityButton("on", Font("Small Text", 13, Font::plain));
    enabledButton->addListener(this);
    enabledButton->setRadius(3.0f);
    enabledButton->setBounds(25,2,20,19);
    addAndMakeVisible(enabledButton);



}

HeadstageOptionsInterface::~HeadstageOptionsInterface()
{

}

void HeadstageOptionsInterface::buttonClicked(Button* button)
{

    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {

        //std::cout << "Acquisition is not active" << std::endl;
        if (isEnabled)
        {
            isEnabled = false;
        }
        else
        {
            isEnabled = true;
        }

        board->enableHeadstage(hsNumber, isEnabled);

        repaint();

        editor->getEditorViewport()->makeEditorVisible(editor, false, true);
    }

}

// void HeadstageOptionsInterface::mouseUp(const MouseEvent& event)
// {
// 	///>>>> ???? WHY ISN"T THIS WORKING?

// 	if (event.eventComponent == this)
// 	{


// 	}

// }

void HeadstageOptionsInterface::paint(Graphics& g)
{
    g.setColour(Colours::lightgrey);

    g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(Font("Small Text",20,Font::plain));

    g.drawText(name, 8, 5, 200, 15, Justification::left, false);

}