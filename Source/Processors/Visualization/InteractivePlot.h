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

#ifndef __INTERACTIVE_PLOT_H
#define __INTERACTIVE_PLOT_H

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "Visualizer.h"
#include "../Editors/GenericEditor.h"

#include <list>
#include <vector>
#include <queue>

#ifndef MAX
#define MAX(x,y)((x)>(y))?(x):(y)
#endif 

#ifndef MIN
#define MIN(x,y)((x)<(y))?(x):(y)
#endif

/** 

	Represents a line on a 2D plot
	
*/
class XYLine
{
public:

	/** Creates a line from vectors of x and y values */
	XYLine(std::vector<float> x, std::vector<float> y);

	/** Sets the colour of the line*/
	void setColour(Colour c);

	/** Sets the width of the line*/
	void setWidth(float width);

	/** Renders the line. Stretches Y such that ymin->0 and ymax->plotHeight 
	    and only display points between [xmin and xmax] */
	void draw(Graphics &g, float xmin, float xmax, float ymin, float ymax, int width, int height, bool showBounds);

	/** Interpolate x value (bilinear interpolation) */
	float interpolate(float x_sample, bool &inrange);

	std::vector<float> x;
	std::vector<float> y;
	
	Colour colour;
	float width;
};

enum DrawComponentMode {ZOOM = 1, PAN = 2};

struct XYRange
{
	float xmin, xmax, ymin, ymax;
};

class InteractivePlot;

/** 
	
	A component that draws the lines, given 
	pan and zoom state

*/
class DrawComponent : public Component
{
public:

	/** Constructor */
	DrawComponent(InteractivePlot* plot);

	/** Destructor */
	~DrawComponent() { }

	/** Adds a line to be drawn */
	void add(XYLine* line);

	/** Clears all lines */
	void clear();

	/** Sets zoom / pan */
	void setMode(DrawComponentMode m);

	/** Sets the current displayed range */
	void setRange(XYRange range);

	/** Sets the limits on the displayable range */
	void setLimit(XYRange range);

	/** Sets whether X-axis is visible */
	void setXAxisShown(bool state);
	
	/** Sets whether Y-axis is visible*/
	void setYAxisShown(bool state);
	
	/** Sets the tick locations for the X and Y axes*/
	void setTickMarks(std::vector<float> xtick, std::vector<float> ytick);
	
	/** Sets whether the axes rescale automatically */
	void setAutoRescale(bool state);
	
	/** Sets the units for the X and Y axes*/
	void setUnits(String xUnits, String yUnits);
	
	/** Sets whether bounds are visible */
	void setBoundsShown(bool state);

	/** Sets whether grid is visible*/
	void showGrid(bool state);

	/** Gets the current range values */
	void getRange(XYRange& range);

private:

	/** The lines to be drawn */
	OwnedArray<XYLine> lines;

	/** Mouse listeners */
	void mouseDown(const juce::MouseEvent& event);
	void mouseDrag(const juce::MouseEvent& event);
	void mouseUp(const juce::MouseEvent& event);
	void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel);

	std::vector<float> computeSamplePositions(int subsample);
	std::list<XYRange> rangeMemory;
	
	int mouseDragX, mouseDragY, mouseDownX, mouseDownY, mousePrevX, mousePrevY;

	DrawComponentMode mode, savedMode;
	bool panning,zooming, autoRescale;

	std::vector<float> xtick, ytick;

	double lowestValue, highestValue;
	
	InteractivePlot* plt;

	String yUnits, xUnits;

	Font font;
	
	void drawTicks(Graphics &g);
	
	void paint(Graphics &g);
	
	XYRange range;

	bool showXAxis, showYAxis, showBounds;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawComponent);
};

/**

	Represents one axis (vertical or horizontal)
	
*/
class Axis : public Component
{
public:

	/** Constructor */
	Axis();

	/** Destructor */
	~Axis() { }

	/** Set tick locations and labels */
	void setTicks(std::vector<float> ticks_, std::vector<String> labels);

	/** Get the tick locations and labels */
	void getTicks(std::vector<float>& tickLocations, std::vector<String>& tickLbl);
	
	/** Set font height */
	void setFontHeight(int height);
	
	/** Set the color of the ticks and labels */
	void setColour(Colour c);
	
	/** Sets the min / max range, and number of ticks between them */
	void setRange(float minvalue, float maxvalue, int numTicks = 10);
	
	/** Sets whether the axis should be inverted */
	void setInverted(bool state);

protected:

	/** Computes the tick locations */
	void determineTickLocations(float minV, float maxV, int numTicks);

	/** Computes linearly spaced values */
	std::vector<float> linspace(float minv, float maxv, int numticks);

	/** Location of tick marks */
	std::vector<float> ticks;

	/** Labels for tick marks */
	std::vector<String> ticksLabels;

	bool axisIsInverted;

	int numDigits;
	Font font;
	int selectedTimeScale;
	
	float gain;
	
	bool horiz;
	
	float minv, maxv;

	Colour colour;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Axis);
};


/**

	Represents the horizontal (X) axis

*/
class XAxis : public Axis
{
public:

	/** Constructor */
	XAxis() { }

	/** Renders the axis */
	void paint(Graphics& g);
};

/**

	Represents the vertical (Y) axis

*/
class YAxis : public Axis
{
public:

	/** Constructor */
	YAxis() { }

	/** Renders the axis */
	void paint(Graphics& g);
};

/**

	A component for drawing 2D lines charts with interactive pan and zoom functionality.

*/

class PLUGIN_API InteractivePlot : 
	public Component, 
	public Button::Listener
{
public:

	/** Constructor */
	InteractivePlot();

	/** Destructor */
    ~InteractivePlot() { }

	/** Plots a line based on X and Y values */
	void plot(std::vector<float> x, 
			  std::vector<float> y, 
			  Colour c = Colours::white,
			  float linewidth = 1.0f);

	/** Clears all lines from the plot */
	void clear();

	/** Draws the plot */
	void show();

	/** Adds a title to the plot */
	void title(String t);

	/** Sets the units of the plot */
	void setUnits(String xUnits, String yUnits);

	/** Sets auto rescale state */
	void autoRescale(bool state);

	/** Sets whether control buttons are visible */
	void showControls(bool state);

	/** Sets whether x-axis is visible */
	void showXAxis(bool state);

	/** Sets whether y-axis is visible */
	void showYAxis(bool state);

	/** Sets whether grid is visible */
	void showGrid(bool state);

	/** Sets wether the bounds of each line are visible*/
	void showBounds(bool state);

	/** Set border color */
	void setBorderColor(Colour c);

	/** Sets ZOOM or PAN mode */
	void setMode(DrawComponentMode mode);

	/** Sets range of both axes*/
	void setRange(XYRange& range);

	/** Copies the current range values*/
	void getRange(XYRange& range);

	/** Sets the limit on the min/max range */
	void setRangeLimit(XYRange& range);

	/** Copies the limit on the min/max range */
	void getRangeLimit(XYRange& range);

	/** Renders the plot */
	void paint(Graphics &g);
	
	/** Called when size of plot is changed */
	void resized();

private:

	/** Respond to button clicks */
	void buttonClicked(Button* btn);

	Font font;
	juce::Colour borderColor;

	String titleString;
	double lowestValue, highestValue;
	bool controlButtonsVisible, gridIsVisible;
	
	ScopedPointer<UtilityButton> zoomButton,
		panButton,
		autoRescaleButton,
		boundsButton;
	
	ScopedPointer<XAxis> xAxis;
	ScopedPointer<YAxis> yAxis;

	XYRange range;
	XYRange limit;

	ScopedPointer<DrawComponent> drawComponent;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractivePlot);
};

#endif // __INTERACTIVE_PLOT_H