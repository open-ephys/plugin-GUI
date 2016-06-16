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

#ifndef PSTHCANVAS_H_INCLUDED
#define PSTHCANVAS_H_INCLUDED

#include <EditorHeaders.h>
#include <VisualizerEditorHeaders.h>
#include "PSTHConditionList.h"
#include "PSTHDisplay.h"
#include "PSTHEditor.h"
#include "PSTHConditionList.h"
#include "PSTHProcessor.h"
class PSTHDisplay;

class PSTHProcessor;
class ConditionList;

class PSTHCanvas : public Visualizer, public Button::Listener
{
public:
	PSTHCanvas(PSTHProcessor* n);
	~PSTHCanvas();

	void paint(Graphics& g);

	void refresh();

	void beginAnimation();
	void endAnimation();

	void refreshState();
	void update();

	void resized();
	void buttonClicked(Button* button);

	void setRasterMode(bool rasterModeActive);
	void setLFPvisibility(bool visible);
	void setSpikesVisibility(bool visible);
	void setSmoothPSTH(bool smooth);
	void setSmoothing(float _gaussianStandardDeviationMS, bool state);
	void setAutoRescale(bool state);
	void setCompactView(bool compact);
	void setMatchRange(bool on);
	bool getMatchRange();
	void setParameter(int, float) {}
	void setParameter(int, int, int, float) {}

	void setRange(double xmin, double xmax, double ymin, double ymax, xyPlotTypes plotType);

	void startRecording() { } // unused
	void stopRecording() { } // unused

	int numElectrodes;
	int maxUnitsPerElectrode;
	int heightPerElectrodePix;
	int widthPerUnit;
	bool updateNeeded;
	int screenHeight, screenWidth;

private:
	int conditionWidth;

	bool showLFP, showSpikes, smoothPlots, autoRescale, compactView, matchRange, inFocusedMode, rasterMode;
	PSTHProcessor* processor;
	ScopedPointer<Viewport> viewport, conditionsViewport;
	ScopedPointer<PSTHDisplay> psthDisplay;
	ScopedPointer<ConditionList> conditionsList;
	ScopedPointer<UtilityButton> visualizationButton, clearAllButton, zoomButton, panButton, resetAxesButton;
	float gaussianStandardDeviationMS;
	int numRows, numCols;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PSTHCanvas);

};

#endif