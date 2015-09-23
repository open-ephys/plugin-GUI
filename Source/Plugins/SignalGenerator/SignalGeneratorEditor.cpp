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


#include "SignalGeneratorEditor.h"
#include "SignalGenerator.h"
#include "../../UI/EditorViewport.h"
#include <stdio.h>


SignalGeneratorEditor::SignalGeneratorEditor(GenericProcessor* parentNode, bool useDefaultParameters=false)
    : GenericEditor(parentNode, useDefaultParameters), amplitudeSlider(0), frequencySlider(0), phaseSlider(0)

{
    desiredWidth = 250;

    int buttonWidth = 31;
    int buttonHeight = 19;

    for (int i = 0; i < 5; i++)
    {
        WaveformSelector* ws = new WaveformSelector(i);
        ws->setBounds(8 + (buttonWidth)*i, 30, buttonWidth, buttonHeight);
        ws->addListener(this);
        waveformSelectors.add(ws);
        addAndMakeVisible(ws);
    }

    amplitudeSlider = new Slider("Amplitude Slider");
    amplitudeSlider->setBounds(10,60,50,60);
    amplitudeSlider->setRange(0,1,0.1);
    amplitudeSlider->addListener(this);
    amplitudeSlider->setSliderStyle(Slider::Rotary);
    amplitudeSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    amplitudeSlider->setValue(0.5);
    addAndMakeVisible(amplitudeSlider);

    frequencySlider = new Slider("Frequency Slider");
    frequencySlider->setBounds(70,60,50,60);
    frequencySlider->setRange(10,1000,10);
    frequencySlider->addListener(this);
    frequencySlider->setSliderStyle(Slider::Rotary);
    frequencySlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    frequencySlider->setValue(1000);
    addAndMakeVisible(frequencySlider);

    phaseSlider = new Slider("Phase Slider");
    phaseSlider->setBounds(130,60,50,60);
    phaseSlider->setRange(0, 360, 1);
    phaseSlider->addListener(this);
    phaseSlider->setSliderStyle(Slider::Rotary);
    phaseSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    addAndMakeVisible(phaseSlider);

    numChannelsLabel = new Label("Number of Channels","5");
    numChannelsLabel->setEditable(true);
    numChannelsLabel->addListener(this);
    numChannelsLabel->setBounds(200,50,25,20);
    addAndMakeVisible(numChannelsLabel);

    upButton = new TriangleButton(1);
    upButton->addListener(this);
    upButton->setBounds(200,30,20,15);
    addAndMakeVisible(upButton);

    downButton = new TriangleButton(2);
    downButton->addListener(this);
    downButton->setBounds(200,75,20,15);
    addAndMakeVisible(downButton);

}

SignalGeneratorEditor::~SignalGeneratorEditor()
{
    deleteAllChildren();
}

// void SignalGeneratorEditor::sliderValueChanged (Slider* slider)
// {

// 	// Array<int> chans = getActiveChannels();

// 	// //std::cout << chans.size() << " channels selected." << std::endl;

// 	// GenericProcessor* p = (GenericProcessor*) getAudioProcessor();

// 	// for (int n = 0; n < chans.size(); n++) {

// 	// 	p->setCurrentChannel(chans[n]);

// 	// 	if (slider == amplitudeSlider)
// 	// 		p->setParameter(0,slider->getValue());
// 	// 	else if (slider == frequencySlider)
// 	// 		p->setParameter(1,slider->getValue());
// 	// 	else if (slider == phaseSlider)
// 	// 		p->setParameter(2,slider->getValue());

// 	// }


// }

void SignalGeneratorEditor::buttonEvent(Button* button)
{

    for (int i = 0; i < waveformSelectors.size(); i++)
    {
        if (button == waveformSelectors[i])
        {

            Array<int> chans = getActiveChannels();

            GenericProcessor* p = getProcessor();

            for (int n = 0; n < chans.size(); n++)
            {

                p->setCurrentChannel(chans[n]);
                p->setParameter(3,(float) i);

            }


        }
    }

    int num = numChannelsLabel->getText().getIntValue();

    if (button == upButton)
    {
        numChannelsLabel->setText(String(++num), dontSendNotification);

    }
    else if (button == downButton)
    {

        if (num > 1)
            numChannelsLabel->setText(String(--num), dontSendNotification);

    }
}

void SignalGeneratorEditor::sliderEvent(Slider* slider)
{

    int paramIndex;

    if (slider == amplitudeSlider)
    {
        paramIndex = 0;
    }
    else if (slider == frequencySlider)
    {
        paramIndex = 1;
    }
    else if (slider == phaseSlider)
    {
        paramIndex = 2;
    }


    GenericProcessor* p = getProcessor();

    Array<int> chans = getActiveChannels();

    for (int n = 0; n < chans.size(); n++)
    {

        p->setCurrentChannel(chans[n]);
        p->setParameter(paramIndex, slider->getValue());

    }

}

void SignalGeneratorEditor::labelTextChanged(Label* label)
{

    SignalGenerator* sg = (SignalGenerator*) getProcessor();
    sg->nOut = numChannelsLabel->getText().getIntValue();
	CoreServices::highlightEditor(this);
}

WaveformSelector::WaveformSelector(int type) : Button("Waveform")
{

    setRadioGroupId(299);
    setClickingTogglesState(true);

    if (type == 0)
    {
        selected = ImageCache::getFromMemory(BinaryData::RadioButtons_selected01_png,
                                             BinaryData::RadioButtons_selected01_pngSize);
        selectedOver= ImageCache::getFromMemory(BinaryData::RadioButtons_selected_over01_png,
                                                BinaryData::RadioButtons_selected_over01_pngSize);
        neutral = ImageCache::getFromMemory(BinaryData::RadioButtons_neutral01_png,
                                            BinaryData::RadioButtons_neutral01_pngSize);
        neutralOver = ImageCache::getFromMemory(BinaryData::RadioButtons01_png,
                                                BinaryData::RadioButtons01_pngSize);
    }
    else if (type == 1)
    {
        selected = ImageCache::getFromMemory(BinaryData::RadioButtons_selected02_png,
                                             BinaryData::RadioButtons_selected02_pngSize);
        selectedOver= ImageCache::getFromMemory(BinaryData::RadioButtons_selected_over02_png,
                                                BinaryData::RadioButtons_selected_over02_pngSize);
        neutral = ImageCache::getFromMemory(BinaryData::RadioButtons_neutral02_png,
                                            BinaryData::RadioButtons_neutral02_pngSize);
        neutralOver = ImageCache::getFromMemory(BinaryData::RadioButtons02_png,
                                                BinaryData::RadioButtons02_pngSize);
    }
    else if (type == 2)
    {
        selected = ImageCache::getFromMemory(BinaryData::RadioButtons_selected03_png,
                                             BinaryData::RadioButtons_selected03_pngSize);
        selectedOver= ImageCache::getFromMemory(BinaryData::RadioButtons_selected_over03_png,
                                                BinaryData::RadioButtons_selected_over03_pngSize);
        neutral = ImageCache::getFromMemory(BinaryData::RadioButtons_neutral03_png,
                                            BinaryData::RadioButtons_neutral03_pngSize);
        neutralOver = ImageCache::getFromMemory(BinaryData::RadioButtons03_png,
                                                BinaryData::RadioButtons03_pngSize);
    }
    else if (type == 3)
    {
        selected = ImageCache::getFromMemory(BinaryData::RadioButtons_selected04_png,
                                             BinaryData::RadioButtons_selected04_pngSize);
        selectedOver= ImageCache::getFromMemory(BinaryData::RadioButtons_selected_over04_png,
                                                BinaryData::RadioButtons_selected_over04_pngSize);
        neutral = ImageCache::getFromMemory(BinaryData::RadioButtons_neutral04_png,
                                            BinaryData::RadioButtons_neutral04_pngSize);
        neutralOver = ImageCache::getFromMemory(BinaryData::RadioButtons04_png,
                                                BinaryData::RadioButtons04_pngSize);
    }
    else if (type == 4)
    {
        selected = ImageCache::getFromMemory(BinaryData::RadioButtons_selected05_png,
                                             BinaryData::RadioButtons_selected05_pngSize);
        selectedOver= ImageCache::getFromMemory(BinaryData::RadioButtons_selected_over05_png,
                                                BinaryData::RadioButtons_selected_over05_pngSize);
        neutral = ImageCache::getFromMemory(BinaryData::RadioButtons_neutral05_png,
                                            BinaryData::RadioButtons_neutral05_pngSize);
        neutralOver = ImageCache::getFromMemory(BinaryData::RadioButtons05_png,
                                                BinaryData::RadioButtons05_pngSize);
    }
    //   else if (type == 5) {
    // selected = ImageCache::getFromMemory (BinaryData::RadioButtons_selected05_png,
    //                                             BinaryData::RadioButtons_selected05_pngSize);
    // 	selectedOver= ImageCache::getFromMemory (BinaryData::RadioButtons_selected_over05_png,
    //                                                BinaryData::RadioButtons_selected_over05_pngSize);
    // 	neutral = ImageCache::getFromMemory (BinaryData::RadioButtons_neutral05_png,
    //                                            BinaryData::RadioButtons_neutral05_pngSize);
    // 	neutralOver = ImageCache::getFromMemory (BinaryData::RadioButtons05_png,
    //                                                BinaryData::RadioButtons05_pngSize);
    //   }

}

void WaveformSelector::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{


    if (getToggleState())
    {
        if (isMouseOver)
            g.drawImage(selectedOver, 0, 0, 31, 19, 0, 0, 31, 19);
        else
            g.drawImage(selected, 0, 0, 31, 19, 0, 0, 31, 19);
    }
    else
    {
        if (isMouseOver)
            g.drawImage(neutralOver, 0, 0, 31, 19, 0, 0, 31, 19);
        else
            g.drawImage(neutral, 0, 0, 31, 19, 0, 0, 31, 19);
    }

    // g.fillAll();

    // g.setColour(Colours::black);
    // g.drawRect(0, 0, getWidth(), getHeight());

    // g.setImageResamplingQuality(Graphics::highResamplingQuality);

}