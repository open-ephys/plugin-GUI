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

#include "PSTHEditor.h"
#include "PSTHProcessor.h"
PSTHEditor::PSTHEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
	: VisualizerEditor(parentNode, useDefaultParameterEditors), psthCanvas(nullptr)
{
	showSortedUnits = true;
	showLFP = true;
	showCompactView = false;
	showSmooth = true;
	showRasters = false;
	showAutoRescale = true;
	showMatchRange = false;
	TTLchannelTrialAlignment = -1;
	smoothingMS = 10;


	tabText = "PSTH";
	desiredWidth = 300;


	saveOptions = new UtilityButton("Save Options", Font("Default", 15, Font::plain));
	saveOptions->addListener(this);
	saveOptions->setColour(Label::textColourId, Colours::white);
	saveOptions->setBounds(180, 30, 100, 20);
	addAndMakeVisible(saveOptions);


	clearDisplay = new UtilityButton("Clear all", Font("Default", 15, Font::plain));
	clearDisplay->addListener(this);
	clearDisplay->setColour(Label::textColourId, Colours::white);
	clearDisplay->setBounds(180, 60, 90, 20);
	addAndMakeVisible(clearDisplay);


	visualizationOptions = new UtilityButton("Visualization Options", Font("Default", 15, Font::plain));
	visualizationOptions->addListener(this);
	visualizationOptions->setColour(Label::textColourId, Colours::white);
	visualizationOptions->setBounds(10, 30, 160, 20);
	addAndMakeVisible(visualizationOptions);

	hardwareTrigger = new Label("hardwareTrigger", "TTL Trial Alignment:");
	hardwareTrigger->setFont(Font("Default", 14, Font::plain));
	hardwareTrigger->setEditable(false);
	hardwareTrigger->setJustificationType(Justification::centredLeft);
	hardwareTrigger->setBounds(10, 90, 120, 20);
	addAndMakeVisible(hardwareTrigger);

	hardwareTrialAlignment = new ComboBox("Hardware Trial Alignment");
	hardwareTrialAlignment->setEditableText(false);
	hardwareTrialAlignment->setJustificationType(Justification::centredLeft);
	hardwareTrialAlignment->addListener(this);
	hardwareTrialAlignment->setBounds(130, 90, 70, 20);
	hardwareTrialAlignment->addItem("Not set", 1);
	for (int k = 0; k<8; k++)
	{
		hardwareTrialAlignment->addItem("TTL " + String(k + 1), k + 2);
	}
	if (TTLchannelTrialAlignment == -1)
		hardwareTrialAlignment->setSelectedId(1, sendNotification);
	else
		hardwareTrialAlignment->setSelectedId(TTLchannelTrialAlignment + 2, sendNotification);

	addAndMakeVisible(hardwareTrialAlignment);


}



void PSTHEditor::saveVisualizerParameters(XmlElement* xml)
{

	XmlElement* xmlNode = xml->createNewChildElement("PSTH_EDITOR");
	xmlNode->setAttribute("showSortedUnits", showSortedUnits);

	xmlNode->setAttribute("showLFP", showLFP);
	xmlNode->setAttribute("showCompactView", showCompactView);
	xmlNode->setAttribute("showSmooth", showSmooth);
	xmlNode->setAttribute("showAutoRescale", showAutoRescale);
	xmlNode->setAttribute("showRasters", showRasters);
	xmlNode->setAttribute("showMatchRange", showMatchRange);
	xmlNode->setAttribute("TTLchannelTrialAlignment", TTLchannelTrialAlignment);
	xmlNode->setAttribute("smoothingMS", smoothingMS);
}

void PSTHEditor::loadVisualizerParameters(XmlElement* xml)
{
	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("PSTH_EDITOR"))
		{

			showLFP = xmlNode->getBoolAttribute("showLFP");
			showCompactView = xmlNode->getBoolAttribute("showCompactView");
			showSmooth = xmlNode->getBoolAttribute("showSmooth");
			showAutoRescale = xmlNode->getBoolAttribute("showAutoRescale");
			showRasters = xmlNode->getBoolAttribute("showRasters", false);
			showMatchRange = xmlNode->getBoolAttribute("showMatchRange");
			TTLchannelTrialAlignment = xmlNode->getIntAttribute("TTLchannelTrialAlignment");
			smoothingMS = xmlNode->getIntAttribute("smoothingMS");

			if (psthCanvas != nullptr)
			{
				psthCanvas->setLFPvisibility(showLFP);
				psthCanvas->setSpikesVisibility(showSortedUnits);
				psthCanvas->setCompactView(showCompactView);
				psthCanvas->setAutoRescale(showAutoRescale);
				psthCanvas->setMatchRange(showMatchRange);
				psthCanvas->setSmoothing(smoothingMS, true);
				psthCanvas->setRasterMode(showRasters);
			}
		}
	}
}


void PSTHEditor::comboBoxChanged(ComboBox* comboBox)
{

	if (comboBox == hardwareTrialAlignment)
	{
		std::cout << "Setting hardware trigger alignment channel to " << comboBox->getSelectedId() - 2 << std::endl;
		PSTHProcessor* processor = (PSTHProcessor*)getProcessor();
		processor->setHardwareTriggerAlignmentChannel(comboBox->getSelectedId() - 2);
	}
}

void PSTHEditor::visualizationMenu()
{
	PSTHProcessor* processor = (PSTHProcessor*)getProcessor();

	PopupMenu m;
	m.addItem(1, "Sorted Units", true, showSortedUnits);
	m.addItem(2, "LFP", true, showLFP);
	m.addItem(3, "Compact View", true, showCompactView);

	PopupMenu smoothingSubMenu;
	int SmoothingFactors[8] = { 0, 1, 2, 5, 10, 20, 50, 100 };
	for (int k = 0; k<8; k++)
	{
		String s;
		if (SmoothingFactors[k] == 0)
			s = "No Smoothing";
		else
			s = String(SmoothingFactors[k]) + " ms";

		smoothingSubMenu.addItem(40 + k, s, true, smoothingMS == SmoothingFactors[k]);
	}

	PopupMenu rangeTimeMenu, preRangeTimeMenu, postRangeTimeMenu;
	double rangeTimes[4] = { 0.5, 1, 1.5, 2 };
	TrialCircularBufferParams params = processor->trialCircularBuffer->getParams();

	for (int k = 0; k<4; k++)
	{
		String s = String(rangeTimes[k], 1) + " sec";
		if (processor->trialCircularBuffer == nullptr)
		{
			preRangeTimeMenu.addItem(50 + k, s, true, false);
			postRangeTimeMenu.addItem(60 + k, s, true, false);
		}
		else
		{
			preRangeTimeMenu.addItem(50 + k, s, true, fabs(params.preSec - rangeTimes[k])<0.01);
			postRangeTimeMenu.addItem(60 + k, s, true, fabs(params.postSec - rangeTimes[k])<0.01);
		}
	}
	rangeTimeMenu.addSubMenu("pre trial", preRangeTimeMenu, true);
	rangeTimeMenu.addSubMenu("post trial", postRangeTimeMenu, true);

	m.addSubMenu("Smooth Curves", smoothingSubMenu);
	m.addItem(7, "Raster mode", true, showRasters);

	m.addSubMenu("Range", rangeTimeMenu, true);
	m.addItem(5, "Auto Rescale", true, showAutoRescale);
	m.addItem(6, "Match range", true, showMatchRange);
	m.addItem(8, "Bar Graph", false, false);
	m.addItem(9, "2D Heat map", false, false);
	const int result = m.show();
	switch (result)
	{
	case 1:
		showSortedUnits = !showSortedUnits;
		psthCanvas->setSpikesVisibility(showSortedUnits);
		break;
	case 2:
		showLFP = !showLFP;
		psthCanvas->setLFPvisibility(showLFP);
		break;
	case 3:
		showCompactView = !showCompactView;
		psthCanvas->setCompactView(showCompactView);
		break;
	case 5:
		showAutoRescale = !showAutoRescale;
		psthCanvas->setAutoRescale(showAutoRescale);
		break;
	case 6:
		showMatchRange = !showMatchRange;
		psthCanvas->setMatchRange(showMatchRange);
		break;
	case 7:
		showRasters = !showRasters;
		psthCanvas->setRasterMode(showRasters);
		break;
	}
	if (result >= 40 && result <= 47)
	{
		smoothingMS = SmoothingFactors[result - 40];
		psthCanvas->setSmoothing(SmoothingFactors[result - 40], result - 40>0);
	}
	else if (result >= 50 && result <= 54)
	{
		// this will require killing
		double newPreSec = rangeTimes[result - 50];
		processor->modifyTimeRange(newPreSec, params.postSec);
	}
	else if (result >= 60 && result <= 64)
	{
		double newPostSec = rangeTimes[result - 60];
		processor->modifyTimeRange(params.preSec, newPostSec);
	}
}

void PSTHEditor::buttonEvent(Button* button)
{
	VisualizerEditor::buttonEvent(button);
	if (psthCanvas == nullptr)
		return;

	PSTHProcessor* processor = (PSTHProcessor*)getProcessor();

	if (button == clearDisplay)
	{
		if (processor->trialCircularBuffer != nullptr)
		{
			processor->trialCircularBuffer->clearAll();
			repaint();
		}
	}
	else if (button == visualizationOptions)
	{
		visualizationMenu();
	}
	else if (button == saveOptions)
	{

		PopupMenu m;


		/*	m.addItem(1,"TTL",true, processor->saveTTLs);
		m.addItem(2,"Network Events",true, processor->saveNetworkEvents);
		m.addItem(7,"Network Events [when recording is off]",true, processor->saveNetworkEventsWhenNotRecording);
		m.addItem(3,"Eye Tracking",true, processor->saveEyeTracking);*/

		//m.addItem(4,"Sorted Spikes: TS only ",true, processor->spikeSavingMode == 1);
		m.addItem(5, "Sorted Spikes: TS+waveform", true, processor->spikeSavingMode == 2);
		m.addItem(6, "All Spikes: TS+waveform", true, processor->spikeSavingMode == 3);

		const int result = m.show();

		if (result == 1)
		{
			processor->saveTTLs = !processor->saveTTLs;
		}
		else if (result == 2)
		{
			processor->saveNetworkEvents = !processor->saveNetworkEvents;
		}
		else if (result == 3)
		{
			processor->saveEyeTracking = !processor->saveEyeTracking;
		}
		else if (result == 4)
		{
			if (processor->spikeSavingMode == 1)
				processor->spikeSavingMode = 0;
			else
				processor->spikeSavingMode = 1;
		}
		else if (result == 5)
		{
			if (processor->spikeSavingMode == 2)
				processor->spikeSavingMode = 0;
			else
				processor->spikeSavingMode = 2;
		}
		else if (result == 6)
		{
			if (processor->spikeSavingMode == 3)
				processor->spikeSavingMode = 0;
			else
				processor->spikeSavingMode = 3;
		}
		else if (result == 7)
		{
			processor->saveNetworkEventsWhenNotRecording = !processor->saveNetworkEventsWhenNotRecording;
		}

	} 

}


Visualizer* PSTHEditor::createNewCanvas()
{
	PSTHProcessor* processor = (PSTHProcessor*)getProcessor();
	psthCanvas = new PSTHCanvas(processor);
	return psthCanvas;
}

void PSTHEditor::updateCanvas()
{
	if (psthCanvas != nullptr)
	{
		psthCanvas->updateNeeded = true;
	}
}

PSTHEditor::~PSTHEditor()
{


}
