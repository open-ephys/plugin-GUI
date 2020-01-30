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
#include "LfpDisplayCanvasElements.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpDisplayOptions.h"

#include <math.h>

using namespace LfpDisplayNodeBeta;

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_) :
     timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f),
    processor(processor_)
{

    nChans = processor->getNumInputs();
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

    viewport = new LfpViewport(this);
    lfpDisplay = new LfpDisplay(this, viewport);
    timescale = new LfpTimescale(this);
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
    resized();
}

void LfpDisplayCanvas::resized()
{

    timescale->setBounds(leftmargin,0,getWidth()-scrollBarThickness-leftmargin,30);
    viewport->setBounds(0,30,getWidth(),getHeight()-90);

    if (lfpDisplay->getSingleChannelState())
        lfpDisplay->setChannelHeight(viewport->getHeight(),false);

    lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness, lfpDisplay->getChannelHeight()*nChans);

    if (optionsDrawerIsOpen)
        options->setBounds(0, getHeight()-200, getWidth(), 200);
    else
        options->setBounds(0, getHeight()-55, getWidth(), 55);

}

void LfpDisplayCanvas::beginAnimation()
{
    std::cout << "Beginning animation." << std::endl;

    displayBufferSize = displayBuffer->getNumSamples();

    for (int i = 0; i < screenBufferIndex.size(); i++)
    {
        screenBufferIndex.set(i,0);
    }

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

    resizeSamplesPerPixelBuffer(nChans);

    sampleRate.clear();
    screenBufferIndex.clear();
    lastScreenBufferIndex.clear();
    displayBufferIndex.clear();

    for (int i = 0; i <= nChans; i++) // extra channel for events
    {
        if (processor->getNumInputs() > 0)
        {
            if (i < nChans) {
                sampleRate.add(processor->getDataChannel(i)->getSampleRate());
            }
            else
            {
                //Since for now the canvas only supports one event channel, find the first TTL one and use that as sampleRate.
                //This is a bit hackish and should be fixed for proper multi-ttl-channel support
                bool got = false;
                for (int c = 0; c < processor->getTotalEventChannels(); c++)
                {
                    if (processor->getEventChannel(c)->getChannelType() == EventChannel::TTL)
                    {
                        sampleRate.add(processor->getEventChannel(c)->getSampleRate());
                        got = true;
                        break;
                    }
                }
                if (!got) {
                  if (i>0) {
                    sampleRate.add(sampleRate[0]);
                    printf("DID NOT FIND AN EVENT CHANNEL. GUESSING SAMPLE RATE FROM OTHER CHANNEL: %g Hz\n", sampleRate[i] + 0.0);
                  } else {
                    sampleRate.add(30000);
                    printf("DID NOT FIND AN EVENT CHANNEL. WILDLY GUESSING SAMPLE RATE: %g Hz\n", sampleRate[i] + 0.0);
                  }
                }
            }
        }
        else
        {
            sampleRate.add(30000);
        }
        
        // std::cout << "Sample rate for ch " << i << " = " << sampleRate[i] << std::endl; 
        displayBufferIndex.add(0);
        screenBufferIndex.add(0);
        lastScreenBufferIndex.add(0);
    }

    if (nChans != lfpDisplay->getNumChannels())
    {
        //std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

        refreshScreenBuffer();

        lfpDisplay->setNumChannels(nChans); // add an extra channel for events

        // update channel names
        for (int i = 0; i < processor->getNumInputs(); i++)
        {
            String chName = processor->getDataChannel(i)->getName();

            //std::cout << chName << std::endl;

            lfpDisplay->channelInfo[i]->setName(chName);
            lfpDisplay->setEnabledState(isChannelEnabled[i], i);

        }

        lfpDisplay->setBounds(0,0,getWidth()-scrollBarThickness*2, lfpDisplay->getTotalHeight());

        resized();
    }
    else
    {
        for (int i = 0; i < processor->getNumInputs(); i++)
        {
            lfpDisplay->channels[i]->updateType();
            lfpDisplay->channelInfo[i]->updateType();
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
    for (int i = 0; i < displayBufferIndex.size(); i++) 
    {
        displayBufferIndex.set(i, processor->getDisplayBufferIndex(i));
        screenBufferIndex.set(i,0);
    }
}

void LfpDisplayCanvas::refreshScreenBuffer()
{
    for (int i = 0; i < screenBufferIndex.size(); i++)
        screenBufferIndex.set(i,0);

    screenBuffer->clear();
    screenBufferMin->clear();
    screenBufferMean->clear();
    screenBufferMax->clear();

}

void LfpDisplayCanvas::updateScreenBuffer()
{         
  // copy new samples from the displayBuffer into the screenBuffer
  int maxSamples = lfpDisplay->getWidth() - leftmargin;

  ScopedLock displayLock(*processor->getMutex());

  int triggerTime = processor->getTriggerSource()>=0 ? processor->getLatestTriggerTime() : -1;
  processor->acknowledgeTrigger();
  for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
    {
      float ratio = sampleRate[channel] * timebase / float(getWidth() - leftmargin - scrollBarThickness); // samples / pixel
      // this number is crucial: converting from samples to values (in px) for the screen buffer
      if (screenBufferIndex[channel] >= maxSamples) // wrap around if we reached right edge before
        {
          if (processor->getTriggerSource()>=0) {
            // we may need to wait for a trigger
            if (triggerTime>=0) {
              const int screenThird = int(maxSamples * ratio / 4);
              const int dispBufLim = displayBufferSize / 2;
              int t0 = triggerTime - std::min(screenThird, dispBufLim);
              if (t0 < 0)
                t0 += displayBufferSize;
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

      int nSamples =  index - dbi; // N new samples (not pixels) to be added to displayBufferIndex

      if (nSamples < 0) // buffer has reset to 0
        // not sure if following is still true: -- xxx 2do bug: this shouldnt happen because it makes the range/histogram display not work properly/look off for one pixel
        {
          nSamples += displayBufferSize;
          //  std::cout << "nsamples 0 " ;
        }

      //if (channel == 15 || channel == 16)
      //     std::cout << channel << " " << sbi << " " << dbi << " " << nSamples << std::endl;

      int valuesNeeded(nSamples / ratio); // N pixels needed for this update

      if (sbi + valuesNeeded > maxSamples)  // crop number of samples to fit canvas width
        {
          valuesNeeded = maxSamples - sbi;
        }
      float subSampleOffset = 0.0;

      //        dbi %= displayBufferSize; // make sure we're not overshooting

      // if (channel == 0)
      //     std::cout << "Channel " 
      //               << channel << " : " 
      //               << sbi << " : " 
      //               << index << " : " 
      //               << dbi << " : " 
      //               << valuesNeeded << " : " 
      //               << ratio 
      //                             << std::endl;
        
      if (valuesNeeded > 0 && valuesNeeded < 1000000)
        {
          for (int i = 0; i < valuesNeeded; i++) // also fill one extra sample for line drawing interpolation to match across draws
            {
              //If paused don't update screen buffers, but update all indexes as needed
              if (!lfpDisplay->isPaused)
                {
                  float gain = 1.0;

                  screenBuffer->clear(channel, sbi, 1);
                  screenBufferMin->clear(channel, sbi, 1);
                  screenBufferMax->clear(channel, sbi, 1);

                  // dbi %= displayBufferSize; // just to be sure

                  // same thing again, but this time add the min,mean, and max of all samples in current pixel
                  float sample_min   =  10000000;
                  float sample_max   = -10000000;
                    
                  int nextpix = dbi + int(ceil(ratio));
                  if (nextpix > displayBufferSize)
                    nextpix = displayBufferSize;

                  if (nextpix - dbi > 1) {
                    // multiple samples, calculate average
                    float sum = 0;
                    for (int j = dbi; j < nextpix; j++)
                      sum += displayBuffer->getSample(channel, j);
                    screenBuffer->addSample(channel, sbi, sum*gain / (nextpix - dbi));
                  } else {
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
                    
                  for (int j = dbi; j < nextpix; j++)
                    {
                        
                      float sample_current = displayBuffer->getSample(channel, j);
                      //sample_mean = sample_mean + sample_current;

                      if (sample_min>sample_current)
                        {
                          sample_min=sample_current;
                        }

                      if (sample_max<sample_current)
                        {
                          sample_max=sample_current;
                        }
                       
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
                          samplesPerPixel[channel][sbi][c]=sample_current;
                          c++;
                        }
                      if (c>0){
                        sampleCountPerPixel[sbi]=c-1; // save count of samples for this pixel
                      }else{
                        sampleCountPerPixel[sbi]=0;
                      }
                      //sample_mean = sample_mean/c;
                      //screenBufferMean->addSample(channel, sbi, sample_mean*gain);
                            
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

bool LfpDisplayCanvas::getDrawMethodState()
{
    
    return options->getDrawMethodState(); //drawMethodButton->getToggleState();
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
            g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,3.0f);
        else
            g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);


}

void LfpDisplayCanvas::refresh()
{

    updateScreenBuffer();

    lfpDisplay->refresh(); // redraws only the new part of the screen buffer

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
}


void LfpDisplayCanvas::loadVisualizerParameters(XmlElement* xml)
{
    options->loadParameters(xml);

}
