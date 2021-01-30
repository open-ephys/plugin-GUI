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
#include "LfpDisplayNode.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"
#include "LfpChannelColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpDisplayCanvas -

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_) :
     timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f), 
    processor(processor_)
{

    nChans = processor->getNumSubprocessorChannels();

    displayBuffer = processor->getDisplayBufferAddress();

    if (displayBuffer != nullptr)
        displayBufferSize = displayBuffer->getNumSamples();

    screenBuffer = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
    screenBuffer->clear();

    screenBufferMin = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
    screenBufferMin->clear();
    screenBufferMean = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
    screenBufferMean->clear();
    screenBufferMax = new AudioSampleBuffer(MAX_N_CHAN, MAX_N_SAMP);
    screenBufferMax->clear();

    viewport = new LfpViewport(this);
    lfpDisplay = new LfpDisplay(this, viewport);
    timescale = new LfpTimescale(this, lfpDisplay);
    options = new LfpDisplayOptions(this, timescale, lfpDisplay, processor);

    lfpDisplay->options = options;

    timescale->setTimebase(timebase);

    viewport->setViewedComponent(lfpDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    isChannelEnabled.insertMultiple(0,true,10000); // max 10k channels

    //viewport->getVerticalScrollBar()->addListener(this->scrollBarMoved(viewport->getVerticalScrollBar(), 1.0));

    addAndMakeVisible(viewport);
    addAndMakeVisible(timescale);
    addAndMakeVisible(options);

    lfpDisplay->setNumChannels(nChans);

    resizeSamplesPerPixelBuffer(nChans);

    TopLevelWindow::getTopLevelWindow(0)->addKeyListener(this);

    optionsDrawerIsOpen = false;

}

LfpDisplayCanvas::~LfpDisplayCanvas()
{

    // de-allocate 3d-array samplesPerPixel [nChans][MAX_N_SAMP][MAX_N_SAMP_PER_PIXEL];

    //for(int i=0;i<nChans;i++)
    //{
    //    for(int j=0;j<MAX_N_SAMP;j++)
    //    {
    //        free(samplesPerPixel[i][j]);
    //    }
    //    free(samplesPerPixel[i]);
    // }
    // free(samplesPerPixel);

    samplesPerPixel.clear();
    
    TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

void LfpDisplayCanvas::resizeSamplesPerPixelBuffer(int numCh)
{
    // allocate samplesPerPixel, behaves like float samplesPerPixel[nChans][MAX_N_SAMP][MAX_N_SAMP_PER_PIXEL]
    //samplesPerPixel = (float***)malloc(nChans * sizeof(float **));

    // 3D array: dimensions channels x samples x samples per pixel
    samplesPerPixel.clear();
    samplesPerPixel.resize(numCh);

    //for(int i = 0; i < numCh; i++)
    //{
        //std::vector< std::vector<float>> v1;
    //    samplesPerPixel[i].resize(MAX_N_SAMP);
        //samplesPerPixel.push_back(v1);
        //samplesPerPixel[i] = (float**)malloc(MAX_N_SAMP * sizeof(float*));

    //    for(int j = 0; j < MAX_N_SAMP; j++)
    //    {
            //std::vector<float> v2;
            //v2.resize(MAX_N_SAMP_PER_PIXEL);
    //        samplesPerPixel[i][j].resize(MAX_N_SAMP_PER_PIXEL);
    //        //samplesPerPixel[i][j] = (float*)malloc(MAX_N_SAMP_PER_PIXEL*sizeof(float));
    //    }
   //}
}

void LfpDisplayCanvas::toggleOptionsDrawer(bool isOpen)
{
    optionsDrawerIsOpen = isOpen;
    auto viewportPosition = viewport->getViewPositionY();   // remember viewport position
    resized();
    viewport->setViewPosition(0, viewportPosition);         // return viewport position
}

void LfpDisplayCanvas::resized()
{

    timescale->setBounds(leftmargin,0,getWidth()-scrollBarThickness-leftmargin,30);
    viewport->setBounds(0,30,getWidth(),getHeight()-90);

    if (nChans > 0)
    {
        if (lfpDisplay->getSingleChannelState())
            lfpDisplay->setChannelHeight(viewport->getHeight(),false);
        
        lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness, lfpDisplay->getChannelHeight()*lfpDisplay->drawableChannels.size());
    }
    else
    {
        lfpDisplay->setBounds(0, 0, getWidth(), getHeight());
    }

    if (optionsDrawerIsOpen)
        options->setBounds(0, getHeight()-200, getWidth(), 200);
    else
        options->setBounds(0, getHeight()-55, getWidth(), 55);

}

void LfpDisplayCanvas::resizeToChannels(bool respectViewportPosition)
{
    lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness, lfpDisplay->getChannelHeight()*lfpDisplay->drawableChannels.size());
    
    // if param is flagged, move the viewport scroll back to same relative position before
    // resize took place
    if (!respectViewportPosition) return;
    
    // get viewport scroll position as ratio against lfpDisplay's dims
    // so that we can set scrollbar back to rough position before resize
    // (else viewport scrolls back to top after resize)
    const double yPositionRatio = viewport->getViewPositionY() / (double)lfpDisplay->getHeight();
    const double xPositionRatio = viewport->getViewPositionX() / (double)lfpDisplay->getWidth();
    
    viewport->setViewPosition(lfpDisplay->getWidth() * xPositionRatio,
                              lfpDisplay->getHeight() * yPositionRatio);
}

void LfpDisplayCanvas::beginAnimation()
{

    if (displayBuffer != nullptr)
    {

        displayBufferSize = displayBuffer->getNumSamples();

        for (int i = 0; i < screenBufferIndex.size(); i++)
        {
            screenBufferIndex.set(i, 0);
        }

        startCallbacks();
    }    
}

void LfpDisplayCanvas::endAnimation()
{
    if (true)
    {

        stopCallbacks();
    }
}

void LfpDisplayCanvas::update()
{

    if (displayBuffer != nullptr)
        displayBufferSize = displayBuffer->getNumSamples();

    nChans = jmax(processor->getNumSubprocessorChannels(), 0);

    resizeSamplesPerPixelBuffer(nChans);

    sampleRate = 30000; // default

    for (auto* arr : { &screenBufferIndex, &lastScreenBufferIndex, &displayBufferIndex })
    {
        arr->clearQuick();
        arr->insertMultiple(0, 0, nChans + 1); // extra channel for events
    }
    
    options->setEnabled(nChans != 0);
    // must manually ensure that overlapSelection propagates up to canvas
    channelOverlapFactor = options->selectedOverlapValue.getFloatValue();

    int firstChannelInSubprocessor = 0;
    for (int i = 0, nInputs = processor->getNumInputs(); i < nInputs; i++)
    {

        if (processor->getDataSubprocId(i) == drawableSubprocessor)
        {
            sampleRate = processor->getDataChannel(i)->getSampleRate();
            firstChannelInSubprocessor = i;
            break;
        }

    }

    if (nChans != lfpDisplay->getNumChannels())
    {
        refreshScreenBuffer();

        if (nChans > 0)
            lfpDisplay->setNumChannels(nChans);

        // update channel names
        //std::cout << "Updating channel names" << std::endl;
        for (int i = 0; i < nChans; i++)
        {
            String chName = processor->getDataChannel(firstChannelInSubprocessor + i)->getName();
            lfpDisplay->channelInfo[i]->setName(chName);
            lfpDisplay->setEnabledState(isChannelEnabled[i], i);
        }
        
        if (nChans == 0) lfpDisplay->setBounds(0, 0, getWidth(), getHeight());
        else {
            lfpDisplay->rebuildDrawableChannelsList();
            lfpDisplay->setBounds(0, 0, getWidth()-scrollBarThickness*2, lfpDisplay->getTotalHeight());
        }
        
        resized();
    }
    else
    {
        for (int i = 0; i < nChans; i++)
        {
            String chName = processor->getDataChannel(firstChannelInSubprocessor + i)->getName();
            lfpDisplay->channelInfo[i]->setName(chName);
            lfpDisplay->channels[i]->updateType();
            lfpDisplay->channelInfo[i]->updateType();
        }
        
        if (nChans > 0)
        {
            lfpDisplay->rebuildDrawableChannelsList();
        }
    }
}

int LfpDisplayCanvas::getChannelHeight()
{
    //return spreads[spreadSelection->getSelectedId()-1].getIntValue();
    return options->getChannelHeight();
    
}

void LfpDisplayCanvas::setParameter(int param, float val)
{
    // not used for anything, since LfpDisplayCanvas is not a processor
}

void LfpDisplayCanvas::refreshState()
{
    // called when the component's tab becomes visible again

    if (true)
    {
        for (int i = 0; i <= displayBufferIndex.size(); i++) // include event channel
        {

            displayBufferIndex.set(i, processor->getDisplayBufferIndex(i));
            screenBufferIndex.set(i, 0);
        }
    }

}

void LfpDisplayCanvas::refreshScreenBuffer()
{
    if (true)
    {
        for (int i = 0; i < screenBufferIndex.size(); i++)
            screenBufferIndex.set(i, 0);

        screenBuffer->clear();
        screenBufferMin->clear();
        screenBufferMean->clear();
        screenBufferMax->clear();
    }

}

void LfpDisplayCanvas::updateScreenBuffer()
{
    if (true)
    {
        // copy new samples from the displayBuffer into the screenBuffer
        int maxSamples = lfpDisplay->getWidth() - leftmargin;

        ScopedLock displayLock(*processor->getMutex());

        int triggerTime = processor->getTriggerSource()>=0 
                          ? processor->getLatestTriggerTime() 
                          : -1;
        processor->acknowledgeTrigger();
                
        for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
        {
            float ratio = sampleRate * timebase / float(getWidth() - leftmargin - scrollBarThickness); // samples / pixel
            // this number is crucial: converting from samples to values (in px) for the screen buffer

            //if (channel == 0)
            //    std::cout << sampleRate[channel] << std::endl;

            if (screenBufferIndex[channel] >= maxSamples) // wrap around if we reached right edge before
            {
                if (processor->getTriggerSource()>=0)
                {
                    // we may need to wait for a trigger
                    if (triggerTime>=0)
                    {
                        const int screenThird = int(maxSamples * ratio / 4);
                        const int dispBufLim = displayBufferSize / 2;
                        int t0 = triggerTime - std::min(screenThird, dispBufLim);
                        if (t0 < 0)
                        {
                            t0 += displayBufferSize;
                        }
                        displayBufferIndex.set(channel, t0); // fast-forward
                    } else {
                        return; // don't display right now
                    }
                }
                
                screenBufferIndex.set(channel, 0);
            }
            // hold these values locally for each channel - is this a good idea?
            int sbi = screenBufferIndex[channel];
            int dbi = displayBufferIndex[channel];

            

            lastScreenBufferIndex.set(channel, sbi);

            int index = processor->getDisplayBufferIndex(channel);

            int nSamples = index - dbi; // N new samples (not pixels) to be added to displayBufferIndex

            if (nSamples < 0)
            {
                nSamples += displayBufferSize;
                //  std::cout << "nsamples 0 " ;
            }

            //if (channel == 15 || channel == 16)
            //     std::cout << channel << " " << sbi << " " << dbi << " " << nSamples << std::endl;

            int valuesNeeded = (int) float(nSamples) / ratio; // N pixels needed for this update

            if (sbi + valuesNeeded > maxSamples)  // crop number of samples to fit canvas width
            {
                valuesNeeded = maxSamples - sbi;
            }
            float subSampleOffset = 0.0;

            //         if (channel == 0)
            //             std::cout << "Channel " 
            //                       << channel << " : " 
            //                       << sbi << " : " 
            //                       << index << " : " 
            //                       << dbi << " : " 
            //                       << valuesNeeded << " : " 
            //                       << ratio 
            //                                     << std::endl;

            if (valuesNeeded > 0 && valuesNeeded < 1000000)
            {
                for (int i = 0; i < valuesNeeded; i++) // also fill one extra sample for line drawing interpolation to match across draws
                {
                    //If paused don't update screen buffers, but update all indexes as needed
                    if (!lfpDisplay->isPaused)
                    {
                        float gain = 1.0;

                        screenBuffer->clear(channel, sbi, 1);
                        screenBufferMean->clear(channel, sbi, 1);
                        screenBufferMin->clear(channel, sbi, 1);
                        screenBufferMax->clear(channel, sbi, 1);

                        // update continuous data channels
                        int nextpix = dbi + int(ceil(ratio));
                        if (nextpix > displayBufferSize)
                            nextpix = displayBufferSize;

                        if (nextpix - dbi > 1) 
                        {
                            // multiple samples, calculate average
                            float sum = 0;
                            for (int j = dbi; j < nextpix; j++)
                                sum += displayBuffer->getSample(channel, j);

                            screenBuffer->addSample(channel, sbi, sum*gain / (nextpix - dbi));
                        } 
                        else
                        {
                            // interpolate between two samples with invAlpha and alpha
                            /* This is only reasonable if there are more pixels
                            than samples. Otherwise, we should calculate average. */
                            float alpha = (float) subSampleOffset;
                            float invAlpha = 1.0f - alpha;

                            float val0 = displayBuffer->getSample(channel, dbi);
                            float val1 = displayBuffer->getSample(channel, (dbi+1)%displayBufferSize);

                            float val = invAlpha * val0  + alpha * val1;

                            screenBuffer->addSample(channel, sbi, val*gain);
                        }
                        

                        // same thing again, but this time add the min,mean, and max of all samples in current pixel
                        float sample_min = 10000000;
                        float sample_max = -10000000;
                        float sample_mean = 0;

                        for (int j = dbi; j < nextpix; j++)
                        {

                            float sample_current = displayBuffer->getSample(channel, j);
                            sample_mean = sample_mean + sample_current;

                            if (sample_min > sample_current)
                            {
                                sample_min = sample_current;
                            }

                            if (sample_max < sample_current)
                            {
                                sample_max = sample_current;
                            }

                        }

                        // update event channel
                        if (channel == nChans)
                        {
                            //std::cout << sample_max << std::endl;
                            screenBuffer->setSample(channel, sbi, sample_max);
                        }

                        // similarly, for each pixel on the screen, we want a list of all values so we can draw a histogram later
                        // for simplicity, we'll just do this as 2d array, samplesPerPixel[px][samples]
                        // with an additional array sampleCountPerPixel[px] that holds the N samples per pixel
                        if (channel < nChans) // we're looping over one 'extra' channel for events above, so make sure not to loop over that one here
                        {
                            int c = 0;
                            for (int j = dbi; j < nextpix && c < MAX_N_SAMP_PER_PIXEL; j++)
                            {
                                float sample_current = displayBuffer->getSample(channel, j);
                                samplesPerPixel[channel][sbi][c] = sample_current;
                                c++;
                            }
                            if (c > 0){
                                sampleCountPerPixel[sbi] = c - 1; // save count of samples for this pixel
                            }
                            else{
                                sampleCountPerPixel[sbi] = 0;
                            }
                            sample_mean = sample_mean / c;
                            screenBufferMean->addSample(channel, sbi, sample_mean*gain);

                            screenBufferMin->addSample(channel, sbi, sample_min*gain);
                            screenBufferMax->addSample(channel, sbi, sample_max*gain);
                        }
                        sbi++;
                    }

                    subSampleOffset += ratio;

                    int steps(floor(subSampleOffset));
                    dbi = (dbi + steps) % displayBufferSize;
                    subSampleOffset -= steps;

                }

                // update values after we're done
                screenBufferIndex.set(channel, sbi);
                displayBufferIndex.set(channel, dbi);
            }

        }
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

int LfpDisplayCanvas::getNumChannelsVisible()
{
    return lfpDisplay->drawableChannels.size();
}

int LfpDisplayCanvas::getChannelSubprocessorIdx(int channel)
{
    return processor->getDataChannel(channel)->getSubProcessorIdx();
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

std::array<float, MAX_N_SAMP_PER_PIXEL> LfpDisplayCanvas::getSamplesPerPixel(int chan, int px)
{
    return samplesPerPixel[chan][px];
}
const int LfpDisplayCanvas::getSampleCountPerPixel(int px)
{
    return sampleCountPerPixel[px];
}

float LfpDisplayCanvas::getMean(int chan)
{
    float total = 0.0f;
    float numPts = 0;

    float sample = 0.0f;
    for (int samp = 0; samp < (lfpDisplay->getWidth() - leftmargin); samp += 10)
    {
        sample = *screenBuffer->getReadPointer(chan, samp);
        total += sample;
        numPts++;
    }

    //std::cout << sample << std::endl;

    return total / numPts;
}

float LfpDisplayCanvas::getStd(int chan)
{
    float std = 0.0f;

    float mean = getMean(chan);
    float numPts = 1;

    for (int samp = 0; samp < (lfpDisplay->getWidth() - leftmargin); samp += 10)
    {
        std += pow((*screenBuffer->getReadPointer(chan, samp) - mean),2);
        numPts++;
    }

    return sqrt(std / numPts);

}

bool LfpDisplayCanvas::getInputInvertedState()
{
    return options->getInputInvertedState(); //invertInputButton->getToggleState();
}

bool LfpDisplayCanvas::getDisplaySpikeRasterizerState()
{
    return options->getDisplaySpikeRasterizerState();
}

bool LfpDisplayCanvas::getDrawMethodState()
{
    
    return options->getDrawMethodState(); //drawMethodButton->getToggleState();
}

void LfpDisplayCanvas::setDrawableSampleRate(float samplerate)
{
//    std::cout << "setting the drawable sample rate in the canvas" << std::endl;
    displayedSampleRate = samplerate;
}

void LfpDisplayCanvas::setDrawableSubprocessor(uint32 sp)
{
    drawableSubprocessor = sp;
    displayBuffer = processor->getDisplayBufferAddress();
    update();
}

void LfpDisplayCanvas::redraw()
{
    fullredraw=true;
    repaint();
    refresh();
}

void LfpDisplayCanvas::paint(Graphics& g)
{
    
    //std::cout << "Painting" << std::endl;

    //g.setColour(Colour(0,0,0)); // for high-precision per-pixel density display, make background black for better visibility
    g.setColour(lfpDisplay->backgroundColour); //background color
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
            g.drawLine(w/10*i+leftmargin,timescale->getHeight(),w/10*i+leftmargin,getHeight()-60-timescale->getHeight(),3.0f);
        else
            g.drawLine(w/10*i+leftmargin,timescale->getHeight(),w/10*i+leftmargin,getHeight()-60-timescale->getHeight(),1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);

}

void LfpDisplayCanvas::refresh()
{
    if (true)
    { 
    updateScreenBuffer();

    lfpDisplay->refresh(); // redraws only the new part of the screen buffer
    }
}

bool LfpDisplayCanvas::keyPressed(const KeyPress& key)
{
    if (key.getKeyCode() == key.spaceKey)
    {
        options->togglePauseButton();
        
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

    options->saveParameters(xml);
    LfpDisplayEditor* ed = (LfpDisplayEditor*) processor->getEditor();
    ed->saveVisualizerParameters(xml);
}

void LfpDisplayCanvas::loadVisualizerParameters(XmlElement* xml)
{
    options->loadParameters(xml);
    LfpDisplayEditor* ed = (LfpDisplayEditor*) processor->getEditor();
    ed->loadVisualizerParameters(xml);
}

