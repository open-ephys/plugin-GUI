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

#include "InteractivePlot.h"

InteractivePlot::InteractivePlot() :
	titleString(""),
	borderColor(Colours::white),
	controlButtonsVisible(false),
	gridIsVisible(false)
{
	xAxis = std::make_unique<XAxis>();
	addAndMakeVisible(xAxis.get());

	yAxis = std::make_unique<YAxis>();
	addAndMakeVisible(yAxis.get());

	drawComponent = std::make_unique<DrawComponent>(this);
	addAndMakeVisible(drawComponent.get());

	font = Font("Default", 15, Font::plain);
    
	range.xmin = -0.5;
	range.xmax = 0.5;
	range.ymin = -1e3;
	range.ymax = 1e3;

	limit.xmin = -1e10;
	limit.xmax = 1e10;
	limit.ymin = -1e10;
	limit.ymax = 1e10;
	 
	zoomButton = std::make_unique<UtilityButton>("Zoom",Font("Default", 15, Font::plain));
	zoomButton->setToggleState(true, dontSendNotification);
	zoomButton ->addListener(this);
	addAndMakeVisible(zoomButton.get());

	panButton = std::make_unique<UtilityButton>("Pan",Font("Default", 15, Font::plain));
	panButton->setToggleState(false, dontSendNotification);
	panButton ->addListener(this);
	addAndMakeVisible(panButton.get());

	autoRescaleButton = std::make_unique<UtilityButton>("Auto Rescale",Font("Default", 15, Font::plain));
	autoRescaleButton->setToggleState(true, sendNotification);
	autoRescaleButton ->addListener(this);
	addAndMakeVisible(autoRescaleButton.get());

	boundsButton= std::make_unique<UtilityButton>("Show Bounds",Font("Default", 15, Font::plain));
	boundsButton->setToggleState(false, sendNotification);
	boundsButton ->addListener(this);
	addAndMakeVisible(boundsButton.get());

	setRange(range);
}

void InteractivePlot::setRangeLimit(XYRange& limit_)
{

	limit.xmin = limit_.xmin;
	limit.xmax = limit_.xmax;
	limit.ymin = limit_.ymin;
	limit.ymax = limit_.ymax;

}


void InteractivePlot::getRangeLimit(XYRange& limit_)
{
	limit_.xmin = limit.xmin;
	limit_.xmax = limit.xmax;
	limit_.ymin = limit.ymin;
	limit_.ymax = limit.ymax;
}

void InteractivePlot::setMode(InteractivePlotMode mode)
{
	if (mode == InteractivePlotMode::ZOOM)
	{
		zoomButton->setToggleState(true, sendNotification);
	} 
	else if (mode == InteractivePlotMode::PAN)
	{
		panButton->setToggleState(true, sendNotification);
	}
}

void InteractivePlot::showBounds(bool state)
{
	drawComponent->setBoundsShown(state);
}

void InteractivePlot::showGrid(bool state)
{
	drawComponent->showGrid(state);
}

void InteractivePlot::showXAxis(bool state)
{
	drawComponent->setXAxisShown(state);
}

void InteractivePlot::showYAxis(bool state)
{
	drawComponent->setYAxisShown(state);
}

void InteractivePlot::showControls(bool state)
{
	controlButtonsVisible = state;
	resized();
}

void InteractivePlot::autoRescale(bool state)
{
	drawComponent->setAutoRescale(state);
}

void InteractivePlot::buttonClicked(Button *btn)
{
	bool prevState = btn->getToggleState();
	
	if (btn == zoomButton.get())
	{
		zoomButton->setToggleState(!prevState, dontSendNotification);
		panButton->setToggleState(false, dontSendNotification);
		drawComponent->setMode(ZOOM);

	} else if  (btn == autoRescaleButton.get())
	{
		btn->setToggleState(!prevState, dontSendNotification);
		drawComponent->setAutoRescale(!prevState);

	} else if (btn == panButton.get())
	{
		btn->setToggleState(!prevState, dontSendNotification);
		drawComponent->setMode(PAN);
		zoomButton->setToggleState(false, dontSendNotification);

	}  else if (btn == boundsButton.get())
	{
		btn->setToggleState(!prevState, dontSendNotification);
		drawComponent->setBoundsShown(!prevState);
	}
}

void InteractivePlot::resized()
{
	
	int w = getWidth();
	int h = getHeight();
	
	if (h == 0 || w == 0)
		return;

	int heightOffset = (controlButtonsVisible) ? 50 : 20;
	int axesWidth = 35; // Fixed
	int axesHeight = 25;
	
	if (MIN(w,h) > 250)
	{
		yAxis->setFontHeight(12);
		xAxis->setFontHeight(12);
	} else
	{
		yAxis->setFontHeight(8);
		xAxis->setFontHeight(8);
	}
	
	yAxis->setBounds(2, heightOffset, axesWidth -2, h - axesHeight - heightOffset);
	xAxis->setBounds(axesWidth, h - axesHeight - 2, w - axesWidth, axesHeight -2);
	drawComponent->setBounds(axesWidth, heightOffset, w - axesWidth - 2, h - axesHeight - heightOffset);

	if (controlButtonsVisible)
	{
		zoomButton ->setBounds(axesWidth,heightOffset-25,80,25);
		panButton ->setBounds(axesWidth +100,heightOffset-25,80,25);
		autoRescaleButton ->setBounds(axesWidth +200,heightOffset-25,80,25);
		boundsButton->setBounds(axesWidth +400,heightOffset-25,80,25);

		zoomButton->setVisible(true);
		panButton->setVisible(true);
		autoRescaleButton->setVisible(true);
		boundsButton->setVisible(true);

	} else 
	{
		zoomButton->setVisible(false);
		panButton->setVisible(false);
		autoRescaleButton->setVisible(false);
		boundsButton->setVisible(false);
	}
}

void InteractivePlot::plot(std::vector<float> x,
	std::vector<float> y,
	Colour c,
	float linewidth)
{

	XYLine* line = new XYLine(x, y);
	line->setColour(c);
	line->setWidth(linewidth);

	drawComponent->add(line);
}


void InteractivePlot::setBorderColor(juce::Colour col) 
{

	borderColor = col;
	
}

void InteractivePlot::getRange(XYRange& range_)
{
	drawComponent->getRange(range_);
}

void InteractivePlot::setRange(XYRange& newRange)
{

	range.xmin = MIN(limit.xmax, MAX(limit.xmin, newRange.xmin));
	range.xmax = MAX(limit.xmin, MIN(limit.xmax, newRange.xmax));

	range.ymin = MIN(limit.ymax, MAX(limit.ymin, newRange.ymin));
	range.ymax = MAX(limit.ymin, MIN(limit.ymax, newRange.ymax));

	// determine tick marks....
	int numTicks = (getWidth() < 250) ? 5 : 7;
	
	yAxis->setRange(range.ymin, range.ymax, numTicks);
	xAxis->setRange(range.xmin, range.xmax, numTicks);
	
	drawComponent->setRange(range);

	std::vector<float> xtick, ytick;
	std::vector<String> xtickLbl, ytickLbl;
	yAxis->getTicks(ytick,ytickLbl);
	xAxis->getTicks(xtick,xtickLbl);

	drawComponent->setTickMarks(xtick,ytick);

}

void InteractivePlot::title(String t)
{
	titleString = t;
}

void InteractivePlot::setUnits(String xUnits, String yUnits)
{
	// to be implemented
}

void InteractivePlot::show()
{
	repaint();

	drawComponent->repaint();
}

void InteractivePlot::paint(Graphics &g)
{
	g.setColour(borderColor);
	g.drawRect(0,0,getWidth(),getHeight(),2);
	g.setColour(juce::Colours::white);
	g.setFont(font);
	g.drawText(titleString, 30,0,getWidth()-30,25,juce::Justification::centred,true);
}

void InteractivePlot::clear()
{
	drawComponent->clear();
}


Axis::Axis() : axisIsInverted(false)
{
	minv = -1e4;
	maxv = 1e4;
	font = Font("Default", 15, Font::plain);
	colour = Colours::white;
	setRange(minv, maxv, 5);
}


void Axis::setInverted(bool state)
{
	axisIsInverted = state;
}

void Axis::getTicks(std::vector<float> &tickLocations, std::vector<String> &tickLbl)
{
	tickLocations = ticks;
	tickLbl = ticksLabels;
}


std::vector<float> Axis::linspace(float minv, float maxv, int numticks)
{
	// generate equi distance points (except first and last one).
	float xRange = maxv-minv;
	float dx = xRange / (numticks-1);
	std::vector<float> ticksLoc;
	ticksLoc.clear();
	ticksLoc.push_back(minv+dx/4);
	for (int k=1;k<numticks-1;k++)
	{
		float tic = minv + dx * k;
		ticksLoc.push_back(tic);
	}
	ticksLoc.push_back(maxv-dx/4);

	return ticksLoc;
}


void Axis::determineTickLocations(float minV, float maxV, int numTicks)
{

	ticks = linspace(minV, maxV, numTicks);
	ticksLabels.resize(ticks.size());
		
	for (int k = 0; k < ticks.size(); k++)
	{
		String tickString = (numDigits == 0) ? String(int(ticks[k]*gain)) : String(ticks[k]*gain,numDigits);
		ticksLabels[k] = tickString;
	}
}

void Axis::setRange(float minvalue, float maxvalue, int numTicks)
{
	minv = minvalue;
	maxv = maxvalue;

	determineTickLocations(minvalue, maxvalue, numTicks);

}

void Axis::setTicks(std::vector<float> ticks_, std::vector<String> tickLabels_)
{
	ticks = ticks_;
	ticksLabels = tickLabels_;
}

void Axis::setFontHeight(int height)
{
	font.setHeight(height);
}

void Axis::setColour(Colour c)
{
	colour = c;
}

void XAxis::paint(Graphics &g)
{

	/*g.setFont(font);

	int w = getWidth();
	int h = getHeight();

	int ticklabelWidth = 60;
	int tickLabelHeight = 20;
	
	g.setColour(colour);

	for (int k = 0; k < ticks.size(); k++)
	{
		
		float xtickloc = (ticks[k] -minv) / (maxv-minv) * w; 
		
		g.drawText(ticksLabels[k], 
				   xtickloc-ticklabelWidth / 2, 
				   0,
				   ticklabelWidth,
				   tickLabelHeight,
				   juce::Justification::centred,
				   true);
	}*/

}

void YAxis::paint(Graphics& g)
{

	/*g.setFont(font);

	int w = getWidth();
	int h = getHeight();

	int ticklabelWidth = 60;
	int tickLabelHeight = 20;

	g.setColour(colour);

	for (int k = 0; k < ticks.size(); k++)
	{
		
		float ytickloc = (ticks[k] - minv) / (maxv - minv) * h;
		
		if (axisIsInverted)
			g.drawText(ticksLabels[k], 0, ytickloc - tickLabelHeight / 2, w - 3, tickLabelHeight, Justification::right, false);
		else
			g.drawText(ticksLabels[k], 0, h - ytickloc - tickLabelHeight / 2, w - 3, tickLabelHeight, Justification::right, false);

	}*/
}


XYLine::XYLine(std::vector<float> x_, std::vector<float> y_) : 
	x(x_), y(y_), colour(Colours::white), width(1.0f)
{
	
}

void XYLine::setColour(Colour c)
{
	colour = c;
}

void XYLine::setWidth(float width_)
{
	width = width_;
}


float XYLine::interpolate(float x_sample, bool &inrange)
{
	/*inrange = (x_sample >= x0 && x_sample <= xn);
	if (!inrange )
		return 0;

	float bin = (x_sample-x0) / dx;
	int lowerbin = floor(bin);
	float fraction = bin-lowerbin;
	int higherbin = lowerbin + 1;
	if (lowerbin == y.size()-1)
		return y[lowerbin];
	else
		return y[lowerbin]*(1-fraction) +  y[higherbin]*(fraction);*/

	return 0.0f;
}


void XYLine::draw(Graphics &g, float xmin, float xmax, float ymin, float ymax, int plotWidth, int plotHeight, bool showBounds)
{
	if (ymax-ymin < 1e-6 || xmax-xmin < 1e-6)
		return;

	g.setColour(colour);

	// function is given in [x,y], where dx is fixed and known.
	// use bilinear interpolation.
	float xrange = xmax-xmin;
	int screenQuantization ;
	if (xrange  < 100 * 1e-3)  // if we are looking at a region that is smaller than 50 ms, try to get better visualization...
	{
		// usually, at this resolution, you see high frequency components. 
		// if you subsample, you will no display things correctly...
		screenQuantization = 1; 
	} else 
	{
		screenQuantization = 5; // draw every 5 pixels
	}
	int numPointsToDraw = plotWidth / screenQuantization;

	float scalex = plotWidth / (numPointsToDraw-1);

	// bilinear interpolation
	bool prevInrange;
	float prevY = interpolate(xmin,prevInrange);
	float prevYscaled = (prevY - ymin) / (ymax-ymin) * plotHeight;

	std::vector<int> screenx, bins; // used to draw upper & lower limits of the signal..
	std::vector<float> min_in_bin, max_in_bin; // used to draw upper & lower limits of the signal..
	if (prevInrange) {
		screenx.push_back(0);
		//bins.push_back( (xmin-x0)/dx);
	}
	bool inrange;


	for (int i=1;i<numPointsToDraw;i++)
	{
		float x_sample = xmin + i * xrange/ (numPointsToDraw-1);
		float currY = interpolate(x_sample, inrange);
		float yScaled = (currY - ymin) / (ymax-ymin) * plotHeight;
		if (inrange && prevInrange) 
		{
			g.drawLine((i-1)*scalex, plotHeight-prevYscaled, (i)*scalex, plotHeight-yScaled);
			screenx.push_back(i*scalex);
			//bins.push_back( (x_sample-x0)/dx);
		}
		prevYscaled = yScaled;
		prevInrange = inrange;
	}

	if (showBounds && screenQuantization > 1)
	{
		// now, we have screen coordinates and corresponding bins.
		// compute minimum and maximum in between each bins...
		for (int i=0;i<screenx.size()-1;i++)
		{
			double minV = 1e10;
			double maxV = -1e10;
			for (int k=bins[i];k<=bins[i+1];k++)
			{
				minV = MIN(y[k],minV);
				maxV = MAX(y[k],maxV);
			}
			min_in_bin.push_back(minV);
			max_in_bin.push_back(maxV);
		}

		// now plot upper & lower bounds.
		float dash_length[2] = {3,3};
		for (int i=0;i<max_in_bin.size()-1;i++)
		{
			double upperBoundLeft = (max_in_bin[i]-ymin)/(ymax-ymin)*plotHeight;
			double upperBoundRight = (max_in_bin[i+1]-ymin)/(ymax-ymin)*plotHeight;

			double lowerBoundLeft = (min_in_bin[i]-ymin)/(ymax-ymin)*plotHeight;
			double lowerBoundRight = (min_in_bin[i+1]-ymin)/(ymax-ymin)*plotHeight;
			juce::Line<float> upperboundLine(screenx[i], plotHeight-upperBoundLeft, screenx[i+1], plotHeight-upperBoundRight);
			juce::Line<float> lowerboundLine(screenx[i], plotHeight-lowerBoundLeft, screenx[i+1], plotHeight-lowerBoundRight);
			g.drawDashedLine(upperboundLine,dash_length,2,1);
			g.drawDashedLine(lowerboundLine,dash_length,2,1);

		}
	}

}


DrawComponent::DrawComponent(InteractivePlot *plt_) : 
	plt(plt_),
	zooming(false),
	panning(false),
	autoRescale(true),
	showXAxis(true),
	showYAxis(true),
	showBounds(false),
	lowestValue(1e10),
	highestValue(-1e10),
	xUnits(""),
	yUnits("")
{

	font = Font("Default", 12, Font::plain);
	setMode(ZOOM); // default mode
}


void DrawComponent::setMode(InteractivePlotMode m)
{
	mode = m;
};

void DrawComponent::setBoundsShown(bool state)
{
	showBounds = state;
}

void DrawComponent::showGrid(bool state)
{
	// toggle grid on and off
}

void DrawComponent::drawTicks(Graphics &g)
{
	int tickHeight = 6;
	float tickThickness = 2;
	double rangeX = (range.xmax - range.xmin);
	double rangeY = (range.ymax - range.ymin);

	if (std::abs(rangeX) < 1e-6 || std::abs(rangeY) < 1e-6)
		return;

	int plotWidth = getWidth();
	int plotHeight = getHeight();

	g.setColour(juce::Colours::white);
	for (int k=0;k < xtick.size();k++)
	{
		// convert to screen coordinates.
		float tickloc = (xtick[k]- range.xmin) / rangeX * plotWidth;
		if (std::abs(tickloc) < 1e10)
			g.drawLine(tickloc,plotHeight,tickloc,plotHeight-(tickHeight),tickThickness);
	}
	for (int k=0;k < ytick.size();k++)
	{
		// convert to screen coordinates.
		float tickloc = (ytick[k]- range.ymin) / rangeY * plotHeight;
		if (std::abs(tickloc) < 1e10)
			g.drawLine(0,plotHeight-tickloc,tickHeight,plotHeight-tickloc, tickThickness);
	}
}


void DrawComponent::add(XYLine* line)
{
	lines.add(line);
	repaint();
}
	
void DrawComponent::clear()
{
	lines.clear();
	repaint();
}


void DrawComponent::paint(Graphics &g)
{
	g.fillAll(juce::Colours::darkkhaki);

	int w = getWidth();
	int h = getHeight();

	for (auto line : lines)
	{
		if (std::abs(range.ymin) < 1e10 & std::abs(range.ymax) < 1e10)
			line->draw(g, range.xmin, range.xmax, range.ymin, range.ymax, w, h, showBounds);
	}
	
	if (lines.size() > 0)
	{
		// draw the horizontal zero line
		if (showXAxis)
		{
			// draw X axis
		}
		if (showYAxis) 
		{
			// draw Y axis
		}
	}

	if (zooming)
	{
		g.setColour(juce::Colours::white);
		int width = abs(mouseDownX-mouseDragX);
		int height= abs(mouseDownY-mouseDragY);
		if (width > 0 & height > 0)
			g.drawRect(MIN(mouseDownX,mouseDragX),MIN(mouseDownY,mouseDragY),width,height,2);
	}

	drawTicks(g);

	g.setFont(font);
	g.setColour(juce::Colours::white);
	
	g.drawText(xUnits + ", " + yUnits, 6, 2, 50, 20, juce::Justification::left, false);

}


void DrawComponent::setRange(XYRange range_)
{
	 range.xmin = range_.xmin;
	 range.xmin = range_.xmax;
	 range.xmin = range_.ymin;
	 range.xmin = range_.ymax;
}


void DrawComponent::setAutoRescale(bool state)
{
	autoRescale = state;
}

void DrawComponent::setTickMarks(std::vector<float> xtick_, std::vector<float> ytick_)
{
	xtick = xtick_;
	ytick = ytick_;
}

void DrawComponent::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel)
{
	if (mode == PAN)
	{
		double xRange = (range.xmax - range.xmin);
		float sn = (wheel.deltaY > 0 )? -1 : 1;
		range.xmin -= 0.1 * sn *xRange;
		range.xmax += 0.1 * sn * xRange;

	} else
	{
		double yRange = (range.ymax - range.ymin);
		float sn = (wheel.deltaY > 0 )? -1 : 1;
		range.ymin -= 0.1 * sn *yRange;
		range.ymax += 0.1 * sn * yRange;
	}

	plt->setRange(range);
	
}

void DrawComponent::mouseUp(const juce::MouseEvent& event)
{
	int plotWidth = getWidth();
	int plotHeight = getHeight();

	panning = false;
	
	if (zooming)
	{
		zooming = false;

		double rangeX = range.xmax - range.xmin;
		double rangeY = range.ymax - range.ymin;
		zooming = false;
		// first, turn off auto rescale, if it is enabled.
		setAutoRescale(false); // zoom is now enabled. We can't have auto rescale and zoom at the same time.

		float downX,downY, upX, upY ;

		downX = float(mouseDownX) /(float)plotWidth * rangeX + range.xmin;
		upX = float(event.x) /(float)plotWidth * rangeX + range.xmin;

		downY = float(plotHeight-(mouseDownY)) /(float)plotHeight * rangeY + range.ymin;
		upY = float(plotHeight-(event.y)) /(float)plotHeight * rangeY + range.ymin;

		
		// convert mouse down and up position to proper x,y range
		// save current zoom 
		if ( ( fabs(downX-upX) < 0.001) ||  ( fabs(downY-upY) < 0.001) )
		{
			// do not zoom more. probably just incorrect click

			return;
		}
		XYRange currentZoom;
		currentZoom.xmin = range.xmin;
		currentZoom.ymin = range.ymin;
		currentZoom.xmax = range.xmax;
		currentZoom.ymax = range.ymax;
		rangeMemory.push_back(currentZoom);

		range.xmin = MIN(downX, upX);
		range.xmax = MAX(downX, upX);
		range.ymin = MIN(downY, upY);
		range.ymax = MAX(downY, upY);

		plt->setRange(range);
		
	}
}

void DrawComponent::mouseDrag(const juce::MouseEvent& event)
{
	mouseDragX = event.x;
	mouseDragY = event.y;
		int w = getWidth();
		int h = getHeight();

	if (panning)
	{

		float range0 = range.xmax - range.xmin;
		float range1 = range.ymax - range.ymin;

		float dx = -float(event.x-mousePrevX) / w*range0;
		float dy = float(event.y-mousePrevY) / h*range1;

		XYRange limit;
		
		plt->getRangeLimit(limit);
		
		if (range.xmin + dx >= limit.xmin && range.xmax + dx <= limit.xmax)
		{
			range.xmin += dx;
			range.xmax += dx;
			range.ymax += dy;
			range.ymin += dy;

			plt->setRange(range);
		}
		mousePrevX = event.x;
		mousePrevY = event.y;
	}

}


void DrawComponent::mouseDown(const juce::MouseEvent& event)
{
		mousePrevX = event.x;
		mousePrevY = event.y;


	if (event.mods.isRightButtonDown())
	{
		if (rangeMemory.size() > 0)
		{
			XYRange prevZoom = rangeMemory.back();
			rangeMemory.pop_back();
			range.xmin = prevZoom.xmin;
			range.xmax = prevZoom.xmax;
			range.ymin = prevZoom.ymin;
			range.ymax = prevZoom.ymax;
			plt->setRange(range);
			
		}

	} else if (event.mods.isLeftButtonDown())
	{

		mouseDownX = event.x;
		mouseDownY = event.y;
		mouseDragX = event.x;
		mouseDragY = event.y;

		if (lines.size() > 0)
		{
			if (mode == ZOOM)
				zooming = true;
			else if (mode == PAN)
				panning = true;
		}
	}
}


void DrawComponent::setXAxisShown(bool state)
{
	showXAxis = state;

	repaint();
	
}

void DrawComponent::setYAxisShown(bool state)
{
	showYAxis = state;
	
}

void DrawComponent::setUnits(String xUnits_, String yUnits_)
{
	xUnits = xUnits_;
	yUnits = yUnits_;
}


void DrawComponent::getRange(XYRange& range_)
{
	range_.xmin = range.xmin;
	range_.xmax = range.xmax;
	range_.ymin = range.ymin;
	range_.ymax = range.ymax;
}
