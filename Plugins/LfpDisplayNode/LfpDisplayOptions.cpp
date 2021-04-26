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
#include "ColourSchemes/ChannelColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpDisplayOptions -
// -------------------------------------------------------------

LfpDisplayOptions::LfpDisplayOptions(LfpDisplayCanvas* canvas_, LfpDisplaySplitter* canvasSplit_, 
                                     LfpTimescale* timescale_, LfpDisplay* lfpDisplay_, 
                                     LfpDisplayNode* processor_)
    : canvas(canvas_),
      canvasSplit(canvasSplit_),
      lfpDisplay(lfpDisplay_),
      timescale(timescale_),
      processor(processor_),
      selectedChannelType(DataChannel::HEADSTAGE_CHANNEL),
      labelFont("Default", 13.0f, Font::plain),
      labelColour(100, 100, 100),
      medianOffsetOnForSpikeRaster(false)
{

    // MAIN OPTIONS

    // Timebase
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
    selectedTimebaseValue = timebases[selectedTimebase - 1];

    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(selectedTimebase, sendNotification);
    timebaseSelection->setEditableText(true);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);

    // Channel height
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
    selectedSpreadValue = spreads[selectedSpread - 1];

    spreadSelection = new ComboBox("Spread");
    spreadSelection->addItemList(spreads, 1);
    spreadSelection->setSelectedId(selectedSpread, sendNotification);
    spreadSelection->addListener(this);
    spreadSelection->setEditableText(true);
    addAndMakeVisible(spreadSelection);

    // Voltage range
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
    tbut = new UtilityButton("DATA", Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false, false, false, false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100, dontSendNotification);
    tbut->setToggleState(true, dontSendNotification);
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
    selectedVoltageRange[DataChannel::AUX_CHANNEL] = 9;
    rangeGain[DataChannel::AUX_CHANNEL] = 0.001f; //mV
    rangeSteps[DataChannel::AUX_CHANNEL] = 10;
    rangeUnits.add("mV");
    typeNames.add("AUX");

    tbut = new UtilityButton("AUX", Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false, false, false, false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100, dontSendNotification);
    tbut->setToggleState(false, dontSendNotification);
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

    tbut = new UtilityButton("ADC", Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false, false, false, false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100, dontSendNotification);
    tbut->setToggleState(false, dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);

    selectedVoltageRangeValues[DataChannel::HEADSTAGE_CHANNEL] = voltageRanges[DataChannel::HEADSTAGE_CHANNEL][selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] - 1];
    selectedVoltageRangeValues[DataChannel::AUX_CHANNEL] = voltageRanges[DataChannel::AUX_CHANNEL][selectedVoltageRange[DataChannel::AUX_CHANNEL] - 1];
    selectedVoltageRangeValues[DataChannel::ADC_CHANNEL] = voltageRanges[DataChannel::ADC_CHANNEL][selectedVoltageRange[DataChannel::ADC_CHANNEL] - 1];

    rangeSelection = new ComboBox("Voltage range");
    rangeSelection->addItemList(voltageRanges[DataChannel::HEADSTAGE_CHANNEL], 1);
    rangeSelection->setSelectedId(selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL], sendNotification);
    rangeSelection->setEditableText(true);
    rangeSelection->addListener(this);
    addAndMakeVisible(rangeSelection);

    // Event overlay
    for (int i = 0; i < 8; i++)
    {

        EventDisplayInterface* eventOptions = new EventDisplayInterface(lfpDisplay, canvasSplit, i);
        eventDisplayInterfaces.add(eventOptions);
        addAndMakeVisible(eventOptions);
        lfpDisplay->setEventDisplayState(i, true);
    }

    // Pause button
    pauseButton = new UtilityButton("Pause", Font("Small Text", 13, Font::plain));
    pauseButton->setRadius(5.0f);
    pauseButton->setEnabledState(true);
    pauseButton->setCorners(true, true, true, true);
    pauseButton->addListener(this);
    pauseButton->setClickingTogglesState(true);
    pauseButton->setToggleState(false, sendNotification);
    addAndMakeVisible(pauseButton);

    // Color scheme
    StringArray colourSchemeNames = lfpDisplay->getColourSchemeNameArray();
    colourSchemeOptionSelection = new ComboBox("colorSchemeOptionSelection");
    colourSchemeOptionSelection->addItemList(colourSchemeNames, 1);
    colourSchemeOptionSelection->setEditableText(false);
    colourSchemeOptionSelection->addListener(this);
    colourSchemeOptionSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(colourSchemeOptionSelection);

    // Color grouping
    colorGroupings.add("1");
    colorGroupings.add("2");
    colorGroupings.add("4");
    colorGroupings.add("8");
    colorGroupings.add("16");

    colorGroupingSelection = new ComboBox("Color Grouping");
    colorGroupingSelection->addItemList(colorGroupings, 1);
    colorGroupingSelection->setSelectedId(1, sendNotification);
    colorGroupingSelection->addListener(this);
    addAndMakeVisible(colorGroupingSelection);

    // THRESHOLDS SECTION
    sectionTitles.add("THRESHOLDS");

    // Spike raster
    spikeRasterSelectionOptions = { "OFF", "-50", "-100", "-150", "-200", "-300", "-400", "-500" };
    selectedSpikeRasterThreshold = 1;
    selectedSpikeRasterThresholdValue = spikeRasterSelectionOptions[selectedSpikeRasterThreshold - 1];

    spikeRasterSelection = new ComboBox("spikeRasterSelection");
    spikeRasterSelection->addItemList(spikeRasterSelectionOptions, 1);
    spikeRasterSelection->setSelectedId(selectedSpikeRasterThreshold, dontSendNotification);
    spikeRasterSelection->setEditableText(true);
    spikeRasterSelection->addListener(this);
    addAndMakeVisible(spikeRasterSelection);

    // Clip warning
    clipThresholds.add("OFF");
    clipThresholds.add("ON");

    clipWarningSelection = new ComboBox("Clip Warning");
    clipWarningSelection->addItemList(clipThresholds, 1);
    clipWarningSelection->setSelectedId(1, dontSendNotification);
    clipWarningSelection->addListener(this);
    clipWarningSelection->setEditableText(false);
    addAndMakeVisible(clipWarningSelection);

    // Saturation warning
    saturationThresholds.add("OFF");
    saturationThresholds.add("0.5");
    saturationThresholds.add("100");
    saturationThresholds.add("1000");
    saturationThresholds.add("5000");
    saturationThresholds.add("6389");

    saturationWarningSelection = new ComboBox("Saturation Warning");
    saturationWarningSelection->addItemList(saturationThresholds, 1);
    saturationWarningSelection->setSelectedId(1, dontSendNotification);
    saturationWarningSelection->addListener(this);
    saturationWarningSelection->setEditableText(false);
    addAndMakeVisible(saturationWarningSelection);


    // CHANNELS SECTION
    sectionTitles.add("CHANNELS");

    // Reverse order
    reverseChannelsDisplayButton = new UtilityButton("OFF", labelFont);
    reverseChannelsDisplayButton->setRadius(5.0f);
    reverseChannelsDisplayButton->setEnabledState(true);
    reverseChannelsDisplayButton->setCorners(true, true, true, true);
    reverseChannelsDisplayButton->addListener(this);
    reverseChannelsDisplayButton->setClickingTogglesState(true);
    reverseChannelsDisplayButton->setToggleState(false, sendNotification);
    addAndMakeVisible(reverseChannelsDisplayButton);

    // Sort by depth
    sortByDepthButton = new UtilityButton("OFF", labelFont);
    sortByDepthButton->setRadius(5.0f);
    sortByDepthButton->setEnabledState(true);
    sortByDepthButton->setCorners(true, true, true, true);
    sortByDepthButton->addListener(this);
    sortByDepthButton->setClickingTogglesState(true);
    sortByDepthButton->setToggleState(false, sendNotification);
    addAndMakeVisible(sortByDepthButton);

    // Channel skip
    channelDisplaySkipOptions.add("None");
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

    // Show channel number button
    showChannelNumberButton = new UtilityButton("OFF", labelFont);
    showChannelNumberButton->setRadius(5.0f);
    showChannelNumberButton->setEnabledState(true);
    showChannelNumberButton->setCorners(true, true, true, true);
    showChannelNumberButton->addListener(this);
    showChannelNumberButton->setClickingTogglesState(true);
    showChannelNumberButton->setToggleState(false, sendNotification);
    addAndMakeVisible(showChannelNumberButton);

    // SIGNAL PROCESSING SECTION
    sectionTitles.add("SIGNALS");

    // invert signal
    invertInputButton = new UtilityButton("OFF", labelFont);
    invertInputButton->setRadius(5.0f);
    invertInputButton->setEnabledState(true);
    invertInputButton->setCorners(true, true, true, true);
    invertInputButton->addListener(this);
    invertInputButton->setClickingTogglesState(true);
    invertInputButton->setToggleState(false, sendNotification);
    addAndMakeVisible(invertInputButton);

    // subtract offset
    medianOffsetPlottingButton = new UtilityButton("OFF", labelFont);
    medianOffsetPlottingButton->setRadius(5.0f);
    medianOffsetPlottingButton->setEnabledState(true);
    medianOffsetPlottingButton->setCorners(true, true, true, true);
    medianOffsetPlottingButton->addListener(this);
    medianOffsetPlottingButton->setClickingTogglesState(true);
    medianOffsetPlottingButton->setToggleState(false, sendNotification);
    addAndMakeVisible(medianOffsetPlottingButton);

    // TRIGGERED DISPLAY
    sectionTitles.add("TRIGGERED DISPLAY");
    // trigger channel selection
    triggerSources.add("None");
    for (int k = 1; k <= 8; k++)
    {
        triggerSources.add(String(k));
    }

    triggerSourceSelection = new ComboBox("Trigger Source");
    triggerSourceSelection->addItemList(triggerSources, 1);
    triggerSourceSelection->setSelectedId(1, sendNotification);
    triggerSourceSelection->addListener(this);
    addAndMakeVisible(triggerSourceSelection);

    // average signal
    averageSignalButton = new UtilityButton("OFF", labelFont);
    averageSignalButton->setRadius(5.0f);
    averageSignalButton->setEnabledState(true);
    averageSignalButton->setCorners(true, true, true, true);
    averageSignalButton->addListener(this);
    averageSignalButton->setClickingTogglesState(true);
    averageSignalButton->setToggleState(false, sendNotification);
    addAndMakeVisible(averageSignalButton);

    // reset triggered display
    resetButton = new UtilityButton("RESET", labelFont);
    resetButton->setRadius(5.0f);
    resetButton->setEnabledState(true);
    resetButton->setCorners(true, true, true, true);
    resetButton->addListener(this);
    resetButton->setClickingTogglesState(false);
    resetButton->setToggleState(false, sendNotification);
    addChildComponent(resetButton);

    // init show/hide options button
    showHideOptionsButton = new ShowHideOptionsButton(this);
    showHideOptionsButton->addListener(this);
    addAndMakeVisible(showHideOptionsButton);

    // do we still need this?
    overlaps.add("0.5");
    overlaps.add("0.75");
    overlaps.add("1");
    overlaps.add("2");
    overlaps.add("3");
    overlaps.add("4");
    overlaps.add("5");
    selectedOverlap = 4;
    selectedOverlapValue = overlaps[selectedOverlap-1];

    overlapSelection = new ComboBox("Overlap");
    overlapSelection->addItemList(overlaps, 1);
    overlapSelection->setSelectedId(selectedOverlap,sendNotification);
    overlapSelection->addListener(this);
    overlapSelection->setEditableText(true);
    addAndMakeVisible(overlapSelection);
    

    //channelDisplaySkipLabel = new Label("Channel Display Skip", "Ch. Skip");
   //channelDisplaySkipLabel->setFont(labelFont);
   //channelDisplaySkipLabel->setColour(Label::textColourId, labelColour);
   //addAndMakeVisible(channelDisplaySkipLabel);

   // init spike raster options

   //spikeRasterLabel = new Label("spikeRasterLabel", "Spike Raster Thresh.");
   //spikeRasterLabel->setFont(labelFont);
   //spikeRasterLabel->setColour(Label::textColourId, labelColour);
  // addAndMakeVisible(spikeRasterLabel);

   // init median offset plotting
  // medianOffsetPlottingLabel = new Label("Median Offset Correction", "Median Offset Correction");
  // medianOffsetPlottingLabel->setFont(labelFont);
  // medianOffsetPlottingLabel->setColour(Label::textColourId, labelColour);
  // addAndMakeVisible(medianOffsetPlottingLabel);

   //init channel name toggle
   //showChannelNumberLabel = new Label("showcChannelLabel", "Show channel number instead of name");
  // showChannelNumberLabel->setFont(labelFont);
   //showChannelNumberLabel->setColour(Label::textColourId, labelColour);
   //addAndMakeVisible(showChannelNumberLabel);
    
    //reverseChannelsDisplayLabel = new Label("Rev. Channels", "Rev. Channels");
   // reverseChannelsDisplayLabel->setFont(labelFont);
    //reverseChannelsDisplayLabel->setColour(Label::textColourId, labelColour);
    //addAndMakeVisible(reverseChannelsDisplayLabel);
    
    //button for controlling drawing algorithm - old line-style or new per-pixel style
    //drawMethodButton = new UtilityButton("DrawMethod", Font("Small Text", 13, Font::plain));
    //drawMethodButton->setRadius(5.0f);
    //drawMethodButton->setEnabledState(true);
    //drawMethodButton->setCorners(true, true, true, true);
   // drawMethodButton->addListener(this);
   // drawMethodButton->setClickingTogglesState(true);
   // drawMethodButton->setToggleState(false, sendNotification);
   // addAndMakeVisible(drawMethodButton);
    
    // two sliders for the two histogram components of the supersampled plotting mode
    // todo: rename these
    //brightnessSliderA = new Slider();
    //brightnessSliderA->setRange (0, 1);
    //brightnessSliderA->setTextBoxStyle(Slider::NoTextBox, false, 50,30);
    //brightnessSliderA->addListener(this);
    //addAndMakeVisible (brightnessSliderA);
    
   // brightnessSliderB = new Slider;
    //brightnessSliderB->setRange (0, 1);
   // brightnessSliderB->setTextBoxStyle(Slider::NoTextBox, false, 50,30);
   // brightnessSliderB->addListener(this);
   // addAndMakeVisible (brightnessSliderB);
    
   // sliderALabel = new Label("Brightness","Brightness");
   // sliderALabel->setFont(Font("Small Text", 13, Font::plain));
    //sliderALabel->setColour(Label::textColourId,Colour(150,150,150));
    //addAndMakeVisible(sliderALabel);
    
   // sliderBLabel = new Label("Min. brightness","Min. brightness");
   // sliderBLabel->setFont(Font("Small Text", 13, Font::plain));
   // sliderBLabel->setColour(Label::textColourId,Colour(150,150,150));
   // addAndMakeVisible(sliderBLabel);
    
    //ScopedPointer<UtilityButton> drawClipWarningButton; // optinally draw (subtle) warning if data is clipped in display
    /*drawClipWarningButton = new UtilityButton("0", Font("Small Text", 13, Font::plain));
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
    addAndMakeVisible(drawSaturateWarningButton);*/
    
    //button for pausing the display - works by skipping buffer updates. This way scrolling etc still works

        // draw the colour scheme options
// TODO: (kelly) this might be better as a modal window
//colourSchemeOptionLabel = new Label("colorSchemeOptionLabel", "Color Scheme");
//colourSchemeOptionLabel->setFont(labelFont);
//colourSchemeOptionLabel->setColour(Label::textColourId, labelColour);
//addAndMakeVisible(colourSchemeOptionLabel);

   // if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
    //    addAndMakeVisible(lfpDisplay->getColourSchemePtr());

 //Ranges for neural data
    

    lfpDisplay->setRange(voltageRanges[DataChannel::HEADSTAGE_CHANNEL][selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::HEADSTAGE_CHANNEL]
        , DataChannel::HEADSTAGE_CHANNEL);
    lfpDisplay->setRange(voltageRanges[DataChannel::ADC_CHANNEL][selectedVoltageRange[DataChannel::ADC_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::ADC_CHANNEL]
        , DataChannel::ADC_CHANNEL);
    lfpDisplay->setRange(voltageRanges[DataChannel::AUX_CHANNEL][selectedVoltageRange[DataChannel::AUX_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::AUX_CHANNEL]
        , DataChannel::AUX_CHANNEL);

    canvasSplit->options = this;
    
}

LfpDisplayOptions::~LfpDisplayOptions()
{

}

void LfpDisplayOptions::resized()
{
    int height = 22;

    // MAIN OPTIONS
    timebaseSelection->setBounds(8, getHeight() - 30, 90, height);
    spreadSelection->setBounds(timebaseSelection->getRight() + 20, getHeight() - 30, 90, height);
    //overlapSelection->setBounds(spreadSelection->getRight() + 15, getHeight() - 30, 75, height);
    rangeSelection->setBounds(spreadSelection->getRight() + 20, getHeight()-30, 90, height);

    int bh = 25 / typeButtons.size();
    for (int i = 0; i < typeButtons.size(); i++)
    {
        typeButtons[i]->setBounds(rangeSelection->getRight()+5, getHeight() - 30 + i * bh, 50, bh);
    }

    for (int i = 0; i < 8; i++)
    {
        eventDisplayInterfaces[i]->setBounds(typeButtons[0]->getRight() + 120 + (floor(i / 2) * 20),
                                                            getHeight() - 45 + (i % 2) * 20, 20, 20); // arrange event channel buttons in two rows
        eventDisplayInterfaces[i]->repaint();
    }

    pauseButton->setBounds(650, getHeight() - 40, 70, 30);
    
    colourSchemeOptionSelection->setBounds(pauseButton->getRight() + 40,
        getHeight() - 30,
        180,
        height);

    colorGroupingSelection->setBounds(colourSchemeOptionSelection->getRight() + 15, getHeight() - 30, 55, height);

    int startHeight = 167;
    int verticalSpacing = 29;
    int xOffset = 200;

    // THRESHOLDS
    spikeRasterSelection->setBounds(96,
        getHeight() - startHeight,
        80,
        height);

    clipWarningSelection->setBounds(96,
        getHeight() - startHeight + verticalSpacing, 
        80, 
        height);

    saturationWarningSelection->setBounds(96,
        getHeight() - startHeight + verticalSpacing * 2,
        80, 
        height);

    // CHANNELS
    
    reverseChannelsDisplayButton->setBounds(getWidth() / 4 + 107,
        getHeight() - startHeight,
        35,
        height);

    sortByDepthButton->setBounds(getWidth() / 4 + 102,
        getHeight() - startHeight + verticalSpacing,
        35,
        height);

    channelDisplaySkipSelection->setBounds(getWidth() / 4 + 47,
        getHeight() - startHeight + verticalSpacing *2,
        80,
        height);
    
    showChannelNumberButton->setBounds(getWidth() / 4 + 102,
             getHeight() - startHeight + +verticalSpacing*3,
             35,
        height);

    // SIGNAL PROCESSING
    invertInputButton->setBounds(getWidth() / 2 + 95,
        getHeight() - startHeight, 
        35, 
        height);

    medianOffsetPlottingButton->setBounds(getWidth() / 2 + 110,
        getHeight()- startHeight + verticalSpacing,
        35,
        height);

    //TRIGGERED DISPLAY
    triggerSourceSelection->setBounds(getWidth() / 4 * 3 + 118,
        getHeight()-startHeight,
        80,
        height);

    averageSignalButton->setBounds(getWidth() / 4 * 3 + 112,
        getHeight() - startHeight + verticalSpacing, 
        35, 
        height);

    resetButton->setBounds(getWidth() / 4 * 3 + 156,
        getHeight() - startHeight + verticalSpacing, 
        50, 
        height);


    showHideOptionsButton->setBounds (getWidth() - 28, getHeight() - 28, 20, 20);

   // colourSchemeOptionLabel->setBounds(medianOffsetPlottingButton->getX(),
    //                                   getHeight()-190,
     //                                  100,
    //                                   22);
    
    
    // set the size of the active colour scheme's options, if it has configurable options
   /* if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
    {
        lfpDisplay->getColourSchemePtr()->setBounds(colourSchemeOptionLabel->getX(),
                                                    colourSchemeOptionLabel->getBottom(),
                                                    200,
                                                    110);
    }*/
}

void LfpDisplayOptions::paint(Graphics& g)
{
    int row1 = 55;
    int row2 = 110;

    g.fillAll(Colours::black);
    g.setFont(Font("Default", 20, Font::plain));

    g.setColour(Colour(100,100,100));

    if (getHeight() > 150)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i > 0)
                g.drawLine(getWidth() / 4 * i, getHeight() - 200, getWidth() / 4 * i, 150);

            g.drawText(sectionTitles[i], 
                getWidth()/4 * i + 10, 
                7, 
                300, 
                20, 
                Justification::left, false);
        }
  
    }

    g.setFont(Font("FiraSans", 16, Font::plain));
    g.drawText("Timebase (s)", timebaseSelection->getX(), timebaseSelection->getY()-22, 300, 20, Justification::left, false);
    g.drawText("Chan height (px)", spreadSelection->getX(), spreadSelection->getY() - 22, 300, 20, Justification::left, false);
    //g.drawText("Overlap", overlapSelection->getX(), overlapSelection->getY() - 22, 300, 20, Justification::left, false);
    g.drawText("Range (" + rangeUnits[selectedChannelType] + ")", rangeSelection->getX(), rangeSelection->getY() - 22, 300, 20, Justification::left, false);

    g.drawText("Overlay", 380, getHeight() - 43, 100, 20, Justification::right, false);
    g.drawText("Events:", 380, getHeight() - 25, 100, 20, Justification::right, false);

    g.drawText("Color scheme", colourSchemeOptionSelection->getX(), colourSchemeOptionSelection->getY() - 22, 300, 20, Justification::left, false);
    g.drawText("Color grouping", colorGroupingSelection->getX(), colorGroupingSelection->getY() - 22, 300, 20, Justification::left, false);

    g.drawText("Spike raster:",
        10,
        spikeRasterSelection->getY(),
        100,
        22,
        Justification::left,
        false);

    g.drawText("Clip warning:",
        10,
        clipWarningSelection->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Sat. warning:",
        10,
        saturationWarningSelection->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Reverse order:",
        getWidth() / 4 + 10,
        reverseChannelsDisplayButton->getY(),
        100,
        22,
        Justification::left,
        false);

    g.drawText("Sort by depth:",
        getWidth() / 4 + 10,
        sortByDepthButton->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Skip:",
        getWidth() / 4 + 10,
        channelDisplaySkipSelection->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Show number:",
        getWidth() / 4 + 10,
        showChannelNumberButton->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Invert signal:",
        getWidth() / 2 + 10,
        invertInputButton->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Subtract offset:",
        getWidth() / 2 + 10,
        medianOffsetPlottingButton->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Trigger channel:",
        getWidth() / 4 * 3 + 10,
        triggerSourceSelection->getY(),
        150,
        22,
        Justification::left,
        false);

    g.drawText("Trial averaging:",
        getWidth() / 4 * 3 + 10,
        averageSignalButton->getY(),
        150,
        22,
        Justification::left,
        false);

    /*g.drawText("Range("+ rangeUnits[selectedChannelType] +")",115,getHeight()-row1,300,20,Justification::left, false);
    
    g.drawText("Size(px)",5,getHeight()-row2,300,20,Justification::left, false);
    g.drawText("Clip",100,getHeight()-row2,300,20,Justification::left, false);
    g.drawText("Warn",168,getHeight()-row2,300,20,Justification::left, false);
    
    g.drawText("Sat. Warning",225,getHeight()-row2,300,20,Justification::left, false);

    g.drawText("Color grouping",365,getHeight()-row2,300,20,Justification::left, false);

    g.drawText("Event disp.",375,getHeight()-row1,300,20,Justification::left, false);
    g.drawText("Trigger",475,getHeight()-row1,300,20,Justification::left, false);

    if(canvasSplit->drawClipWarning)
    {
        g.setColour(Colours::white);
        g.fillRoundedRectangle(173,getHeight()-90-1,24,24,6.0f);
    }
    
    if(canvasSplit->drawSaturationWarning)
    {
        g.setColour(Colours::red);
        g.fillRoundedRectangle(323,getHeight()-90-1,24,24,6.0f);
    }*/


}

int LfpDisplayOptions::getChannelHeight()
{
    return (int)spreadSelection->getText().getIntValue();
}

bool LfpDisplayOptions::getDrawMethodState()
{
    
    return true; // drawMethodButton->getToggleState();
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

        canvasSplit->repaint();
        canvasSplit->refresh();
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
            canvasSplit->repaint();
            canvasSplit->refresh();
        }
    }
}

void LfpDisplayOptions::togglePauseButton(bool sendUpdate)
{
    pauseButton->setToggleState(!pauseButton->getToggleState(), sendUpdate ? sendNotification : dontSendNotification);
}

void LfpDisplayOptions::setChannelsReversed(bool state)
{
    lfpDisplay->setChannelsReversed(state);
    canvasSplit->fullredraw = true;

    reverseChannelsDisplayButton->setToggleState(state, dontSendNotification);

    if (state)
    {
        reverseChannelsDisplayButton->setLabel("ON");
    }
    else {
        reverseChannelsDisplayButton->setLabel("OFF");
    }
}

void LfpDisplayOptions::setInputInverted(bool state)
{
    lfpDisplay->setInputInverted(state);

    invertInputButton->setToggleState(state, dontSendNotification);

    if (state)
    {
        invertInputButton->setLabel("ON");
    }
    else {
        invertInputButton->setLabel("OFF");
    }

}

void LfpDisplayOptions::setMedianOffset(bool state)
{
    if (lfpDisplay->getSpikeRasterPlotting())
    {
        medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
        medianOffsetPlottingButton->setLabel("ON");
        return;
    }
    else
    {
        lfpDisplay->setMedianOffsetPlotting(state);
        medianOffsetPlottingButton->setToggleState(state, dontSendNotification);
    }


    if (state)
    {
        medianOffsetPlottingButton->setLabel("ON");
    }
    else {
        medianOffsetPlottingButton->setLabel("OFF");
    }
}

void LfpDisplayOptions::setAveraging(bool state)
{
    canvasSplit->setAveraging(state);

    averageSignalButton->setToggleState(state, dontSendNotification);

    if (state)
    {
        averageSignalButton->setLabel("ON");
        resetButton->setVisible(true);
    }
    else {
        averageSignalButton->setLabel("OFF");
        resetButton->setVisible(false);
    }
}

void LfpDisplayOptions::setSortByDepth(bool state)
{
    lfpDisplay->orderChannelsByDepth(state);

    sortByDepthButton->setToggleState(state, dontSendNotification);

    if (state)
    {
        sortByDepthButton->setLabel("ON");
    }
    else {
        sortByDepthButton->setLabel("OFF");
    }
}

void LfpDisplayOptions::setShowChannelNumbers(bool state)
{

    showChannelNumberButton->setToggleState(state, dontSendNotification);

    int numChannels = lfpDisplay->channelInfo.size();

    for (int i = 0; i < numChannels; ++i)
    {
        lfpDisplay->channelInfo[i]->repaint();
    }

    if (state)
    {
        showChannelNumberButton->setLabel("ON");
    }
    else {
        showChannelNumberButton->setLabel("OFF");
    }
}

void LfpDisplayOptions::buttonClicked(Button* b)
{
    if (b == invertInputButton)
    {
        setInputInverted(b->getToggleState());
        return;
    }
    if (b == reverseChannelsDisplayButton)
    {
        setChannelsReversed(b->getToggleState());
        return;
    }
    if (b == medianOffsetPlottingButton)
    {
        setMedianOffset(b->getToggleState());
        return;
    } 
    
    if (b == averageSignalButton)
    {
        setAveraging(b->getToggleState());
        return;
    }

    if (b == sortByDepthButton)
    {
        setSortByDepth(b->getToggleState());
        return;
    }

    if (b == resetButton)
    {
        canvasSplit->resetTrials();
    }

    /*if (b == drawMethodButton)
    {
        lfpDisplay->setDrawMethod(b->getToggleState()); // this should be done the same way as drawClipWarning - or the other way around.
        
        return;
    }
    if (b == drawClipWarningButton)
    {
        canvasSplit->drawClipWarning = b->getToggleState();
        canvasSplit->redraw();
        return;
    }
    if (b == drawSaturateWarningButton)
    {
        canvasSplit->drawSaturationWarning = b->getToggleState();
        canvasSplit->redraw();
        return;
    }*/
    
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
        setShowChannelNumbers(b->getToggleState());
        return;
    }

    int idx = typeButtons.indexOf((UtilityButton*) b);

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
    canvasSplit->setTimebase(timebase);
    
    if (canvasSplit->timebase) // if timebase != 0
    {
        if (canvasSplit->timebase < timebases[0].getFloatValue())
        {
            timebaseSelection->setSelectedId(1, dontSendNotification);
            canvasSplit->setTimebase(timebases[0].getFloatValue());
        }
        else if (canvasSplit->timebase > timebases[timebases.size()-1].getFloatValue())
        {
            timebaseSelection->setSelectedId(timebases.size(), dontSendNotification);
            canvasSplit->setTimebase(timebases[timebases.size()-1].getFloatValue());
        }
        else{
            timebaseSelection->setText(String(canvasSplit->timebase, 1), dontSendNotification);
        }
    }
    else
    {
        if (selectedSpread == 0)
        {
            timebaseSelection->setText(selectedTimebaseValue, dontSendNotification);
            canvasSplit->setTimebase(selectedTimebaseValue.getFloatValue());
        }
        else
        {
            timebaseSelection->setSelectedId(selectedTimebase,dontSendNotification);
            canvasSplit->setTimebase(timebases[selectedTimebase-1].getFloatValue());
        }
        
    }

    timescale->setTimebase(canvasSplit->timebase);
}

void LfpDisplayOptions::comboBoxChanged(ComboBox* cb)
{
    if (canvasSplit->getNumChannels() == 0) return;
    
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

                if (medianOffsetOnForSpikeRaster)
                {
                    medianOffsetPlottingButton->setToggleState(false, sendNotification);
                    medianOffsetOnForSpikeRaster = false;
                }
                return;
            }
            
            if (val > 500)
            {
                val = 500;
            }
            
            val *= -1;
            
            spikeRasterSelection->setText(String(val), dontSendNotification);
            lfpDisplay->setSpikeRasterThreshold(val);

            if (!medianOffsetPlottingButton->getToggleState())
            {
                medianOffsetPlottingButton->setToggleState(true, sendNotification);
                medianOffsetOnForSpikeRaster = true;
            }
            else {
                medianOffsetOnForSpikeRaster = false;
            }
                
            lfpDisplay->setSpikeRasterPlotting(true);
        }
        else if (cb->getSelectedItemIndex() == 0) // if "Off"
        {

            lfpDisplay->setSpikeRasterPlotting(false);

            if (medianOffsetOnForSpikeRaster)
            {
                medianOffsetPlottingButton->setToggleState(false, sendNotification);
                medianOffsetOnForSpikeRaster = false;
            }
            return;
        }
        else
        {
            auto val = cb->getText().getFloatValue();

            lfpDisplay->setSpikeRasterThreshold(val);

            if (!medianOffsetPlottingButton->getToggleState())
            {
                medianOffsetPlottingButton->setToggleState(true, sendNotification);
                medianOffsetOnForSpikeRaster = true;
            }
                

            lfpDisplay->setSpikeRasterPlotting(true);
        }
    }
    else if (cb == colourSchemeOptionSelection)
    {
        lfpDisplay->setActiveColourSchemeIdx(cb->getSelectedId()-1);

        lfpDisplay->setColors();
        canvasSplit->redraw();
    }
    else if (cb == timebaseSelection)
    {
        if (cb->getSelectedId())
        {
            canvasSplit->setTimebase(timebases[cb->getSelectedId() - 1].getFloatValue());
        }
        else
        {
            setTimebaseAndSelectionText(cb->getText().getFloatValue());
        }

        timescale->setTimebase(canvasSplit->timebase);
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
        canvasSplit->redraw();
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
                    canvasSplit->resized();
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

        if (!lfpDisplay->getSingleChannelState()) canvasSplit->redraw();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == saturationWarningSelection)
    {
        if (cb->getSelectedId() > 1)
        {
            selectedSaturationValueFloat = (saturationThresholds[cb->getSelectedId()-1].getFloatValue());
            canvasSplit->drawSaturationWarning = true;
        }
        else
        {
            canvasSplit->drawSaturationWarning = false;
        }

        canvasSplit->redraw();

        //std::cout << "Setting saturation warning to to " << selectedSaturationValueFloat << std::endl;
    }
    else if (cb == clipWarningSelection)
    {
        if (cb->getSelectedId() == 1)
        {
            canvasSplit->drawClipWarning = false;
        }
        else
        {
            canvasSplit->drawClipWarning = true;
        }

        canvasSplit->redraw();

    //std::cout << "Setting saturation warning to to " << selectedSaturationValueFloat << std::endl;
    }
    else if (cb == overlapSelection)
    {
        if (cb->getSelectedId())
        {
            canvasSplit->channelOverlapFactor = (overlaps[cb->getSelectedId()-1].getFloatValue());
            canvasSplit->resized();
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
                canvasSplit->channelOverlapFactor= overlap;
                canvasSplit->resized();
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
        canvasSplit->redraw();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == colorGroupingSelection)
    {
        // set color grouping here
        lfpDisplay->setColorGrouping(colorGroupings[cb->getSelectedId()-1].getIntValue());// so that channel colors get re-assigned
        canvasSplit->redraw();
    }
    else if (cb == triggerSourceSelection)
    {
        canvasSplit->setTriggerChannel(cb->getSelectedId() - 2);
        processor->setParameter(cb->getSelectedId()-2, float(canvasSplit->splitID));
    }


    
}

void LfpDisplayOptions::sliderValueChanged(Slider* sl)
{
    /*if (sl == brightnessSliderA)
        canvasSplit->histogramParameterA = sl->getValue();

    if (sl == brightnessSliderB)
        canvasSplit->histogramParameterB = sl->getValue();

    canvasSplit->fullredraw=true;
    //repaint();
    canvasSplit->refresh();*/

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

    XmlElement* xmlNode = xml->createNewChildElement("LFPDISPLAY" + String(canvasSplit->splitID));

    xmlNode->setAttribute("SubprocessorID",canvasSplit->subprocessorSelection->getSelectedId());

    xmlNode->setAttribute("Range",selectedVoltageRangeValues[0]+","+selectedVoltageRangeValues[1]+
        ","+selectedVoltageRangeValues[2]);
    xmlNode->setAttribute("Timebase",timebaseSelection->getText());
    xmlNode->setAttribute("Spread",spreadSelection->getText());
    xmlNode->setAttribute("colourScheme", colourSchemeOptionSelection->getSelectedId());
    xmlNode->setAttribute("colorGrouping",colorGroupingSelection->getSelectedId());
    
    xmlNode->setAttribute("spikeRaster", spikeRasterSelection->getText());
    xmlNode->setAttribute("clipWarning", clipWarningSelection->getSelectedId());
    xmlNode->setAttribute("satWarning", saturationWarningSelection->getSelectedId());

    xmlNode->setAttribute("reverseOrder", reverseChannelsDisplayButton->getToggleState());
    xmlNode->setAttribute("sortByDepth", sortByDepthButton->getToggleState());
    xmlNode->setAttribute("channelSkip", channelDisplaySkipSelection->getSelectedId());
    xmlNode->setAttribute("showChannelNum", showChannelNumberButton->getToggleState());
    xmlNode->setAttribute("subtractOffset", medianOffsetPlottingButton->getToggleState());

    xmlNode->setAttribute("isInverted",invertInputButton->getToggleState());
    
    xmlNode->setAttribute("triggerSource", triggerSourceSelection->getSelectedId());
    xmlNode->setAttribute("trialAvg", averageSignalButton->getToggleState());

    xmlNode->setAttribute("singleChannelView", lfpDisplay->getSingleChannelShown());

    

    int eventButtonState = 0;

    for (int i = 0; i < 8; i++)
    {
        if (lfpDisplay->eventDisplayEnabled[i])
        {
            eventButtonState += (1 << i);
        }
    }

    xmlNode->setAttribute("EventButtonState", eventButtonState);

    String channelDisplayState = "";

    for (int i = 0; i < canvasSplit->nChans; i++)
    {
        if (lfpDisplay->savedChannelState[i])
        {
            channelDisplayState += "1";
        }
        else
        {
            channelDisplayState += "0";
        }
    }

    xmlNode->setAttribute("ChannelDisplayState", channelDisplayState);

    xmlNode->setAttribute("ScrollX",canvasSplit->viewport->getViewPositionX());
    xmlNode->setAttribute("ScrollY",canvasSplit->viewport->getViewPositionY());
}

void LfpDisplayOptions::loadParameters(XmlElement* xml)
{

    canvasSplit->isLoading = true;

    forEachXmlChildElement(*xml, xmlNode)
    {

        if (xmlNode->hasTagName("LFPDISPLAY" + String(canvasSplit->splitID)))
        {
            uint32 id = xmlNode->getIntAttribute("SubprocessorID");

            //std::cout << "Loading options for display " << canvasSplit->splitID << std::endl;

            //std::cout << "Saved subprocessor ID: " << id << std::endl;

            if (canvasSplit->displayBuffer != nullptr)
                canvasSplit->displayBuffer->removeDisplay(canvasSplit->splitID);

            /*std::cout << "Available IDs: " << std::endl;

            for (auto db : processor->getDisplayBuffers())
            {
                std::cout << " " << db->id << std::endl;
            }*/

            if (processor->displayBufferMap.find(id) == processor->displayBufferMap.end())
                canvasSplit->displayBuffer = processor->getDisplayBuffers().getFirst();   
            else
                canvasSplit->displayBuffer = processor->displayBufferMap[id];

            canvasSplit->displayBuffer->addDisplay(canvasSplit->splitID);

            //std::cout << "Set to ID: " << canvasSplit->displayBuffer->id << std::endl;
            
            // RANGE
            StringArray ranges;
            ranges.addTokens(xmlNode->getStringAttribute("Range"),",",String::empty);
            selectedVoltageRangeValues[0] = ranges[0];
            selectedVoltageRangeValues[1] = ranges[1];
            selectedVoltageRangeValues[2] = ranges[2];
            selectedVoltageRange[0] = voltageRanges[0].indexOf(ranges[0])+1;
            selectedVoltageRange[1] = voltageRanges[1].indexOf(ranges[1])+1;
            selectedVoltageRange[2] = voltageRanges[2].indexOf(ranges[2])+1;
            rangeSelection->setText(ranges[0]);
            lfpDisplay->setRange(ranges[0].getFloatValue() * rangeGain[0], DataChannel::HEADSTAGE_CHANNEL);
            lfpDisplay->setRange(ranges[1].getFloatValue() * rangeGain[1], DataChannel::AUX_CHANNEL);
            lfpDisplay->setRange(ranges[2].getFloatValue() * rangeGain[2], DataChannel::ADC_CHANNEL);

            // TIMEBASE
            timebaseSelection->setText(xmlNode->getStringAttribute("Timebase"), dontSendNotification);
            canvasSplit->setTimebase(xmlNode->getStringAttribute("Timebase").getFloatValue());

            // SPREAD
            spreadSelection->setText(xmlNode->getStringAttribute("Spread"), dontSendNotification);

            // COLOUR SCHEME
            lfpDisplay->setActiveColourSchemeIdx(xmlNode->getIntAttribute("colourScheme") - 1);
            colourSchemeOptionSelection->setSelectedId(xmlNode->getIntAttribute("colourScheme"), dontSendNotification);

            // COLOUR GROUPING
            colorGroupingSelection->setSelectedId(xmlNode->getIntAttribute("colorGrouping"), dontSendNotification);
            lfpDisplay->setColorGrouping(colorGroupings[colorGroupingSelection->getSelectedId() - 1].getIntValue());

            // SPIKE RASTER
            String spikeRasterThresh = xmlNode->getStringAttribute("spikeRaster", "OFF");
            spikeRasterSelection->setText(spikeRasterThresh, dontSendNotification);
            if (!spikeRasterThresh.equalsIgnoreCase("OFF"))
            {
                lfpDisplay->setSpikeRasterPlotting(true);
                lfpDisplay->setSpikeRasterThreshold(spikeRasterThresh.getFloatValue());
            }

            // CLIP WARNING
            int clipWarning = xmlNode->getIntAttribute("clipWarning", 1);
            clipWarningSelection->setSelectedId(clipWarning, dontSendNotification);
            if (clipWarning == 2)
                canvasSplit->drawClipWarning = true;

            // SATURATION WARNING
            saturationWarningSelection->setSelectedId(xmlNode->getIntAttribute("satWarning"), dontSendNotification);

            if (saturationWarningSelection->getSelectedId() > 1)
            {
                selectedSaturationValueFloat = (saturationThresholds[saturationWarningSelection->getSelectedId() - 1].getFloatValue());
                canvasSplit->drawSaturationWarning = true;
            }

            // TOGGLE BUTTONS
            setChannelsReversed(xmlNode->getBoolAttribute("reverseOrder", false));
            setSortByDepth(xmlNode->getBoolAttribute("sortByDepth", false));
            setShowChannelNumbers(xmlNode->getBoolAttribute("showChannelNum", false));
            setInputInverted(xmlNode->getBoolAttribute("isInverted", false));
            setAveraging(xmlNode->getBoolAttribute("trialAvg", false));
            setMedianOffset(xmlNode->getBoolAttribute("subtractOffset", false));

            // CHANNEL SKIP
            channelDisplaySkipSelection->setSelectedId(xmlNode->getIntAttribute("channelSkip"), dontSendNotification);
            const int skipAmt = pow(2, channelDisplaySkipSelection->getSelectedId() - 1);
            lfpDisplay->setChannelDisplaySkipAmount(skipAmt);

            // TRIGGER SOURCE
            triggerSourceSelection->setSelectedId(xmlNode->getIntAttribute("triggerSource"), dontSendNotification);
            canvasSplit->setTriggerChannel(triggerSourceSelection->getSelectedId() - 2);
            processor->setParameter(triggerSourceSelection->getSelectedId() - 2, float(canvasSplit->splitID));
            
           // drawMethodButton->setToggleState(xmlNode->getBoolAttribute("drawMethod", true), sendNotification);

            lfpDisplay->setScrollPosition(xmlNode->getIntAttribute("ScrollX"),
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
                    lfpDisplay->setEnabledState(true, i, true);
                }
                else
                {
                    lfpDisplay->setEnabledState(false, i, true);
                }

            }

            lfpDisplay->setSingleChannelView(xmlNode->getIntAttribute("singleChannelView", -1));

            lfpDisplay->setColors();
            canvasSplit->redraw();

            lfpDisplay->restoreViewPosition();
        }

        
    }

   // std::cout << "Finished loading LFP options." << std::endl;

}

