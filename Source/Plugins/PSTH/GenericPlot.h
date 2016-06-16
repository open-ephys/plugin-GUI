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

#ifndef GENERICPLOT_H_INCLUDED
#define GENERICPLOT_H_INCLUDED

#include "../Common/StringTS.h"
#include <EditorHeaders.h>
#include "PSTHDisplay.h"
#include "MatlabLikePlot.h"
#include "TrialCircularBuffer.h"

class PSTHDisplay;
class GenericPlot : public Component
{
public:
	GenericPlot(String name, PSTHDisplay* dsp, int plotID_, xyPlotTypes plotType,
		TrialCircularBuffer* tcb_, int electrodeID_, int subID_, int row_, int col_, bool _rasterMode, bool inPanMode);
	void resized();
	void paint(Graphics& g);
	int getRow()
	{
		return row;
	}
	int getCol()
	{
		return col;
	}
	int getPlotID()
	{
		return plotID;
	}
	bool isFullScreen()
	{
		return fullScreenMode;
	}
	void toggleFullScreen(bool state)
	{
		fullScreenMode = state;
	}
	void setSmoothState(bool state);
	void setAutoRescale(bool state);
	void buildSmoothKernel(float gaussianStandardDeviationMS);
	xyPlotTypes getPlotType();
	void setMode(DrawComponentMode mode);

	void setXRange(double xmin, double xmax);
	void setYRange(double ymin, double ymax);

	void handleEventFromMatlabLikePlot(String event);
	void resetAxes();
private:
	void paintSpikeRaster(Graphics& g);
	void paintSpikes(Graphics& g);
	void paintLFPraster(Graphics& g);
	void paintLFP(Graphics& g);

	ScopedPointer<MatlabLikePlot> mlp;
	PSTHDisplay* display;
	TrialCircularBuffer* tcb;

	int plotID;
	xyPlotTypes plotType;
	int electrodeID;
	int subID;
	int row, col;
	bool rasterMode;
	bool fullScreenMode;
	bool smoothPlot;
	bool autoRescale;
	bool inPanMode;
	float guassianStandardDeviationMS;
	String plotName;
	std::vector<float> smoothKernel;
};

#endif