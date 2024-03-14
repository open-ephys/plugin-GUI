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

#include "SpikePlots.h"

#include "SpikeDisplay.h"
#include "SpikeDisplayCanvas.h"
#include "SpikeDisplayNode.h"

SpikePlot::SpikePlot(SpikeDisplayCanvas* sdc, 
                     int elecNum, 
                     int p, 
                     String name_,
                     std::string identifier_) :
    canvas(sdc), 
    electrodeNumber(elecNum),
    plotType(p),
    limitsChanged(true), 
    name(name_),
    identifier(identifier_)
{

    font = Font("Default", 15, Font::plain);

    switch (p)
    {
        case SINGLE_PLOT:
            nWaveAx = 1;
            nProjAx = 0;
            nChannels = 1;
            minWidth = 200;
            aspectRatio = 1.0f;
            break;
        case STEREO_PLOT:
            nWaveAx = 2;
            nProjAx = 1;
            nChannels = 2;
            minWidth = 300;
            aspectRatio = 0.5f;
            break;
        case TETRODE_PLOT:
            nWaveAx = 4;
            nProjAx = 6;
            nChannels = 4;
            minWidth = 400;
            aspectRatio = 0.33f;
            break;
        default: // unsupported number of axes provided
            plotType = SINGLE_PLOT;
            nWaveAx = 1;
            nProjAx = 0;
            nChannels = 1;
            minWidth = 200;
            aspectRatio = 1.0f;
    }

    initAxes();

    for (int i = 0; i < nChannels; i++)
    {
        UtilityButton* rangeButton = new UtilityButton("250", Font("Small Text", 10, Font::plain));
        rangeButton->setRadius(3.0f);
        rangeButton->addListener(this);
        addAndMakeVisible(rangeButton);

        rangeButtons.add(rangeButton);
        setDisplayThresholdForChannel(i, 0);
    }

    monitorButton = std::make_unique<UtilityButton>("MON", Font("Small Text", 8, Font::plain));
    monitorButton->setTooltip("Monitor this electrode (requires an Audio Monitor in the signal chain)");
    monitorButton->addListener(this);
    addAndMakeVisible(monitorButton.get());

    mostRecentSpikes.ensureStorageAllocated(bufferSize);

}

SpikePlot::~SpikePlot()
{
    if (thresholdCoordinator)
    {
        thresholdCoordinator->deregisterSpikePlot(this);
    }
}

void SpikePlot::setId(std::string id)
{
    identifier = id;
}

void SpikePlot::paint(Graphics& g)
{
    
    g.setColour(Colours::white);
    g.drawRect(0, 0, getWidth(), getHeight());

    g.setFont(font);
    g.drawText(name, 10, 0, 200, 20, Justification::left, false);

}

void SpikePlot::refresh()
{

    {
        const ScopedLock myScopedLock(spikeArrayLock);

        for (auto spike : mostRecentSpikes)
        {
            processSpikeObject(spike);
        }

        mostRecentSpikes.clearQuick(true);
        spikesInBuffer = 0;
    }
    
    repaint();
}

void SpikePlot::processSpikeObject(const Spike* s)
{

    for (int i = 0; i < waveAxes.size(); ++i)
    {
        setDetectorThresholdForChannel(i, s->getThreshold(i));
    }

    // first, check if it's above threshold
    bool aboveThreshold = false;

    for (auto ax : waveAxes)
    {
         aboveThreshold = aboveThreshold | ax->checkThreshold(s);
    }

    if (aboveThreshold)
    {
        for (auto ax : waveAxes)
            ax->updateSpikeData(s);

        for (auto ax : projectionAxes)
            ax->updateSpikeData(s);
    }

}

void SpikePlot::addSpikeToBuffer(const Spike* spike)
{
    if (spikesInBuffer < bufferSize)
    {
        const ScopedLock myScopedLock(spikeArrayLock);

        mostRecentSpikes.add(new Spike(*spike));
        spikesInBuffer++;
    }
}

void SpikePlot::initAxes()
{
    initLimits();

    for (int i = 0; i < nWaveAx; i++)
    {
        WaveAxes* wAx = new WaveAxes(canvas, electrodeNumber, WAVE1 + i, identifier);
        waveAxes.add(wAx);
        addAndMakeVisible(wAx);
        ranges.add(250.0f); // default range is 250 microvolts
    }

    for (int i = 0; i < nProjAx; i++)
    {
        Projection proj = Projection(int(PROJ1x2) + i);
        ProjectionAxes* pAx = new ProjectionAxes(canvas, proj);
        projectionAxes.add(pAx);
        addAndMakeVisible(pAx);
    }

    setLimitsOnAxes(); // initialize the ranges
}

void SpikePlot::resized()
{

    float width = getWidth()-10;
    float height = getHeight()-25;

    float axesWidth = 0;
    float axesHeight = 0;

    // to compute the axes positions we need to know how many columns of proj and wave axes should exist
    // using these two values we can calculate the positions of all of the sub axes
    int nProjCols = 0;
    int nWaveCols = 0;

    switch (plotType)
    {
        case SINGLE_PLOT:
            nProjCols = 0;
            nWaveCols = 1;
            axesWidth = width;
            axesHeight = height;
            break;

        case STEREO_PLOT:
            nProjCols = 1;
            nWaveCols = 2;
            axesWidth = width/2;
            axesHeight = height;
            break;
        case TETRODE_PLOT:
            nProjCols = 3;
            nWaveCols = 2;
            axesWidth = width/4;
            axesHeight = height/2;
            break;
    }

    for (int i = 0; i < nWaveAx; i++)
    {
        waveAxes[i]->setBounds(5 + (i % nWaveCols) * axesWidth/nWaveCols, 20 + (i/nWaveCols) * axesHeight, axesWidth/nWaveCols, axesHeight);
        rangeButtons[i]->setBounds(8 + (i % nWaveCols) * axesWidth/nWaveCols,
                                   20 + (i/nWaveCols) * axesHeight + axesHeight - 18,
                                   25, 15);
    }

    for (int i = 0; i < nProjAx; i++)
        projectionAxes[i]->setBounds(5 + (1 + i%nProjCols) * axesWidth, 20 + (i/nProjCols) * axesHeight, axesWidth, axesHeight);

    monitorButton->setBounds(getWidth() - 40, 3, 35, 15);
}

void SpikePlot::resetAudioMonitorState()
{
    monitorButton->setToggleState(false, dontSendNotification);
}

void SpikePlot::buttonClicked(Button* button)
{

    if (button == monitorButton.get())
    {
        canvas->resetAudioMonitorState();
        button->setToggleState(true, dontSendNotification);
        canvas->processor->setParameter(10, electrodeNumber);

        return;
    }

    UtilityButton* buttonThatWasClicked = (UtilityButton*) button;

    int index = rangeButtons.indexOf(buttonThatWasClicked);
    String label;

    if (ranges[index] == 250.0f)
    {
        ranges.set(index, 500.0f);
        label = "500";
    }
    else if (ranges[index] == 500.0f)
    {
        ranges.set(index, 100.0f);
        label = "100";
    }
    else if (ranges[index] == 100.0f)
    {
        ranges.set(index, 250.0f);
        label = "250";
    }

    buttonThatWasClicked->setLabel(label);

    setLimitsOnAxes();

    canvas->cache->setMonitor(identifier, monitorButton->getToggleState());
    canvas->cache->setRange(identifier, index, ranges[index]);

}

void SpikePlot::setLimitsOnAxes()
{
    //std::cout<<"SpikePlot::setLimitsOnAxes()"<<std::endl;

    for (int i = 0; i < nWaveAx; i++)
        waveAxes[i]->setRange(ranges[i]);

    // Each projection sets its limits using the limits of the two waveform dims it represents.
    // Convert projection number to indices, and then set the limits using those indices
    int j1, j2;
    for (int i = 0; i < nProjAx; i++)
    {
        projectionAxes[i]->n2ProjIdx(projectionAxes[i]->getProjection(), &j1, &j2);
        projectionAxes[i]->setRange(ranges[j1], ranges[j2]);
    }
}

void SpikePlot::initLimits()
{
    for (int i = 0; i < nChannels; i++)
    {
        limits[i][0] = 1209;//-1*pow(2,11);
        limits[i][1] = 11059;//pow(2,14)*1.6;
    }

}

void SpikePlot::getBestDimensions(int* w, int* h)
{
    switch (plotType)
    {
        case TETRODE_PLOT:
            *w = 4;
            *h = 2;
            break;
        case STEREO_PLOT:
            *w = 2;
            *h = 1;
            break;
        case SINGLE_PLOT:
            *w = 1;
            *h = 1;
            break;
        default:
            *w = 1;
            *h = 1;
            break;
    }
}

void SpikePlot::clear()
{
    for (auto ax : waveAxes)
        ax->clear();

    for (auto ax : projectionAxes)
        ax->clear();
}

float SpikePlot::getDisplayThresholdForChannel(int i)
{
    return waveAxes[i]->getDisplayThreshold();
}

void SpikePlot::setDisplayThresholdForChannel(int i, float thresh)
{
    LOGD("Setting threshold for channel ", i, " to ", thresh, "");
    waveAxes[i]->setDisplayThreshold(thresh);
}

float SpikePlot::getRangeForChannel(int i)
{
    return waveAxes[i]->getRange();
}

void SpikePlot::setRangeForChannel(int i, float range)
{
    //std::cout << "Setting range to " << range << std::endl;
    ranges.set(i, range);
    waveAxes[i]->setRange(range);
    rangeButtons[i]->setLabel(String(int(range)));
}

bool SpikePlot::getMonitorState()
{
    return monitorButton->getToggleState();
}

void SpikePlot::setMonitorState(bool state)
{
    monitorButton->setToggleState(state, sendNotification);
}

void SpikePlot::setDetectorThresholdForChannel(int i, float t)
{
    // std::cout << "Setting threshold to " << t << std::endl;
    waveAxes[i]->setDetectorThreshold(t);
}

void SpikePlot::registerThresholdCoordinator(SpikeThresholdCoordinator* stc)
{
    thresholdCoordinator = stc;
    stc->registerSpikePlot(this);

    for (int i=0; i < waveAxes.size(); i++)
    {
        waveAxes[i]->registerThresholdCoordinator(stc);
    }
}

void SpikePlot::setAllThresholds(float displayThreshold, float range)
{
    String label;

    for (int i=0; i< nWaveAx; i++)
    {
        ranges.set(i,range);
        waveAxes[i]->setDisplayThreshold(displayThreshold);
    }

    if (range == 100)
    {
        label = "100";
    }
    else if (range == 250)
    {
        label = "250";
    }
    else
    {
        label = "500";
    }

    for (int i=0; i < rangeButtons.size(); i++)
    {
        rangeButtons[i]->setLabel(label);
    }

    setLimitsOnAxes();
}

void SpikePlot::invertSpikes(bool shouldInvert)
{
    for (int i = 0; i < nWaveAx; i++)
    {
        waveAxes[i]->invertSpikes(shouldInvert);
    }
}


GenericAxes::GenericAxes(SpikeDisplayCanvas* canvas, SpikePlotType type_) : 
    canvas(canvas),
    type(type_),
    gotFirstSpike(false)
{
    ylims[0] = 0;
    ylims[1] = 1;

    xlims[0] = 0;
    xlims[1] = 1;

    font = Font("Default", 12, Font::plain);

    colours.add(Colour(255, 224, 93));
    colours.add(Colour(255, 178, 99));
    colours.add(Colour(255, 109, 161));
    colours.add(Colour(246, 102, 255));
    colours.add(Colour(175, 98, 255));
    colours.add(Colour(90, 241, 233));
    colours.add(Colour(109, 175, 136));
    colours.add(Colour(255, 224, 93));
    colours.add(Colour(160, 237, 181));

}

void GenericAxes::setYLims(double ymin, double ymax)
{

    //std::cout << "setting y limits to " << ymin << " " << ymax << std::endl;
    ylims[0] = ymin;
    ylims[1] = ymax;
}
void GenericAxes::getYLims(double* min, double* max)
{
    *min = ylims[0];
    *max = ylims[1];
}
void GenericAxes::setXLims(double xmin, double xmax)
{
    xlims[0] = xmin;
    xlims[1] = xmax;
}
void GenericAxes::getXLims(double* min, double* max)
{
    *min = xlims[0];
    *max = xlims[1];
}

int GenericAxes::roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
    {
        return numToRound;
    }

    int remainder = numToRound % multiple;

    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}


void GenericAxes::makeLabel(int val, int gain, bool convert, char* s)
{
    if (convert)
    {
        double volt = ad16ToUv(val, gain);// / 1000.;
        if (abs(val) > 1e6)
        {
            //val = val/(1e6);
            sprintf(s, "%.2fV", volt);
        }
        else if (abs(val) > 1e3)
        {
            //val = val/(1e3);
            sprintf(s, "%.2fmV", volt);
        }
        else
            sprintf(s, "%.2fuV", volt);
    }
    else
    {
        sprintf(s, "%d", (int)val);
    }
}

double GenericAxes::ad16ToUv(int x, int gain)
{
    int result = (double)(x * 20e6) / (double)(gain * pow(2.0, 16));
    return result;
}


WaveAxes::WaveAxes(SpikeDisplayCanvas* canvas, int electrodeIndex, int channel_, std::string identifier_) : GenericAxes(canvas, WAVE_AXES),
    electrodeIndex(electrodeIndex),
    drawGrid(true),
    displayThresholdLevel(0.0f),
    detectorThresholdLevel(0.0f),
    spikesReceivedSinceLastRedraw(0),
    spikeIndex(0),
    bufferSize(5),
    range(250.0f),
    isOverThresholdSlider(false),
    isDraggingThresholdSlider(false),
    thresholdCoordinator(nullptr),
    spikesInverted(false),
    channel(channel_),
    identifier(identifier_)
{
    addMouseListener(this, true);

    thresholdColour = Colours::red;

    font = Font("Small Text",10,Font::plain);

    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }
}

void WaveAxes::setRange(float r)
{
    range = r;

    repaint();
}

void WaveAxes::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.fillRect(0,0,getWidth(), getHeight());

    // draw the grid lines for the waveforms
    if (drawGrid)
        drawWaveformGrid(g);

    // draw the threshold line and labels
    drawThresholdSlider(g);

    // if no spikes have been received then don't plot anything
    if (!gotFirstSpike)
    {
        return;
    }

    for (int spikeNum = 0; spikeNum < bufferSize; spikeNum++)
    {

        if (spikeNum != spikeIndex)
        {
            plotSpike(spikeBuffer[spikeNum], g);
        }

    }

    plotSpike(spikeBuffer[spikeIndex], g);
    
    spikesReceivedSinceLastRedraw = 0;

}

void WaveAxes::plotSpike(const Spike* s, Graphics& g)
{
	if (!s) return;

    float h = getHeight();

    const SpikeChannel* sc = s->getChannelInfo();

    if (sc == nullptr)
        return;

    //compute the spatial width for each waveform sample
	int nSamples = sc->getTotalSamples();
	float dx = getWidth() / float(nSamples);

    if (s->getSortedId() > 0)
        g.setColour(colours[(s->getSortedId() - 1) % 8]);
    else
       g.setColour(Colours::white);

    // type corresponds to channel so we need to calculate the starting
    // sample based upon which channel is getting plotted
    int sampIdx = nSamples * channel; 
    
    int dataSize =s->getChannelInfo()->getDataSize();
    
    // prevent crashes when acquisition is not active,
    // or immediately after acquisition starts
    //if (   (dataSize < 1)
    //    || (dataSize > 640)
    //    || (sampIdx + nSamples > dataSize)
    //    || (nSamples < 0))
    //{
    //    return;
    //}
        
    int dSamples = 1;

    float x = 0.0f;
	const float* data = s->getDataPointer();

	for (int i = 0; i < nSamples - 1; i++)
	{
		//std::cout << s.data[sampIdx] << std::endl;

		float s1, s2;

		if (spikesInverted)
		{
			s1 = h / 2 + data[sampIdx] / range * h;
			s2 = h / 2 + data[sampIdx + 1] / range * h;
		}
		else
		{
			s1 = h / 2 - data[sampIdx] / range * h;
			s2 = h / 2 - data[sampIdx + 1] / range * h;

		}
		
        g.drawLine(x,
			s1,
			x + dx,
			s2);

		sampIdx += dSamples;
		x += dx;
	}

}

void WaveAxes::drawThresholdSlider(Graphics& g)
{

    // draw display threshold (editable)
    float h;

    if (spikesInverted)
        h = getHeight()*(0.5f - displayThresholdLevel/range);
    else
        h = getHeight()*(0.5f + displayThresholdLevel/range);

    g.setColour(thresholdColour);
    g.drawLine(0, h, getWidth(), h);

    g.drawText(String(int(displayThresholdLevel)),2,h,25,10,Justification::left, false);

    // draw detector threshold (not editable)
    if (!spikesInverted)
        h = getHeight()*(0.5f - detectorThresholdLevel/range);
    else
        h = getHeight()*(0.5f + detectorThresholdLevel/range);

    g.setColour(Colours::orange);
    g.drawLine(0, h, getWidth(), h);
}

void WaveAxes::drawWaveformGrid(Graphics& g)
{

    float h = getHeight();
    float w = getWidth();

    g.setColour(Colours::darkgrey);

    for (float y = -range/2; y < range/2; y += 25.0f)
    {
        if (y == 0)
            g.drawLine(0,h/2 + y/range*h, w, h/2+ y/range*h,2.0f);
        else
            g.drawLine(0,h/2 + y/range*h, w, h/2+ y/range*h);
    }

}

bool WaveAxes::updateSpikeData(const Spike* s)
{
    
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    if (spikesReceivedSinceLastRedraw < bufferSize)
    {

        Spike* newSpike = new Spike(*s);

        spikeIndex++;
        spikeIndex %= bufferSize;

        spikeBuffer.set(spikeIndex, newSpike);

        spikesReceivedSinceLastRedraw++;

    }

    return true;

}

bool WaveAxes::checkThreshold(const Spike* s)
{
	int nSamples = s->getChannelInfo()->getTotalSamples();
    int sampIdx = nSamples*type;
	const float* data = s->getDataPointer();

    for (int i = 0; i < nSamples-1; i++)
    {

        if (data[sampIdx] > displayThresholdLevel)
        {
            return true;
        }

        sampIdx++;
    }

    return false;

}

void WaveAxes::clear()
{

    spikeBuffer.clear();
    spikeIndex = 0;

    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }

    repaint();
}

void WaveAxes::mouseMove(const MouseEvent& event)
{

    float y = event.y;

    float h = getHeight()*(0.5f - displayThresholdLevel/range);

    if (spikesInverted)
        h = getHeight()*(0.5f - displayThresholdLevel/range);
    else
        h = getHeight()*(0.5f + displayThresholdLevel/range);

    if (y > h - 10.0f && y < h + 10.0f && !isOverThresholdSlider)
    {
        thresholdColour = Colours::yellow;

        repaint();

        isOverThresholdSlider = true;

    }
    else if ((y < h - 10.0f || y > h + 10.0f) && isOverThresholdSlider)
    {

        thresholdColour = Colours::red;
        repaint();

        isOverThresholdSlider = false;
    }


}

void WaveAxes::mouseDrag(const MouseEvent& event)
{
    if (isOverThresholdSlider)
    {

        float thresholdSliderPosition =  float(event.y) / float(getHeight());

        if (spikesInverted)
        {
            if (thresholdSliderPosition > 0.5f)
                thresholdSliderPosition = 0.5f;
            else if (thresholdSliderPosition < 0.0f)
                thresholdSliderPosition = 0.0f;

            displayThresholdLevel = (0.5f - thresholdSliderPosition) * range;
        }
        else
        {
            if (thresholdSliderPosition < 0.5f)
                thresholdSliderPosition = 0.5f;
            else if (thresholdSliderPosition > 1.0f)
                thresholdSliderPosition = 1.0f;

            displayThresholdLevel = (thresholdSliderPosition-0.5f) * range;
        }

        //std::cout << "Threshold = " << thresholdLevel << std::endl;

        if (thresholdCoordinator)
        {
            thresholdCoordinator->thresholdChanged(displayThresholdLevel,range);
        }

        canvas->cache->setThreshold(identifier, channel, displayThresholdLevel);
        canvas->cache->setRange(identifier, channel, range);

        repaint();
    }
}


void WaveAxes::mouseExit(const MouseEvent& event)
{
    if (isOverThresholdSlider)
    {
        isOverThresholdSlider = false;
        thresholdColour = Colours::red;
        repaint();
    }
}

float WaveAxes::getDisplayThreshold()
{
    return displayThresholdLevel;
}

void WaveAxes::setDetectorThreshold(float t)
{
    detectorThresholdLevel = t;
}

void WaveAxes::registerThresholdCoordinator(SpikeThresholdCoordinator* stc)
{
    thresholdCoordinator = stc;
}

void WaveAxes::setDisplayThreshold(float threshold)
{

    displayThresholdLevel = threshold;

    repaint();
}

// --------------------------------------------------

ProjectionAxes::ProjectionAxes(SpikeDisplayCanvas* canvas, Projection proj_) : GenericAxes(canvas, PROJECTION_AXES), imageDim(500),
    rangeX(250), rangeY(250), spikesReceivedSinceLastRedraw(0), proj(proj_)
{
    projectionImage = Image(Image::RGB, imageDim, imageDim, true);

    clear();

    n2ProjIdx(proj, &ampDim1, &ampDim2);

}

void ProjectionAxes::setRange(float x, float y)
{
    rangeX = (int) x;
    rangeY = (int) y;

    repaint();
}

void ProjectionAxes::paint(Graphics& g)
{
    g.drawImage(projectionImage,
                0, 0, getWidth(), getHeight(),
                0, imageDim-rangeY, rangeX, rangeY);
}

bool ProjectionAxes::updateSpikeData(const Spike* s)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    int idx1, idx2;
    calcWaveformPeakIdx(s, ampDim1, ampDim2, &idx1, &idx2);

    // add peaks to image
    Colour col;

    if (s->getSortedId() > 0)
        col = colours[(s->getSortedId() - 1) % 8];
    else
        col = Colours::white;

	const float* data = s->getDataPointer();
    updateProjectionImage(data[idx1], data[idx2], 1, col);

    return true;
}

void ProjectionAxes::updateProjectionImage(float x, float y, float gain, Colour col)
{
    Graphics g(projectionImage);

    if (gain != 0)
    {
		float xf = x;
        float yf = float(imageDim) - y; // in microvolts

        g.setColour(col);
        g.fillEllipse(xf,yf,2.0f,2.0f);
    }

}

void ProjectionAxes::calcWaveformPeakIdx(const Spike* s, int d1, int d2, int* idx1, int* idx2)
{

    float max1 = -1*pow(2.0,15);
    float max2 = max1;
	int nSamples = s->getChannelInfo()->getTotalSamples();
	const float* data = s->getDataPointer();

    for (int i = 0; i < nSamples; i++)
    {
        if (data[d1*nSamples + i] > max1)
        {
            *idx1 = d1*nSamples+i;
            max1 = data[*idx1];
        }
        if (data[d2*nSamples+i] > max2)
        {
            *idx2 = d2*nSamples+i;
            max2 = data[*idx2];
        }
    }
}



void ProjectionAxes::clear()
{
    projectionImage.clear(Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
                          Colours::black);

    repaint();
}

void ProjectionAxes::n2ProjIdx(Projection proj, int* p1, int* p2)
{
    int d1, d2;

    if (proj == PROJ1x2)
    {
        d1 = 0;
        d2 = 1;
    }
    else if (proj == PROJ1x3)
    {
        d1 = 0;
        d2 = 2;
    }
    else if (proj == PROJ1x4)
    {
        d1 = 0;
        d2 = 3;
    }
    else if (proj == PROJ2x3)
    {
        d1 = 1;
        d2 = 2;
    }
    else if (proj == PROJ2x4)
    {
        d1 = 1;
        d2 = 3;
    }
    else if (proj == PROJ3x4)
    {
        d1 = 2;
        d2 = 3;
    }
    else
    {
        //std::cout<<"Invalid projection: "<<proj<<"! Cannot determine d1 and d2"<<std::endl;
        *p1 = -1;
        *p2 = -1;
        return;
    }
    *p1 = d1;
    *p2 = d2;
}

// --------------------------------------------------
