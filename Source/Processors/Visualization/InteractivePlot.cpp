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

#include <math.h>

InteractivePlot::InteractivePlot() :
	backgroundColour(Colours::darkgrey),
	axisColour(Colours::white),
	gridColour(Colours::grey),
	controlButtonsVisible(false),
	gridIsVisible(true)
{
	xAxis = std::make_unique<XAxis>();
	xAxis->setAxisColour(axisColour);
	addAndMakeVisible(xAxis.get());

	yAxis = std::make_unique<YAxis>();
	yAxis->setAxisColour(axisColour);
	addAndMakeVisible(yAxis.get());

	drawComponent = std::make_unique<DrawComponent>(xAxis.get(), yAxis.get());
	drawComponent->setBackgroundColour(backgroundColour);
	drawComponent->setGridColour(gridColour);
	addAndMakeVisible(drawComponent.get());

	titleLabel = std::make_unique<Label>("Title Label", "Title");
	titleLabel->setFont(Font("Fira Code", "Bold", 15.0f));
	titleLabel->setColour(Label::textColourId, axisColour);
	titleLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(titleLabel.get());

	xLabel = std::make_unique<Label>("X-Axis Label", "X Label");
	xLabel->setFont(Font("Fira Code", "Regular", 15.0f));
	xLabel->setColour(Label::textColourId, axisColour);
	xLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(xLabel.get());

	yLabel = std::make_unique<Label>("Y-Axis Label", "Y Label");
	yLabel->setFont(Font("Fira Code", "Regular", 15.0f));
	yLabel->setColour(Label::textColourId, axisColour);
	yLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(yLabel.get());

	autoRescaleButton = std::make_unique<UtilityButton>("Rescale",Font("Default", 15, Font::plain));
	autoRescaleButton ->addListener(this);
	addAndMakeVisible(autoRescaleButton.get());

}

void InteractivePlot::setInteractive(InteractivePlotMode mode)
{
	if (mode == InteractivePlotMode::ON)
	{
		controlButtonsVisible = true;
		autoRescaleButton->setVisible(true);
		drawComponent->setMode(mode);
	} 
	else if (mode == InteractivePlotMode::OFF)
	{
		controlButtonsVisible = false;
		autoRescaleButton->setVisible(false);
		drawComponent->setMode(mode);
	}
}

void InteractivePlot::setBackgroundColour(Colour c)
{
	drawComponent->setBackgroundColour(c);
}

void InteractivePlot::setGridColour(Colour c)
{
	drawComponent->setGridColour(c);
}

void InteractivePlot::setAxisColour(Colour c)
{
	xAxis->setAxisColour(c);
	yAxis->setAxisColour(c);
}

void InteractivePlot::showGrid(bool state)
{
	drawComponent->showGrid(state);
	gridIsVisible = state;
}

void InteractivePlot::showXAxis(bool state)
{
	xAxis->setVisible(state);
}

void InteractivePlot::showYAxis(bool state)
{
	yAxis->setVisible(state);
}


void InteractivePlot::buttonClicked(Button *btn)
{
	bool prevState = btn->getToggleState();
	
	if  (btn == autoRescaleButton.get())
	{
		drawComponent->rescale();
	}
}

void InteractivePlot::resized()
{
	
	int w = getWidth();
	int h = getHeight();
	
	if (h == 0 || w == 0)
		return;

	int heightOffset = 20;
	int labelWidth = yLabel->getFont().getStringWidth(yLabel->getText());
	int axesWidth =  labelWidth + 30;
	int axesHeight = 50;
	int padding = 10;

	titleLabel->setBounds(axesWidth + padding + 2, 0, w - axesWidth - padding*2, 20);
	xLabel->setBounds(axesWidth + padding + 2, getHeight() - 20, w - axesWidth - padding*2, 20);
	yLabel->setBounds(0, 0, labelWidth, h);
	
	if (MIN(w,h) > 250)
	{
		yAxis->setFontHeight(12);
		xAxis->setFontHeight(12);
	} else
	{
		yAxis->setFontHeight(8);
		xAxis->setFontHeight(8);
	}
	
	yAxis->setBounds(10, heightOffset, axesWidth - 4, h - axesHeight - heightOffset);
	xAxis->setBounds(axesWidth + 1, h - axesHeight - 4, w - axesWidth, axesHeight - 4);

	drawComponent->setBounds(axesWidth + padding + 2, heightOffset + padding, w - axesWidth - 2 - padding * 2, h - axesHeight - heightOffset - padding * 2);

	autoRescaleButton->setBounds(axesWidth + padding + 5, drawComponent->getBottom() - 30, 65, 25);

}

void InteractivePlot::plot(std::vector<float> x,
	std::vector<float> y,
	Colour c,
	float width,
	float opacity,
	PlotType type)
{

	XYLine* line = new XYLine(x, y);
	line->setColour(c);
	line->setWidth(width);
	line->setOpacity(opacity);
	line->setType(type);

	drawComponent->add(line);
}

void InteractivePlot::getRange(XYRange& range_)
{
	drawComponent->getRange(range_);
}

void InteractivePlot::setRange(XYRange& newRange)
{

	//LOGD("Requested range: ", newRange.xmin, " ", newRange.xmax, " ", newRange.ymin, " ", newRange.ymax);
	drawComponent->setRange(newRange);
}

void InteractivePlot::title(String t)
{
	titleLabel->setText(t, dontSendNotification);
}

void InteractivePlot::xlabel(String label)
{
	xLabel->setText(label, dontSendNotification);
}

void InteractivePlot::ylabel(String label)
{
	yLabel->setText(label, dontSendNotification);
}

void InteractivePlot::show()
{
	repaint();

	drawComponent->repaint();
}


void InteractivePlot::clear()
{
	drawComponent->clear();
}


Axis::Axis() 
	: axisIsInverted(false),
	  min(-1e4),
	  max(1e4),
      colour(Colours::white)
{

	font = Font("Default", 15, Font::plain);

}


void Axis::setInverted(bool state)
{
	axisIsInverted = state;
}

void Axis::getTicks(std::vector<float> &tickLocations_, std::vector<float>& tickValues_)
{
	tickLocations_ = ticks;
	tickValues_ = tickValues;
}


std::vector<float> Axis::linspace(float minv, float maxv, int numticks)
{

	float range = maxv - minv;
	float dx = range / (numticks - 1);

	std::vector<float> locations;
	locations.clear();
	locations.push_back(minv);

	for (int k = 1; k < numticks - 1; k++)
	{
		float tick = minv + dx * k;
		locations.push_back(tick);
	}
	locations.push_back(maxv);

	return locations;
}


void Axis::determineTickLocations(float min, float max, int numTicks)
{

	ticks.clear();
	tickValues.clear();

	float range = max - min;
	bool useScientificNotation = false;
	int numDecimalPlaces = 1;
	float stepSize;

	float log_range = log10(range);

	if (range >= 1)
	{
		if (fmod(log_range, 1) >= 0 && fmod(log_range, 1) < 0.3979400086720375)
			stepSize = round(pow(10, floor(log_range) - 1 + 0.3979400086720375));

		else if (fmod(log_range, 1) >= 0.3979400086720375 && fmod(log_range, 1) < 0.6989700043360187)
			stepSize = round(pow(10, floor(log_range) - 1 + 0.6989700043360187));

		else
			stepSize = round(pow(10, floor(log_range)));

	}
	else {
		stepSize = 1;
	}

	float start = round(min / stepSize) * stepSize;
	float tick = start;

	while (tick < max)
	{
		ticks.push_back((tick - min) / (max- min));
		tickValues.push_back(tick);
		tick += stepSize;
	}
	
	tickLabels.resize(ticks.size());

	for (int k = 0; k < ticks.size(); k++)
	{
		
		String tickString = String(int(tickValues[k])); // , numDecimalPlaces, useScientificNotation);
		tickLabels[k] = tickString;
	}

	repaint();
}

void Axis::setRange(float minvalue, float maxvalue, int numTicks)
{
	min = minvalue;
	max = maxvalue;

	determineTickLocations(minvalue, maxvalue, numTicks);

}

void Axis::setTicks(std::vector<float> ticks_, std::vector<String> tickLabels_)
{
	ticks = ticks_;
	tickLabels = tickLabels_;
}

void Axis::setFontHeight(int height)
{
	font.setHeight(height);
}

void Axis::setAxisColour(Colour c)
{
	colour = c;
}

void XAxis::paint(Graphics &g)
{

	g.setFont(font);
	g.setColour(colour);
	
	int ticklabelWidth = 60;
	int tickLabelHeight = 20;

	int padding = 10;

	int w = getWidth();

	g.drawLine(padding, 3, w-padding, 3, 2.0);

	//std::cout << w - padding * 2 << std::endl;
	
	for (int k = 0; k < ticks.size(); k++)
	{
		
		float xtickloc = padding + (ticks[k]) * (w - padding * 2); 

		//std::cout << xtickloc << std::endl;

		if (axisIsInverted)
			xtickloc = w - xtickloc;

		g.drawLine(xtickloc, 3, xtickloc, 13, 2.0);
		
		xtickloc -= ticklabelWidth / 2;

		if (k == 0)
			xtickloc += 5;

		g.drawText(tickLabels[k], 
				xtickloc,
				   10,
				   ticklabelWidth,
				   tickLabelHeight,
				   juce::Justification::centred,
				   true);
	}

}

void YAxis::paint(Graphics& g)
{

	g.setFont(font);
	g.setColour(colour);

	int w = getWidth();
	int h = getHeight();

	int padding = 10;

	g.drawLine(w-3, padding, w-3, h-padding, 2.0);

	int ticklabelWidth = 60;
	int tickLabelHeight = 20;


	for (int k = 0; k < ticks.size(); k++)
	{
		
		float ytickloc = padding + ((ticks[k]) * (h - padding*2));

		if (!axisIsInverted)
			ytickloc = h - ytickloc;

		g.drawLine(w -13, ytickloc, w-3, ytickloc, 2.0);
		
		g.drawText(tickLabels[k], 
					0, ytickloc - tickLabelHeight / 2, 
					w - 15, 
					tickLabelHeight, 
					Justification::right, 
					false);

	}
}


XYLine::XYLine(std::vector<float> x_, std::vector<float> y_) : 
	x(x_), y(y_), 
	colour(Colours::white), 
	width(1.0f), 
	opacity(1.0f), 
	type(PlotType::LINE)
{

	range.xmin = 1e10;
	range.xmax = -1e10;
	range.ymin = 1e10;
	range.ymax = -1e10;

	for (int i = 0; i < x.size(); i++)
	{
		range.xmin = MIN(x[i], range.xmin);
		range.xmax = MAX(x[i], range.xmax);
	}

	for (int i = 0; i < y.size(); i++)
	{
		range.ymin = MIN(y[i], range.ymin);
		range.ymax = MAX(y[i], range.ymax);
	}
	
	//range.print();
}

void XYLine::setColour(Colour c)
{
	colour = c;
}

void XYLine::setWidth(float width_)
{
	width = width_;
}


void XYLine::setType(PlotType type_)
{
	type = type_;
}

void XYLine::setOpacity(float opacity_)
{
	opacity = opacity_;
}

XYRange XYLine::getBounds()
{
	return range;
}

void XYLine::draw(Graphics &g, XYRange& range, int plotWidth, int plotHeight)
{
	
	float yrange = range.ymax - range.ymin;
	float xrange = range.xmax - range.xmin;

	if (yrange < 1e-6 || xrange < 1e-6)
		return;

	g.setColour(colour.withAlpha(opacity));

	if (type == PlotType::LINE)
	{
		for (int i = 1; i < x.size(); i++)
		{
			float x_start = (x[i - 1] - range.xmin) / xrange;
			float x_end = (x[i] - range.xmin) / xrange;

			if ((x_start < 0 && x_end < 0) ||
				(x_start > 1 && x_end > 1))
				continue;

			if (i >= y.size())
				continue;

			float y_start = (y[i - 1] - range.ymin) / yrange;
			float y_end = (y[i] - range.ymin) / yrange;

			if ((y_start < 0 && y_end < 0) ||
				(y_start > 1 && y_end > 1))
				continue;

			if (true)
			{
				x_start = x_start * plotWidth;
				x_end = x_end * plotWidth;
			}
			//else
			//{
			//	x_start = plotWidth - x_start * plotWidth;
			//	x_end = plotWidth - x_end * plotWidth;
			//}
				

			if (true)
			{
				y_start = plotHeight - y_start * plotHeight;
				y_end = plotHeight - y_end * plotHeight;
			}
				
			//else
			//	y_start = y_start * plotHeight;

			g.drawLine(x_start,
				y_start,
				x_end,
				y_end,
				width);

		}

		return;
	}

	if (type == PlotType::SCATTER)
	{

		for (int i = 0; i < x.size(); i++)
		{
			float x_start = (x[i] - range.xmin) / xrange;

			if ((x_start < 0) ||
				(x_start > 1))
				continue;

			if (i >= y.size())
				continue;

			float y_start = (y[i] - range.ymin) / yrange;

			if ((y_start < 0) ||
				(y_start > 1))
				continue;

			if (true)
				x_start = x_start * plotWidth;
			//else
			//	x_start = plotWidth - x_start * plotWidth;

			if (true)
				y_start = plotHeight - y_start * plotHeight;
			//else
			//	y_start = y_start * plotHeight;

			g.fillEllipse(x_start,
				y_start,
				width,
				width);

		}

		return;
	}

	if (type == PlotType::FILLED)
	{
		Path path;

		float x_start = (x[0] - range.xmin) / xrange;
		float y_start = (0 - range.ymin) / yrange;

		if (true)
			x_start = x_start * plotWidth;
		//else
		//	x_start = plotWidth - x_start * plotWidth;

		if (true)
			y_start = plotHeight - y_start * plotHeight;
		//else
		//	y_start = y_start * plotHeight;

		path.startNewSubPath(x_start, y_start);

		for (int i = 0; i < x.size(); i++)
		{
			x_start = (x[i] - range.xmin) / xrange;

			if (i >= y.size())
				continue;

			y_start = (y[i] - range.ymin) / yrange;

			if (true)
				x_start = x_start * plotWidth;
			//else
			//	x_start = plotWidth - x_start * plotWidth;

			if (true)
				y_start = plotHeight - y_start * plotHeight;
			//else
			//	y_start = y_start * plotHeight;

			path.lineTo(x_start, y_start);

		}

		y_start = (0 - range.ymin) / yrange;

		if (true)
			y_start = plotHeight - y_start * plotHeight;
		//else
		//	y_start = y_start * plotHeight;

		path.lineTo(x_start, y_start);
		path.closeSubPath();

		g.fillPath(path);
		
		return;

	}

	if (type == PlotType::BAR)
	{
		float barWidth = width / xrange * plotWidth;
		float y0 = (0 - range.ymin) / yrange;

		if (true)
			y0 = plotHeight - y0 * plotHeight;
		//else
		//	y0 = y0 * plotHeight;

		for (int i = 0; i < x.size(); i++)
		{
			float x_start = (x[i] - range.xmin) / xrange;

			if (i >= y.size())
				continue;

			float y_start = (y[i] - range.ymin) / yrange;

			float barHeight = y[i] / yrange * plotHeight;

			if (true)
				x_start = x_start * plotWidth;
			//else
			//	x_start = plotWidth - x_start * plotWidth;

			if (true)
				y_start = plotHeight - y_start * plotHeight;
			//else
			//	y_start = y_start * plotHeight;

			if (y[i] > 0)
			{
				g.fillRect(x_start - barWidth / 2,
					y_start,
					barWidth,
					barHeight);
			}
			else {
				g.fillRect(x_start - barWidth / 2,
					y0,
					barWidth,
					-barHeight);
			}

		}

		return;
	}

}


DrawComponent::DrawComponent(Axis* x, Axis* y) :
	xAxis(x),
	yAxis(y),
	gridIsVisible(true),
	backgroundColour(Colours::darkgrey),
	gridColour(Colours::grey),
	mode(InteractivePlotMode::ON),
	firstLine(true)
{

}


void DrawComponent::setMode(InteractivePlotMode m)
{
	mode = m;
};


void DrawComponent::showGrid(bool state)
{
	gridIsVisible = state;
}

void DrawComponent::setBackgroundColour(Colour c)
{
	backgroundColour = c;
}

void DrawComponent::setGridColour(Colour c)
{
	gridColour = c;
}


void DrawComponent::add(XYLine* line)
{
	lines.add(line);
	

	if (firstLine)
	{
		limit = line->getBounds();
		firstLine = false;
	}
	else {
		XYRange bounds = line->getBounds();
		limit.xmin = MIN(limit.xmin, bounds.xmin);
		limit.xmax = MAX(limit.xmax, bounds.xmax);
		limit.ymin = MIN(limit.ymin, bounds.ymin);
		limit.ymax = MAX(limit.ymax, bounds.ymax);
	}

	//limit.print();

	repaint();
}
	
void DrawComponent::clear()
{
	lines.clear();
	firstLine = true;
	repaint();
}

void DrawComponent::drawGrid(Graphics& g)
{

	g.setColour(gridColour);

	xAxis->getTicks(xticks, xtickvalues);
	yAxis->getTicks(yticks, ytickvalues);

	float w = getWidth();
	float h = getHeight();

	for (int k = 0; k < xticks.size(); k++)
	{

		float tickloc = xticks[k] * w;

		if (xtickvalues[k] == 0)
			g.drawLine(tickloc, 0, tickloc, h, 2.0);
		else
			g.drawLine(tickloc, 0, tickloc, h);
	}

	for (int k = 0; k < yticks.size(); k++)
	{
		float tickloc = h - yticks[k] * h;

		if (ytickvalues[k] == 0)
			g.drawLine(0, tickloc, w, tickloc, 2.0);
		else
			g.drawLine(0, tickloc, w, tickloc);
	}

}

void DrawComponent::paint(Graphics &g)
{
	g.fillAll(backgroundColour);

	if (gridIsVisible)
		drawGrid(g);

	for (auto line : lines)
	{
		line->draw(g, range, getWidth(), getHeight());
	}
	
}


void DrawComponent::setRange(XYRange& range_)
{
	 range.xmin = range_.xmin;
	 range.xmax = range_.xmax;
	 range.ymin = range_.ymin;
	 range.ymax = range_.ymax;

	 yAxis->setRange(range.ymin, range.ymax, 10);
	 xAxis->setRange(range.xmin, range.xmax, 10);

	 repaint();
}


void DrawComponent::setLimit(XYRange& limit_)
{
	limit.xmin = limit_.xmin;
	limit.xmin = limit_.xmax;
	limit.ymin = limit_.ymin;
	limit.ymax = limit_.ymax;
}


void DrawComponent::rescale()
{
	if (!firstLine)
		setRange(limit);
}

void DrawComponent::setTickMarks(std::vector<float> xtick_, std::vector<float> ytick_)
{
	xticks = xtick_;
	yticks = ytick_;
}

void DrawComponent::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel)
{

	if (mode == InteractivePlotMode::OFF)
		return;

	float scale = 1.0 + (0.2 * wheel.deltaY);

	float xrange = range.xmax - range.xmin;
	float yrange = range.ymax - range.ymin;
	float xcenter = (range.xmax + range.xmin) / 2;
	float ycenter = (range.ymax + range.ymin) / 2;

	XYRange newRange{ xcenter - (xrange * scale / 2),
					  xcenter + (xrange * scale / 2),
					  ycenter - (yrange * scale / 2),
					  ycenter + (yrange * scale / 2) };

	setRange(newRange);
	
}

void DrawComponent::mouseDrag(const juce::MouseEvent& event)
{

	if (mode == InteractivePlotMode::OFF)
		return;

	int w = getWidth();
	int h = getHeight();

	float xrange = range.xmax - range.xmin;
	float yrange = range.ymax - range.ymin;

	float dx = float(event.getDistanceFromDragStartX()) / w * xrange;
	float dy = float(event.getDistanceFromDragStartY()) / h * yrange;

	XYRange newRange{ originalRange.xmin - dx,
					  originalRange.xmax - dx,
					  originalRange.ymin + dy,
					  originalRange.ymax + dy };

	setRange(newRange);

}


void DrawComponent::mouseDown(const juce::MouseEvent& event)
{

	originalRange = range;

}

void DrawComponent::getRange(XYRange& range_)
{
	range_.xmin = range.xmin;
	range_.xmax = range.xmax;
	range_.ymin = range.ymin;
	range_.ymax = range.ymax;
}
