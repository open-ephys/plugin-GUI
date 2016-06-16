#include "PSTHCanvas.h"

PSTHCanvas::PSTHCanvas(PSTHProcessor* n) :
processor(n)
{
	screenWidth = screenHeight = 0;
	conditionWidth = 200;

	inFocusedMode = false;
	showLFP = true;
	showSpikes = true;
	matchRange = false;
	smoothPlots = true;
	autoRescale = true;
	compactView = false;
	rasterMode = false;
	gaussianStandardDeviationMS = 10;
	viewport = new Viewport();
	psthDisplay = new PSTHDisplay(viewport, this);
	viewport->setViewedComponent(psthDisplay, false);
	viewport->setScrollBarsShown(true, true);
	addAndMakeVisible(viewport);


	visualizationButton = new UtilityButton("Visualization Options", Font("Default", 15, Font::plain));
	visualizationButton->addListener(this);
	addAndMakeVisible(visualizationButton);

	clearAllButton = new UtilityButton("Clear all", Font("Default", 15, Font::plain));
	clearAllButton->addListener(this);
	addAndMakeVisible(clearAllButton);

	conditionsViewport = new Viewport();
	conditionsList = new ConditionList(n, conditionsViewport, this);
	conditionsViewport->setViewedComponent(conditionsList, false);
	conditionsViewport->setScrollBarsShown(true, true);
	addAndMakeVisible(conditionsViewport);

	zoomButton = new UtilityButton("Zoom", Font("Default", 15, Font::plain));
	zoomButton->addListener(this);
	zoomButton->setColour(Label::textColourId, Colours::white);
	addAndMakeVisible(zoomButton);
	panButton = new UtilityButton("Pan", Font("Default", 15, Font::plain));
	panButton->addListener(this);
	panButton->setColour(Label::textColourId, Colours::white);
	panButton->setToggleState(true, dontSendNotification);

	addAndMakeVisible(panButton);
	resetAxesButton = new UtilityButton("Reset Axes", Font("Default", 15, Font::plain));
	resetAxesButton->addListener(this);
	resetAxesButton->setColour(Label::textColourId, Colours::white);
	addAndMakeVisible(resetAxesButton);


	resized();
	update();

}


PSTHCanvas::~PSTHCanvas()
{

}

void PSTHCanvas::beginAnimation()
{

	std::cout << "PSTHCanvas starting animation." << std::endl;
	startCallbacks();

}

void PSTHCanvas::buttonClicked(Button* button)
{
	if (button == visualizationButton)
	{
		PSTHEditor* ed = (PSTHEditor*)processor->getEditor();
		ed->visualizationMenu();
	}
	else if (button == clearAllButton)
	{
		processor->trialCircularBuffer->clearAll();
	}
	else if (button == zoomButton)
	{
		zoomButton->setToggleState(true, dontSendNotification);
		panButton->setToggleState(false, dontSendNotification);
		for (int k = 0; k<psthDisplay->psthPlots.size(); k++)
		{
			psthDisplay->psthPlots[k]->setMode(ZOOM);
		}
	}
	else if (button == panButton)
	{
		zoomButton->setToggleState(false, dontSendNotification);
		panButton->setToggleState(true, dontSendNotification);
		for (int k = 0; k<psthDisplay->psthPlots.size(); k++)
		{
			psthDisplay->psthPlots[k]->setMode(PAN);
		}
	}
	else if (button == resetAxesButton)
	{
		for (int k = 0; k<psthDisplay->psthPlots.size(); k++)
		{
			psthDisplay->psthPlots[k]->resetAxes();
		}
	}
}

void PSTHCanvas::endAnimation()
{
	std::cout << "PSTHCanvas ending animation." << std::endl;

	stopCallbacks();
}

void PSTHCanvas::setRasterMode(bool rasterModeActive)
{
	rasterMode = rasterModeActive;
	update();
}

void PSTHCanvas::setLFPvisibility(bool visible)
{
	showLFP = visible;
	update();
}

void PSTHCanvas::setSpikesVisibility(bool visible)
{
	showSpikes = visible;
	update();
}

void PSTHCanvas::setSmoothing(float _gaussianStandardDeviationMS, bool smooth_enabled)
{
	gaussianStandardDeviationMS = _gaussianStandardDeviationMS;
	for (int k = 0; k< psthDisplay->psthPlots.size(); k++)
	{
		if (smooth_enabled)
		{
			psthDisplay->psthPlots[k]->buildSmoothKernel(gaussianStandardDeviationMS);
			psthDisplay->psthPlots[k]->setSmoothState(true);
		}
		else
		{
			psthDisplay->psthPlots[k]->setSmoothState(false);
		}
		psthDisplay->psthPlots[k]->repaint();
	}

}

void PSTHCanvas::setSmoothPSTH(bool smooth)
{
	smoothPlots = smooth;
	for (int k = 0; k< psthDisplay->psthPlots.size(); k++)
	{
		psthDisplay->psthPlots[k]->setSmoothState(smoothPlots);
		psthDisplay->psthPlots[k]->repaint();
	}

}

void PSTHCanvas::setCompactView(bool compact)
{
	compactView = compact;
	update();
}

void PSTHCanvas::setMatchRange(bool on)
{
	matchRange = on;
	//update();
}

bool PSTHCanvas::getMatchRange()
{
	return matchRange;
}

void PSTHCanvas::setAutoRescale(bool state)
{
	autoRescale = state;
	for (int k = 0; k< psthDisplay->psthPlots.size(); k++)
	{
		psthDisplay->psthPlots[k]->setAutoRescale(autoRescale);
		psthDisplay->psthPlots[k]->repaint();
	}
}

void PSTHCanvas::setRange(double xmin, double xmax, double ymin, double ymax, xyPlotTypes plotType)
{

	for (int k = 0; k< psthDisplay->psthPlots.size(); k++)
	{
		if (psthDisplay->psthPlots[k]->getPlotType() == plotType)
		{
			psthDisplay->psthPlots[k]->setXRange(xmin, xmax);
			psthDisplay->psthPlots[k]->setYRange(ymin, ymax);
		}
		else
		{
			psthDisplay->psthPlots[k]->setXRange(xmin, xmax);
		}
	}
}


void PSTHCanvas::refreshState()
{
	update();
	resized();
}


void PSTHCanvas::update()
{
	//std::cout << "Updating SpikeDisplayCanvas" << std::endl;
	// clear all XY plots and create new ones...
	// delete all existing plots.
	// lock psth
	bool inPanMode = panButton->getToggleState();

	heightPerElectrodePix = 300;
	widthPerUnit = 300;
	int maxUnitsPerRow = (screenWidth - conditionWidth) / widthPerUnit;
	updateNeeded = false;
	for (int k = 0; k < psthDisplay->psthPlots.size(); k++)
	{
		delete psthDisplay->psthPlots[k];
	}
	psthDisplay->psthPlots.clear();
	if (processor->trialCircularBuffer == nullptr)
		return;

	const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);

	//processor->trialCircularBuffer->lockPSTH();
	numElectrodes = processor->trialCircularBuffer->getNumElectrodes();
	int maxUnitsPerElectrode = 0;
	int row = 0;
	int plotCounter = 0;
	numCols = 0;
	numRows = 0;
	int plotID = 0;
	for (int e = 0; e < numElectrodes; e++)
	{
		int offset = 0;
		bool plottedSomething = false;
		int electrodeID = processor->trialCircularBuffer->getElectrodeID(e);
		String electrodeName = processor->trialCircularBuffer->getElectrodeName(e);

		if (showLFP)
		{
			std::vector<int> channels = processor->trialCircularBuffer->getElectrodeChannels(e);
			offset = channels.size();
			for (int u = 0; u<channels.size(); u++)
			{
				GenericPlot* newplot;
				if (compactView)
				{
					String plotName = electrodeName + " Ch:" + String(1 + u);
					newplot = new GenericPlot(plotName, psthDisplay, ++plotID, LFP_PLOT, processor->trialCircularBuffer,
						electrodeID,
						channels[u],
						plotCounter, row, rasterMode, inPanMode);

					plotCounter++;
					numCols++;
					numCols = min(maxUnitsPerRow, numCols);

					if (plotCounter >= maxUnitsPerRow)
					{
						plotCounter = 0;
						row++;
					}
				}
				else
				{
					String plotName = electrodeName + ":Ch " + String(1 + u);
					newplot = new GenericPlot(plotName, psthDisplay, ++plotID, LFP_PLOT, processor->trialCircularBuffer,
						electrodeID,
						channels[u],
						u, row, rasterMode, inPanMode);
					numCols = max(numCols, u);

				}
				newplot->setSmoothState(smoothPlots);
				newplot->setAutoRescale(autoRescale);
				newplot->buildSmoothKernel(gaussianStandardDeviationMS);
				psthDisplay->psthPlots.push_back(newplot);
				psthDisplay->addAndMakeVisible(newplot);
				plottedSomething = true;
			}

		}

		if (showSpikes)
		{
			int numUnits = processor->trialCircularBuffer->getNumUnitsInElectrode(e);
			maxUnitsPerElectrode = MAX(maxUnitsPerElectrode, numUnits);
			if (numUnits > 0)
			{
				for (int u = 0; u<numUnits; u++)
				{
					GenericPlot* newplot;
					if (compactView)
					{
						String plotName = electrodeName + " Unit:" + String(processor->trialCircularBuffer->getUnitID(e, u));
						newplot = new GenericPlot(plotName, psthDisplay, ++plotID, SPIKE_PLOT, processor->trialCircularBuffer,
							electrodeID,
							processor->trialCircularBuffer->getUnitID(e, u),
							plotCounter, row, rasterMode, inPanMode);
						plotCounter++;
						numCols++;
						numCols = min(maxUnitsPerRow, numCols);

						if (plotCounter >= maxUnitsPerRow)
						{
							plotCounter = 0;
							row++;
						}
					}
					else
					{
						String plotName = electrodeName + " Unit:" + String(processor->trialCircularBuffer->getUnitID(e, u));
						newplot = new GenericPlot(plotName, psthDisplay, ++plotID, SPIKE_PLOT, processor->trialCircularBuffer,
							electrodeID,
							processor->trialCircularBuffer->getUnitID(e, u),
							offset + u, row, rasterMode, inPanMode);
						numCols = max(numCols, offset + u);
					}
					newplot->setSmoothState(smoothPlots);
					newplot->setAutoRescale(autoRescale);
					newplot->buildSmoothKernel(gaussianStandardDeviationMS);

					psthDisplay->psthPlots.push_back(newplot);
					psthDisplay->addAndMakeVisible(newplot);
				}
				plottedSomething = true;
			}
		}
		if (!compactView &&  plottedSomething)
			row++;
	}
	if (compactView)
	{
		numRows = row + 1;//MAX(1,row);
	}
	else
	{
		numRows = row;
		numCols = numCols + 1;
	}

	if (maxUnitsPerElectrode == 0 && !showLFP)
	{
		// nothing to be drawn...
		//processor->trialCircularBuffer->unlockPSTH();
		return;
	}

	psthDisplay->resized();
	psthDisplay->repaint();
	psthDisplay->refresh();
	resized();
	repaint();

	//processor->trialCircularBuffer->unlockPSTH();
	conditionsList->updateConditionButtons();
}


void PSTHCanvas::resized()
{
	screenWidth = getWidth();
	screenHeight = getHeight();

	//int scrollBarThickness = viewport->getScrollBarThickness();

	viewport->setBounds(0, 30, getWidth() - conditionWidth, getHeight() - 30);
	int totalHeight = numRows * heightPerElectrodePix;
	int totalWidth = numCols * widthPerUnit;
	psthDisplay->setBounds(0, 0, totalWidth, totalHeight);

	int numConditions = 0;
	if (processor->trialCircularBuffer != nullptr)
	{
		numConditions = processor->trialCircularBuffer->getNumConditions();
	}

	conditionsViewport->setBounds(getWidth() - conditionWidth, 30, conditionWidth, getHeight());
	conditionsList->setBounds(0, 0, conditionWidth, 50 + 20 * numConditions);

	visualizationButton->setBounds(20, 5, 150, 20);
	clearAllButton->setBounds(200, 5, 150, 20);

	zoomButton->setBounds(360, 5, 60, 20);
	panButton->setBounds(440, 5, 60, 20);
	resetAxesButton->setBounds(510, 5, 150, 20);

}

void PSTHCanvas::paint(Graphics& g)
{
	if (updateNeeded)
		update();
	g.fillAll(Colours::grey);

}

void PSTHCanvas::refresh()
{
	repaint();
	psthDisplay->refresh();
}
