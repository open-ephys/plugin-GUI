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

#include "LfpDisplayCanvas.h"

#include <math.h>

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_) :
	screenBufferIndex(0), timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f),
	processor(processor_),
	displayBufferIndex(0)
{

	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

	displayBuffer = processor->getDisplayBufferAddress();
	displayBufferSize = displayBuffer->getNumSamples();
	std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;

	screenBuffer = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
	screenBuffer->clear();

	screenBufferMin = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
	screenBufferMin->clear();
	screenBufferMean = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
	screenBufferMean->clear();
	screenBufferMax = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
	screenBufferMax->clear();

	viewport = new Viewport();
	lfpDisplay = new LfpDisplay(this, viewport);
	timescale = new LfpTimescale(this);

	timescale->setTimebase(timebase);

	viewport->setViewedComponent(lfpDisplay, false);
	viewport->setScrollBarsShown(true, false);

	scrollBarThickness = viewport->getScrollBarThickness();

	isChannelEnabled.insertMultiple(0,true,10000); // max 10k channels

	//viewport->getVerticalScrollBar()->addListener(this->scrollBarMoved(viewport->getVerticalScrollBar(), 1.0));



	addAndMakeVisible(viewport);
	addAndMakeVisible(timescale);

	voltageRanges.add("-"); // placeholder for custom ranges (set by scroll wheel etc.)
	voltageRanges.add("50");
	voltageRanges.add("100");
	voltageRanges.add("250");
	voltageRanges.add("500");
	voltageRanges.add("1000");
	voltageRanges.add("2000");
	voltageRanges.add("5000");

	timebases.add("1.0");
	timebases.add("2.0");
	timebases.add("5.0");
	timebases.add("10.0");

	spreads.add("-"); // placeholder for custom ranges (set by scroll wheel etc.)
	spreads.add("10");
	spreads.add("20");
	spreads.add("30");
	spreads.add("40");
	spreads.add("50");
	spreads.add("60");

	colorGroupings.add("1");
	colorGroupings.add("2");
	colorGroupings.add("4");
	colorGroupings.add("8");
	colorGroupings.add("16");


	rangeSelection = new ComboBox("Voltage range");
	rangeSelection->addItemList(voltageRanges, 1);
	rangeSelection->setSelectedId(5, sendNotification);
	rangeSelection->addListener(this);
	rangeSelection->setItemEnabled(1,false); //  '-' option not enabled- use this for manually selected ranges later
	addAndMakeVisible(rangeSelection);


	timebaseSelection = new ComboBox("Timebase");
	timebaseSelection->addItemList(timebases, 1);
	timebaseSelection->setSelectedId(2, sendNotification);
	timebaseSelection->addListener(this);
	addAndMakeVisible(timebaseSelection);


	spreadSelection = new ComboBox("Spread");
	spreadSelection->addItemList(spreads, 1);
	spreadSelection->setSelectedId(6,sendNotification);
	spreadSelection->addListener(this);
	spreadSelection->setItemEnabled(1,false); //  '-' option not enabled- use this for manually selected ranges later
	addAndMakeVisible(spreadSelection);

	colorGroupingSelection = new ComboBox("Color Grouping");
	colorGroupingSelection->addItemList(colorGroupings, 1);
	colorGroupingSelection->setSelectedId(1,sendNotification);
	colorGroupingSelection->addListener(this);
	addAndMakeVisible(colorGroupingSelection);

	invertInputButton = new UtilityButton("Invert", Font("Small Text", 13, Font::plain));
	invertInputButton->setRadius(5.0f);
	invertInputButton->setEnabledState(true);
	invertInputButton->setCorners(true, true, true, true);
	invertInputButton->addListener(this);
	invertInputButton->setClickingTogglesState(true);
	invertInputButton->setToggleState(false, sendNotification);
	addAndMakeVisible(invertInputButton);

	//button for controlling drawing algorithm - old line-style or new per-pixel style
	drawMethodButton = new UtilityButton("DrawMethod", Font("Small Text", 13, Font::plain));
	drawMethodButton->setRadius(5.0f);
	drawMethodButton->setEnabledState(true);
	drawMethodButton->setCorners(true, true, true, true);
	drawMethodButton->addListener(this);
	drawMethodButton->setClickingTogglesState(true);
	drawMethodButton->setToggleState(false, sendNotification);
	addAndMakeVisible(drawMethodButton);

	//button for pausing the diaplsy - works by skipping buffer updates. This way scrolling etc still works
	pauseButton = new UtilityButton("Pause", Font("Small Text", 13, Font::plain));
	pauseButton->setRadius(5.0f);
	pauseButton->setEnabledState(true);
	pauseButton->setCorners(true, true, true, true);
	pauseButton->addListener(this);
	pauseButton->setClickingTogglesState(true);
	pauseButton->setToggleState(false, sendNotification);
	addAndMakeVisible(pauseButton);


	lfpDisplay->setNumChannels(nChans);
	lfpDisplay->setRange(1000.0f);

	// add event display-specific controls (currently just an enable/disable button)
	for (int i = 0; i < 8; i++)
	{


		EventDisplayInterface* eventOptions = new EventDisplayInterface(lfpDisplay, this, i);
		eventDisplayInterfaces.add(eventOptions);
		addAndMakeVisible(eventOptions);
		eventOptions->setBounds(500+(floor(i/2)*20), getHeight()-20-(i%2)*20, 40, 20);

		lfpDisplay->setEventDisplayState(i,true);

	}

	TopLevelWindow::getTopLevelWindow(0)->addKeyListener(this);
}

LfpDisplayCanvas::~LfpDisplayCanvas()
{

	deleteAndZero(screenBuffer);
	deleteAndZero(screenBufferMin);
	deleteAndZero(screenBufferMean);
	deleteAndZero(screenBufferMax);

	TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

void LfpDisplayCanvas::resized()
{

	timescale->setBounds(leftmargin,0,getWidth()-scrollBarThickness-leftmargin,30);
	viewport->setBounds(0,30,getWidth(),getHeight()-90);

	if (lfpDisplay->getSingleChannelState())
		lfpDisplay->setChannelHeight(viewport->getHeight(),false);

	lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness, lfpDisplay->getChannelHeight()*nChans);
	
	rangeSelection->setBounds(5,getHeight()-30,100,25);
	timebaseSelection->setBounds(175,getHeight()-30,100,25);
	spreadSelection->setBounds(345,getHeight()-30,100,25);
	colorGroupingSelection->setBounds(620,getHeight()-30,100,25);

	invertInputButton->setBounds(750,getHeight()-50,100,22);
	drawMethodButton->setBounds(750,getHeight()-25,100,22);
	pauseButton->setBounds(880,getHeight()-50,50,44);

	for (int i = 0; i < 8; i++)
	{
		eventDisplayInterfaces[i]->setBounds(500+(floor(i/2)*20), getHeight()-40+(i%2)*20, 40, 20); // arrange event channel buttons in two rows
		eventDisplayInterfaces[i]->repaint();
	}


	// std::cout << "Canvas thinks LfpDisplay should be this high: "
	//  << lfpDisplay->getTotalHeight() << std::endl;

}

void LfpDisplayCanvas::beginAnimation()
{
	std::cout << "Beginning animation." << std::endl;

	displayBufferSize = displayBuffer->getNumSamples();

	screenBufferIndex = 0;

	startCallbacks();
}

void LfpDisplayCanvas::endAnimation()
{
	std::cout << "Ending animation." << std::endl;

	stopCallbacks();
}

void LfpDisplayCanvas::update()
{
	nChans = jmax(processor->getNumInputs(),1);
	sampleRate = processor->getSampleRate();

	std::cout << "Setting sample rate of LfpDisplayCanvas to " << sampleRate << std::endl;

	if (nChans != lfpDisplay->getNumChannels())
	{
		std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

		refreshScreenBuffer();

		lfpDisplay->setNumChannels(nChans);

		// update channel names
		for (int i = 0; i < processor->getNumInputs(); i++)
		{

			String chName = processor->channels[i]->getName();

			//std::cout << chName << std::endl;

			lfpDisplay->channelInfo[i]->setName(chName);
			lfpDisplay->enableChannel(isChannelEnabled[i], i);

		}

		lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness*2, lfpDisplay->getTotalHeight());

		resized();
	}

}

void LfpDisplayCanvas::buttonClicked(Button* b)
{
	if (b == invertInputButton)
	{
		lfpDisplay->setInputInverted(b->getToggleState());
	}
	if (b == drawMethodButton)
	{
		lfpDisplay->setDrawMethod(b->getToggleState());
	}
	if (b == pauseButton)
	{
		lfpDisplay->isPaused = b->getToggleState();
	}

}


void LfpDisplayCanvas::comboBoxChanged(ComboBox* cb)
{

	if (cb == timebaseSelection)
	{
		timebase = timebases[cb->getSelectedId()-1].getFloatValue();
	}
	else if (cb == rangeSelection)
	{
		lfpDisplay->setRange(voltageRanges[cb->getSelectedId()-1].getFloatValue());
		//std::cout << "Setting range to " << voltageRanges[cb->getSelectedId()-1].getFloatValue() << std::endl;
	}
	else if (cb == spreadSelection)
	{
		//spread = spreads[cb->getSelectedId()-1].getFloatValue();
		lfpDisplay->setChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
		resized();
		
		//std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
	}
	else if (cb == colorGroupingSelection)
	{
		// set color grouping hre

		lfpDisplay->setColorGrouping(colorGroupings[cb->getSelectedId()-1].getIntValue());// so that channel colors get re-assigned

	}

	timescale->setTimebase(timebase);
}


int LfpDisplayCanvas::getChannelHeight()
{
	return spreads[spreadSelection->getSelectedId()-1].getIntValue();
}


void LfpDisplayCanvas::setParameter(int param, float val)
{
	// if (param == 0)
	// {
	//     timebase = val;
	//     refreshScreenBuffer();
	// }
	// else
	// {
	//     displayGain = val; //* 0.0001f;
	// }

	// repaint();
}

void LfpDisplayCanvas:: setRangeSelection(float range)
{
			//rangeSelection->setItemEnabled(0,true); //keep custom range unavailable for direct selection
			rangeSelection->setSelectedId(1,true);  // but show it for display
			rangeSelection->changeItemText(1,String(int(range))); // and set to correct number

			repaint();
			refresh();
			
}

void LfpDisplayCanvas:: setSpreadSelection(int spread)
{
			spreadSelection->setSelectedId(1,true);
			spreadSelection->changeItemText(1,String(int(spread))); // and set to correct number

			repaint();
			refresh();
			
}

void LfpDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	displayBufferIndex = processor->getDisplayBufferIndex();
	screenBufferIndex = 0;

}

void LfpDisplayCanvas::refreshScreenBuffer()
{

	screenBufferIndex = 0;

	screenBuffer->clear();
	screenBufferMin->clear();
	screenBufferMean->clear();
	screenBufferMax->clear();


	// int w = lfpDisplay->getWidth();
	// //std::cout << "Refreshing buffer size to " << w << "pixels." << std::endl;

	// for (int i = 0; i < w; i++)
	// {
	//  float x = float(i);

	//  for (int n = 0; n < nChans; n++)
	//  {
	//      waves[n][i*2] = x;
	//      waves[n][i*2+1] = 0.5f; // line in center of display
	//  }
	// }

}

void LfpDisplayCanvas::updateScreenBuffer()
{

	// copy new samples from the displayBuffer into the screenBuffer (waves)
	int maxSamples = lfpDisplay->getWidth() - leftmargin;

	if (screenBufferIndex >= maxSamples) // wrap around if we reached right edge before
		screenBufferIndex = 0;

	lastScreenBufferIndex = screenBufferIndex;

	int index = processor->getDisplayBufferIndex();

	int nSamples =  index - displayBufferIndex; // N new samples (not pixels) to be added to displayBufferIndex

	if (nSamples < 0) // buffer has reset to 0
	{
		nSamples = (displayBufferSize - displayBufferIndex) + index;
	}

	float ratio = sampleRate * timebase / float(getWidth() - leftmargin - scrollBarThickness); // samples / pixel
	// this number is crucial: converting from samples to values (in px) for the screen buffer
	int valuesNeeded = (int) float(nSamples) / ratio; // N pixels needed for this update

	if (screenBufferIndex + valuesNeeded > maxSamples)  // crop number of samples to fit cavas width
	{
		valuesNeeded = maxSamples - screenBufferIndex;
	}

	float subSampleOffset = 0.0;

	displayBufferIndex = displayBufferIndex % displayBufferSize; // make sure we're not overshooting
	int nextPos = (displayBufferIndex +1) % displayBufferSize; //  position next to displayBufferIndex in display buffer to copy from

	if (valuesNeeded > 0 && valuesNeeded < 1000)
	{
		for (int i = 0; i < valuesNeeded; i++) // also fill one extra sample for line drawing interpolation to match across draws
		{
			//If paused don't update screen buffers, but update all indexes as needed
			if (!lfpDisplay->isPaused)
			{
				float gain = 1.0;
				float alpha = (float) subSampleOffset;
				float invAlpha = 1.0f - alpha;

				screenBuffer->clear(screenBufferIndex, 1);
				screenBufferMin->clear(screenBufferIndex, 1);
				screenBufferMean->clear(screenBufferIndex, 1);
				screenBufferMax->clear(screenBufferIndex, 1);


				displayBufferIndex = displayBufferIndex % displayBufferSize; // just to be sure

				for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
				{

					// interpolate between two samples with invAlpha and alpha
					screenBuffer->addFrom(channel, // destChannel
						screenBufferIndex, // destStartSample
						displayBuffer->getReadPointer(channel, displayBufferIndex), // source
						1, // numSamples
						invAlpha*gain); // gain


					screenBuffer->addFrom(channel, // destChannel
						screenBufferIndex, // destStartSample
						displayBuffer->getReadPointer(channel, nextPos), // source
						1, // numSamples
						alpha*gain); // gain

					// same thing again, but this time add the min,mean, and max of all samples in current pixel
					float sample_min   =  1000000;
					float sample_max   = -1000000;
					float sample_mean  =  0;
					int c=0;
					int nextpix = (displayBufferIndex +(int)ratio) % displayBufferSize; //  position to next pixels index
					for (int j = displayBufferIndex; j < nextpix; j++)
					{
						float sample_current = displayBuffer->getSample(channel, j);
						sample_mean = sample_mean + sample_current;

						if (sample_min>sample_current)
						{
							sample_min=sample_current;
						}

						if (sample_max<sample_current)
						{
							sample_max=sample_current;
						}
						c++;


					}
					sample_mean=sample_mean/c;
					screenBufferMean->addSample(channel, screenBufferIndex, sample_mean*gain);
					screenBufferMin->addSample(channel, screenBufferIndex, sample_min*gain);
					screenBufferMax->addSample(channel, screenBufferIndex, sample_max*gain);

				}
				screenBufferIndex++;
			}

			subSampleOffset += ratio;

			while (subSampleOffset >= 1.0)
			{
				if (++displayBufferIndex > displayBufferSize)
					displayBufferIndex = 0;

				nextPos = (displayBufferIndex + 1) % displayBufferSize;
				subSampleOffset -= 1.0;
			}

		}


	}
	else
	{
		//std::cout << "Skip." << std::endl;
	}
}

const float LfpDisplayCanvas::getXCoord(int chan, int samp)
{
	return samp;
}

int LfpDisplayCanvas::getNumChannels()
{
	return nChans;
}

const float LfpDisplayCanvas::getYCoord(int chan, int samp)
{
	return *screenBuffer->getReadPointer(chan, samp);
}

const float LfpDisplayCanvas::getYCoordMean(int chan, int samp)
{
	return *screenBufferMean->getReadPointer(chan, samp);
}
const float LfpDisplayCanvas::getYCoordMin(int chan, int samp)
{
	return *screenBufferMin->getReadPointer(chan, samp);
}
const float LfpDisplayCanvas::getYCoordMax(int chan, int samp)
{
	return *screenBufferMax->getReadPointer(chan, samp);
}


bool LfpDisplayCanvas::getInputInvertedState()
{
	return invertInputButton->getToggleState();
}

bool LfpDisplayCanvas::getDrawMethodState()
{
	return drawMethodButton->getToggleState();
}

void LfpDisplayCanvas::paint(Graphics& g)
{

	//std::cout << "Painting" << std::endl;
	g.setColour(Colour(0,18,43)); //background color
	g.fillRect(0, 0, getWidth(), getHeight());

	g.setGradientFill(ColourGradient(Colour(50,50,50),0,0,
		Colour(25,25,25),0,30,
		false));

	g.fillRect(0, 0, getWidth()-scrollBarThickness, 30);

	g.setColour(Colours::black);

	g.drawLine(0,30,getWidth()-scrollBarThickness,30);

	g.setColour(Colour(25,25,60)); // timing grid color

	int w = getWidth()-scrollBarThickness-leftmargin;

	for (int i = 0; i < 10; i++)
	{
		if (i == 5 || i == 0)
			g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,3.0f);
		else
			g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,1.0f);
	}

	g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);

	g.setFont(Font("Default", 16, Font::plain));

	g.setColour(Colour(100,100,100));

	g.drawText("Voltage range (uV)",5,getHeight()-55,300,20,Justification::left, false);
	g.drawText("Timebase (s)",175,getHeight()-55,300,20,Justification::left, false);
	g.drawText("Spread (px)",345,getHeight()-55,300,20,Justification::left, false);
	g.drawText("Color grouping",620,getHeight()-55,300,20,Justification::left, false);


	g.drawText("Event disp.",500,getHeight()-55,300,20,Justification::left, false);



}

void LfpDisplayCanvas::refresh()
{

	updateScreenBuffer(); 

	lfpDisplay->refresh(); // redraws only the new part of the screen buffer

	//getPeer()->performAnyPendingRepaintsNow();

}

bool LfpDisplayCanvas::keyPressed(const KeyPress& key)
{
	if (key.getKeyCode() == key.spaceKey)
	{
		pauseButton->setToggleState(!pauseButton->getToggleState(),true);
		return true;
	}

	return false;
}

bool LfpDisplayCanvas::keyPressed(const KeyPress& key, Component* orig)
{
	if (getTopLevelComponent() == orig && isVisible())
	{
		return keyPressed(key);
	}
	return false;
}

void LfpDisplayCanvas::saveVisualizerParameters(XmlElement* xml)
{

	XmlElement* xmlNode = xml->createNewChildElement("LFPDISPLAY");


	xmlNode->setAttribute("Range",rangeSelection->getSelectedId());
	xmlNode->setAttribute("Timebase",timebaseSelection->getSelectedId());
	xmlNode->setAttribute("Spread",spreadSelection->getSelectedId());
	xmlNode->setAttribute("colorGrouping",colorGroupingSelection->getSelectedId());
	xmlNode->setAttribute("isInverted",invertInputButton->getToggleState());
	xmlNode->setAttribute("drawMethod",drawMethodButton->getToggleState());

	int eventButtonState = 0;

	for (int i = 0; i < 8; i++)
	{
		if (lfpDisplay->eventDisplayEnabled[i])
		{
			eventButtonState += (1 << i);
		}
	}

	xmlNode->setAttribute("EventButtonState", eventButtonState);

	String channelDisplayState = "";

	for (int i = 0; i < nChans; i++)
	{
		if (lfpDisplay->getEnabledState(i))
		{
			channelDisplayState += "1";
		}
		else
		{
			channelDisplayState += "0";
		}
	}

	xmlNode->setAttribute("ChannelDisplayState", channelDisplayState);

	xmlNode->setAttribute("ScrollX",viewport->getViewPositionX());
	xmlNode->setAttribute("ScrollY",viewport->getViewPositionY());
}


void LfpDisplayCanvas::loadVisualizerParameters(XmlElement* xml)
{
	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("LFPDISPLAY"))
		{
			rangeSelection->setSelectedId(xmlNode->getIntAttribute("Range"));
			timebaseSelection->setSelectedId(xmlNode->getIntAttribute("Timebase"));
			spreadSelection->setSelectedId(xmlNode->getIntAttribute("Spread"));
			if (xmlNode->hasAttribute("colorGrouping"))
			{
				colorGroupingSelection->setSelectedId(xmlNode->getIntAttribute("colorGrouping"));
			}
			else
			{
				colorGroupingSelection->setSelectedId(1);
			}

			invertInputButton->setToggleState(xmlNode->getBoolAttribute("isInverted", true), sendNotification);

			drawMethodButton->setToggleState(xmlNode->getBoolAttribute("drawMethod", true), sendNotification);

			viewport->setViewPosition(xmlNode->getIntAttribute("ScrollX"),
				xmlNode->getIntAttribute("ScrollY"));

			int eventButtonState = xmlNode->getIntAttribute("EventButtonState");

			for (int i = 0; i < 8; i++)
			{
				lfpDisplay->eventDisplayEnabled[i] = (eventButtonState >> i) & 1;

				eventDisplayInterfaces[i]->checkEnabledState();
			}

			String channelDisplayState = xmlNode->getStringAttribute("ChannelDisplayState");

			for (int i = 0; i < channelDisplayState.length(); i++)
			{

				if (channelDisplayState.substring(i,i+1).equalsIgnoreCase("1"))
				{
					//std::cout << "LfpDisplayCanvas enabling channel " << i << std::endl;
					lfpDisplay->enableChannel(true, i);
					isChannelEnabled.set(i,true); //lfpDisplay->enableChannel(true, i);
				}
				else
				{
					//std::cout << "LfpDisplayCanvas disabling channel " << i << std::endl;
					lfpDisplay->enableChannel(false, i);
					isChannelEnabled.set(i,false);
				}


			}
		}
	}

}


// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplayCanvas* c) : canvas(c)
{

	font = Font("Default", 16, Font::plain);
}

LfpTimescale::~LfpTimescale()
{

}

void LfpTimescale::paint(Graphics& g)
{



	g.setFont(font);

	g.setColour(Colour(100,100,100));

	g.drawText("ms:",5,0,100,getHeight(),Justification::left, false);

	for (int i = 1; i < 10; i++)
	{
		if (i == 5)
			g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),3.0f);
		else
			g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),1.0f);

		g.drawText(labels[i-1],getWidth()/10*i+3,0,100,getHeight(),Justification::left, false);
	}

}

void LfpTimescale::setTimebase(float t)
{
	timebase = t;

	labels.clear();

	for (float i = 1.0f; i < 10.0; i++)
	{
		String labelString = String(timebase/10.0f*1000.0f*i);

		labels.add(labelString.substring(0,4));
	}

	repaint();

}


// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v) :
	canvas(c), viewport(v), range(1000.0f), singleChan(-1)
{

	totalHeight = 0;
	colorGrouping=1;

	addMouseListener(this, true);

	// hue cycle
	//for (int i = 0; i < 15; i++)
	//{
	//    channelColours.add(Colour(float(sin((3.14/2)*(float(i)/15))),float(1.0),float(1),float(1.0)));
	//}

	//hand-built palette
	channelColours.add(Colour(224,185,36));
	channelColours.add(Colour(214,210,182));
	channelColours.add(Colour(243,119,33));
	channelColours.add(Colour(186,157,168));
	channelColours.add(Colour(237,37,36));
	channelColours.add(Colour(179,122,79));
	channelColours.add(Colour(217,46,171));
	channelColours.add(Colour(217, 139,196));
	channelColours.add(Colour(101,31,255));
	channelColours.add(Colour(141,111,181));
	channelColours.add(Colour(48,117,255));
	channelColours.add(Colour(184,198,224));
	channelColours.add(Colour(116,227,156));
	channelColours.add(Colour(150,158,155));
	channelColours.add(Colour(82,173,0));
	channelColours.add(Colour(125,99,32));

	isPaused=false;

}

LfpDisplay::~LfpDisplay()
{
	deleteAllChildren();
}



int LfpDisplay::getNumChannels()
{
	return numChans;
}



int LfpDisplay::getColorGrouping()
{
	return colorGrouping;
}

void LfpDisplay::setColorGrouping(int i)
{
	colorGrouping=i;
	setColors(); // so that channel colors get re-assigned

}


void LfpDisplay::setNumChannels(int numChannels)
{
	numChans = numChannels;

	deleteAllChildren();

	channels.clear();
	channelInfo.clear();

	totalHeight = 0;

	for (int i = 0; i < numChans; i++)
	{

		//std::cout << "Adding new display for channel " << i << std::endl;

		LfpChannelDisplay* lfpChan = new LfpChannelDisplay(canvas, this, i);

		//lfpChan->setColour(channelColours[i % channelColours.size()]);
		lfpChan->setRange(range);
		lfpChan->setChannelHeight(canvas->getChannelHeight());

		addAndMakeVisible(lfpChan);

		channels.add(lfpChan);

		LfpChannelDisplayInfo* lfpInfo = new LfpChannelDisplayInfo(canvas, this, i);

		//lfpInfo->setColour(channelColours[i % channelColours.size()]);
		lfpInfo->setRange(range);
		lfpInfo->setChannelHeight(canvas->getChannelHeight());

		addAndMakeVisible(lfpInfo);

		channelInfo.add(lfpInfo);

		totalHeight += lfpChan->getChannelHeight();

	}

	setColors();

	//std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;

	// // this doesn't seem to do anything:
	//canvas->fullredraw = true;
	//refresh();

}

void LfpDisplay::setColors()
{
	for (int i = 0; i < numChans; i++)
	{

		channels[i]->setColour(channelColours[(int(i/colorGrouping)+1) % channelColours.size()]);
		channelInfo[i]->setColour(channelColours[(int(i/colorGrouping)+1)  % channelColours.size()]);
	}

}


int LfpDisplay::getTotalHeight()
{
	return totalHeight;
}

void LfpDisplay::resized()
{

	int totalHeight = 0;

	for (int i = 0; i < channels.size(); i++)
	{

		LfpChannelDisplay* disp = channels[i];

		disp->setBounds(canvas->leftmargin,
			totalHeight-disp->getChannelOverlap()/2,
			getWidth(),
			disp->getChannelHeight()+disp->getChannelOverlap());

		LfpChannelDisplayInfo* info = channelInfo[i];

		info->setBounds(0,
			totalHeight-disp->getChannelHeight()/4,
			canvas->leftmargin,
			disp->getChannelHeight());

		totalHeight += disp->getChannelHeight();
		
	}

	canvas->fullredraw = true; //issue full redraw
	if (singleChan != -1)
		viewport->setViewPosition(Point<int>(0,singleChan*getChannelHeight()));

	refresh();

	// std::cout << "Total height: " << totalHeight << std::endl;

}

void LfpDisplay::paint(Graphics& g)
{

}

void LfpDisplay::refresh()
{


	int topBorder = viewport->getViewPositionY();
	int bottomBorder = viewport->getViewHeight() + topBorder;

	// ensure that only visible channels are redrawn
	for (int i = 0; i < numChans; i++)
	{

		int componentTop = channels[i]->getY();
		int componentBottom = channels[i]->getHeight() + componentTop;

		if ((topBorder <= componentBottom && bottomBorder >= componentTop))
		{
			if (canvas->fullredraw)
			{
				channels[i]->fullredraw = true;
				channels[i]->repaint();
				channelInfo[i]->repaint();

			}
			else
			{
				channels[i]->repaint(canvas->lastScreenBufferIndex-2, 0, (canvas->screenBufferIndex-canvas->lastScreenBufferIndex)+3, getChildComponent(i)->getHeight());  //repaint only the updated portion
				// we redraw from -2 to +1 relative to the real redraw window, the -2 makes sure that the lines join nicely, and the +1 draws the vertical update line
			}
			//std::cout << i << std::endl;
		}

	}

	canvas->fullredraw = false;
}

void LfpDisplay::setRange(float r)
{
	range = r;

	for (int i = 0; i < numChans; i++)
	{
		channels[i]->setRange(range);
	}
	canvas->fullredraw = true; //issue full redraw
}

int LfpDisplay::getRange()
{
	return channels[0]->getRange();
}


void LfpDisplay::setChannelHeight(int r, bool resetSingle)
{

	for (int i = 0; i < numChans; i++)
	{
		channels[i]->setChannelHeight(r);
		channelInfo[i]->setChannelHeight(r);
	}
	if (resetSingle && singleChan != -1)
	{
		setSize(getWidth(),numChans*getChannelHeight());
		viewport->setScrollBarsShown(true,false);
		viewport->setViewPosition(Point<int>(0,singleChan*r));
		singleChan = -1;
		for (int n = 0; n < numChans; n++)
		{
			channelInfo[n]->setEnabledState(true);
		}
	}

	resized();

}

void LfpDisplay::setInputInverted(bool isInverted)
{

	for (int i = 0; i < numChans; i++)
	{
		channels[i]->setInputInverted(isInverted);
	}

	resized();

}

void LfpDisplay::setDrawMethod(bool isDrawMethod)
{
	for (int i = 0; i < numChans; i++)
	{
		channels[i]->setDrawMethod(isDrawMethod);
	}
	resized();

}


int LfpDisplay::getChannelHeight()
{
	return channels[0]->getChannelHeight();
}



void LfpDisplay::mouseWheelMove(const MouseEvent&  e, const MouseWheelDetails&   wheel)
{

	//std::cout << "Mouse wheel " <<  e.mods.isCommandDown() << "  " << wheel.deltaY << std::endl;
	
	if (e.mods.isCommandDown())  // CTRL + scroll wheel -> change channel spacing
	{
		int h = getChannelHeight();
		int hdiff=0;
		if (wheel.deltaY>0)
		{
			hdiff=2;
		}
		else
		{
			if (h>5)
				hdiff=-2;
		}

		if (abs(h)>100) // accelerate scrolling for large ranges
			hdiff=hdiff*3;

		setChannelHeight(h+hdiff);
		int oldX=viewport->getViewPositionX();
		int oldY=viewport->getViewPositionY();

		setBounds(0,0,getWidth()-0, getChannelHeight()*canvas->nChans); // update height so that the scrollbar is correct

		int mouseY=e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
		int scrollBy = (mouseY/h)*hdiff*2;// compensate for motion of point under current mouse position
		viewport->setViewPosition(oldX,oldY+scrollBy); // set back to previous position plus offset

		canvas->setSpreadSelection(h+hdiff); // update combobox

	}
	else
	{
		if (e.mods.isShiftDown())  // SHIFT + scroll wheel -> change channel range
		{
			int h= getRange();
			if (wheel.deltaY>0)
			{
				setRange(h+10);
			}
			else
			{
				if (h>11)
					setRange(h-10);
			}
			
			canvas->setRangeSelection(h); // update combobox

		}
		else    // just scroll
		{
			//  passes the event up to the viewport so the screen scrolls
			if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
				viewport->mouseWheelMove(e.getEventRelativeTo(canvas), wheel);

		}
	}

	canvas->fullredraw = true;//issue full redraw
	refresh();

}

void LfpDisplay::toggleSingleChannel(int chan)
{
	std::cout << "Toggle channel " << chan << std::endl;
	if (chan != singleChan)
	{
		singleChan = chan;
		int newHeight = viewport->getHeight();
		setChannelHeight(newHeight,false);
		setSize(getWidth(),numChans*getChannelHeight());
		viewport->setScrollBarsShown(false,false);
		//viewport->setViewPosition(Point<int>(0,chan*newHeight));
		for (int n = 0; n < numChans; n++)
		{
			if ( n != chan) channelInfo[n]->setEnabledState(false);
		}

	}
	else
	{
		setChannelHeight(canvas->getChannelHeight());
	}
}

bool LfpDisplay::getSingleChannelState()
{
	if (singleChan < 0) return false;
	else return true;
}


void LfpDisplay::mouseDown(const MouseEvent& event)
{
	//int y = event.getMouseDownY(); //relative to each channel pos
	MouseEvent canvasevent = event.getEventRelativeTo(viewport);
	int y = canvasevent.getMouseDownY() + viewport->getViewPositionY(); // need to account for scrolling

	int dist=0;
	int mindist=10000;
	int closest=5;
	for (int n = 0; n < numChans; n++) // select closest instead of relying ot eventComponent
	{
		channels[n]->deselect();

		int cpos=(channels[n]->getY() + (channels[n]->getHeight()/2));
		dist=int(abs(y - cpos));

		//std::cout << "Mouse down at " << y << " pos is "<< cpos << "n:" << n << "  dist " << dist << std::endl;

		if (dist<mindist)
		{
			mindist=dist-1;
			closest=n;
		}
	}

	//LfpChannelDisplay* lcd = (LfpChannelDisplay*) event.eventComponent;
	//lcd->select();

	channels[closest]->select();
	if (event.getNumberOfClicks() == 2) toggleSingleChannel(closest);

	canvas->fullredraw = true;//issue full redraw

	refresh();

}


bool LfpDisplay::setEventDisplayState(int ch, bool state)
{
	eventDisplayEnabled[ch] = state;
	return eventDisplayEnabled[ch];
}


bool LfpDisplay::getEventDisplayState(int ch)
{
	return eventDisplayEnabled[ch];
}

void LfpDisplay::enableChannel(bool state, int chan)
{

	if (chan < numChans)
	{
		channelInfo[chan]->setEnabledState(state);
		canvas->isChannelEnabled.set(chan, state);
	}
}

void LfpDisplay::setEnabledState(bool state, int chan)
{

	if (chan < numChans)
	{
		channels[chan]->setEnabledState(state);
		canvas->isChannelEnabled.set(chan, state);
	}
}

bool LfpDisplay::getEnabledState(int chan)
{
	if (chan < numChans)
	{
		return channels[chan]->getEnabledState();
	}

	return false;
}


// ------------------------------------------------------------------

LfpChannelDisplay::LfpChannelDisplay(LfpDisplayCanvas* c, LfpDisplay* d, int channelNumber) :
	canvas(c), display(d), isSelected(false), chan(channelNumber),
	channelOverlap(300), channelHeight(40), range(1000.0f),
	isEnabled(true), inputInverted(false), canBeInverted(true), drawMethod(false)
{


	name = String(channelNumber+1); // default is to make the channelNumber the name


	channelHeightFloat = (float) channelHeight;

	channelFont = Font("Default", channelHeight*0.6, Font::plain);

	lineColour = Colour(255,255,255);

}

LfpChannelDisplay::~LfpChannelDisplay()
{

}

void LfpChannelDisplay::setEnabledState(bool state)
{

	if (state)
		std::cout << "Setting channel " << name << " to true." << std::endl;
	else
		std::cout << "Setting channel " << name << " to false." << std::endl;

	isEnabled = state;

}

void LfpChannelDisplay::paint(Graphics& g)
{

	//g.fillAll(Colours::grey);

	g.setColour(Colours::yellow);   // draw most recent drawn sample position
	g.drawLine(canvas->screenBufferIndex+1, 0, canvas->screenBufferIndex+1, getHeight());




	//g.setColour(Colours::red); // draw oldest drawn sample position
	//g.drawLine(canvas->lastScreenBufferIndex, 0, canvas->lastScreenBufferIndex, getHeight()-channelOverlap);

	if (isEnabled)
	{

		int center = getHeight()/2;

		if (isSelected)
		{

			g.setColour(Colours::lightgrey);
			g.fillRect(0,center-channelHeight/2,10,channelHeight);
			g.drawLine(0,center+channelHeight/2,getWidth(),center+channelHeight/2);
			g.drawLine(0,center-channelHeight/2,getWidth(),center-channelHeight/2);

			g.setColour(Colour(25,25,25));
			g.drawLine(0,center+channelHeight/4,10,center+channelHeight/4);
			g.drawLine(0,center-channelHeight/4,10,center-channelHeight/4);

		}


		g.setColour(Colour(40,40,40));
		g.drawLine(0, getHeight()/2, getWidth(), getHeight()/2);

		int stepSize = 1;
		int from = 0; // for vertical line drawing in the LFP data
		int to = 0;

		//for (int i = 0; i < getWidth()-stepSize; i += stepSize) // redraw entire display
		int ifrom = canvas->lastScreenBufferIndex - 3; // need to start drawing a bit before the actual redraw windowfor the interpolated line to join correctly

		if (ifrom < 0)
			ifrom = 0;

		int ito = canvas->screenBufferIndex - 1;

		if (fullredraw)
		{
			ifrom = 0; //canvas->leftmargin;
			ito = getWidth()-stepSize;
			fullredraw = false;
		}

		for (int i = ifrom; i < ito ; i += stepSize) // redraw only changed portion
		{

			// draw event markers
			int rawEventState = canvas->getYCoord(canvas->getNumChannels(), i);// get last channel+1 in buffer (represents events)
			for (int ev_ch = 0; ev_ch < 8 ; ev_ch++) // for all event channels
			{
				if (display->getEventDisplayState(ev_ch))  // check if plotting for this channel is enabled
				{
					if (rawEventState & (1 << ev_ch))    // events are  representet by a bit code, so we have to extract the individual bits with a mask
					{
						g.setColour(display->channelColours[ev_ch*2]); // get color from lfp color scheme
						g.setOpacity(0.35f);
						g.drawLine(i, center-channelHeight/2 , i, center+channelHeight/2);
					}
				}
			}

			//std::cout << "e " << canvas->getYCoord(canvas->getNumChannels()-1, i) << std::endl;
			g.setColour(lineColour);

			if (drawMethod) // switched between to line drawing and pixel wise drawing
			{

				// drawLine makes for ok anti-aliased plots, but is pretty slow
				g.drawLine(i,
					(canvas->getYCoord(chan, i)/range*channelHeightFloat)+getHeight()/2,
					i+stepSize,
					(canvas->getYCoord(chan, i+stepSize)/range*channelHeightFloat)+getHeight()/2);


			}
			else
			{

				// // pixel wise line plot has no anti-aliasing, but runs much faster
				double a = (canvas->getYCoordMax(chan, i)/range*channelHeightFloat)+getHeight()/2;
				double b = (canvas->getYCoordMin(chan, i)/range*channelHeightFloat)+getHeight()/2;
				double m = (canvas->getYCoordMean(chan, i)/range*channelHeightFloat)+getHeight()/2;
				if (a<b)
				{
					from = (a);
					to = (b);
				}
				else
				{
					from = (b);
					to = (a);
				}

				//g.setColour(lineColour.withMultipliedBrightness( 1+(((((float)(to-from)*range)/getHeight())-0.01)*2)  )); // make spikes etc slightly brighter


				if ((to-from) < 200)  // if there is too much vertical range in one pixel, don't draw the full line for speed reasons
				{
					for (int j = from; j <= to; j += 1)
					{
						g.setPixel(i,j);
					}
				}
				else if ((to-from) < 400)
				{
					for (int j = from; j <= to; j += 2)
					{
						g.setPixel(i,j);
					}
				}
				else
				{
					g.setPixel(i,to);
					g.setPixel(i,from);
				}

				//draw mean
				//g.setColour(Colours::black);
				//g.setPixel(i,m);

			}

		}
	}

	// g.setColour(lineColour.withAlpha(0.7f)); // alpha on seems to decrease draw speed
	// g.setFont(channelFont);
	//  g.setFont(channelHeightFloat*0.6);

	// g.drawText(String(chan+1), 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);


}






void LfpChannelDisplay::setRange(float r)
{
	range = r;

	//std::cout << "Range: " << r << std::endl;
}

int LfpChannelDisplay::getRange()
{
	return range;
}


void LfpChannelDisplay::select()
{
	isSelected = true;
}

void LfpChannelDisplay::deselect()
{
	isSelected = false;
}

void LfpChannelDisplay::setColour(Colour c)
{
	lineColour = c;
}


void LfpChannelDisplay::setChannelHeight(int c)
{
	channelHeight = c;

	channelHeightFloat = (float) channelHeight;

	if (!inputInverted)
		channelHeightFloat = -channelHeightFloat;

	channelOverlap = channelHeight*5;
}

int LfpChannelDisplay::getChannelHeight()
{

	return channelHeight;
}

void LfpChannelDisplay::setChannelOverlap(int overlap)
{
	channelOverlap = overlap;
}


int LfpChannelDisplay::getChannelOverlap()
{
	return channelOverlap;
}

void LfpChannelDisplay::setCanBeInverted(bool _canBeInverted)
{
	canBeInverted = _canBeInverted;
}

void LfpChannelDisplay::setInputInverted(bool isInverted)
{
	if (canBeInverted)
	{
		inputInverted = isInverted;
		setChannelHeight(channelHeight);
	}
}

void LfpChannelDisplay::setDrawMethod(bool isDrawMethod)
{

	drawMethod = isDrawMethod;

}


void LfpChannelDisplay::setName(String name_)
{
	name = name_;
}

// -------------------------------

LfpChannelDisplayInfo::LfpChannelDisplayInfo(LfpDisplayCanvas* canvas_, LfpDisplay* display_, int ch)
	: LfpChannelDisplay(canvas_, display_, ch)
{

	chan = ch;

	enableButton = new UtilityButton("CH"+String(ch+1), Font("Small Text", 13, Font::plain));
	enableButton->setRadius(5.0f);

	enableButton->setEnabledState(true);
	enableButton->setCorners(true, true, true, true);
	enableButton->addListener(this);
	enableButton->setClickingTogglesState(true);
	enableButton->setToggleState(true, dontSendNotification);

	addAndMakeVisible(enableButton);

}

void LfpChannelDisplayInfo::buttonClicked(Button* button)
{

	bool state = button->getToggleState();

	display->setEnabledState(state, chan);

	UtilityButton* b = (UtilityButton*) button;

	// if (state)
	// {
	//  b->setLabel("ON");
	// } else {
	//  b->setLabel("OFF");
	// }

	std::cout << "Turn channel " << chan << " to " << button->getToggleState() << std::endl;

}

void LfpChannelDisplayInfo::setEnabledState(bool state)
{
	enableButton->setToggleState(state, sendNotification);
}

void LfpChannelDisplayInfo::paint(Graphics& g)
{

	int center = getHeight()/2;

	g.setColour(lineColour);

	g.fillRoundedRectangle(5,center-8,41,22,8.0f);

	//  g.setFont(channelFont);
	// g.setFont(channelHeightFloat*0.3);

	//  g.drawText(name, 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);

}

void LfpChannelDisplayInfo::resized()
{

	int center = getHeight()/2;

	enableButton->setBounds(8,center-5,35,16);
}


// Event display Options --------------------------------------------------------------------

EventDisplayInterface::EventDisplayInterface(LfpDisplay* display_, LfpDisplayCanvas* canvas_, int chNum):
	isEnabled(true), display(display_), canvas(canvas_)
{

	channelNumber = chNum;

	chButton = new UtilityButton(String(channelNumber+1), Font("Small Text", 13, Font::plain));
	chButton->setRadius(5.0f);
	chButton->setBounds(4,4,14,14);
	chButton->setEnabledState(true);
	chButton->setCorners(true, false, true, false);
	//chButton.color = display->channelColours[channelNumber*2];
	chButton->addListener(this);
	addAndMakeVisible(chButton);


	checkEnabledState();

}

EventDisplayInterface::~EventDisplayInterface()
{

}

void EventDisplayInterface::checkEnabledState()
{
	isEnabled = display->getEventDisplayState(channelNumber);

	//repaint();
}

void EventDisplayInterface::buttonClicked(Button* button)
{
	checkEnabledState();
	if (isEnabled)
	{
		display->setEventDisplayState(channelNumber, false);
	}
	else
	{
		display->setEventDisplayState(channelNumber, true);
	}

	repaint();

}


void EventDisplayInterface::paint(Graphics& g)
{

	checkEnabledState();

	if (isEnabled)
	{
		g.setColour(display->channelColours[channelNumber*2]);
		g.fillRoundedRectangle(2,2,18,18,5.0f);
	}


	//g.drawText(String(channelNumber), 8, 2, 200, 15, Justification::left, false);

}
