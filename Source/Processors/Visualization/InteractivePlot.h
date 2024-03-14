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

enum PLUGIN_API InteractivePlotMode { ON = 1, OFF = 2 };

enum PLUGIN_API PlotType { LINE, SCATTER, BAR, FILLED };

struct PLUGIN_API XYRange
{
	float xmin, xmax, ymin, ymax;

	void print()
	{
		std::cout << xmin << " " << xmax << " " << ymin << " " << ymax << std::endl;
	}

};


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

	/** Sets the colour of the line*/
	void setOpacity(float opacity);

	/** Sets the plot type*/
	void setType(PlotType type);

	/** Sets the width of the line, bar, or dot */
	void setWidth(float width);

	/** Returns the X and Y min/max for this line*/
	XYRange getBounds();

	/** Renders the line. Stretches Y such that ymin->0 and ymax->plotHeight 
	    and only display points between [xmin and xmax] */
	void draw(Graphics &g, XYRange& range, int width, int height);

	/** The x values */
	std::vector<float> x;

	/** The x values */
	std::vector<float> y;

private:

	/** The parameters of this line */
	Colour colour;
	float width;
	PlotType type;
	float opacity;
	XYRange range;
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

	/** Sets the min / max range, and number of ticks between them */
	void setRange(float minvalue, float maxvalue, int numTicks = 10);

	/** Set tick locations and labels */
	void setTicks(std::vector<float> ticks_, std::vector<String> labels);

	/** Get the tick locations */
	void getTicks(std::vector<float>& tickLocations, std::vector<float>& tickValues);
	
	/** Set font height */
	void setFontHeight(int height);
	
	/** Set the color of the ticks and labels */
	void setAxisColour(Colour c);
	
	/** Sets whether the axis should be inverted */
	void setInverted(bool state);

	bool axisIsInverted;

protected:

	/** Computes the tick locations */
	void determineTickLocations(float min, float max, int numTicks);

	/** Computes linearly spaced values between min and max point */
	std::vector<float> linspace(float min, float max, int numticks);

	/** Location of tick marks */
	std::vector<float> ticks;

	/** Value of tick marks */
	std::vector<float> tickValues;

	/** Labels for tick marks */
	std::vector<String> tickLabels;

	float min, max;

	Colour colour;
	Font font;

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

	A component that draws the lines, given
	pan and zoom state

*/
class PLUGIN_API DrawComponent : public Component
{
public:

	/** Constructor */
	DrawComponent(Axis* x, Axis* y);

	/** Destructor */
	~DrawComponent() { }

	/** Adds a line to be drawn */
	void add(XYLine* line);

	/** Clears all lines */
	void clear();

	/** Sets zoom / pan */
	void setMode(InteractivePlotMode m);

	/** Sets the current displayed range */
	void setRange(XYRange& range);

	/** Sets the limits on the displayable range */
	void setLimit(XYRange& range);

	/** Sets the tick locations for the X and Y axes*/
	void setTickMarks(std::vector<float> xtick, std::vector<float> ytick);

	/** Rescales plot to full extent of available lines */
	void rescale();

	/** Sets whether grid is visible*/
	void showGrid(bool state);

	/** Sets the background colour*/
	void setBackgroundColour(Colour c);

	/** Sets the grid colour*/
	void setGridColour(Colour c);

	/** Gets the current range values */
	void getRange(XYRange& range);

private:

	/** The lines to be drawn */
	OwnedArray<XYLine> lines;

	/** Mouse listeners */
	void mouseDown(const juce::MouseEvent& event);
	void mouseDrag(const juce::MouseEvent& event);
	void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);

	//std::vector<float> computeSamplePositions(int subsample);
	std::list<XYRange> rangeMemory;

	InteractivePlotMode mode;

	std::vector<float> xticks, yticks, xtickvalues, ytickvalues;

	void drawGrid(Graphics& g);

	void paint(Graphics& g);

	XYRange range, limit, originalRange;

	Colour backgroundColour, gridColour;

	Axis* xAxis;
	Axis* yAxis;
	bool gridIsVisible;

	bool firstLine;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawComponent);
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

	/** Adds a line based on X and Y values */
	void plot(std::vector<float> x, 
			  std::vector<float> y, 
			  Colour c = Colours::white,
			  float width = 1.0f,
			  float opacity = 1.0f,
			  PlotType type = PlotType::LINE);

	/** Clears all lines from the plot */
	void clear();

	/** Draws the plot */
	void show();

	/** Adds a title to the plot */
	void title(String t);

	/** Sets the x-axis label */
	void xlabel(String label);
	
	/** Sets the y-axis label */
	void ylabel(String label);

	/** Set whether the plot is interactive */
	void setInteractive(InteractivePlotMode mode);

	/** Sets whether x-axis is visible */
	void showXAxis(bool state);

	/** Sets whether y-axis is visible */
	void showYAxis(bool state);

	/** Sets whether grid is visible */
	void showGrid(bool state);

	/** Sets the background colour */
	void setBackgroundColour(Colour c);

	/** Sets the background colour */
	void setGridColour(Colour c);

	/** Sets the background colour */
	void setAxisColour(Colour c);

	/** Sets range of both axes*/
	void setRange(XYRange& range);

	/** Copies the current range values*/
	void getRange(XYRange& range);

	/** Called when size of plot is changed */
	void resized();

private:

	/** Respond to button clicks */
	void buttonClicked(Button* btn);

	Font font;
	juce::Colour backgroundColour;
	juce::Colour axisColour;
	juce::Colour gridColour;

	bool controlButtonsVisible, gridIsVisible;
	
	std::unique_ptr<UtilityButton> autoRescaleButton;
	
	std::unique_ptr<Label> titleLabel;
	std::unique_ptr<Label> xLabel;
	std::unique_ptr<Label> yLabel;
	std::unique_ptr<XAxis> xAxis;
	std::unique_ptr<YAxis> yAxis;
	std::unique_ptr<DrawComponent> drawComponent;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractivePlot);
};

#endif // __INTERACTIVE_PLOT_H
