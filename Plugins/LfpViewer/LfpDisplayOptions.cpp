/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "ColourSchemes/ChannelColourScheme.h"
#include "EventDisplayInterface.h"
#include "LfpBitmapPlotter.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "LfpDisplay.h"
#include "LfpDisplayCanvas.h"
#include "LfpDisplayNode.h"
#include "LfpTimescale.h"
#include "LfpViewport.h"
#include "PerPixelBitmapPlotter.h"
#include "ShowHideOptionsButton.h"
#include "SupersampledBitmapPlotter.h"

#include <math.h>

#define MS_FROM_START Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - start) * 1000

using namespace LfpViewer;

LfpDisplayOptions::LfpDisplayOptions (LfpDisplayCanvas* canvas_, LfpDisplaySplitter* canvasSplit_, LfpTimescale* timescale_, LfpDisplay* lfpDisplay_, LfpDisplayNode* processor_)
    : canvas (canvas_),
      canvasSplit (canvasSplit_),
      lfpDisplay (lfpDisplay_),
      timescale (timescale_),
      processor (processor_),
      selectedChannelType (ContinuousChannel::Type::ELECTRODE),
      labelColour (100, 100, 100),
      medianOffsetOnForSpikeRaster (false),
      ttlWordString ("NONE")
{
    setBufferedToImage (true);
    // MAIN OPTIONS

    // Timebase
    timebases.add ("0.050");
    timebases.add ("0.100");
    timebases.add ("0.250");
    timebases.add ("0.500");
    timebases.add ("1.0");
    timebases.add ("2.0");
    timebases.add ("3.0");
    timebases.add ("4.0");
    timebases.add ("5.0");
    timebases.add ("10.0");
    timebases.add ("20.0");
    selectedTimebase = 6;
    selectedTimebaseValue = timebases[selectedTimebase - 1];

    timebaseSelection = std::make_unique<ComboBox> ("Timebase");
    for (int i = 0; i < timebases.size(); i++)
        timebaseSelection->addItem (timebases[i], i + 1);
    timebaseSelection->setSelectedId (selectedTimebase, sendNotification);
    timebaseSelection->setEditableText (true);
    timebaseSelection->addListener (this);
    addAndMakeVisible (timebaseSelection.get());

    // Channel height
    spreads.add ("6");
    spreads.add ("10");
    spreads.add ("20");
    spreads.add ("30");
    spreads.add ("40");
    spreads.add ("50");
    spreads.add ("60");
    spreads.add ("70");
    spreads.add ("80");
    spreads.add ("90");
    spreads.add ("100");
    selectedSpread = 5;
    selectedSpreadValue = spreads[selectedSpread - 1];

    spreadSelection = std::make_unique<ComboBox> ("Spread");
    for (int i = 0; i < spreads.size(); i++)
        spreadSelection->addItem (spreads[i], i + 1);
    spreadSelection->setSelectedId (selectedSpread, sendNotification);
    spreadSelection->addListener (this);
    spreadSelection->setEditableText (true);
    addAndMakeVisible (spreadSelection.get());

    // Voltage range
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("25");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("50");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("100");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("250");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("400");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("500");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("750");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("1000");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("2000");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("5000");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("10000");
    voltageRanges[ContinuousChannel::Type::ELECTRODE].add ("15000");
    selectedVoltageRange[ContinuousChannel::Type::ELECTRODE] = 4;
    rangeGain[ContinuousChannel::Type::ELECTRODE] = 1; //uV
    rangeSteps[ContinuousChannel::Type::ELECTRODE] = 20;
    rangeUnits.add (CharPointer_UTF8 ("\xC2\xB5V"));
    typeNames.add ("DATA");

    UtilityButton* tbut;
    tbut = new UtilityButton ("DATA");
    tbut->setEnabledState (true);
    tbut->setCorners (false, false, false, false);
    tbut->addListener (this);
    tbut->setClickingTogglesState (true);
    tbut->setRadioGroupId (100, dontSendNotification);
    tbut->setToggleState (true, dontSendNotification);
    addAndMakeVisible (tbut);
    typeButtons.add (tbut);

    //Ranges for AUX/accelerometer data
    voltageRanges[ContinuousChannel::Type::AUX].add ("25");
    voltageRanges[ContinuousChannel::Type::AUX].add ("50");
    voltageRanges[ContinuousChannel::Type::AUX].add ("100");
    voltageRanges[ContinuousChannel::Type::AUX].add ("250");
    voltageRanges[ContinuousChannel::Type::AUX].add ("400");
    voltageRanges[ContinuousChannel::Type::AUX].add ("500");
    voltageRanges[ContinuousChannel::Type::AUX].add ("750");
    voltageRanges[ContinuousChannel::Type::AUX].add ("1000");
    voltageRanges[ContinuousChannel::Type::AUX].add ("2000");
    selectedVoltageRange[ContinuousChannel::Type::AUX] = 9;
    rangeGain[ContinuousChannel::Type::AUX] = 0.001f; //mV
    rangeSteps[ContinuousChannel::Type::AUX] = 10;
    rangeUnits.add ("mV");
    typeNames.add ("AUX");

    tbut = new UtilityButton ("AUX");
    tbut->setEnabledState (true);
    tbut->setCorners (false, false, false, false);
    tbut->addListener (this);
    tbut->setClickingTogglesState (true);
    tbut->setRadioGroupId (100, dontSendNotification);
    tbut->setToggleState (false, dontSendNotification);
    addAndMakeVisible (tbut);
    typeButtons.add (tbut);

    //Ranges for ADC data
    voltageRanges[ContinuousChannel::Type::ADC].add ("0.01");
    voltageRanges[ContinuousChannel::Type::ADC].add ("0.05");
    voltageRanges[ContinuousChannel::Type::ADC].add ("0.1");
    voltageRanges[ContinuousChannel::Type::ADC].add ("0.5");
    voltageRanges[ContinuousChannel::Type::ADC].add ("1.0");
    voltageRanges[ContinuousChannel::Type::ADC].add ("2.0");
    voltageRanges[ContinuousChannel::Type::ADC].add ("5.0");
    voltageRanges[ContinuousChannel::Type::ADC].add ("10.0");
    selectedVoltageRange[ContinuousChannel::Type::ADC] = 8;
    rangeGain[ContinuousChannel::Type::ADC] = 1; //V
    rangeSteps[ContinuousChannel::Type::ADC] = 0.1; //in V
    rangeUnits.add ("V");
    typeNames.add ("ADC");

    tbut = new UtilityButton ("ADC");
    tbut->setEnabledState (true);
    tbut->setCorners (false, false, false, false);
    tbut->addListener (this);
    tbut->setClickingTogglesState (true);
    tbut->setRadioGroupId (100, dontSendNotification);
    tbut->setToggleState (false, dontSendNotification);
    addAndMakeVisible (tbut);
    typeButtons.add (tbut);

    for (auto* typeButton : typeButtons)
        typeButton->setFont (FontOptions ("Silkscreen", "Plain", 12.0f));

    selectedVoltageRangeValues[ContinuousChannel::Type::ELECTRODE] = voltageRanges[ContinuousChannel::Type::ELECTRODE][selectedVoltageRange[ContinuousChannel::Type::ELECTRODE] - 1];
    selectedVoltageRangeValues[ContinuousChannel::Type::AUX] = voltageRanges[ContinuousChannel::Type::AUX][selectedVoltageRange[ContinuousChannel::Type::AUX] - 1];
    selectedVoltageRangeValues[ContinuousChannel::Type::ADC] = voltageRanges[ContinuousChannel::Type::ADC][selectedVoltageRange[ContinuousChannel::Type::ADC] - 1];

    rangeSelection = std::make_unique<ComboBox> ("Voltage range");
    for (int i = 0; i < voltageRanges[ContinuousChannel::Type::ELECTRODE].size(); i++)
        rangeSelection->addItem (voltageRanges[ContinuousChannel::Type::ELECTRODE][i], i + 1);
    rangeSelection->setSelectedId (selectedVoltageRange[ContinuousChannel::Type::ELECTRODE], sendNotification);
    rangeSelection->setEditableText (true);
    rangeSelection->addListener (this);
    addAndMakeVisible (rangeSelection.get());

    // Event overlay
    for (int i = 0; i < 8; i++)
    {
        EventDisplayInterface* eventOptions = new EventDisplayInterface (lfpDisplay, canvasSplit, i);
        eventDisplayInterfaces.add (eventOptions);
        addAndMakeVisible (eventOptions);
        lfpDisplay->setEventDisplayState (i, true);
    }

    // TTL Word
    ttlWordLabel = std::make_unique<Label> ("TTL word");
    addAndMakeVisible (ttlWordLabel.get());
    startTimer (250);

    // Pause button
    pauseButton = std::make_unique<UtilityButton> ("Pause");
    pauseButton->setRadius (5.0f);
    pauseButton->setEnabledState (true);
    pauseButton->setCorners (true, true, true, true);
    pauseButton->addListener (this);
    pauseButton->setClickingTogglesState (true);
    pauseButton->setToggleState (false, sendNotification);
    addAndMakeVisible (pauseButton.get());

    // Colour scheme
    Array<String> colourSchemeNames = lfpDisplay->getColourSchemeNameArray();
    colourSchemeOptionSelection = std::make_unique<ComboBox> ("colourSchemeOptionSelection");
    for (int i = 0; i < colourSchemeNames.size(); i++)
        colourSchemeOptionSelection->addItem (colourSchemeNames[i], i + 1);
    colourSchemeOptionSelection->setEditableText (false);
    colourSchemeOptionSelection->addListener (this);
    colourSchemeOptionSelection->setSelectedId (1, dontSendNotification);
    addAndMakeVisible (colourSchemeOptionSelection.get());

    // Colour grouping
    colourGroupings.add ("1");
    colourGroupings.add ("2");
    colourGroupings.add ("4");
    colourGroupings.add ("8");
    colourGroupings.add ("16");

    colourGroupingSelection = std::make_unique<ComboBox> ("Colour Grouping");
    for (int i = 0; i < colourGroupings.size(); i++)
        colourGroupingSelection->addItem (colourGroupings[i], i + 1);
    colourGroupingSelection->setSelectedId (1, sendNotification);
    colourGroupingSelection->addListener (this);
    addAndMakeVisible (colourGroupingSelection.get());

    // THRESHOLDS SECTION
    sectionTitles.add ("THRESHOLDS");

    // Spike raster
    spikeRasterSelectionOptions = { "OFF", "-50", "-100", "-150", "-200", "-300", "-400", "-500" };
    selectedSpikeRasterThreshold = 1;
    selectedSpikeRasterThresholdValue = spikeRasterSelectionOptions[selectedSpikeRasterThreshold - 1];

    spikeRasterSelection = std::make_unique<ComboBox> ("spikeRasterSelection");
    for (int i = 0; i < spikeRasterSelectionOptions.size(); i++)
        spikeRasterSelection->addItem (spikeRasterSelectionOptions[i], i + 1);
    spikeRasterSelection->setSelectedId (selectedSpikeRasterThreshold, dontSendNotification);
    spikeRasterSelection->setEditableText (true);
    spikeRasterSelection->addListener (this);
    addAndMakeVisible (spikeRasterSelection.get());

    // Clip warning
    clipThresholds.add ("OFF");
    clipThresholds.add ("ON");

    clipWarningSelection = std::make_unique<ComboBox> ("Clip Warning");
    for (int i = 0; i < clipThresholds.size(); i++)
        clipWarningSelection->addItem (clipThresholds[i], i + 1);
    clipWarningSelection->setSelectedId (1, dontSendNotification);
    clipWarningSelection->addListener (this);
    clipWarningSelection->setEditableText (false);
    addAndMakeVisible (clipWarningSelection.get());

    // Saturation warning
    saturationThresholds.add ("OFF");
    saturationThresholds.add ("0.5");
    saturationThresholds.add ("100");
    saturationThresholds.add ("1000");
    saturationThresholds.add ("5000");
    saturationThresholds.add ("6389");

    saturationWarningSelection = std::make_unique<ComboBox> ("Saturation Warning");
    for (int i = 0; i < saturationThresholds.size(); i++)
        saturationWarningSelection->addItem (saturationThresholds[i], i + 1);
    saturationWarningSelection->setSelectedId (1, dontSendNotification);
    saturationWarningSelection->addListener (this);
    saturationWarningSelection->setEditableText (false);
    addAndMakeVisible (saturationWarningSelection.get());

    // CHANNELS SECTION
    sectionTitles.add ("CHANNELS");

    // Reverse order
    reverseChannelsDisplayButton = std::make_unique<UtilityButton> ("OFF");
    reverseChannelsDisplayButton->setRadius (5.0f);
    reverseChannelsDisplayButton->setEnabledState (true);
    reverseChannelsDisplayButton->setCorners (true, true, true, true);
    reverseChannelsDisplayButton->addListener (this);
    reverseChannelsDisplayButton->setClickingTogglesState (true);
    reverseChannelsDisplayButton->setToggleState (false, sendNotification);
    addAndMakeVisible (reverseChannelsDisplayButton.get());

    // Sort by depth
    sortByDepthButton = std::make_unique<UtilityButton> ("OFF");
    sortByDepthButton->setRadius (5.0f);
    sortByDepthButton->setEnabledState (true);
    sortByDepthButton->setCorners (true, true, true, true);
    sortByDepthButton->addListener (this);
    sortByDepthButton->setClickingTogglesState (true);
    sortByDepthButton->setToggleState (false, sendNotification);
    addAndMakeVisible (sortByDepthButton.get());

    // Channel skip
    channelDisplaySkipOptions.add ("None");
    channelDisplaySkipOptions.add ("2");
    channelDisplaySkipOptions.add ("4");
    channelDisplaySkipOptions.add ("8");
    channelDisplaySkipOptions.add ("16");
    channelDisplaySkipOptions.add ("32");
    channelDisplaySkipOptions.add ("64");
    selectedChannelDisplaySkip = 1;
    selectedChannelDisplaySkipValue = channelDisplaySkipOptions[selectedChannelDisplaySkip - 1];

    channelDisplaySkipSelection = std::make_unique<ComboBox> ("Channel Skip");
    for (int i = 0; i < channelDisplaySkipOptions.size(); i++)
        channelDisplaySkipSelection->addItem (channelDisplaySkipOptions[i], i + 1);
    channelDisplaySkipSelection->setSelectedId (selectedChannelDisplaySkip, sendNotification);
    channelDisplaySkipSelection->setEditableText (false);
    channelDisplaySkipSelection->addListener (this);
    addAndMakeVisible (channelDisplaySkipSelection.get());

    // Show channel number button
    showChannelNumberButton = std::make_unique<UtilityButton> ("OFF");
    showChannelNumberButton->setRadius (5.0f);
    showChannelNumberButton->setEnabledState (true);
    showChannelNumberButton->setCorners (true, true, true, true);
    showChannelNumberButton->addListener (this);
    showChannelNumberButton->setClickingTogglesState (true);
    showChannelNumberButton->setToggleState (false, sendNotification);
    addAndMakeVisible (showChannelNumberButton.get());

    // SIGNAL PROCESSING SECTION
    sectionTitles.add ("SIGNALS");

    // invert signal
    invertInputButton = std::make_unique<UtilityButton> ("OFF");
    invertInputButton->setRadius (5.0f);
    invertInputButton->setEnabledState (true);
    invertInputButton->setCorners (true, true, true, true);
    invertInputButton->addListener (this);
    invertInputButton->setClickingTogglesState (true);
    invertInputButton->setToggleState (false, sendNotification);
    addAndMakeVisible (invertInputButton.get());

    // subtract offset
    medianOffsetPlottingButton = std::make_unique<UtilityButton> ("OFF");
    medianOffsetPlottingButton->setRadius (5.0f);
    medianOffsetPlottingButton->setEnabledState (true);
    medianOffsetPlottingButton->setCorners (true, true, true, true);
    medianOffsetPlottingButton->addListener (this);
    medianOffsetPlottingButton->setClickingTogglesState (true);
    medianOffsetPlottingButton->setToggleState (false, sendNotification);
    addAndMakeVisible (medianOffsetPlottingButton.get());

    // TRIGGERED DISPLAY
    sectionTitles.add ("TRIGGERED DISPLAY");
    // trigger channel selection
    triggerSources.add ("None");
    for (int k = 1; k <= 16; k++)
    {
        triggerSources.add (String (k));
    }

    triggerSourceSelection = std::make_unique<ComboBox> ("Trigger Source");
    for (int i = 0; i < triggerSources.size(); i++)
        triggerSourceSelection->addItem (triggerSources[i], i + 1);
    triggerSourceSelection->setSelectedId (1, sendNotification);
    triggerSourceSelection->addListener (this);
    addAndMakeVisible (triggerSourceSelection.get());

    // average signal
    averageSignalButton = std::make_unique<UtilityButton> ("OFF");
    averageSignalButton->setRadius (5.0f);
    averageSignalButton->setEnabledState (true);
    averageSignalButton->setCorners (true, true, true, true);
    averageSignalButton->addListener (this);
    averageSignalButton->setClickingTogglesState (true);
    averageSignalButton->setToggleState (false, sendNotification);
    addAndMakeVisible (averageSignalButton.get());

    // reset triggered display
    resetButton = std::make_unique<UtilityButton> ("RESET");
    resetButton->setRadius (5.0f);
    resetButton->setEnabledState (true);
    resetButton->setCorners (true, true, true, true);
    resetButton->addListener (this);
    resetButton->setClickingTogglesState (false);
    resetButton->setToggleState (false, sendNotification);
    addChildComponent (resetButton.get());

    // init show/hide options button
    showHideOptionsButton = std::make_unique<ShowHideOptionsButton> (this);
    showHideOptionsButton->addListener (this);
    addAndMakeVisible (showHideOptionsButton.get());

    // do we still need this?
    overlaps.add ("0.5");
    overlaps.add ("0.75");
    overlaps.add ("1");
    overlaps.add ("2");
    overlaps.add ("3");
    overlaps.add ("4");
    overlaps.add ("5");
    selectedOverlap = 4;
    selectedOverlapValue = overlaps[selectedOverlap - 1];

    overlapSelection = std::make_unique<ComboBox> ("Overlap");
    for (int i = 0; i < overlaps.size(); i++)
        overlapSelection->addItem (overlaps[i], i + 1);
    overlapSelection->setSelectedId (selectedOverlap, sendNotification);
    overlapSelection->addListener (this);
    overlapSelection->setEditableText (true);
    addAndMakeVisible (overlapSelection.get());

    //Ranges for neural data
    lfpDisplay->setRange (voltageRanges[ContinuousChannel::Type::ELECTRODE][selectedVoltageRange[ContinuousChannel::Type::ELECTRODE] - 1].getFloatValue()
                              * rangeGain[ContinuousChannel::Type::ELECTRODE],
                          ContinuousChannel::Type::ELECTRODE);
    lfpDisplay->setRange (voltageRanges[ContinuousChannel::Type::ADC][selectedVoltageRange[ContinuousChannel::Type::ADC] - 1].getFloatValue()
                              * rangeGain[ContinuousChannel::Type::AUX],
                          ContinuousChannel::Type::ADC);
    lfpDisplay->setRange (voltageRanges[ContinuousChannel::Type::AUX][selectedVoltageRange[ContinuousChannel::Type::AUX] - 1].getFloatValue()
                              * rangeGain[ContinuousChannel::Type::AUX],
                          ContinuousChannel::Type::AUX);
}

void LfpDisplayOptions::timerCallback()
{
    ttlWordLabel->setText (ttlWordString, dontSendNotification);
}

void LfpDisplayOptions::resized()
{
    int height = 22;

    // MAIN OPTIONS
    timebaseSelection->setBounds (8, getHeight() - 30, 90, height);
    spreadSelection->setBounds (timebaseSelection->getRight() + 20, getHeight() - 30, 90, height);
    rangeSelection->setBounds (spreadSelection->getRight() + 25, getHeight() - 30, 90, height);

    int bh = 39 / typeButtons.size();
    for (int i = 0; i < typeButtons.size(); i++)
    {
        typeButtons[i]->setBounds (rangeSelection->getRight() + 5, getHeight() - 42 + i * bh, 50, bh);
    }

    for (int i = 0; i < 8; i++)
    {
        eventDisplayInterfaces[i]->setBounds (typeButtons[0]->getRight() + 80 + (floor (i / 2) * 20),
                                              getHeight() - 45 + (i % 2) * 20,
                                              20,
                                              20); // arrange event channel buttons in two rows
        eventDisplayInterfaces[i]->repaint();
    }

    ttlWordLabel->setBounds (565, getHeight() - 30, 90, height);

    pauseButton->setBounds (680, getHeight() - 40, 70, 30);

    colourSchemeOptionSelection->setBounds (pauseButton->getRight() + 30,
                                            getHeight() - 30,
                                            180,
                                            height);

    colourGroupingSelection->setBounds (colourSchemeOptionSelection->getRight() + 15, getHeight() - 30, 55, height);

    int startHeight = 167;
    int verticalSpacing = 29;
    int xOffset = 200;

    // THRESHOLDS
    spikeRasterSelection->setBounds (96,
                                     getHeight() - startHeight,
                                     80,
                                     height);

    clipWarningSelection->setBounds (96,
                                     getHeight() - startHeight + verticalSpacing,
                                     80,
                                     height);

    saturationWarningSelection->setBounds (96,
                                           getHeight() - startHeight + verticalSpacing * 2,
                                           80,
                                           height);

    // CHANNELS

    reverseChannelsDisplayButton->setBounds (getWidth() / 4 + 107,
                                             getHeight() - startHeight,
                                             35,
                                             height);

    sortByDepthButton->setBounds (getWidth() / 4 + 102,
                                  getHeight() - startHeight + verticalSpacing,
                                  35,
                                  height);

    channelDisplaySkipSelection->setBounds (getWidth() / 4 + 47,
                                            getHeight() - startHeight + verticalSpacing * 2,
                                            80,
                                            height);

    showChannelNumberButton->setBounds (getWidth() / 4 + 102,
                                        getHeight() - startHeight + +verticalSpacing * 3,
                                        35,
                                        height);

    // SIGNAL PROCESSING
    invertInputButton->setBounds (getWidth() / 2 + 95,
                                  getHeight() - startHeight,
                                  35,
                                  height);

    medianOffsetPlottingButton->setBounds (getWidth() / 2 + 110,
                                           getHeight() - startHeight + verticalSpacing,
                                           35,
                                           height);

    //TRIGGERED DISPLAY
    triggerSourceSelection->setBounds (getWidth() / 4 * 3 + 118,
                                       getHeight() - startHeight,
                                       80,
                                       height);

    averageSignalButton->setBounds (getWidth() / 4 * 3 + 112,
                                    getHeight() - startHeight + verticalSpacing,
                                    35,
                                    height);

    resetButton->setBounds (getWidth() / 4 * 3 + 156,
                            getHeight() - startHeight + verticalSpacing,
                            50,
                            height);

    showHideOptionsButton->setBounds (getWidth() - 28, getHeight() - 28, 20, 20);
}

void LfpDisplayOptions::paint (Graphics& g)
{
    int row1 = 55;
    int row2 = 110;

    ttlWordLabel->setColour (Label::outlineColourId, findColour (ThemeColours::outline));

    g.fillAll (findColour (ThemeColours::componentBackground));
    g.setFont (FontOptions ("Inter", "Medium", 18.0f));

    g.setColour (findColour (ThemeColours::defaultText));

    if (getHeight() > 150)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i > 0)
                g.drawLine (getWidth() / 4 * i, getHeight() - 200, getWidth() / 4 * i, 150);

            g.drawText (sectionTitles[i],
                        getWidth() / 4 * i + 10,
                        7,
                        300,
                        20,
                        Justification::left,
                        false);
        }
    }

    g.setFont (FontOptions ("Inter", "Regular", 16.0f));

    g.drawText ("Timebase (s)", timebaseSelection->getX(), timebaseSelection->getY() - 22, 300, 20, Justification::left, false);
    g.drawText ("Chan height (px)", spreadSelection->getX(), spreadSelection->getY() - 22, 300, 20, Justification::left, false);
    g.drawText ("Range (" + rangeUnits[selectedChannelType] + ")", rangeSelection->getX(), rangeSelection->getY() - 22, 300, 20, Justification::left, false);

    g.drawText ("Overlay", 350, getHeight() - 43, 100, 20, Justification::right, false);
    g.drawText ("Events:", 350, getHeight() - 25, 100, 20, Justification::right, false);

    g.drawText ("TTL word:", ttlWordLabel->getX(), ttlWordLabel->getY() - 22, 70, 20, Justification::left, false);

    g.drawText ("Colour scheme", colourSchemeOptionSelection->getX(), colourSchemeOptionSelection->getY() - 22, 300, 20, Justification::left, false);
    g.drawText ("Colour grouping", colourGroupingSelection->getX(), colourGroupingSelection->getY() - 22, 300, 20, Justification::left, false);

    g.drawText ("Spike raster:",
                10,
                spikeRasterSelection->getY(),
                100,
                22,
                Justification::left,
                false);

    g.drawText ("Clip warning:",
                10,
                clipWarningSelection->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Sat. warning:",
                10,
                saturationWarningSelection->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Reverse order:",
                getWidth() / 4 + 10,
                reverseChannelsDisplayButton->getY(),
                100,
                22,
                Justification::left,
                false);

    g.drawText ("Sort by depth:",
                getWidth() / 4 + 10,
                sortByDepthButton->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Skip:",
                getWidth() / 4 + 10,
                channelDisplaySkipSelection->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Show number:",
                getWidth() / 4 + 10,
                showChannelNumberButton->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Invert signal:",
                getWidth() / 2 + 10,
                invertInputButton->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Subtract offset:",
                getWidth() / 2 + 10,
                medianOffsetPlottingButton->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Trigger channel:",
                getWidth() / 4 * 3 + 10,
                triggerSourceSelection->getY(),
                150,
                22,
                Justification::left,
                false);

    g.drawText ("Trial averaging:",
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

    g.drawText("Colour grouping",365,getHeight()-row2,300,20,Justification::left, false);

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
    return (int) spreadSelection->getText().getIntValue();
}

bool LfpDisplayOptions::getInputInvertedState()
{
    return invertInputButton->getToggleState();
}

bool LfpDisplayOptions::getChannelNameState()
{
    return showChannelNumberButton->getToggleState();
}

void LfpDisplayOptions::setPausedState (bool isPaused)
{
    pauseButton->setToggleState (isPaused, dontSendNotification);

    timebaseSelection->setEnabled (! isPaused);
}

void LfpDisplayOptions::setRangeSelection (float range, bool canvasMustUpdate)
{
    if (canvasMustUpdate)
    {
        rangeSelection->setText (String (range / rangeGain[selectedChannelType]), sendNotification);
    }
    else
    {
        rangeSelection->setText (String (range / rangeGain[selectedChannelType]), dontSendNotification);

        selectedVoltageRange[selectedChannelType] = rangeSelection->getSelectedId();
        selectedVoltageRangeValues[selectedChannelType] = rangeSelection->getText();

        canvasSplit->repaint();
        canvasSplit->refresh();
    }
}

void LfpDisplayOptions::setSpreadSelection (int spread, bool canvasMustUpdate, bool deferDisplayRefresh)
{
    if (canvasMustUpdate)
    {
        spreadSelection->setText (String (spread), sendNotification);
    }
    else
    {
        spreadSelection->setText (String (spread), dontSendNotification);
        selectedSpread = spreadSelection->getSelectedId();
        selectedSpreadValue = spreadSelection->getText();

        if (! deferDisplayRefresh)
        {
            canvasSplit->repaint();
            canvasSplit->refresh();
        }
    }
}

void LfpDisplayOptions::togglePauseButton (bool sendUpdate)
{
    pauseButton->setToggleState (! pauseButton->getToggleState(), sendUpdate ? sendNotification : dontSendNotification);
}

void LfpDisplayOptions::setChannelsReversed (bool state)
{
    if (lfpDisplay->getChannelsReversed() == state) // ignore if we're not changing state
        return;

    lfpDisplay->setChannelsReversed (state);
    canvasSplit->fullredraw = true;

    reverseChannelsDisplayButton->setToggleState (state, dontSendNotification);

    if (state)
    {
        reverseChannelsDisplayButton->setLabel ("ON");
    }
    else
    {
        reverseChannelsDisplayButton->setLabel ("OFF");
    }
}

void LfpDisplayOptions::setInputInverted (bool state)
{
    lfpDisplay->setInputInverted (state);

    invertInputButton->setToggleState (state, dontSendNotification);

    if (state)
    {
        invertInputButton->setLabel ("ON");
    }
    else
    {
        invertInputButton->setLabel ("OFF");
    }
}

void LfpDisplayOptions::setMedianOffset (bool state)
{
    if (lfpDisplay->getSpikeRasterPlotting())
    {
        medianOffsetPlottingButton->setToggleState (true, dontSendNotification);
        medianOffsetPlottingButton->setLabel ("ON");
        return;
    }
    else
    {
        lfpDisplay->setMedianOffsetPlotting (state);
        medianOffsetPlottingButton->setToggleState (state, dontSendNotification);
    }

    if (state)
    {
        medianOffsetPlottingButton->setLabel ("ON");
    }
    else
    {
        medianOffsetPlottingButton->setLabel ("OFF");
    }
}

void LfpDisplayOptions::setAveraging (bool state)
{
    canvasSplit->setAveraging (state);

    averageSignalButton->setToggleState (state, dontSendNotification);

    if (state)
    {
        averageSignalButton->setLabel ("ON");
        resetButton->setVisible (true);
    }
    else
    {
        averageSignalButton->setLabel ("OFF");
        resetButton->setVisible (false);
    }
}

void LfpDisplayOptions::setSortByDepth (bool state)
{
    if (lfpDisplay->shouldOrderChannelsByDepth() == state)
        return;

    if (canvasSplit->displayBuffer != nullptr)
        lfpDisplay->orderChannelsByDepth (state);

    sortByDepthButton->setToggleState (state, dontSendNotification);

    if (state)
    {
        sortByDepthButton->setLabel ("ON");
    }
    else
    {
        sortByDepthButton->setLabel ("OFF");
    }
}

void LfpDisplayOptions::setShowChannelNumbers (bool state)
{
    showChannelNumberButton->setToggleState (state, dontSendNotification);

    int numChannels = lfpDisplay->channelInfo.size();

    for (int i = 0; i < numChannels; ++i)
    {
        lfpDisplay->channelInfo[i]->repaint();
    }

    if (state)
    {
        showChannelNumberButton->setLabel ("ON");
    }
    else
    {
        showChannelNumberButton->setLabel ("OFF");
    }
}

void LfpDisplayOptions::setTTLWord (String word)
{
    ttlWordString = word;
}

void LfpDisplayOptions::buttonClicked (Button* b)
{
    if (b == invertInputButton.get())
    {
        setInputInverted (b->getToggleState());
        return;
    }
    if (b == reverseChannelsDisplayButton.get())
    {
        setChannelsReversed (b->getToggleState());
        return;
    }
    if (b == medianOffsetPlottingButton.get())
    {
        setMedianOffset (b->getToggleState());
        return;
    }

    if (b == averageSignalButton.get())
    {
        setAveraging (b->getToggleState());
        return;
    }

    if (b == sortByDepthButton.get())
    {
        setSortByDepth (b->getToggleState());
        return;
    }

    if (b == resetButton.get())
    {
        canvasSplit->resetTrials();
    }

    if (b == pauseButton.get())
    {
        lfpDisplay->pause (b->getToggleState());
        timescale->setPausedState (b->getToggleState());
        return;
    }

    if (b == showHideOptionsButton.get())
    {
        canvas->toggleOptionsDrawer (b->getToggleState());
    }

    if (b == showChannelNumberButton.get())
    {
        setShowChannelNumbers (b->getToggleState());
        return;
    }

    int idx = typeButtons.indexOf ((UtilityButton*) b);

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

        setSelectedType ((ContinuousChannel::Type) idx, false);
    }
}

void LfpDisplayOptions::setTimebaseAndSelectionText (float timebase)
{
    canvasSplit->setTimebase (timebase);

    if (canvasSplit->timebase) // if timebase != 0
    {
        if (canvasSplit->timebase < timebases[0].getFloatValue())
        {
            timebaseSelection->setSelectedId (1, dontSendNotification);
            canvasSplit->setTimebase (timebases[0].getFloatValue());
        }
        else if (canvasSplit->timebase > timebases[timebases.size() - 1].getFloatValue())
        {
            timebaseSelection->setSelectedId (timebases.size(), dontSendNotification);
            canvasSplit->setTimebase (timebases[timebases.size() - 1].getFloatValue());
        }
        else
        {
            timebaseSelection->setText (String (canvasSplit->timebase, 1), dontSendNotification);
        }
    }
    else
    {
        if (selectedSpread == 0)
        {
            timebaseSelection->setText (selectedTimebaseValue, dontSendNotification);
            canvasSplit->setTimebase (selectedTimebaseValue.getFloatValue());
        }
        else
        {
            timebaseSelection->setSelectedId (selectedTimebase, dontSendNotification);
            canvasSplit->setTimebase (timebases[selectedTimebase - 1].getFloatValue());
        }
    }

    timescale->setTimebase (canvasSplit->timebase);
}

void LfpDisplayOptions::comboBoxChanged (ComboBox* cb)
{
    if (canvasSplit->getNumChannels() == 0)
        return;

    if (cb == channelDisplaySkipSelection.get())
    {
        const int skipAmt = pow (2, cb->getSelectedId() - 1);
        lfpDisplay->setChannelDisplaySkipAmount (skipAmt);
    }
    else if (cb == spikeRasterSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            if (val == 0) // if value is zero, just disable plotting and set text to "Off"
            {
                cb->setSelectedItemIndex (0, dontSendNotification);
                lfpDisplay->setSpikeRasterPlotting (false);

                if (medianOffsetOnForSpikeRaster)
                {
                    medianOffsetPlottingButton->setToggleState (false, sendNotification);
                    medianOffsetOnForSpikeRaster = false;
                }
                return;
            }

            if (val > 500)
            {
                val = 500;
            }

            val *= -1;

            spikeRasterSelection->setText (String (val), dontSendNotification);
            lfpDisplay->setSpikeRasterThreshold (val);

            if (! medianOffsetPlottingButton->getToggleState())
            {
                medianOffsetPlottingButton->setToggleState (true, sendNotification);
                medianOffsetOnForSpikeRaster = true;
            }
            else
            {
                medianOffsetOnForSpikeRaster = false;
            }

            lfpDisplay->setSpikeRasterPlotting (true);
        }
        else if (cb->getSelectedItemIndex() == 0) // if "Off"
        {
            lfpDisplay->setSpikeRasterPlotting (false);

            if (medianOffsetOnForSpikeRaster)
            {
                medianOffsetPlottingButton->setToggleState (false, sendNotification);
                medianOffsetOnForSpikeRaster = false;
            }
            return;
        }
        else
        {
            auto val = cb->getText().getFloatValue();

            lfpDisplay->setSpikeRasterThreshold (val);

            if (! medianOffsetPlottingButton->getToggleState())
            {
                medianOffsetPlottingButton->setToggleState (true, sendNotification);
                medianOffsetOnForSpikeRaster = true;
            }

            lfpDisplay->setSpikeRasterPlotting (true);
        }
    }
    else if (cb == colourSchemeOptionSelection.get())
    {
        lfpDisplay->setActiveColourSchemeIdx (cb->getSelectedId() - 1);

        lfpDisplay->setColours();
        canvasSplit->redraw();
    }
    else if (cb == timebaseSelection.get())
    {
        if (cb->getSelectedId())
        {
            canvasSplit->setTimebase (timebases[cb->getSelectedId() - 1].getFloatValue());
        }
        else
        {
            setTimebaseAndSelectionText (cb->getText().getFloatValue());
        }

        timescale->setTimebase (canvasSplit->timebase);
    }
    else if (cb == rangeSelection.get())
    {
        if (cb->getSelectedId())
        {
            lfpDisplay->setRange (voltageRanges[selectedChannelType][cb->getSelectedId() - 1].getFloatValue() * rangeGain[selectedChannelType], selectedChannelType);
        }
        else
        {
            float vRange = cb->getText().getFloatValue();
            if (vRange)
            {
                if (vRange < voltageRanges[selectedChannelType][0].getFloatValue())
                {
                    cb->setSelectedId (1, dontSendNotification);
                    vRange = voltageRanges[selectedChannelType][0].getFloatValue();
                }
                else if (vRange > voltageRanges[selectedChannelType][voltageRanges[selectedChannelType].size() - 1].getFloatValue())
                {
                    // cb->setSelectedId(voltageRanges[selectedChannelType].size(),dontSendNotification);
                    // vRange = voltageRanges[selectedChannelType][voltageRanges[selectedChannelType].size()-1].getFloatValue();
                }
                else
                {
                    if (rangeGain[selectedChannelType] > 1)
                        cb->setText (String (vRange, 1), dontSendNotification);
                    else
                        cb->setText (String (vRange), dontSendNotification);
                }
                lfpDisplay->setRange (vRange * rangeGain[selectedChannelType], selectedChannelType);
            }
            else
            {
                if (selectedVoltageRange[selectedChannelType])
                    cb->setText (selectedVoltageRangeValues[selectedChannelType], dontSendNotification);
                else
                    cb->setSelectedId (selectedVoltageRange[selectedChannelType], dontSendNotification);
            }
        }
        selectedVoltageRange[selectedChannelType] = cb->getSelectedId();
        selectedVoltageRangeValues[selectedChannelType] = cb->getText();
        canvasSplit->redraw();
    }
    else if (cb == spreadSelection.get())
    {
        if (cb->getSelectedId())
        {
            if (lfpDisplay->getSingleChannelState())
            {
                lfpDisplay->cacheNewChannelHeight (spreads[cb->getSelectedId() - 1].getIntValue());
            }
            else
            {
                lfpDisplay->setChannelHeight (spreads[cb->getSelectedId() - 1].getIntValue());
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
                    cb->setSelectedId (1, dontSendNotification);
                    spread = spreads[0].getFloatValue();
                }
                else if (spread > spreads[spreads.size() - 1].getFloatValue())
                {
                    cb->setSelectedId (spreads.size(), dontSendNotification);
                    spread = spreads[spreads.size() - 1].getFloatValue();
                }
                else
                {
                    cb->setText (String (spread), dontSendNotification);
                }

                // if single channel focus is on, cache the value
                if (lfpDisplay->getSingleChannelState())
                {
                    lfpDisplay->cacheNewChannelHeight (spread);
                }
                else
                {
                    lfpDisplay->setChannelHeight (spread);
                    canvasSplit->resized();
                }
            }
            else
            {
                if (selectedSpread == 0)
                    cb->setText (selectedSpreadValue, dontSendNotification);
                else
                    cb->setSelectedId (selectedSpread, dontSendNotification);
            }
        }
        selectedSpread = cb->getSelectedId();
        selectedSpreadValue = cb->getText();

        if (! lfpDisplay->getSingleChannelState())
            canvasSplit->redraw();
    }
    else if (cb == saturationWarningSelection.get())
    {
        if (cb->getSelectedId() > 1)
        {
            selectedSaturationValueFloat = (saturationThresholds[cb->getSelectedId() - 1].getFloatValue());
            canvasSplit->drawSaturationWarning = true;
        }
        else
        {
            canvasSplit->drawSaturationWarning = false;
        }

        canvasSplit->redraw();
    }
    else if (cb == clipWarningSelection.get())
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
    }
    else if (cb == overlapSelection.get())
    {
        if (cb->getSelectedId())
        {
            canvasSplit->channelOverlapFactor = (overlaps[cb->getSelectedId() - 1].getFloatValue());
            canvasSplit->resized();
        }
        else
        {
            float overlap = cb->getText().getFloatValue();
            if (overlap)
            {
                if (overlap < overlaps[0].getFloatValue())
                {
                    cb->setSelectedId (1, dontSendNotification);
                    overlap = overlaps[0].getFloatValue();
                }
                else if (overlap > overlaps[overlaps.size() - 1].getFloatValue())
                {
                    cb->setSelectedId (overlaps.size(), dontSendNotification);
                    overlap = overlaps[overlaps.size() - 1].getFloatValue();
                }
                else
                {
                    cb->setText (String (overlap), dontSendNotification);
                }
                canvasSplit->channelOverlapFactor = overlap;
                canvasSplit->resized();
            }
            else
            {
                if (selectedSpread == 0)
                    cb->setText (selectedSpreadValue, dontSendNotification);
                else
                    cb->setSelectedId (selectedSpread, dontSendNotification);
            }
        }
        selectedSpread = cb->getSelectedId();
        selectedSpreadValue = cb->getText();
        lfpDisplay->setChannelHeight (lfpDisplay->getChannelHeight());
        canvasSplit->redraw();
    }
    else if (cb == colourGroupingSelection.get())
    {
        // set colour grouping here
        lfpDisplay->setColourGrouping (colourGroupings[cb->getSelectedId() - 1].getIntValue()); // so that channel colours get re-assigned
        canvasSplit->redraw();
    }
    else if (cb == triggerSourceSelection.get())
    {
        canvasSplit->setTriggerChannel (cb->getSelectedId() - 2);
        processor->setParameter (cb->getSelectedId() - 2, float (canvasSplit->splitID));
    }
}

ContinuousChannel::Type LfpDisplayOptions::getChannelType (int n)
{
    if (n < processor->getNumInputs())
    {
        const ContinuousChannel* chan = processor->getContinuousChannel (n);
        return processor->getContinuousChannel (n)->getChannelType();
    }
    else
    {
        return ContinuousChannel::Type::ELECTRODE;
    }
}

ContinuousChannel::Type LfpDisplayOptions::getSelectedType()
{
    return selectedChannelType;
}

void LfpDisplayOptions::setSelectedType (ContinuousChannel::Type type, bool toggleButton)
{
    if (selectedChannelType == type)
        return; //Nothing to do here
    selectedChannelType = type;
    rangeSelection->clear (dontSendNotification);

    for (int i = 0; i < voltageRanges[type].size(); i++)
        rangeSelection->addItem (voltageRanges[type][i], i + 1);

    int id = selectedVoltageRange[type];
    if (id)
        rangeSelection->setSelectedId (id, sendNotification);
    else
        rangeSelection->setText (selectedVoltageRangeValues[selectedChannelType], dontSendNotification);

    repaint (5, getHeight() - 55, 300, 100);

    if (toggleButton)
        typeButtons[type]->setToggleState (true, dontSendNotification);
}

String LfpDisplayOptions::getTypeName (ContinuousChannel::Type type)
{
    return typeNames[type];
}

int LfpDisplayOptions::getRangeStep (ContinuousChannel::Type type)
{
    return rangeSteps[type];
}

void LfpDisplayOptions::saveParameters (XmlElement* xml)
{
    XmlElement* xmlNode = xml->createNewChildElement ("LFPDISPLAY" + String (canvasSplit->splitID));

    //xmlNode->setAttribute("SubprocessorID", canvasSplit->streamSelection->getSelectedId());

    xmlNode->setAttribute ("stream_key", canvasSplit->getStreamKey());

    xmlNode->setAttribute ("Range", selectedVoltageRangeValues[0] + "," + selectedVoltageRangeValues[1] + "," + selectedVoltageRangeValues[2]);
    xmlNode->setAttribute ("Timebase", timebaseSelection->getText());
    xmlNode->setAttribute ("Spread", spreadSelection->getText());
    xmlNode->setAttribute ("colourScheme", colourSchemeOptionSelection->getSelectedId());
    xmlNode->setAttribute ("colourGrouping", colourGroupingSelection->getSelectedId());

    xmlNode->setAttribute ("spikeRaster", spikeRasterSelection->getText());
    xmlNode->setAttribute ("clipWarning", clipWarningSelection->getSelectedId());
    xmlNode->setAttribute ("satWarning", saturationWarningSelection->getSelectedId());

    xmlNode->setAttribute ("reverseOrder", reverseChannelsDisplayButton->getToggleState());
    xmlNode->setAttribute ("sortByDepth", sortByDepthButton->getToggleState());
    xmlNode->setAttribute ("channelSkip", channelDisplaySkipSelection->getSelectedId());
    xmlNode->setAttribute ("showChannelNum", showChannelNumberButton->getToggleState());
    xmlNode->setAttribute ("subtractOffset", medianOffsetPlottingButton->getToggleState());

    xmlNode->setAttribute ("isInverted", invertInputButton->getToggleState());

    xmlNode->setAttribute ("triggerSource", triggerSourceSelection->getSelectedId());
    xmlNode->setAttribute ("trialAvg", averageSignalButton->getToggleState());

    xmlNode->setAttribute ("singleChannelView", lfpDisplay->getSingleChannelShown());

    int eventButtonState = 0;

    for (int i = 0; i < 8; i++)
    {
        if (lfpDisplay->eventDisplayEnabled[i])
        {
            eventButtonState += (1 << i);
        }
    }

    xmlNode->setAttribute ("EventButtonState", eventButtonState);

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

    xmlNode->setAttribute ("ChannelDisplayState", channelDisplayState);

    xmlNode->setAttribute ("ScrollX", canvasSplit->viewport->getViewPositionX());
    xmlNode->setAttribute ("ScrollY", canvasSplit->viewport->getViewPositionY());
}

void LfpDisplayOptions::loadParameters (XmlElement* xml)
{
    canvasSplit->isLoading = true;

    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName ("LFPDISPLAY" + String (canvasSplit->splitID)))
        {
            //uint32 id = xmlNode->getIntAttribute("SubprocessorID");

            String streamKey = xmlNode->getStringAttribute ("stream_key");

            int64 start = Time::getHighResolutionTicks();

            if (canvasSplit->displayBuffer != nullptr)
                canvasSplit->displayBuffer->removeDisplay (canvasSplit->splitID);

            int streamId = 0;

            if (auto stream = processor->getDataStream (streamKey))
                streamId = stream->getStreamId();

            if (processor->displayBufferMap.find (streamId) == processor->displayBufferMap.end())
                canvasSplit->displayBuffer = processor->getDisplayBuffers().getFirst();
            else
                canvasSplit->displayBuffer = processor->displayBufferMap[streamId];

            if (canvasSplit->displayBuffer != nullptr)
                canvasSplit->displayBuffer->addDisplay (canvasSplit->splitID);

            //LOGD("    Added displays in ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            // RANGE
            StringArray ranges;
            String rangeString = xmlNode->getStringAttribute ("Range");
            ranges.addTokens (rangeString, ",", "\"");

            selectedVoltageRangeValues[0] = ranges[0];
            selectedVoltageRangeValues[1] = ranges[1];
            selectedVoltageRangeValues[2] = ranges[2];
            selectedVoltageRange[0] = voltageRanges[0].indexOf (ranges[0]) + 1;
            selectedVoltageRange[1] = voltageRanges[1].indexOf (ranges[1]) + 1;
            selectedVoltageRange[2] = voltageRanges[2].indexOf (ranges[2]) + 1;
            rangeSelection->setText (ranges[0]);
            lfpDisplay->setRange (ranges[0].getFloatValue() * rangeGain[0], ContinuousChannel::Type::ELECTRODE);
            lfpDisplay->setRange (ranges[1].getFloatValue() * rangeGain[1], ContinuousChannel::Type::AUX);
            lfpDisplay->setRange (ranges[2].getFloatValue() * rangeGain[2], ContinuousChannel::Type::ADC);

            // LOGD("    Set range in ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            // TIMEBASE
            timebaseSelection->setText (xmlNode->getStringAttribute ("Timebase"), dontSendNotification);
            canvasSplit->setTimebase (xmlNode->getStringAttribute ("Timebase").getFloatValue());

            //LOGD("    Set timebase in ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            // SPREAD
            spreadSelection->setText (xmlNode->getStringAttribute ("Spread"), dontSendNotification);

            // COLOUR SCHEME
            lfpDisplay->setActiveColourSchemeIdx (xmlNode->getIntAttribute ("colourScheme") - 1);
            colourSchemeOptionSelection->setSelectedId (xmlNode->getIntAttribute ("colourScheme"), dontSendNotification);

            // COLOUR GROUPING
            colourGroupingSelection->setSelectedId (xmlNode->getIntAttribute ("colourGrouping", 1), dontSendNotification);
            lfpDisplay->setColourGrouping (colourGroupings[colourGroupingSelection->getSelectedId() - 1].getIntValue());

            // SPIKE RASTER
            String spikeRasterThresh = xmlNode->getStringAttribute ("spikeRaster", "OFF");
            spikeRasterSelection->setText (spikeRasterThresh, dontSendNotification);
            if (! spikeRasterThresh.equalsIgnoreCase ("OFF"))
            {
                lfpDisplay->setSpikeRasterPlotting (true);
                lfpDisplay->setSpikeRasterThreshold (spikeRasterThresh.getFloatValue());
            }

            // CLIP WARNING
            int clipWarning = xmlNode->getIntAttribute ("clipWarning", 1);
            clipWarningSelection->setSelectedId (clipWarning, dontSendNotification);
            if (clipWarning == 2)
                canvasSplit->drawClipWarning = true;

            // SATURATION WARNING
            saturationWarningSelection->setSelectedId (xmlNode->getIntAttribute ("satWarning"), dontSendNotification);

            if (saturationWarningSelection->getSelectedId() > 1)
            {
                selectedSaturationValueFloat = (saturationThresholds[saturationWarningSelection->getSelectedId() - 1].getFloatValue());
                canvasSplit->drawSaturationWarning = true;
            }

            //LOGD("    Additional settings in ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            // TOGGLE BUTTONS

            setChannelsReversed (xmlNode->getBoolAttribute ("reverseOrder", false));

            //LOGD("    --> setChannelsReversed: ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            setSortByDepth (xmlNode->getBoolAttribute ("sortByDepth", false));

            //LOGD("    --> setSortByDepth: ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            setShowChannelNumbers (xmlNode->getBoolAttribute ("showChannelNum", false));

            //LOGD("    --> setShowChannelNumbers: ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            bool shouldInvert = xmlNode->getBoolAttribute ("isInverted", false);

            if (invertInputButton->getToggleState() != shouldInvert)
                setInputInverted (shouldInvert);

            //LOGD("    --> setInputInverted: ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            setAveraging (xmlNode->getBoolAttribute ("trialAvg", false));
            //LOGD("    --> setAveraging: ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            setMedianOffset (xmlNode->getBoolAttribute ("subtractOffset", false));

            //LOGD("    --> setMedianOffset: ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            // CHANNEL SKIP
            channelDisplaySkipSelection->setSelectedId (xmlNode->getIntAttribute ("channelSkip"), dontSendNotification);
            const int skipAmt = pow (2, channelDisplaySkipSelection->getSelectedId() - 1);
            lfpDisplay->setChannelDisplaySkipAmount (skipAmt);

            // TRIGGER SOURCE
            triggerSourceSelection->setSelectedId (xmlNode->getIntAttribute ("triggerSource"), dontSendNotification);
            canvasSplit->setTriggerChannel (triggerSourceSelection->getSelectedId() - 2);
            processor->setParameter (triggerSourceSelection->getSelectedId() - 2, float (canvasSplit->splitID));

            //LOGD("    Set trigger source in ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            // drawMethodButton->setToggleState(xmlNode->getBoolAttribute("drawMethod", true), sendNotification);

            lfpDisplay->setScrollPosition (xmlNode->getIntAttribute ("ScrollX"),
                                           xmlNode->getIntAttribute ("ScrollY"));

            //LOGD("    Set scroll position in ", MS_FROM_START, " milliseconds");
            start = Time::getHighResolutionTicks();

            int eventButtonState = xmlNode->getIntAttribute ("EventButtonState");

            for (int i = 0; i < 8; i++)
            {
                lfpDisplay->eventDisplayEnabled[i] = (eventButtonState >> i) & 1;

                eventDisplayInterfaces[i]->checkEnabledState();
            }

            String channelDisplayState = xmlNode->getStringAttribute ("ChannelDisplayState");

            for (int i = 0; i < channelDisplayState.length(); i++)
            {
                if (channelDisplayState.substring (i, i + 1).equalsIgnoreCase ("1"))
                {
                    lfpDisplay->setEnabledState (true, i, true);
                }
                else
                {
                    lfpDisplay->setEnabledState (false, i, true);
                }
            }

            //LOGD("    Set channel display state in ", MS_FROM_START, " milliseconds");

            if (canvas->optionsDrawerIsOpen)
                showHideOptionsButton->setToggleState (true, dontSendNotification);

            start = Time::getHighResolutionTicks();

            lfpDisplay->setSingleChannelView (xmlNode->getIntAttribute ("singleChannelView", -1));

            lfpDisplay->setColours();
            canvasSplit->redraw();

            lfpDisplay->restoreViewPosition();

            //LOGD("    Restored view in ", MS_FROM_START, " milliseconds");
        }
    }

    // std::cout << "Finished loading LFP options." << std::endl;
}
