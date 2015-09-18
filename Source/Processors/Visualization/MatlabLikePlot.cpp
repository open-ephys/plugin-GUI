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

#include "MatlabLikePlot.h"
/**********************************************************************/
MatlabLikePlot::MatlabLikePlot()
{
	vertAxesComponent = new AxesComponent(false,false);
	addAndMakeVisible(vertAxesComponent);
	horizAxesComponent = new AxesComponent(true,false);
	addAndMakeVisible(horizAxesComponent);
	drawComponent = new DrawComponent(this);
	addAndMakeVisible(drawComponent);
	font = Font("Default", 15, Font::plain);
    xmin = -0.5;
	xmax = 0.5;
	ymin = -1e3;
	ymax = 1e3;
	firingRateMode = false;

	xmin_limit = -1e10;
	xmax_limit = 1e10;
	ymin_limit = -1e10;
	ymax_limit = 1e10;

	maxImageHeight = 0;
	title = "";
	borderColor = juce::Colours::white;
	controlButtonsVisible = false;
	 
	zoomButton = new UtilityButton("Zoom",Font("Default", 15, Font::plain));
	zoomButton ->addListener(this);
	addAndMakeVisible(zoomButton );

	panButton = new UtilityButton("Pan",Font("Default", 15, Font::plain));
	panButton ->addListener(this);
	addAndMakeVisible(panButton );


	autoRescaleButton = new UtilityButton("Auto Rescale",Font("Default", 15, Font::plain));
	autoRescaleButton ->addListener(this);
	addAndMakeVisible(autoRescaleButton );

	dcRemoveButton = new UtilityButton("Remove DC",Font("Default", 15, Font::plain));
	dcRemoveButton ->addListener(this);
	addAndMakeVisible(dcRemoveButton);

	boundsButton= new UtilityButton("Show Bounds",Font("Default", 15, Font::plain));
	boundsButton ->addListener(this);
	addAndMakeVisible(boundsButton);


	activateButton = new UtilityButton("*",Font("Default", 15, Font::plain));
	activateButton ->addListener(this);
	addAndMakeVisible(activateButton);

	activateButton->setVisible(false);

	autoRescaleButton->setToggleState(true, sendNotification);
	panButton->setToggleState(false, dontSendNotification);
	dcRemoveButton->setToggleState(false, sendNotification);
	zoomButton->setToggleState(true, dontSendNotification);
	boundsButton->setToggleState(false, sendNotification);
	//ScopedPointer<UtilityButton> zoomButton, panButton, verticalShiftButton, , frequencyButton;

	setRange(xmin,xmax,ymin,ymax,false);
}

void MatlabLikePlot::setRangeLimit(float xmin_limit_, float xmax_limit_ ,float ymin_limit_ , float ymax_limit_)
{
	xmin_limit = xmin_limit_;
	xmax_limit = xmax_limit_;
	ymin_limit = ymin_limit_;
	ymax_limit = ymax_limit_;
	setRange(xmin,xmax,ymin,ymax,false);
}
void MatlabLikePlot::setFiringRateMode(bool state)
{
	firingRateMode = state;
}

void MatlabLikePlot::getRangeLimit(float &xmin_limit_, float &xmax_limit_ ,float &ymin_limit_ , float &ymax_limit_)
{
	xmin_limit_ = xmin_limit;
	xmax_limit_ = xmax_limit;
	ymin_limit_ = ymin_limit;
	ymax_limit_ = ymax_limit;
}

void MatlabLikePlot::setMode(DrawComponentMode mode)
{
	if (mode == DrawComponentMode::ZOOM)
	{
		zoomButton->setToggleState(true, sendNotification);
	} else if (mode == DrawComponentMode::PAN)
	{
		panButton->setToggleState(true, sendNotification);
	}
}

MatlabLikePlot::~MatlabLikePlot()
{
	removeChildComponent(vertAxesComponent);
	removeChildComponent(horizAxesComponent);
	removeChildComponent(drawComponent);
}

void MatlabLikePlot::setActivateButtonVisiblilty(bool vis, int id)
{
	activateButton->setVisible(vis);
	activateButton->setToggleState(id > 0, dontSendNotification);
	if (id > 0)
		activateButton->setLabel(String(id));
	else
		activateButton->setLabel("*");
}

void MatlabLikePlot::setImageMode(bool state)
{
	vertAxesComponent->setFlip(state);
	drawComponent->setImageMode(state);
}

void MatlabLikePlot::drawImage(Image I, float maxValue)
{
	float xmin, xmax, ymin, ymax;
	drawComponent->getRange(xmin, xmax, ymin, ymax);

	if (!drawComponent->getImageSet() || maxImageHeight <  I.getHeight())
	{
		// first time we draw an image / new image size. Set the y scale properly!
		maxImageHeight = I.getHeight();
		setRange(xmin,xmax,0,I.getHeight()-1,false);
	}

	drawComponent->drawImage(I,maxValue);
}

void MatlabLikePlot::setShowBounds(bool state)
{
	drawComponent->setShowBounds(state);
}

void MatlabLikePlot::setTriggered()
{
	Time t;
	triggeredTS = t.getHighResolutionTicks();
	drawComponent->setShowTriggered(true);
}

void MatlabLikePlot::setHorizonal0Visible(bool state)
{
	drawComponent->setHorizonal0Visible(state);
}

void MatlabLikePlot::setVertical0Visible(bool state)
{
	drawComponent->setVertical0Visible(state);
}

void MatlabLikePlot::setControlButtonsVisibile(bool state)
{
	controlButtonsVisible = state;
	resized();
}

void MatlabLikePlot::setAutoRescale(bool state)
{
	drawComponent->setAutoRescale(state);
}

void MatlabLikePlot::buttonClicked(Button *btn)
{
	bool prevState = btn->getToggleState();
	if (btn == zoomButton)
	{
		zoomButton->setToggleState(!prevState, dontSendNotification);
		panButton->setToggleState(false, dontSendNotification);
		drawComponent->setMode(ZOOM);
	} else if  (btn == autoRescaleButton)
	{
		btn->setToggleState(!prevState, dontSendNotification);
		drawComponent->setAutoRescale(!prevState);
	} else if (btn == panButton)
	{
		btn->setToggleState(!prevState, dontSendNotification);
		drawComponent->setMode(PAN);
		zoomButton->setToggleState(false, dontSendNotification);
	}  else if (btn == dcRemoveButton)
	{
		btn->setToggleState(!prevState, dontSendNotification);
	}  else if (btn == boundsButton)
	{
		btn->setToggleState(!prevState, dontSendNotification);
		drawComponent->setShowBounds(!prevState);
	} else if (btn == activateButton)
	{
		btn->setToggleState(!prevState, dontSendNotification);
		if (!prevState)
			addEvent("StartInterval");
		else
			addEvent("StopInterval");
	}
}

void MatlabLikePlot::resized()
{
	int w = getWidth();
	int h = getHeight();
	if (h == 0 || w == 0)
		return;

	int heightOffset, AxesWidth = 75,AxesHeight;

	
		// small plot. Use smaller fonts?

		heightOffset = (controlButtonsVisible) ? 50 : 20;
		AxesWidth = 35; // Fixed
		AxesHeight = 25;
		if (MIN(w,h) > 250)
		{
			vertAxesComponent->setFontHeight(12);
			horizAxesComponent->setFontHeight(12);
		} else
		{
			vertAxesComponent->setFontHeight(8);
			horizAxesComponent->setFontHeight(8);
		}
	

	vertAxesComponent->setBounds(2,heightOffset,AxesWidth-2, h - AxesHeight-heightOffset);
	horizAxesComponent->setBounds(AxesWidth,h-AxesHeight-2,w - AxesWidth, AxesHeight-2);
	drawComponent->setBounds(AxesWidth,heightOffset, w-AxesWidth-2,h-AxesHeight-heightOffset);
	activateButton->setBounds(5,5,25-5,heightOffset-5);
	if (controlButtonsVisible)
	{
		zoomButton ->setBounds(AxesWidth,heightOffset-25,80,25);
		panButton ->setBounds(AxesWidth+100,heightOffset-25,80,25);
		autoRescaleButton ->setBounds(AxesWidth+200,heightOffset-25,80,25);
		dcRemoveButton->setBounds(AxesWidth+300,heightOffset-25,80,25);
		boundsButton->setBounds(AxesWidth+400,heightOffset-25,80,25);

		zoomButton->setVisible(true);
		panButton->setVisible(true);
		autoRescaleButton->setVisible(true);
		dcRemoveButton->setVisible(true);
		boundsButton->setVisible(true);

	} else 
	{
		zoomButton->setVisible(false);
		panButton->setVisible(false);
		autoRescaleButton->setVisible(false);
		dcRemoveButton->setVisible(false);
		boundsButton->setVisible(false);
	}
}

void MatlabLikePlot::plotxy(XYline l)
{
	if (dcRemoveButton->getToggleState())
	{
		l.removeMean();
	}

	drawComponent->plotxy(l);
}


void MatlabLikePlot::setBorderColor(juce::Colour col) 
{
	borderColor = col;
	
}


void MatlabLikePlot::getRange(float &xmin, float &xmax, float &ymin, float &ymax)
{
	drawComponent->getRange(xmin, xmax, ymin, ymax);
}

void MatlabLikePlot::setRange(float xmin_, float xmax_, float ymin_, float ymax_, bool sendMessage)
{

	xmin = MIN(xmax_limit, MAX(xmin_limit,xmin_));
	xmax = MAX(xmin_limit, MIN(xmax_limit,xmax_));

	ymin = MIN(ymax_limit, MAX(ymin_limit,ymin_));
	ymax = MAX(ymin_limit, MIN(ymax_limit,ymax_));

	// determine tick marks....
	int numTicks = (getWidth() < 250) ? 5 : 7;
	String ampScale = vertAxesComponent->setRange(ymin, ymax,numTicks, drawComponent->getImageMode(), firingRateMode);
	String timeScale = horizAxesComponent->setRange(xmin,xmax,numTicks, false, false);
	drawComponent->setRange(xmin, xmax, ymin, ymax);

	std::vector<float> xtick, ytick;
	std::vector<String> xtickLbl, ytickLbl;
	vertAxesComponent->getTicks(ytick,ytickLbl);
	horizAxesComponent->getTicks(xtick,xtickLbl);

	drawComponent->setTickMarks(xtick,ytick);
	drawComponent->setScaleString(ampScale,timeScale);
	if (sendMessage)
		addEvent("NewRange "+String(xmin)+" "+String(xmax)+" "+String(ymin)+" "+String(ymax));
}

void MatlabLikePlot::setTitle(String t)
{
	title = t;
}

void MatlabLikePlot::setThresholdLineVisibility(bool state)
{
	drawComponent->setThresholdLineVisibility(state);
}

void MatlabLikePlot::setThresholdLineValue(double Value)
{
	drawComponent->setThresholdLineValue(Value);
}

double MatlabLikePlot::getThresholdLineValue()
{
	return drawComponent->getThresholdLineValue();
}


void MatlabLikePlot::paint(Graphics &g)
{
	Time t;
	if (t.getHighResolutionTicks()-triggeredTS > 0.5*t.getHighResolutionTicksPerSecond())
		drawComponent->setShowTriggered(false);

	g.setColour(borderColor);
	g.drawRect(0,0,getWidth(),getHeight(),2);
	g.setColour(juce::Colours::white);
	g.setFont(font);
	g.drawText(title, 30,0,getWidth()-30,25,juce::Justification::centred,true);
}

void MatlabLikePlot::clearplot()
{
	maxImageHeight = 0;
	drawComponent->clearplot();
}

bool MatlabLikePlot::eventsAvail()
{
	return recentEvents.size() > 0;
}

String MatlabLikePlot::getLastEvent()
{
	String e = recentEvents.back();
	recentEvents.pop_back();
	return  e;
}
void MatlabLikePlot::setAuxiliaryString(String S)
{
	drawComponent->setAuxiliaryString(S);
}

void MatlabLikePlot::addEvent(String e)
{
		recentEvents.push_front(e);

	if (recentEvents.size() > 10)
		recentEvents.pop_back();


}

void MatlabLikePlot::mouseDoubleClick(const juce::MouseEvent& event)
{
	if (event.mods.isRightButtonDown())
		addEvent("DblkClickRight");
	else if (event.mods.isLeftButtonDown())
		addEvent("DblkClickLeft");
}

/*************************************************************************/
AxesComponent::AxesComponent(bool horizontal, bool flip) : flipDirection(flip), horiz(horizontal)
{
	minv = -1e4;
	maxv = 1e4;
	font = Font("Default", 15, Font::plain);
	setRange(minv,maxv,5,false,false);
}

void AxesComponent::setFlip(bool state)
{
	flipDirection = state;
}

void AxesComponent::getTicks(std::vector<float> &tickLocations, std::vector<String> &tickLbl)
{
	tickLocations = ticks;
	tickLbl = ticksLabels;
}



std::vector<float> AxesComponent::linspace(float minv, float maxv, int numticks)
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

std::vector<float> AxesComponent::roundlin(float minv, float maxv, int numticks)
{
	// generate equi distance points (except first and last one).
	float xRange = maxv-minv;
	float dx = xRange / (numticks-1);
	std::vector<float> ticksLoc;
	ticksLoc.clear();
	ticksLoc.push_back((int)(minv+dx/4));
	for (int k=1;k<numticks-1;k++)
	{
		float tic = minv + dx * k;
		ticksLoc.push_back(int(tic));
	}
	ticksLoc.push_back((int)(maxv-dx/4));

	return ticksLoc;
}


void AxesComponent::determineTickLocations(float minV, float maxV, int numTicks, bool imageMode)
{
	if (!imageMode)
	{
		ticks = linspace(minV, maxV, numTicks);
		ticksLabels.resize(ticks.size());
		
		for (int k=0;k<ticks.size();k++)
		{
			String tickString = (numDigits == 0) ? String(int(ticks[k]*gain)) : String(ticks[k]*gain,numDigits);
			ticksLabels[k] = tickString;
		}
	} else
	{
		ticks = roundlin(minV, maxV, numTicks);
		ticksLabels.resize(ticks.size());
		int numTTLchannels = 8;

		for (int k=0;k<ticks.size();k++)
		{
			String tickString;
			if (ticks[k] <= numTTLchannels)
				tickString = String("T "+String(ticks[k]));
			else
				tickString = String(ticks[k]-numTTLchannels);

			ticksLabels[k] = tickString;
		}
	
	}
}



String AxesComponent::setRange(float minvalue, float maxvalue, int numTicks, bool imageMode,bool firingRateMode)
{
	minv = minvalue;
	maxv = maxvalue;

	String timeScale[3] = {"uS","ms","sec"};
	String voltageScale[3] = {"uV","mV","V"};

	// make sure final displayed numbers are always between 0..1000

	if (horiz)
	{
		// values are given in Seconds.
		if (maxv-minv < 1e-3) {
			// range is in the uSec range.
			gain = 1e6;
			selectedTimeScale = 0;
			numDigits = 0;
		} else if (maxv-minv < 1) {
			// range is in the ms range.
			gain = 1e3;
			selectedTimeScale = 1;
			numDigits = 0;
		} else {
			// range is in the seconds...
			gain = 1;
			selectedTimeScale = 2;
			numDigits = 2;
		}
		determineTickLocations(minvalue, maxvalue, numTicks, imageMode);
		return timeScale[selectedTimeScale];
	} else
	{
		if(firingRateMode)
		{
			gain = 1;
			if (maxv-minv < 10)  
				numDigits = 1;
			else
				numDigits = 0;

			determineTickLocations(minvalue, maxvalue, numTicks, false);
			return "Hz";
		} else 
		{
			// values are given in uV
			if (maxv-minv < 1000) {
				// stay in uV
				selectedTimeScale = 0;
				gain = 1;
				numDigits = 0;
			} else if (maxv-minv > 1000 && maxv-minv < 1000000)
			{
				// convert to mV
				gain = 1e-3; // convert to mV
				selectedTimeScale = 1;
				numDigits = 1;
			} else 
			{
				// convert to V
				gain = 1e-6; // convert to V
				selectedTimeScale = 2;
				numDigits = 2;
			}
		}
		determineTickLocations(minvalue, maxvalue, numTicks, imageMode);
		return voltageScale[selectedTimeScale];
	}

}

void AxesComponent::setTicks(std::vector<float> ticks_, std::vector<String> tickLabels_)
{
	ticks = ticks_;
	ticksLabels = tickLabels_;
}

void AxesComponent::setFontHeight(int height)
{
	font.setHeight(height);
}
void AxesComponent::paint(Graphics &g)
{
	g.fillAll(juce::Colours::grey);
	g.setFont(font);
	// paint tick labels...
	int w = getWidth();
	int h = getHeight();

	int ticklabelWidth = 60;
	int tickLabelHeight = 20;
	g.setColour(juce::Colours::white);
	if (horiz)
	{
		for (int k=0;k<ticks.size();k++)
		{
			float xtickloc = (ticks[k] -minv) / (maxv-minv) * w; 
			g.drawText(ticksLabels[k], xtickloc-ticklabelWidth/2, 0,ticklabelWidth,tickLabelHeight,juce::Justification::centred,true);
		}
	} else 
	{
		for (int k=0;k<ticks.size();k++)
		{
			float ytickloc = (ticks[k] -minv) / (maxv-minv) * h; 
			if (flipDirection)
				g.drawText(ticksLabels[k],0,ytickloc-tickLabelHeight/2,w-3,tickLabelHeight,Justification::right,false);
			else
				g.drawText(ticksLabels[k],0,h-ytickloc-tickLabelHeight/2,w-3,tickLabelHeight,Justification::right,false);
	
	
		}
	}

}
/*************************************************************************/
XYline::XYline(float x0_, float dx_, std::vector<float> y_, float gain_, juce::Colour color_) : gain(gain_), dx(dx_), x0(x0_), y(y_), color(color_)
{
	// adjust gain
	numpts = y.size();
	for (int k=0;k<numpts;k++)
	{
		y[k] *= gain;
	}
	xn = x0 + dx * (numpts-1);
	verticalLine = false;
	fixedDx = true;
}

XYline::XYline(float x0_, float ymin, float ymax, juce::Colour color_) : x0(x0_), color(color_) 
{
	gain = 1.0;
	y.clear();
	y.push_back(ymin);
	y.push_back(ymax);
	verticalLine = true;
	numpts = y.size();
	xn = x0;
	fixedDx = false;
}

int XYline::getNumPoints()
{
	return numpts;
}

void XYline::smooth(std::vector<float> smoothKernel)
{
	std::vector<float> smoothy;
	smoothy.resize(y.size());

	int numKernelBins = smoothKernel.size();
	int zeroIndex = (numKernelBins-1)/2;
	int numXbins = y.size();
	for (int k=0;k<numXbins;k++)
	{
		float response = 0;
		for (int j=-zeroIndex;j<zeroIndex;j++)
		{
			if (k+j >=0 && k+j < numXbins)
				response+=y[k+j] * smoothKernel[j+zeroIndex];
		}
		smoothy[k] = response;
	}
	y = smoothy;
}

void XYline::getYRange(float xmin, float xmax, double &lowestValue, double &highestValue)
{
	int startIndex = MIN(numpts,MAX(0, (xmin-x0)/dx));
	int endIndex = MIN(numpts,MAX(0, (xmax-x0)/dx));
	for (int k=startIndex;k<endIndex;k++)
	{
		lowestValue = MIN(lowestValue,y[k]);
		highestValue =MAX(highestValue,y[k]);
	}
}

void XYline::removeMean()
{
	// first, remove gain...
	// then, compute mean
	// then, return gain.
	mean = 0;
	for (int k=0;k<y.size();k++)
	{
		y[k] /= gain;
		mean += y[k];
	}	
	mean /= y.size();
	for (int k=0;k<y.size();k++)
	{
		y[k] = (y[k]-mean)*gain;
	}	
}


float XYline::interp(float x_sample, bool &inrange)
{
	return interp_bilinear(x_sample, inrange);
}

float XYline::interp_bilinear(float x_sample, bool &inrange)
{
	inrange = (x_sample >= x0 && x_sample <= xn);
	if (!inrange )
		return 0;

	float bin = (x_sample-x0) / dx;
	int lowerbin = floor(bin);
	float fraction = bin-lowerbin;
	int higherbin = lowerbin + 1;
	if (lowerbin == y.size()-1)
		return y[lowerbin];
	else
		return y[lowerbin]*(1-fraction) +  y[higherbin]*(fraction);
}


float XYline::interp_cubic(float x_sample, bool &inrange)
{
	inrange = (x_sample >= x0 && x_sample <= xn);
	if (!inrange )
		return 0;

	float bin = (x_sample-x0) / dx;
	int lowerbin = floor(bin);
	float t = bin-lowerbin;
	int higherbin = lowerbin + 1;
	if (lowerbin == y.size()-1)
		return y[lowerbin];
	else {
		if (lowerbin-2 >= 0 & lowerbin+2 < y.size())
		{
			int k = lowerbin;
			double tsqr = t*t;
			double tcub = t*tsqr;
			double m0 = (y[k+1]-y[k-1]) / 2;
			k = lowerbin+1;
			double m1 = (y[k+1]-y[k-1]) / 2;
			return (2*tcub - 3*tsqr + 1)*y[lowerbin] + (-2*tcub+3*tsqr)*y[higherbin]  + (tcub-2*tsqr+t)*m0 + (tcub-tsqr)*m1;
		} else {
			inrange = false;
			return 0;
		}
	}
}

/*
 FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

 Inputs:
	data : vector of complex* data points of size 2*NFFT+1.
		* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
			data[2*n] = real(x(n))
			data[2*n+1] = imag(x(n))
		if length(Nx) < NFFT, the remainder of the array must be padded with zeros

	nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
	isign:  if set to 1, 
				computes the forward FFT
			if set to -1, 
				computes Inverse FFT - in this case the output values have
				to be manually normalized by multiplying with 1/NFFT.
 Outputs:
	data[] : The FFT or IFFT results are stored in data, overwriting the input.


	
void XYline::four1(std::vector<float> &data, int nn, int isign)
{
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    const double twopi = 6.28318530718;
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
	if (j > i) {
	    tempr = data[j-1];     data[j-1] = data[i-1];     data[i-1] = tempr;
	    tempr = data[j+1-1]; data[j+1-1] = data[i+1-1]; data[i+1-1] = tempr;
	}
	m = n >> 1;
	while (m >= 2 && j > m) {
	    j -= m;
	    m >>= 1;
	}
	j += m;
    }
    mmax = 2;
    while (n > mmax) {
	istep = 2*mmax;
	theta = twopi/(isign*mmax);
	wtemp = sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi = sin(theta);
	wr = 1.0;
	wi = 0.0;
	for (m = 1; m < mmax; m += 2) {
	    for (i = m; i <= n; i += istep) {
		j =i + mmax;
		tempr = wr*data[j-1]   - wi*data[j+1-1];
		tempi = wr*data[j+1-1] + wi*data[j-1];
		data[j-1]   = data[i-1]   - tempr;
		data[j+1-1] = data[i+1-1] - tempi;
		data[i-1] += tempr;
		data[i+1-1] += tempi;
	    }
	    wr = (wtemp = wr)*wpr - wi*wpi + wr;
	    wi = wi*wpr + wtemp*wpi + wi;
	}
	mmax = istep;
    }
}


XYline XYline::getFFT()
{
	int Nx = numpts;
	// calculate NFFT as the next higher power of 2 >= Nx 
	int NFFT = (int)pow(2.0, ceil(log((double)Nx)/log(2.0)));
	std::vector<float> X;
	X.resize(2*NFFT); // to hold complex 

	// Storing x(n) in a complex array to make it work with four1. 
	//This is needed even though x(n) is purely real in this case. 
	for(int i=0; i<Nx; i++)
	{
		X[2*i] = y[i];
		X[2*i+1] = 0.0;
	}
	// pad the remainder of the array with zeros (0 + 0 j) 
	for(int i=Nx; i<NFFT; i++)
	{
		X[2*i] = 0.0;
		X[2*i+1] = 0.0;
	}

	// calculate FFT 
	four1(X, NFFT, 1);


	// now convert to power spectrum, and remove the second half of the signal (mirror symmetry of real function).
	float Fs = 1.0/dx;
	float df = (NFFT/2+1) / (Fs/2);
	std::vector<float> powerspectrum;
	powerspectrum.resize(NFFT/2+1);
	for (int k=0;k<NFFT/2+1;k++)
	{
		powerspectrum[k] = log10(2 *  sqrt(X[2*k]*X[2*k]+ X[2*k+1]*X[2*k+1]));
	}

	return XYline(0,df, powerspectrum,1.0, color);
}

*/





/************************************************
* FFT code from the book Numerical Recipes in C *
* Visit www.nr.com for the licence.             *
************************************************/

// The following line must be defined before including math.h to correctly define M_PI

#define PI	3.14159265359	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

/*
 FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

 Inputs:
	data[] : array of complex* data points of size 2*NFFT+1.
		data[0] is unused,
		* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
			data[2*n+1] = real(x(n))
			data[2*n+2] = imag(x(n))
		if length(Nx) < NFFT, the remainder of the array must be padded with zeros

	nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
	isign:  if set to 1, 
				computes the forward FFT
			if set to -1, 
				computes Inverse FFT - in this case the output values have
				to be manually normalized by multiplying with 1/NFFT.
 Outputs:
	data[] : The FFT or IFFT results are stored in data, overwriting the input.
*/

void XYline::four1(double data[], int nn, int isign)
{
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
	if (j > i) {
	    tempr = data[j];     data[j] = data[i];     data[i] = tempr;
	    tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
	}
	m = n >> 1;
	while (m >= 2 && j > m) {
	    j -= m;
	    m >>= 1;
	}
	j += m;
    }
    mmax = 2;
    while (n > mmax) {
	istep = 2*mmax;
	theta = TWOPI/(isign*mmax);
	wtemp = sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi = sin(theta);
	wr = 1.0;
	wi = 0.0;
	for (m = 1; m < mmax; m += 2) {
	    for (i = m; i <= n; i += istep) {
		j =i + mmax;
		tempr = wr*data[j]   - wi*data[j+1];
		tempi = wr*data[j+1] + wi*data[j];
		data[j]   = data[i]   - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp = wr)*wpr - wi*wpi + wr;
	    wi = wi*wpr + wtemp*wpi + wi;
	}
	mmax = istep;
    }
}

/******************************************/

XYline XYline::getFFT()
{
	int Nx = numpts;
	// calculate NFFT as the next higher power of 2 >= Nx 
	int NFFT = (int)pow(2.0, ceil(log((double)Nx)/log(2.0)));

	int i;

	double *data_aug;

	/* calculate NFFT as the next higher power of 2 >= Nx */
	NFFT = (int)pow(2.0, ceil(log((double)Nx)/log(2.0)));

	/* allocate memory for NFFT complex numbers (note the +1) */
	data_aug = (double *) malloc((2*NFFT+1) * sizeof(double));

	/* Storing x(n) in a complex array to make it work with four1. 
	This is needed even though x(n) is purely real in this case. */
	for(i=0; i<Nx; i++)
	{
		data_aug[2*i+1] = y[i];
		data_aug[2*i+2] = 0.0;
	}
	/* pad the remainder of the array with zeros (0 + 0 j) */
	for(i=Nx; i<NFFT; i++)
	{
		data_aug[2*i+1] = 0.0;
		data_aug[2*i+2] = 0.0;
	}

	four1(data_aug, NFFT, 1);


	// now convert to power spectrum, and remove the second half of the signal (mirror symmetry of real function).
	float Fs = 1.0/dx;
	float df =((Fs/2)/(NFFT/2));
	std::vector<float> powerspectrum;
	powerspectrum.resize(NFFT/2+1);
	for (int k=0;k<NFFT/2+1;k++)
	{
		powerspectrum[k] = sqrt(data_aug[2*k+1]/NFFT*data_aug[2*k+1]/NFFT+ data_aug[2*k+2]/NFFT*data_aug[2*k+2]/NFFT);
	}
	delete data_aug;

	return XYline(0,df, powerspectrum,1.0, color);
}


// draw a function.
// stretch Y such that ymin->0 and ymax->plotHeight
// and only display points between [xmin and xmax]
void XYline::draw(Graphics &g, float xmin, float xmax, float ymin, float ymax, int plotWidth, int plotHeight, bool showBounds)
{
	if (ymax-ymin < 1e-6 || xmax-xmin < 1e-6)
		return;

	g.setColour(color);
	if (verticalLine)
	{
		float drawX = (x0 - xmin) / (xmax-xmin) * plotWidth;
		float y0 = (y[0] -ymin) / (ymax-ymin) * plotHeight;
		float y1 = (y[1] -ymin) / (ymax-ymin) * plotHeight;
		g.drawLine(drawX, plotHeight-y0, drawX,  plotHeight-y1);
		return;
	}
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
	float prevY = interp(xmin,prevInrange);
	float prevYscaled = (prevY - ymin) / (ymax-ymin) * plotHeight;

	std::vector<int> screenx, bins; // used to draw upper & lower limits of the signal..
	std::vector<float> min_in_bin, max_in_bin; // used to draw upper & lower limits of the signal..
	if (prevInrange) {
		screenx.push_back(0);
		bins.push_back( (xmin-x0)/dx);
	}
	bool inrange;


	for (int i=1;i<numPointsToDraw;i++)
	{
		float x_sample = xmin + i * xrange/ (numPointsToDraw-1);
		float currY = interp(x_sample, inrange);
		float yScaled = (currY - ymin) / (ymax-ymin) * plotHeight;
		if (inrange && prevInrange) 
		{
			g.drawLine((i-1)*scalex, plotHeight-prevYscaled, (i)*scalex, plotHeight-yScaled);
			screenx.push_back(i*scalex);
			bins.push_back( (x_sample-x0)/dx);
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
	//	g.setColour(juce::Colours::white);
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

/*************************************************************************/
DrawComponent::DrawComponent(MatlabLikePlot *mlp_) : mlp(mlp_)
{
	zooming = false;
	panning = false;
	autoRescale = true;
	horiz0 = false;
	vert0 = false;
	lowestValue = 1e10;
	highestValue = -1e10;
	thresholdLineVisibility = false;
	thresholdLineValue = 0;
	overThresholdLine = false;
	showTriggered = false;
	showBounds = false;
	imageMode = imageSet = false;
	ampScale = "";
	timeScale = "";
	maxImageValue = 0;
	auxString = "";

	font = Font("Default", 12, Font::plain);
	setMode(ZOOM); // default mode
}

void DrawComponent::setMode(DrawComponentMode m)
{
	mode = m;
};

void DrawComponent::setShowTriggered(bool state)
{
	showTriggered = state;
}

void DrawComponent::setShowBounds(bool state)
{
	showBounds = state;
}
void DrawComponent::drawTicks(Graphics &g)
{
	int tickHeight = 6;
	float tickThickness = 2;
	double rangeX = (xmax-xmin);
	double rangeY = (ymax-ymin);
	if (std::abs(rangeX) < 1e-6 || std::abs(rangeY) < 1e-6)
		return;

	int plotWidth = getWidth();
	int plotHeight = getHeight();

	g.setColour(juce::Colours::white);
	for (int k=0;k < xtick.size();k++)
	{
		// convert to screen coordinates.
		float tickloc = (xtick[k]- xmin) / rangeX * plotWidth;
		if (std::abs(tickloc) < 1e10)
			g.drawLine(tickloc,plotHeight,tickloc,plotHeight-(tickHeight),tickThickness);
	}
	for (int k=0;k < ytick.size();k++)
	{
		// convert to screen coordinates.
		float tickloc = (ytick[k]- ymin) / rangeY * plotHeight;
		if (std::abs(tickloc) < 1e10)
			g.drawLine(0,plotHeight-tickloc,tickHeight,plotHeight-tickloc, tickThickness);
	}
}


void DrawComponent::plotxy(XYline l)
{
	l.getYRange(xmin,xmax,lowestValue, highestValue);
	if (std::abs(lowestValue) < 1e10 && std::abs(highestValue) < 1e10)
		lines.push_back(l);
}
	
void DrawComponent::clearplot()
{
	lowestValue = 1e10;
	highestValue = -1e10;
	imageSet = false;
	lines.clear();
}


void DrawComponent::paint(Graphics &g)
{
	g.fillAll(juce::Colours::black);

	int w = getWidth();
	int h = getHeight();


	if (autoRescale && !imageMode)
	{
		ymin = lowestValue;
		ymax = highestValue;
		mlp->setRange(xmin,xmax,ymin,ymax,false);
	}

	if 	(imageMode && imageSet)
	{
		g.drawImage(image,0,0,w,h,0,0,image.getWidth(),image.getHeight());
	}
	else {

		// now draw curves.
		for (int k=0;k<lines.size();k++) 
		{
			if (std::abs(ymin) < 1e10 & std::abs(ymax) < 1e10)
				lines[k].draw(g,xmin,xmax,ymin,ymax,w,h,showBounds);
		}
		if (lines.size() > 0)
		{
			// draw the horizontal zero line
			if (horiz0)
			{
				std::vector<float> y;
				y.push_back(0);
				y.push_back(0);
				XYline l(xmin,xmax-xmin,y,1.0,Colours::white);
				l.draw(g, xmin,xmax,ymin,ymax,w,h,false);
			}
			if (vert0) 
			{
				// draw the vertical zero line
				XYline lv(0,ymin,ymax,Colours::white);
				lv.draw(g, xmin,xmax,ymin,ymax,w,h,false);
			}

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
	if (imageMode)
		g.drawText(String(maxImageValue,1)+ " Hz" ,6,2,80,20,juce::Justification::left,false);
	else
		g.drawText(ampScale + ", " + timeScale,6,2,50,20,juce::Justification::left,false);

	if (thresholdLineVisibility)
	{
		juce::Colour col = (overThresholdLine) ? juce::Colours::red : juce::Colours::grey;
		float thresholdLineValuePix = h - ((thresholdLineValue-ymin) / (ymax-ymin) * h);
		g.setColour(col);
		g.drawLine(0,thresholdLineValuePix,w,thresholdLineValuePix,2);
	}

	if (showTriggered)
	{
		g.setFont(Font("Default", 20, Font::plain));
		g.setColour(juce::Colours::lightgreen);
		g.drawText("Trig'd",w-80,2,60,20,juce::Justification::right,false);
	}
	if (auxString != "")
	{
		g.setFont(Font("Default", 12, Font::plain));
		g.setColour(juce::Colours::white);
		g.drawText(auxString,w-100,2,80,20,juce::Justification::right,false);
	}

}


void DrawComponent::setRange(float xmin_, float xmax_, float ymin_, float ymax_)
{
	 xmin = xmin_;
	 xmax = xmax_;
	 ymin = ymin_;
	 ymax = ymax_;
}

void DrawComponent::setYRange(double lowestValue, double highestValue)
{
	ymin = lowestValue;
	ymax = highestValue;

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

void DrawComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
	// draw parent function?
	mlp->mouseDoubleClick(event);
}

void DrawComponent::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel)
{
	if (mode == PAN)
	{
		double xRange = (xmax-xmin);
		float sn = (wheel.deltaY > 0 )? -1 : 1;
		xmin -= 0.1 * sn *xRange;
		xmax += 0.1 * sn * xRange;

	} else
	{
		double yRange = (ymax-ymin);
		float sn = (wheel.deltaY > 0 )? -1 : 1;
		ymin -= 0.1 * sn *yRange;
		ymax += 0.1 * sn * yRange;
	}

	mlp->setRange(xmin,xmax,ymin,ymax,true);
	
}

void DrawComponent::mouseUp(const juce::MouseEvent& event)
{
		int plotWidth = getWidth();
		int plotHeight = getHeight();

	panning = false;
	if (mode == THRES_UPDATE)
	{
		thresholdLineValue = ymin + (ymax-ymin) * ((plotHeight - event.y) / (float)plotHeight) ;
		mode = savedMode;
		mlp->addEvent("ThresholdChange");

	} else if (zooming)
	{
		zooming = false;

		double rangeX = xmax-xmin;
		double rangeY = ymax-ymin;
		zooming = false;
		// first, turn off auto rescale, if it is enabled.
		setAutoRescale(false); // zoom is now enabled. We can't have auto rescale and zoom at the same time.

		float downX,downY, upX, upY ;

			downX = float(mouseDownX) /(float)plotWidth * rangeX + xmin;
			upX = float(event.x) /(float)plotWidth * rangeX + xmin;

		if (imageMode)
		{
			downY = float((mouseDownY)) /(float)plotHeight * rangeY + ymin;
			upY = float((event.y)) /(float)plotHeight * rangeY + ymin;
	
		}
		else
		{

			downY = float(plotHeight-(mouseDownY)) /(float)plotHeight * rangeY + ymin;
			upY = float(plotHeight-(event.y)) /(float)plotHeight * rangeY + ymin;
		}
		
		// convert mouse down and up position to proper x,y range
		// save current zoom 
		if ( ( fabs(downX-upX) < 0.001) ||  ( fabs(downY-upY) < 0.001) )
		{
			// do not zoom more. probably just incorrect click

			return;
		}
		range CurrentZoom;
		CurrentZoom.xmin=xmin;
		CurrentZoom.ymin=ymin;
		CurrentZoom.xmax=xmax;
		CurrentZoom.ymax=ymax;
		rangeMemory.push_back(CurrentZoom);

		xmin = MIN(downX, upX);
		xmax = MAX(downX, upX);
		ymin = MIN(downY, upY);
		ymax = MAX(downY, upY);
		if (imageMode)
		{
			ymin = MIN(image.getHeight()-1,MAX(0,ymin));
			ymax = MAX(0,MIN(image.getHeight()-1,ymax));
		}
	
		mlp->setRange(xmin,xmax,ymin,ymax,true);
		
	}
}

void DrawComponent::mouseDrag(const juce::MouseEvent& event)
{
	mouseDragX = event.x;
	mouseDragY = event.y;
		int w = getWidth();
		int h = getHeight();

	if (mode == THRES_UPDATE)
	{
		thresholdLineValue = ymin + (ymax-ymin) * ((h - event.y) / (float)h) ;
		mlp->addEvent("ThresholdChange");
	} else if (panning)
	{

		float range0 = xmax-xmin;
		float range1 = ymax-ymin;

		float dx = -float(event.x-mousePrevX) / w*range0;
		float dy = float(event.y-mousePrevY) / h*range1;

		float xmin_limit, xmax_limit, ymin_limit, ymax_limit;
		mlp->getRangeLimit(xmin_limit, xmax_limit, ymin_limit, ymax_limit);
		
		if (xmin+dx >=xmin_limit && xmax +dx <=xmax_limit)
		{
			xmin+=dx;
			xmax+=dx;
			if (!imageMode)
			{
				ymax+=dy;
				ymin+=dy;
			}
			mlp->setRange(xmin,xmax,ymin,ymax,true);
		}
		mousePrevX = event.x;
		mousePrevY = event.y;
	}


}


void DrawComponent::mouseMove(const juce::MouseEvent& event)
{
	int h = getHeight();
	
	// convert threshold line to pixels.
	float thresholdLineValuePix = h - ((thresholdLineValue-ymin) / (ymax-ymin) * h);
	// check if we are close to the threshold line...
	overThresholdLine = std::abs(event.y - thresholdLineValuePix) < 5;

}

void DrawComponent::mouseDown(const juce::MouseEvent& event)
{
		mousePrevX = event.x;
		mousePrevY = event.y;


	if (event.mods.isRightButtonDown())
	{
		if (rangeMemory.size() > 0)
		{
			range prevZoom = rangeMemory.back();
			rangeMemory.pop_back();
			xmin = prevZoom.xmin;
			xmax = prevZoom.xmax;
			ymin = prevZoom.ymin;
			ymax = prevZoom.ymax;
			mlp->setRange(xmin,xmax,ymin,ymax,true);
			
		}

	} else if (event.mods.isLeftButtonDown())
	{
		if (overThresholdLine)
		{
			savedMode = mode;
			mode = THRES_UPDATE;
		}

		mouseDownX = event.x;
		mouseDownY = event.y;
		mouseDragX = event.x;
		mouseDragY = event.y;
		if (lines.size() > 0 || imageMode)
		{
			if (mode == ZOOM)
				zooming = true;
			else if (mode == PAN)
				panning = true;
		}
	}
}


void DrawComponent::setHorizonal0Visible(bool state)
{
	horiz0 = state;
	
}

void DrawComponent::setVertical0Visible(bool state)
{
	vert0 = state;
	
}

void DrawComponent::setScaleString(String ampScale_, String timeScale_)
{
	ampScale = ampScale_;
	timeScale = timeScale_;
}

void DrawComponent::setThresholdLineVisibility(bool state)
{
	thresholdLineVisibility = state;
}

void DrawComponent::setThresholdLineValue(double Value)
{
	thresholdLineValue = Value;
}

double DrawComponent::getThresholdLineValue()
{
	return thresholdLineValue;
}

void DrawComponent::drawImage(Image I, float maxValue)
{
	imageSet = true;
	image = I;
	maxImageValue = maxValue;
}


void DrawComponent::setImageMode(bool state)
{
	imageMode = state;
}

bool DrawComponent::getImageMode()
{
	return imageMode;
}

void  DrawComponent::setAuxiliaryString(String s)
{
	auxString = s;
}

bool DrawComponent::getImageSet()
{
	return imageSet;
}

void DrawComponent::getRange(float &minx, float &maxx, float &miny, float &maxy)
{
	minx = xmin;
	maxx = xmax;
	miny = ymin;
	maxy = ymax;
}