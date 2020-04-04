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

#include "LfpDisplayOptions.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "ShowHideOptionsButton.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"
#include "LfpChannelColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpDisplayOptions -
// -------------------------------------------------------------

LfpDisplayOptions::LfpDisplayOptions(LfpDisplayCanvas* canvas_, LfpTimescale* timescale_, 
                                     LfpDisplay* lfpDisplay_, LfpDisplayNode* processor_)
    : canvas(canvas_),
      lfpDisplay(lfpDisplay_),
      timescale(timescale_),
      processor(processor_),
      selectedChannelType(DataChannel::HEADSTAGE_CHANNEL),
      labelFont("Default", 13.0f, Font::plain),
      labelColour(100, 100, 100)
{
    // draw the colour scheme options
    // TODO: (kelly) this might be better as a modal window
    colourSchemeOptionLabel = new Label("colorSchemeOptionLabel", "Color Scheme");
    colourSchemeOptionLabel->setFont(labelFont);
    colourSchemeOptionLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(colourSchemeOptionLabel);
    
    StringArray colourSchemeNames = lfpDisplay->getColourSchemeNameArray();
    colourSchemeOptionSelection = new ComboBox("colorSchemeOptionSelection");
    colourSchemeOptionSelection->addItemList(colourSchemeNames, 1);
    colourSchemeOptionSelection->setEditableText(false);
    colourSchemeOptionSelection->addListener(this);
    colourSchemeOptionSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(colourSchemeOptionSelection);
    
    if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
        addAndMakeVisible(lfpDisplay->getColourSchemePtr());
    
 //Ranges for neural data
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("25");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("50");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("100");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("250");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("400");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("500");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("750");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("1000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("2000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("5000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("10000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("15000");
    selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] = 4;
    rangeGain[DataChannel::HEADSTAGE_CHANNEL] = 1; //uV
    rangeSteps[DataChannel::HEADSTAGE_CHANNEL] = 10;
    rangeUnits.add(CharPointer_UTF8("\xC2\xB5V"));
    typeNames.add("DATA");

    UtilityButton* tbut;
    tbut = new UtilityButton("DATA",Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false,false,false,false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100,dontSendNotification);
    tbut->setToggleState(true,dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);
    
    //Ranges for AUX/accelerometer data
    voltageRanges[DataChannel::AUX_CHANNEL].add("25");
    voltageRanges[DataChannel::AUX_CHANNEL].add("50");
    voltageRanges[DataChannel::AUX_CHANNEL].add("100");
    voltageRanges[DataChannel::AUX_CHANNEL].add("250");
    voltageRanges[DataChannel::AUX_CHANNEL].add("400");
    voltageRanges[DataChannel::AUX_CHANNEL].add("500");
    voltageRanges[DataChannel::AUX_CHANNEL].add("750");
    voltageRanges[DataChannel::AUX_CHANNEL].add("1000");
    voltageRanges[DataChannel::AUX_CHANNEL].add("2000");
    //voltageRanges[DataChannel::AUX_CHANNEL].add("5000");
    selectedVoltageRange[DataChannel::AUX_CHANNEL] = 9;
    rangeGain[DataChannel::AUX_CHANNEL] = 0.001f; //mV
    rangeSteps[DataChannel::AUX_CHANNEL] = 10;
    rangeUnits.add("mV");
    typeNames.add("AUX");
    
    tbut = new UtilityButton("AUX",Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false,false,false,false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100,dontSendNotification);
    tbut->setToggleState(false,dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);

    //Ranges for ADC data
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.01");
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.05");
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.1");
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.5");
     voltageRanges[DataChannel::ADC_CHANNEL].add("1.0");
     voltageRanges[DataChannel::ADC_CHANNEL].add("2.0");
     voltageRanges[DataChannel::ADC_CHANNEL].add("5.0");
     voltageRanges[DataChannel::ADC_CHANNEL].add("10.0");
     selectedVoltageRange[DataChannel::ADC_CHANNEL] = 8;
     rangeGain[DataChannel::ADC_CHANNEL] = 1; //V
     rangeSteps[DataChannel::ADC_CHANNEL] = 0.1; //in V
    rangeUnits.add("V");
    typeNames.add("ADC");

    tbut = new UtilityButton("ADC",Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false,false,false,false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100,dontSendNotification);
    tbut->setToggleState(false,dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);

    selectedVoltageRangeValues[DataChannel::HEADSTAGE_CHANNEL] = voltageRanges[DataChannel::HEADSTAGE_CHANNEL][selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] - 1];
    selectedVoltageRangeValues[DataChannel::AUX_CHANNEL] = voltageRanges[DataChannel::AUX_CHANNEL][selectedVoltageRange[DataChannel::AUX_CHANNEL] - 1];
    selectedVoltageRangeValues[DataChannel::ADC_CHANNEL] = voltageRanges[DataChannel::ADC_CHANNEL][selectedVoltageRange[DataChannel::ADC_CHANNEL] - 1];
    
    // init channel display skipping options
    channelDisplaySkipOptions.add("All");
    channelDisplaySkipOptions.add("2");
    channelDisplaySkipOptions.add("4");
    channelDisplaySkipOptions.add("8");
    channelDisplaySkipOptions.add("16");
    channelDisplaySkipOptions.add("32");
    channelDisplaySkipOptions.add("64");
    selectedChannelDisplaySkip = 1;
    selectedChannelDisplaySkipValue = channelDisplaySkipOptions[selectedChannelDisplaySkip - 1];
    
    channelDisplaySkipSelection = new ComboBox("Channel Skip");
    channelDisplaySkipSelection->addItemList(channelDisplaySkipOptions, 1);
    channelDisplaySkipSelection->setSelectedId(selectedChannelDisplaySkip, sendNotification);
    channelDisplaySkipSelection->setEditableText(false);
    channelDisplaySkipSelection->addListener(this);
    addAndMakeVisible(channelDisplaySkipSelection);
    
    channelDisplaySkipLabel = new Label("Channel Display Skip", "Ch. Skip");
    channelDisplaySkipLabel->setFont(labelFont);
    channelDisplaySkipLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(channelDisplaySkipLabel);
    
    // init spike raster options
    spikeRasterSelectionOptions = {"Off", "-50", "-100", "-150", "-200", "-300", "-400", "-500"};
    selectedSpikeRasterThreshold = 1;
    selectedSpikeRasterThresholdValue = spikeRasterSelectionOptions[selectedSpikeRasterThreshold - 1];
    
    spikeRasterSelection = new ComboBox("spikeRasterSelection");
    spikeRasterSelection->addItemList(spikeRasterSelectionOptions, 1);
    spikeRasterSelection->setSelectedId(selectedSpikeRasterThreshold, dontSendNotification);
    spikeRasterSelection->setEditableText(true);
    spikeRasterSelection->addListener(this);
    addAndMakeVisible(spikeRasterSelection);
    
    spikeRasterLabel = new Label("spikeRasterLabel", "Spike Raster Thresh.");
    spikeRasterLabel->setFont(labelFont);
    spikeRasterLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(spikeRasterLabel);
    
    // init median offset plotting
    medianOffsetPlottingLabel = new Label("Median Offset Correction", "Median Offset Correction");
    medianOffsetPlottingLabel->setFont(labelFont);
    medianOffsetPlottingLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(medianOffsetPlottingLabel);
    
    medianOffsetPlottingButton = new UtilityButton("0", labelFont);
    medianOffsetPlottingButton->setRadius(5.0f);
    medianOffsetPlottingButton->setEnabledState(true);
    medianOffsetPlottingButton->setCorners(true, true, true, true);
    medianOffsetPlottingButton->addListener(this);
    medianOffsetPlottingButton->setClickingTogglesState(true);
    medianOffsetPlottingButton->setToggleState(false, sendNotification);
    addAndMakeVisible(medianOffsetPlottingButton);

    //init channel name toggle
    showChannelNumberLabel = new Label("showcChannelLabel", "Show channel number instead of name");
    showChannelNumberLabel->setFont(labelFont);
    showChannelNumberLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(showChannelNumberLabel);

    showChannelNumberButton = new UtilityButton("0", labelFont);
    showChannelNumberButton->setRadius(5.0f);
    showChannelNumberButton->setEnabledState(true);
    showChannelNumberButton->setCorners(true, true, true, true);
    showChannelNumberButton->addListener(this);
    showChannelNumberButton->setClickingTogglesState(true);
    showChannelNumberButton->setToggleState(false, sendNotification);
    addAndMakeVisible(showChannelNumberButton);

    // init show/hide options button
    showHideOptionsButton = new ShowHideOptionsButton(this);
    showHideOptionsButton->addListener(this);
    addAndMakeVisible(showHideOptionsButton);

    // init timebases options
    timebases.add("0.010");
    timebases.add("0.025");
    timebases.add("0.050");
    timebases.add("0.100");
    timebases.add("0.250");
    timebases.add("0.500");
    timebases.add("1.0");
    timebases.add("2.0");
    timebases.add("3.0");
    timebases.add("4.0");
    timebases.add("5.0");
    timebases.add("10.0");
    timebases.add("20.0");
    selectedTimebase = 8;
    selectedTimebaseValue = timebases[selectedTimebase-1];

    spreads.add("10");
    spreads.add("20");
    spreads.add("30");
    spreads.add("40");
    spreads.add("50");
    spreads.add("60");
    spreads.add("70");
    spreads.add("80");
    spreads.add("90");
    spreads.add("100");
    selectedSpread = 4;
    selectedSpreadValue = spreads[selectedSpread-1];

    overlaps.add("0.5");
    overlaps.add("0.75");
    overlaps.add("1");
    overlaps.add("2");
    overlaps.add("3");
    overlaps.add("4");
    overlaps.add("5");
    selectedOverlap = 4;
    selectedOverlapValue = overlaps[selectedOverlap-1];

    saturationThresholds.add("0.5");
    saturationThresholds.add("100");
    saturationThresholds.add("1000");
    saturationThresholds.add("5000");
    saturationThresholds.add("6389");
    
    selectedSaturation = 5;
    selectedSaturationValue = saturationThresholds[selectedSaturation-1];
    
    colorGroupings.add("1");
    colorGroupings.add("2");
    colorGroupings.add("4");
    colorGroupings.add("8");
    colorGroupings.add("16");

    triggerSources.add("none");
    for (int k=1; k<=8; k++)
    {
        triggerSources.add(String(k));
    }

    rangeSelection = new ComboBox("Voltage range");
    rangeSelection->addItemList(voltageRanges[DataChannel::HEADSTAGE_CHANNEL], 1);
    rangeSelection->setSelectedId(selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL], sendNotification);
    rangeSelection->setEditableText(true);
    rangeSelection->addListener(this);
    addAndMakeVisible(rangeSelection);

    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(selectedTimebase, sendNotification);
    timebaseSelection->setEditableText(true);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);

    spreadSelection = new ComboBox("Spread");
    spreadSelection->addItemList(spreads, 1);
    spreadSelection->setSelectedId(selectedSpread,sendNotification);
    spreadSelection->addListener(this);
    spreadSelection->setEditableText(true);
    addAndMakeVisible(spreadSelection);

    overlapSelection = new ComboBox("Overlap");
    overlapSelection->addItemList(overlaps, 1);
    overlapSelection->setSelectedId(selectedOverlap,sendNotification);
    overlapSelection->addListener(this);
    overlapSelection->setEditableText(true);
    addAndMakeVisible(overlapSelection);
    
    saturationWarningSelection = new ComboBox("Sat.Warn");
    saturationWarningSelection->addItemList(saturationThresholds, 1);
    saturationWarningSelection->setSelectedId(selectedSaturation,sendNotification);
    saturationWarningSelection->addListener(this);
    saturationWarningSelection->setEditableText(true);
    addAndMakeVisible(saturationWarningSelection);
    
    colorGroupingSelection = new ComboBox("Color Grouping");
    colorGroupingSelection->addItemList(colorGroupings, 1);
    colorGroupingSelection->setSelectedId(1,sendNotification);
    colorGroupingSelection->addListener(this);
    addAndMakeVisible(colorGroupingSelection);

    triggerSourceSelection = new ComboBox("Trigger Source");
    triggerSourceSelection->addItemList(triggerSources, 1);
    triggerSourceSelection->setSelectedId(1, sendNotification);
    triggerSourceSelection->addListener(this);
    addAndMakeVisible(triggerSourceSelection);

    invertInputButton = new UtilityButton("Invert", Font("Small Text", 13, Font::plain));
    invertInputButton->setRadius(5.0f);
    invertInputButton->setEnabledState(true);
    invertInputButton->setCorners(true, true, true, true);
    invertInputButton->addListener(this);
    invertInputButton->setClickingTogglesState(true);
    invertInputButton->setToggleState(false, sendNotification);
    addAndMakeVisible(invertInputButton);
    
    // toggle button to reverse the order of channels
    reverseChannelsDisplayButton = new UtilityButton("0", labelFont);
    reverseChannelsDisplayButton->setRadius(5.0f);
    reverseChannelsDisplayButton->setEnabledState(true);
    reverseChannelsDisplayButton->setCorners(true, true, true, true);
    reverseChannelsDisplayButton->addListener(this);
    reverseChannelsDisplayButton->setClickingTogglesState(true);
    reverseChannelsDisplayButton->setToggleState(lfpDisplay->getChannelsReversed(), sendNotification);
    addAndMakeVisible(reverseChannelsDisplayButton);
    
    reverseChannelsDisplayLabel = new Label("Rev. Channels", "Rev. Channels");
    reverseChannelsDisplayLabel->setFont(labelFont);
    reverseChannelsDisplayLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(reverseChannelsDisplayLabel);
    
    //button for controlling drawing algorithm - old line-style or new per-pixel style
    drawMethodButton = new UtilityButton("DrawMethod", Font("Small Text", 13, Font::plain));
    drawMethodButton->setRadius(5.0f);
    drawMethodButton->setEnabledState(true);
    drawMethodButton->setCorners(true, true, true, true);
    drawMethodButton->addListener(this);
    drawMethodButton->setClickingTogglesState(true);
    drawMethodButton->setToggleState(false, sendNotification);
    addAndMakeVisible(drawMethodButton);
    
    // two sliders for the two histogram components of the supersampled plotting mode
    // todo: rename these
    brightnessSliderA = new Slider();
    brightnessSliderA->setRange (0, 1);
    brightnessSliderA->setTextBoxStyle(Slider::NoTextBox, false, 50,30);
    brightnessSliderA->addListener(this);
    addAndMakeVisible (brightnessSliderA);
    
    brightnessSliderB = new Slider;
    brightnessSliderB->setRange (0, 1);
    brightnessSliderB->setTextBoxStyle(Slider::NoTextBox, false, 50,30);
    brightnessSliderB->addListener(this);
    addAndMakeVisible (brightnessSliderB);
    
    sliderALabel = new Label("Brightness","Brightness");
    sliderALabel->setFont(Font("Small Text", 13, Font::plain));
    sliderALabel->setColour(Label::textColourId,Colour(150,150,150));
    addAndMakeVisible(sliderALabel);
    
    sliderBLabel = new Label("Min. brightness","Min. brightness");
    sliderBLabel->setFont(Font("Small Text", 13, Font::plain));
    sliderBLabel->setColour(Label::textColourId,Colour(150,150,150));
    addAndMakeVisible(sliderBLabel);
    
    //ScopedPointer<UtilityButton> drawClipWarningButton; // optinally draw (subtle) warning if data is clipped in display
    drawClipWarningButton = new UtilityButton("0", Font("Small Text", 13, Font::plain));
    drawClipWarningButton->setRadius(5.0f);
    drawClipWarningButton->setEnabledState(true);
    drawClipWarningButton->setCorners(true, true, true, true);
    drawClipWarningButton->addListener(this);
    drawClipWarningButton->setClickingTogglesState(true);
    drawClipWarningButton->setToggleState(false, sendNotification);
    addAndMakeVisible(drawClipWarningButton);
    
    //ScopedPointer<UtilityButton> drawSaturateWarningButton; // optionally raise hell if the actual data is saturating
    drawSaturateWarningButton = new UtilityButton("0", Font("Small Text", 13, Font::plain));
    drawSaturateWarningButton->setRadius(5.0f);
    drawSaturateWarningButton->setEnabledState(true);
    drawSaturateWarningButton->setCorners(true, true, true, true);
    drawSaturateWarningButton->addListener(this);
    drawSaturateWarningButton->setClickingTogglesState(true);
    drawSaturateWarningButton->setToggleState(false, sendNotification);
    addAndMakeVisible(drawSaturateWarningButton);
    
    //button for pausing the display - works by skipping buffer updates. This way scrolling etc still works
    pauseButton = new UtilityButton("Pause", Font("Small Text", 13, Font::plain));
    pauseButton->setRadius(5.0f);
    pauseButton->setEnabledState(true);
    pauseButton->setCorners(true, true, true, true);
    pauseButton->addListener(this);
    pauseButton->setClickingTogglesState(true);
    pauseButton->setToggleState(false, sendNotification);
    addAndMakeVisible(pauseButton);

    // add event display-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 8; i++)
    {

        EventDisplayInterface* eventOptions = new EventDisplayInterface(lfpDisplay, canvas, i);
        eventDisplayInterfaces.add(eventOptions);
        addAndMakeVisible(eventOptions);
        eventOptions->setBounds(700+(floor(i/2)*20), getHeight()-20-(i%2)*20, 40, 20);

        lfpDisplay->setEventDisplayState(i,true);

    }

    lfpDisplay->setRange(voltageRanges[DataChannel::HEADSTAGE_CHANNEL][selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::HEADSTAGE_CHANNEL]
        , DataChannel::HEADSTAGE_CHANNEL);
    lfpDisplay->setRange(voltageRanges[DataChannel::ADC_CHANNEL][selectedVoltageRange[DataChannel::ADC_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::ADC_CHANNEL]
        , DataChannel::ADC_CHANNEL);
    lfpDisplay->setRange(voltageRanges[DataChannel::AUX_CHANNEL][selectedVoltageRange[DataChannel::AUX_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::AUX_CHANNEL]
        , DataChannel::AUX_CHANNEL);
    
}

LfpDisplayOptions::~LfpDisplayOptions()
{

}

void LfpDisplayOptions::resized()
{
    rangeSelection->setBounds(5,getHeight()-30,80,25);
    timebaseSelection->setBounds(175,getHeight()-30,60,25);
    
    spreadSelection->setBounds(5,getHeight()-90,60,25);
    
    overlapSelection->setBounds(100,getHeight()-90,60,25);

    drawClipWarningButton->setBounds(175,getHeight()-89,20,20);
    drawSaturateWarningButton->setBounds(325, getHeight()-89, 20, 20);
    
    colorGroupingSelection->setBounds(400,getHeight()-90,60,25);
    triggerSourceSelection->setBounds(375,getHeight()-30,60,25);

    invertInputButton->setBounds(35,getHeight()-190,100,22);
    drawMethodButton->setBounds(35,getHeight()-160,100,22);

    pauseButton->setBounds(465,getHeight()-50,50,44);
    
    // Reverse Channels Display
    reverseChannelsDisplayButton->setBounds(pauseButton->getRight() + 5,
                                 getHeight() - 50,
                                 20,
                                 20);
    reverseChannelsDisplayLabel->setBounds(reverseChannelsDisplayButton->getRight(),
                                           reverseChannelsDisplayButton->getY(),
                                           120,
                                           22);
    
    // Channel Display Skip Selector
    channelDisplaySkipSelection->setBounds(reverseChannelsDisplayButton->getX(),
                                           reverseChannelsDisplayButton->getBottom(),
                                           60,
                                           25);
    channelDisplaySkipLabel->setBounds(channelDisplaySkipSelection->getRight(),
                                       channelDisplaySkipSelection->getY() + 2,
                                       100,
                                       22);
    
    // Median Offset Plotting Button
    medianOffsetPlottingButton->setBounds(reverseChannelsDisplayLabel->getRight() + 5,
                                          reverseChannelsDisplayButton->getY(),
                                          20,
                                          20);
    medianOffsetPlottingLabel->setBounds(medianOffsetPlottingButton->getRight(),
                                         medianOffsetPlottingButton->getY(),
                                         150,
                                         22);
    
    //Channel name toggle
    showChannelNumberButton->setBounds(medianOffsetPlottingLabel->getRight() + 5,
        medianOffsetPlottingLabel->getY(),
        20,
        20);
    showChannelNumberLabel->setBounds(showChannelNumberButton->getRight(),
        showChannelNumberButton->getY(),
        200,
        22);
    
    // Spike raster plotting button
    spikeRasterSelection->setBounds(medianOffsetPlottingButton->getX(),
                                    medianOffsetPlottingButton->getBottom(),
                                    60,
                                    25);
    spikeRasterLabel->setBounds(spikeRasterSelection->getRight(),
                                spikeRasterSelection->getY(),
                                120,
                                22);
    
    // Saturation Warning Selection
    saturationWarningSelection->setBounds(250, getHeight()-90, 60, 25);
    
    for (int i = 0; i < 8; i++)
    {
        eventDisplayInterfaces[i]->setBounds(270+(floor(i/2)*20), getHeight()-40+(i%2)*20, 40, 20); // arrange event channel buttons in two rows
        eventDisplayInterfaces[i]->repaint();
    }
    
    brightnessSliderA->setBounds(170,getHeight()-190,100,22);
    sliderALabel->setBounds(270, getHeight()-190, 180, 22);
    brightnessSliderA->setValue(0.9); //set default value
    
    brightnessSliderB->setBounds(170,getHeight()-160,100,22);
    sliderBLabel->setBounds(270, getHeight()-160, 180, 22);
    brightnessSliderB->setValue(0.1); //set default value
    
    showHideOptionsButton->setBounds (getWidth() - 28, getHeight() - 28, 20, 20);
    
    int bh = 25/typeButtons.size();
    for (int i = 0; i < typeButtons.size(); i++)
    {
        typeButtons[i]->setBounds(95,getHeight()-30+i*bh,50,bh);
    }
    
    colourSchemeOptionLabel->setBounds(medianOffsetPlottingButton->getX(),
                                       getHeight()-190,
                                       100,
                                       22);
    colourSchemeOptionSelection->setBounds(colourSchemeOptionLabel->getRight(),
                                           colourSchemeOptionLabel->getY(),
                                           80,
                                           25);
    
    // set the size of the active colour scheme's options, if it has configurable options
    if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
    {
        lfpDisplay->getColourSchemePtr()->setBounds(colourSchemeOptionLabel->getX(),
                                                    colourSchemeOptionLabel->getBottom(),
                                                    200,
                                                    110);
    }
}

void LfpDisplayOptions::paint(Graphics& g)
{
    int row1 = 55;
    int row2 = 110;

    g.fillAll(Colours::black);
    g.setFont(Font("Default", 16, Font::plain));

    g.setColour(Colour(100,100,100));

    g.drawText("Range("+ rangeUnits[selectedChannelType] +")",5,getHeight()-row1,300,20,Justification::left, false);
    g.drawText("Timebase(s)",160,getHeight()-row1,300,20,Justification::left, false);
    g.drawText("Size(px)",5,getHeight()-row2,300,20,Justification::left, false);
    g.drawText("Clip",100,getHeight()-row2,300,20,Justification::left, false);
    g.drawText("Warn",168,getHeight()-row2,300,20,Justification::left, false);
    
    g.drawText("Sat. Warning",225,getHeight()-row2,300,20,Justification::left, false);

    g.drawText("Color grouping",365,getHeight()-row2,300,20,Justification::left, false);

    g.drawText("Event disp.",270,getHeight()-row1,300,20,Justification::left, false);
    g.drawText("Trigger",375,getHeight()-row1,300,20,Justification::left, false);

    if(canvas->drawClipWarning)
    {
        g.setColour(Colours::white);
        g.fillRoundedRectangle(173,getHeight()-90-1,24,24,6.0f);
    }
    
    if(canvas->drawSaturationWarning)
    {
        g.setColour(Colours::red);
        g.fillRoundedRectangle(323,getHeight()-90-1,24,24,6.0f);
    }
    
}

int LfpDisplayOptions::getChannelHeight()
{
    return (int)spreadSelection->getText().getIntValue();
}

bool LfpDisplayOptions::getDrawMethodState()
{
    
    return drawMethodButton->getToggleState();
}

bool LfpDisplayOptions::getInputInvertedState()
{
    return invertInputButton->getToggleState();
}

bool LfpDisplayOptions::getChannelNameState()
{
    return showChannelNumberButton->getToggleState();
}

bool LfpDisplayOptions::getDisplaySpikeRasterizerState()
{
//    return spikeRasterButton->getToggleState();
    return false;
}

void LfpDisplayOptions::setDisplaySpikeRasterizerState(bool isEnabled)
{
//    spikeRasterButton->setToggleState(isEnabled, dontSendNotification);
    
//    if (isEnabled) medianOffsetPlottingButton->setToggleState(true, sendNotification);
}

void LfpDisplayOptions::setRangeSelection(float range, bool canvasMustUpdate)
{
    if (canvasMustUpdate)
    {
        rangeSelection->setText(String(range/rangeGain[selectedChannelType]), sendNotification); 
    }
    else
    {
        rangeSelection->setText(String(range/rangeGain[selectedChannelType]),dontSendNotification);
        
        selectedVoltageRange[selectedChannelType] = rangeSelection->getSelectedId();
        selectedVoltageRangeValues[selectedChannelType] = rangeSelection->getText();

        canvas->repaint();
        canvas->refresh();
    }

}

void LfpDisplayOptions::setSpreadSelection(int spread, bool canvasMustUpdate, bool deferDisplayRefresh)
{
    
    if (canvasMustUpdate)
    {
        spreadSelection->setText(String(spread),sendNotification);
    }
    else
    {
        spreadSelection->setText(String(spread),dontSendNotification);
        selectedSpread = spreadSelection->getSelectedId();
        selectedSpreadValue = spreadSelection->getText();

        if (!deferDisplayRefresh)
        {
            canvas->repaint();
            canvas->refresh();
        }
    }
}

void LfpDisplayOptions::togglePauseButton(bool sendUpdate)
{
    pauseButton->setToggleState(!pauseButton->getToggleState(), sendUpdate ? sendNotification : dontSendNotification);
}

void LfpDisplayOptions::buttonClicked(Button* b)
{
    if (b == invertInputButton)
    {
        lfpDisplay->setInputInverted(b->getToggleState());
        return;
    }
    if (b == reverseChannelsDisplayButton)
    {
        lfpDisplay->setChannelsReversed(b->getToggleState());
        return;
    }
    if (b == medianOffsetPlottingButton)
    {
        if (lfpDisplay->getSpikeRasterPlotting())
        {
            medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
        }
        else
        {
            lfpDisplay->setMedianOffsetPlotting(b->getToggleState());
        }
        return;
    }
    if (b == drawMethodButton)
    {
        lfpDisplay->setDrawMethod(b->getToggleState()); // this should be done the same way as drawClipWarning - or the other way around.
        
        return;
    }
    if (b == drawClipWarningButton)
    {
        canvas->drawClipWarning = b->getToggleState();
        canvas->redraw();
        return;
    }
    if (b == drawSaturateWarningButton)
    {
        canvas->drawSaturationWarning = b->getToggleState();
        canvas->redraw();
        return;
    }
    
    if (b == pauseButton)
    {
        lfpDisplay->isPaused = b->getToggleState();
        return;
    }

    if (b == showHideOptionsButton)
    {
        canvas->toggleOptionsDrawer(b->getToggleState());
    }

    if (b == showChannelNumberButton)
    {
        int numChannels = lfpDisplay->channelInfo.size();
        for (int i = 0; i < numChannels; ++i)
        {
            lfpDisplay->channelInfo[i]->repaint();
        }
        return;
    }

    int idx = typeButtons.indexOf((UtilityButton*)b);

    if ((idx >= 0) && (b->getToggleState()))
    {
        for (int i = 0; i < lfpDisplay->getNumChannels(); i++)
        {
            if (lfpDisplay->channels[i]->getSelected())
            {
                lfpDisplay->channels[i]->deselect();
                lfpDisplay->channels[i]->repaint();
            }
        } 

        setSelectedType((DataChannel::DataChannelTypes) idx, false);
    }

}

void LfpDisplayOptions::setTimebaseAndSelectionText(float timebase)
{
    canvas->timebase = timebase;
    
    if (canvas->timebase) // if timebase != 0
    {
        if (canvas->timebase < timebases[0].getFloatValue())
        {
            timebaseSelection->setSelectedId(1, dontSendNotification);
            canvas->timebase = timebases[0].getFloatValue();
        }
        else if (canvas->timebase > timebases[timebases.size()-1].getFloatValue())
        {
            timebaseSelection->setSelectedId(timebases.size(), dontSendNotification);
            canvas->timebase = timebases[timebases.size()-1].getFloatValue();
        }
        else{
            timebaseSelection->setText(String(canvas->timebase, 1), dontSendNotification);
        }
    }
    else
    {
        if (selectedSpread == 0)
        {
            timebaseSelection->setText(selectedTimebaseValue, dontSendNotification);
            canvas->timebase = selectedTimebaseValue.getFloatValue();
        }
        else
        {
            timebaseSelection->setSelectedId(selectedTimebase,dontSendNotification);
            canvas->timebase = timebases[selectedTimebase-1].getFloatValue();
        }
        
    }
}

void LfpDisplayOptions::comboBoxChanged(ComboBox* cb)
{
    if (canvas->getNumChannels() == 0) return;
    
    if (cb == channelDisplaySkipSelection)
    {
        const int skipAmt = pow(2, cb->getSelectedId() - 1);
        lfpDisplay->setChannelDisplaySkipAmount(skipAmt);
    }
    else if (cb == spikeRasterSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val == 0) // if value is zero, just disable plotting and set text to "Off"
            {
                cb->setSelectedItemIndex(0, dontSendNotification);
                lfpDisplay->setSpikeRasterPlotting(false);
                return;
            }
            
            if (val > 500)
            {
                val = 500;
            }
            
            val *= -1;
            
            spikeRasterSelection->setText(String(val), dontSendNotification);
            lfpDisplay->setSpikeRasterThreshold(val);
            medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
            lfpDisplay->setMedianOffsetPlotting(true);
            lfpDisplay->setSpikeRasterPlotting(true);
        }
        else if (cb->getSelectedItemIndex() == 0) // if "Off"
        {
            lfpDisplay->setSpikeRasterPlotting(false);
            return;
        }
        else
        {
            auto val = cb->getText().getFloatValue();
            
            lfpDisplay->setSpikeRasterThreshold(val);
            medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
            lfpDisplay->setMedianOffsetPlotting(true);
            lfpDisplay->setSpikeRasterPlotting(true);
        }
    }
    else if (cb == colourSchemeOptionSelection)
    {
        // hide the old colour scheme config options if they are displayed
        if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
            removeChildComponent(lfpDisplay->getColourSchemePtr());
        
        // change the active colour scheme ptr
        lfpDisplay->setActiveColourSchemeIdx(cb->getSelectedId()-1);
        
        // show the new colour scheme's config options if has any
        
        if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
        {
            lfpDisplay->getColourSchemePtr()->setBounds(colourSchemeOptionLabel->getX(),
                                                        colourSchemeOptionLabel->getBottom(),
                                                        200,
                                                        110);
            addAndMakeVisible(lfpDisplay->getColourSchemePtr());
        }
        
        // update the lfpDisplay's colors and redraw
        lfpDisplay->setColors();
        canvas->redraw();
    }
    else if (cb == timebaseSelection)
    {
        if (cb->getSelectedId())
        {
            canvas->timebase = timebases[cb->getSelectedId()-1].getFloatValue();
        }
        else
        {
            setTimebaseAndSelectionText(cb->getText().getFloatValue());
        }
    }
    else if (cb == rangeSelection)
    {
        if (cb->getSelectedId())
        {
        lfpDisplay->setRange(voltageRanges[selectedChannelType][cb->getSelectedId()-1].getFloatValue()*rangeGain[selectedChannelType]
            ,selectedChannelType);
        }
        else
        {
            float vRange = cb->getText().getFloatValue();
            if (vRange)
            {
                if (vRange < voltageRanges[selectedChannelType][0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    vRange = voltageRanges[selectedChannelType][0].getFloatValue();
                }
                else if (vRange > voltageRanges[selectedChannelType][voltageRanges[selectedChannelType].size()-1].getFloatValue())
                {
                   // cb->setSelectedId(voltageRanges[selectedChannelType].size(),dontSendNotification);
                   // vRange = voltageRanges[selectedChannelType][voltageRanges[selectedChannelType].size()-1].getFloatValue();
                }
                else
                {
                    if (rangeGain[selectedChannelType] > 1)
                        cb->setText(String(vRange,1),dontSendNotification);
                    else
                        cb->setText(String(vRange),dontSendNotification);
                }
                lfpDisplay->setRange(vRange*rangeGain[selectedChannelType],selectedChannelType);
            }
            else
            {
                if (selectedVoltageRange[selectedChannelType])
                    cb->setText(selectedVoltageRangeValues[selectedChannelType],dontSendNotification);
                else
                    cb->setSelectedId(selectedVoltageRange[selectedChannelType],dontSendNotification);
            }
        }
        selectedVoltageRange[selectedChannelType] = cb->getSelectedId();
        selectedVoltageRangeValues[selectedChannelType] = cb->getText();
        //std::cout << "Setting range to " << voltageRanges[cb->getSelectedId()-1].getFloatValue() << std::endl;
        canvas->redraw();
    }
    else if (cb == spreadSelection)
    {
        
        if (cb->getSelectedId())
        {
            if (lfpDisplay->getSingleChannelState())
            {
                lfpDisplay->cacheNewChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
            }
            else
            {
                lfpDisplay->setChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
                resized();
            }
        }
        else
        {
            int spread = cb->getText().getIntValue();
            if (spread)
            {
                if (spread < spreads[0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    spread = spreads[0].getFloatValue();
                }
                else if (spread > spreads[spreads.size()-1].getFloatValue())
                {
                    cb->setSelectedId(spreads.size(),dontSendNotification);
                    spread = spreads[spreads.size()-1].getFloatValue();
                }
                else
                {
                    cb->setText(String(spread),dontSendNotification);
                }
                
                // if single channel focus is on, cache the value
                if (lfpDisplay->getSingleChannelState())
                {
                    lfpDisplay->cacheNewChannelHeight(spread);
                }
                else
                {
                    lfpDisplay->setChannelHeight(spread);
                    canvas->resized();
                }
            }
            else
            {
                if (selectedSpread == 0)
                    cb->setText(selectedSpreadValue,dontSendNotification);
                else
                    cb->setSelectedId(selectedSpread,dontSendNotification);
            }
        }
        selectedSpread = cb->getSelectedId();
        selectedSpreadValue = cb->getText();

        if (!lfpDisplay->getSingleChannelState()) canvas->redraw();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == saturationWarningSelection)
    {
        if (cb->getSelectedId())
        {
            selectedSaturationValueFloat = (saturationThresholds[cb->getSelectedId()-1].getFloatValue());
        }
        else
        {
            selectedSaturationValueFloat = cb->getText().getFloatValue();
            if (selectedSaturationValueFloat)
            {
                 std::cout << "Setting saturation warning to to " << selectedSaturationValueFloat << std::endl;
                if (selectedSaturationValueFloat < 0)
                {
                    cb->setSelectedId(1,dontSendNotification);
                    selectedSaturationValueFloat = saturationThresholds[0].getFloatValue();
                }
                else
                {
                  //  cb->setText(String(selectedSaturationValueFloat),dontSendNotification);
                }
            }
            else
            {
               // cb->setSelectedId(1,dontSendNotification);
                //selectedSaturationValueFloat = saturationThresholds[0].getFloatValue();

            }
        }
        canvas->redraw();

        std::cout << "Setting saturation warning to to " << selectedSaturationValueFloat << std::endl;
    }
    else if (cb == overlapSelection)
    {
        if (cb->getSelectedId())
        {
            canvas->channelOverlapFactor = (overlaps[cb->getSelectedId()-1].getFloatValue());
            canvas->resized();
        }
        else
        {
            float overlap = cb->getText().getFloatValue();
            if (overlap)
            {
                if (overlap < overlaps[0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    overlap = overlaps[0].getFloatValue();
                }
                else if (overlap > overlaps[overlaps.size()-1].getFloatValue())
                {
                    cb->setSelectedId(overlaps.size(),dontSendNotification);
                    overlap = overlaps[overlaps.size()-1].getFloatValue();
                }
                else
                {
                    cb->setText(String(overlap),dontSendNotification);
                }
                canvas->channelOverlapFactor= overlap;
                canvas->resized();
            }
            else
            {
                if (selectedSpread == 0)
                    cb->setText(selectedSpreadValue,dontSendNotification);
                else
                    cb->setSelectedId(selectedSpread,dontSendNotification);
            }
        }
        selectedSpread = cb->getSelectedId();
        selectedSpreadValue = cb->getText();
        lfpDisplay->setChannelHeight( lfpDisplay->getChannelHeight());
        canvas->redraw();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == colorGroupingSelection)
    {
        // set color grouping here
        lfpDisplay->setColorGrouping(colorGroupings[cb->getSelectedId()-1].getIntValue());// so that channel colors get re-assigned
        canvas->redraw();
    }
    else if (cb == triggerSourceSelection)
    {
        processor->setTriggerSource(cb->getSelectedId() - 2);
    }


    timescale->setTimebase(canvas->timebase);
}

void LfpDisplayOptions::sliderValueChanged(Slider* sl)
{
    if (sl == brightnessSliderA)
        canvas->histogramParameterA = sl->getValue();

    if (sl == brightnessSliderB)
        canvas->histogramParameterB = sl->getValue();

    canvas->fullredraw=true;
    //repaint();
    canvas->refresh();

}

void LfpDisplayOptions::sliderEvent(Slider* sl) {}

DataChannel::DataChannelTypes LfpDisplayOptions::getChannelType(int n)
{
    if (n < processor->getNumInputs())
        return processor->getDataChannel(n)->getChannelType();
    else
        return DataChannel::HEADSTAGE_CHANNEL;
}

DataChannel::DataChannelTypes LfpDisplayOptions::getSelectedType()
{
    return selectedChannelType;
}

void LfpDisplayOptions::setSelectedType(DataChannel::DataChannelTypes type, bool toggleButton)
{
    if (selectedChannelType == type)
        return; //Nothing to do here
    selectedChannelType = type;
    rangeSelection->clear(dontSendNotification);
    rangeSelection->addItemList(voltageRanges[type],1);

    int id = selectedVoltageRange[type];
    if (id)
        rangeSelection->setSelectedId(id,sendNotification);
    else
        rangeSelection->setText(selectedVoltageRangeValues[selectedChannelType],dontSendNotification);
    
    repaint(5,getHeight()-55,300,100);

    if (toggleButton)
        typeButtons[type]->setToggleState(true,dontSendNotification);
}

String LfpDisplayOptions::getTypeName(DataChannel::DataChannelTypes type)
{
    return typeNames[type];
}

int LfpDisplayOptions::getRangeStep(DataChannel::DataChannelTypes type)
{
    return rangeSteps[type];
}

void LfpDisplayOptions::saveParameters(XmlElement* xml)
{
    // TODO: (kelly) add savers for:
    //      - channel reverse
    //      - channel zoom slider
    //      - channel display skip
    std::cout << "Saving lfp display params" << std::endl;

    XmlElement* xmlNode = xml->createNewChildElement("LFPDISPLAY");

    lfpDisplay->reactivateChannels();

    xmlNode->setAttribute("Range",selectedVoltageRangeValues[0]+","+selectedVoltageRangeValues[1]+
        ","+selectedVoltageRangeValues[2]);
    xmlNode->setAttribute("Timebase",timebaseSelection->getText());
    xmlNode->setAttribute("Spread",spreadSelection->getText());
    xmlNode->setAttribute("colorGrouping",colorGroupingSelection->getSelectedId());
    xmlNode->setAttribute("triggerSource", triggerSourceSelection->getSelectedId());
    xmlNode->setAttribute("isInverted",invertInputButton->getToggleState());
    xmlNode->setAttribute("drawMethod",drawMethodButton->getToggleState());

    int eventButtonState = 0;

    for (int i = 0; i < 8; i++)
    {
        if (lfpDisplay->eventDisplayEnabled[i])
        {
            eventButtonState += (1 << i);
        }
    }

    lfpDisplay->reactivateChannels();

    xmlNode->setAttribute("EventButtonState", eventButtonState);

    String channelDisplayState = "";

    for (int i = 0; i < canvas->nChans; i++)
    {
        if (lfpDisplay->getEnabledState(i))
        {
            channelDisplayState += "1";
        }
        else
        {
            channelDisplayState += "0";
        }
        //std::cout << channelDisplayState;
    }

    //std::cout << std::endl;

    xmlNode->setAttribute("ChannelDisplayState", channelDisplayState);

    xmlNode->setAttribute("ScrollX",canvas->viewport->getViewPositionX());
    xmlNode->setAttribute("ScrollY",canvas->viewport->getViewPositionY());
}

void LfpDisplayOptions::loadParameters(XmlElement* xml)
{
    // TODO: (kelly) add loaders for:
    //      - channel reverse
    //      - channel zoom slider
    //      - channel display skip
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("LFPDISPLAY"))
        {
            StringArray ranges;
            ranges.addTokens(xmlNode->getStringAttribute("Range"),",",String::empty);
            selectedVoltageRangeValues[0] = ranges[0];
            selectedVoltageRangeValues[1] = ranges[1];
            selectedVoltageRangeValues[2] = ranges[2];
            selectedVoltageRange[0] = voltageRanges[0].indexOf(ranges[0])+1;
            selectedVoltageRange[1] = voltageRanges[1].indexOf(ranges[1])+1;
            selectedVoltageRange[2] = voltageRanges[2].indexOf(ranges[2])+1;
            rangeSelection->setText(ranges[0]);

            timebaseSelection->setText(xmlNode->getStringAttribute("Timebase"));
            spreadSelection->setText(xmlNode->getStringAttribute("Spread"));
            if (xmlNode->hasAttribute("colorGrouping"))
            {
                colorGroupingSelection->setSelectedId(xmlNode->getIntAttribute("colorGrouping"));
            }
            else
            {
                colorGroupingSelection->setSelectedId(1);
            }
            if (xmlNode->hasAttribute("triggerSource"))
            {
                triggerSourceSelection->setSelectedId(xmlNode->getIntAttribute("triggerSource"));
            }
            else
            {
                triggerSourceSelection->setSelectedId(1);
            }
              
            invertInputButton->setToggleState(xmlNode->getBoolAttribute("isInverted", true), sendNotification);

            drawMethodButton->setToggleState(xmlNode->getBoolAttribute("drawMethod", true), sendNotification);

            canvas->viewport->setViewPosition(xmlNode->getIntAttribute("ScrollX"),
                                      xmlNode->getIntAttribute("ScrollY"));

            int eventButtonState = xmlNode->getIntAttribute("EventButtonState");

            for (int i = 0; i < 8; i++)
            {
                lfpDisplay->eventDisplayEnabled[i] = (eventButtonState >> i) & 1;

                eventDisplayInterfaces[i]->checkEnabledState();
            }

            String channelDisplayState = xmlNode->getStringAttribute("ChannelDisplayState");

            for (int i = 0; i < channelDisplayState.length(); i++)
            {

                if (channelDisplayState.substring(i,i+1).equalsIgnoreCase("1"))
                {
                    //std::cout << "LfpDisplayCanvas enabling channel " << i << std::endl;
                    //lfpDisplay->enableChannel(true, i);
                    canvas->isChannelEnabled.set(i,true); //lfpDisplay->enableChannel(true, i);
                }
                else
                {
                    //lfpDisplay->enableChannel(false, i);
                    canvas->isChannelEnabled.set(i,false);
                }

            }
        }
    }

}

