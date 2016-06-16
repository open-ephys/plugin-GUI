#include "GenericPlot.h"

/*************************************************************************/
// Generic plot replaces XYPlot class, with better class organization and code encapsulation.
// All low level plotting of curves is handled by MatlabLikePlot object (including zooming / panning / ...)
// All raster plots will be handled by another class (?)
// 

GenericPlot::GenericPlot(String name, PSTHDisplay* dsp, int plotID_, xyPlotTypes plotType_,
	TrialCircularBuffer *tcb_, int electrodeID_, int subID_, int row_, int col_, bool rasterMode_, bool panM) : tcb(tcb_), electrodeID(electrodeID_), plotID(plotID_),
	plotType(plotType_), subID(subID_), row(row_), col(col_), rasterMode(rasterMode_), display(dsp), plotName(name), inPanMode(panM)
{
	fullScreenMode = false;
	mlp = new MatlabLikePlot();
	mlp->setControlButtonsVisibile(false);
	TrialCircularBufferParams params = tcb->getParams();

	if (inPanMode)
		mlp->setMode(DrawComponentMode::PAN);
	else
		mlp->setMode(DrawComponentMode::ZOOM);

	if (rasterMode) {
		mlp->setImageMode(true);
		mlp->setAutoRescale(false);
	}
	else
		mlp->setImageMode(false);


	addAndMakeVisible(mlp);

	mlp->setTitle(plotName);
	if (plotType == SPIKE_PLOT)
	{
		//mlp->setTitle("Unit "+String(electrodeID)+":"+String(subID));
		mlp->setFiringRateMode(true);
		mlp->setBorderColor(tcb->getUnitColor(electrodeID, subID));
		int uniqueIntervalID = tcb->getUnitUniqueInterval(electrodeID, subID);
		mlp->setActivateButtonVisiblilty(true, uniqueIntervalID);
		mlp->setRangeLimit(-params.preSec, params.postSec + params.maxTrialTimeSeconds, 0, 1e3);
	}
	else if (plotType == LFP_PLOT)
	{
		//	mlp->setTitle("Ch "+String(electrodeID)+":"+String(subID));
		mlp->setBorderColor(juce::Colours::white);
		mlp->setRangeLimit(-params.preSec, params.postSec + params.maxTrialTimeSeconds, -1e3, 1e3);
	}

	guassianStandardDeviationMS = 5; // default smoothing
	buildSmoothKernel(guassianStandardDeviationMS);

	smoothPlot = plotType == SPIKE_PLOT; // don't smooth LFPs
	fullScreenMode = false;
}

xyPlotTypes GenericPlot::getPlotType()
{
	return plotType;
}

void GenericPlot::resized()
{
	int w = getWidth();
	int h = getHeight();
	mlp->setBounds(0, 0, w, h);
}

void GenericPlot::paintSpikeRaster(Graphics &g)
{
	//tictoc.Tic(16);
	int numTrialTypes = tcb->getNumTrialTypesInUnit(electrodeID, subID);
	if (numTrialTypes > 0)
	{
		int numTrials = tcb->getNumTrialsInUnit(electrodeID, subID);
		mlp->setAuxiliaryString(String(numTrials) + " trials");

		float xmin, xmax, ymin, ymax, maxValue;
		mlp->getRange(xmin, xmax, ymin, ymax);
		juce::Image rasterImage = tcb->getTrialsAverageUnitResponseAsJuceImage(electrodeID, subID, guassianStandardDeviationMS, xmin, xmax, ymin, ymax, maxValue);
		mlp->drawImage(rasterImage, maxValue);
	}
	//tictoc.Toc(17);
}

void GenericPlot::paintSpikes(Graphics &g)
{
	//tictoc.Tic(15);
	std::vector<XYline> lines = tcb->getUnitConditionCurves(electrodeID, subID);
	int numTrials = tcb->getNumTrialsInUnit(electrodeID, subID);
	mlp->setAuxiliaryString(String(numTrials) + " trials");

	mlp->clearplot();
	for (int k = 0; k<lines.size(); k++)
	{
		if (smoothPlot)
		{
			lines[k].smooth(smoothKernel);
		}
		mlp->plotxy(lines[k]);
	}
	//tictoc.Toc(15);
}

void GenericPlot::paintLFPraster(Graphics &g)
{
	//tictoc.Tic(14);
	int numTrialTypes = tcb->getNumTrialTypesInChannel(electrodeID, subID);
	if (numTrialTypes > 0)
	{
		int numTrials = tcb->getNumTrialsInChannel(electrodeID, subID);
		mlp->setAuxiliaryString(String(numTrials) + " trials");

		float xmin, xmax, ymin, ymax, maxValue;
		mlp->getRange(xmin, xmax, ymin, ymax);
		juce::Image rasterImage = tcb->getTrialsAverageChannelResponseAsJuceImage(electrodeID, subID, guassianStandardDeviationMS, xmin, xmax, ymin, ymax, maxValue);
		mlp->drawImage(rasterImage, maxValue);
	}
	//tictoc.Toc(14);
}

void GenericPlot::paintLFP(Graphics &g)
{
	//tictoc.Tic(13);
	std::vector<XYline> lines = tcb->getElectrodeConditionCurves(electrodeID, subID);
	mlp->clearplot();

	int numTrials = tcb->getNumTrialsInChannel(electrodeID, subID);
	mlp->setAuxiliaryString(String(numTrials) + " trials");

	for (int k = 0; k<lines.size(); k++)
	{
		if (smoothPlot)
		{
			lines[k].smooth(smoothKernel);
		}
		mlp->plotxy(lines[k]);
	}
	//tictoc.Toc(13);
}

void GenericPlot::paint(Graphics &g)
{
	//printf("Entering GenericPlot::paint\n");
	//tictoc.Tic(12);
	if (mlp->eventsAvail())
	{
		String lastEvent = mlp->getLastEvent();
		handleEventFromMatlabLikePlot(lastEvent);
	}

	if (plotType == SPIKE_PLOT)
	{
		if (rasterMode)
			paintSpikeRaster(g);
		else
			paintSpikes(g);
	}
	else if (plotType == LFP_PLOT)
	{
		if (rasterMode)
			paintLFPraster(g);
		else
			paintLFP(g);
	}
	//printf("Exitting GenericPlot::paint\n");
	//tictoc.Toc(12);
}


void GenericPlot::setSmoothState(bool state)
{
	smoothPlot = state;
}

void GenericPlot::setAutoRescale(bool state)
{
	autoRescale = state;
	mlp->setAutoRescale(state);
}

void GenericPlot::setXRange(double xmin, double xmax)
{
	float curr_minx, curr_miny, curr_maxx, curr_maxy;
	mlp->getRange(curr_minx, curr_maxx, curr_miny, curr_maxy);
	mlp->setRange(xmin, xmax, curr_miny, curr_maxy, false);
}

void GenericPlot::setYRange(double ymin, double ymax)
{
	float curr_minx, curr_miny, curr_maxx, curr_maxy;
	mlp->getRange(curr_minx, curr_maxx, curr_miny, curr_maxy);
	mlp->setRange(curr_minx, curr_maxx, ymin, ymax, false);
}


void GenericPlot::setMode(DrawComponentMode mode)
{
	mlp->setMode(mode);
}

void GenericPlot::resetAxes()
{
	TrialCircularBufferParams params = tcb->getParams();
	if (plotType == SPIKE_PLOT)
	{
		std::vector<XYline> lines = tcb->getUnitConditionCurves(electrodeID, subID);
		double trial_xmin, trial_xmax;
		tcb->getUnitConditionRange(electrodeID, subID, trial_xmin, trial_xmax);

		float xmin = 0, xmax = 0;
		double ymin = 0, ymax = 0;
		float highestY = 0;
		for (int k = 0; k<lines.size(); k++)
		{
			lines[k].getYRange(xmin, xmax, ymin, ymax);
			highestY = MAX(highestY, ymax);
		}
		mlp->setRange(trial_xmin, trial_xmax, 0, highestY, false);

	}
	else if (plotType == LFP_PLOT)
	{
		double trial_xmin, trial_xmax;
		tcb->getElectrodeConditionRange(electrodeID, subID, trial_xmin, trial_xmax);

		std::vector<XYline> lines = tcb->getElectrodeConditionCurves(electrodeID, subID);
		float xmin=0, xmax=0;
		double ymin=0, ymax=0;
		float highestY = -1e10, lowestY = 1e10;
		for (int k = 0; k<lines.size(); k++)
		{
			lines[k].getYRange(xmin, xmax, ymin, ymax);
			highestY = MAX(highestY, ymax);
			lowestY = MIN(lowestY, ymin);
		}

		mlp->setRange(trial_xmin, trial_xmax, lowestY, highestY, false);
	}
}

void GenericPlot::buildSmoothKernel(float gaussianStandardDeviationMS_)
{
	guassianStandardDeviationMS = gaussianStandardDeviationMS_;
	// assume each bin correponds to one millisecond.
	// build the gaussian kernel
	int numKernelBins = 2 * (int)(guassianStandardDeviationMS*3.5) + 1; // +- 3.5 standard deviations.
	int zeroIndex = (numKernelBins - 1) / 2;
	smoothKernel.resize(numKernelBins);
	float sumZ = 0;
	for (int k = 0; k<numKernelBins; k++)
	{
		float z = float(k - zeroIndex);
		smoothKernel[k] = exp(-(z*z) / (2 * guassianStandardDeviationMS*guassianStandardDeviationMS));
		sumZ += smoothKernel[k];
	}
	// normalize kernel
	for (int k = 0; k<numKernelBins; k++)
	{
		smoothKernel[k] /= sumZ;
	}
}

void GenericPlot::handleEventFromMatlabLikePlot(String event)
{
	std::vector<String> command = StringTS(event).splitString(' ');
	//addEvent("NewRange "+String(xmin)+" "+String(xmax)+" "+String(ymin)+" "+String(ymax));
	if (command[0] == "DblkClickRight")
	{

		if (plotType == SPIKE_PLOT)
		{
			tcb->clearUnitStatistics(electrodeID, subID);
		}
		else if (plotType == LFP_PLOT)
		{
			tcb->clearChanneltatistics(electrodeID, subID);
		}

	}
	else
		if (command[0] == "DblkClickLeft")
		{
			// full screen toggle
			display->focusOnPlot(plotID);
		}
		else if (command[0] == "StartInterval")
		{


			int uniqueIntervalID = tcb->setUnitUniqueInterval(electrodeID, subID, true);
			mlp->setActivateButtonVisiblilty(true, uniqueIntervalID);
			// post this as a network message as well
			//StringTS s("UnitIntervalStart " + String(electrodeID) + " " + String(subID) + " " + String(uniqueIntervalID));
			//display->processor->handleNetworkMessage(s);
		}
		else if (command[0] == "StopInterval")
		{
			int uniqueIntervalID = tcb->setUnitUniqueInterval(electrodeID, subID, false);
			mlp->setActivateButtonVisiblilty(true, -1);
			// post this as a network message as well
			//StringTS s("UnitIntervalStop " + String(electrodeID) + " " + String(subID) + " " + String(uniqueIntervalID));
			//display->processor->handleNetworkMessage(s);
		}
		else if (command[0] == "NewRange")
		{
			if (display->canvas->getMatchRange())
			{
				double xmin = command[1].getDoubleValue();
				double xmax = command[2].getDoubleValue();
				double ymin = command[3].getDoubleValue();
				double ymax = command[4].getDoubleValue();
				display->canvas->setRange(xmin, xmax, ymin, ymax, plotType);
			}
		}
}
