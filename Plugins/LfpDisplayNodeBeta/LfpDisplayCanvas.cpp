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

    for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
    {
        if (screenBufferIndex[channel] >= maxSamples) // wrap around if we reached right edge before
            screenBufferIndex.set(channel, 0);

         // hold these values locally for each channel - is this a good idea?
        int sbi = screenBufferIndex[channel];
        int dbi = displayBufferIndex[channel];
        
        lastScreenBufferIndex.set(channel, sbi);

        int index = processor->getDisplayBufferIndex(channel);

        int nSamples =  index - dbi; // N new samples (not pixels) to be added to displayBufferIndex

        if (nSamples < 0) // buffer has reset to 0 -- xxx 2do bug: this shouldnt happen because it makes the range/histogram display not work properly/look off for one pixel
        {
          nSamples += displayBufferSize;
           //  std::cout << "nsamples 0 " ;
        }

        //if (channel == 15 || channel == 16)
        //     std::cout << channel << " " << sbi << " " << dbi << " " << nSamples << std::endl;


        float ratio = sampleRate[channel] * timebase / float(getWidth() - leftmargin - scrollBarThickness); // samples / pixel
        // this number is crucial: converting from samples to values (in px) for the screen buffer
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

// =============================================================


ShowHideOptionsButton::ShowHideOptionsButton(LfpDisplayOptions* options) : Button("Button")
{
    setClickingTogglesState(true);
}
ShowHideOptionsButton::~ShowHideOptionsButton()
{

}

void ShowHideOptionsButton::paintButton(Graphics& g, bool, bool) 
{   
    g.setColour(Colours::white);

    Path p;

    float h = getHeight();
    float w = getWidth();

    if (getToggleState())
    {
        p.addTriangle(0.5f*w, 0.2f*h,
                      0.2f*w, 0.8f*h,
                      0.8f*w, 0.8f*h);
    }
    else
    {
        p.addTriangle(0.8f*w, 0.8f*h,
                      0.2f*w, 0.5f*h,
                      0.8f*w, 0.2f*h);
    }

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    g.strokePath(p, pst);
}


// -------------------------------------------------------------

LfpDisplayOptions::LfpDisplayOptions(LfpDisplayCanvas* canvas_, LfpTimescale* timescale_, 
                                     LfpDisplay* lfpDisplay_, LfpDisplayNode* processor_)
    : canvas(canvas_), lfpDisplay(lfpDisplay_), timescale(timescale_), processor(processor_),
      selectedChannelType(DataChannel::HEADSTAGE_CHANNEL)
{
 //Ranges for neural data
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("25");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("50");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("100");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("250");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("400");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("500");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("750");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("1000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("2000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("5000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("10000");
    voltageRanges[DataChannel::HEADSTAGE_CHANNEL].add("15000");
    selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] = 8;
    rangeGain[DataChannel::HEADSTAGE_CHANNEL] = 1; //uV
    rangeSteps[DataChannel::HEADSTAGE_CHANNEL] = 10;
    rangeUnits.add("uV");
    typeNames.add("DATA");

    UtilityButton* tbut;
    tbut = new UtilityButton("DATA",Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false,false,false,false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100,dontSendNotification);
    tbut->setToggleState(true,dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);
    
    //Ranges for AUX/accelerometer data
    voltageRanges[DataChannel::AUX_CHANNEL].add("25");
    voltageRanges[DataChannel::AUX_CHANNEL].add("50");
    voltageRanges[DataChannel::AUX_CHANNEL].add("100");
    voltageRanges[DataChannel::AUX_CHANNEL].add("250");
    voltageRanges[DataChannel::AUX_CHANNEL].add("400");
    voltageRanges[DataChannel::AUX_CHANNEL].add("500");
    voltageRanges[DataChannel::AUX_CHANNEL].add("750");
    voltageRanges[DataChannel::AUX_CHANNEL].add("1000");
    voltageRanges[DataChannel::AUX_CHANNEL].add("2000");
    //voltageRanges[DataChannel::AUX_CHANNEL].add("5000");
    selectedVoltageRange[DataChannel::AUX_CHANNEL] = 9;
    rangeGain[DataChannel::AUX_CHANNEL] = 0.001; //mV
    rangeSteps[DataChannel::AUX_CHANNEL] = 10;
    rangeUnits.add("mV");
    typeNames.add("AUX");
    
    tbut = new UtilityButton("AUX",Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false,false,false,false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100,dontSendNotification);
    tbut->setToggleState(false,dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);

    //Ranges for ADC data
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.01");
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.05");
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.1");
     voltageRanges[DataChannel::ADC_CHANNEL].add("0.5");
     voltageRanges[DataChannel::ADC_CHANNEL].add("1.0");
     voltageRanges[DataChannel::ADC_CHANNEL].add("2.0");
     voltageRanges[DataChannel::ADC_CHANNEL].add("5.0");
     voltageRanges[DataChannel::ADC_CHANNEL].add("10.0");
     selectedVoltageRange[DataChannel::ADC_CHANNEL] = 8;
     rangeGain[DataChannel::ADC_CHANNEL] = 1; //V
     rangeSteps[DataChannel::ADC_CHANNEL] = 0.1; //in V
    rangeUnits.add("V");
    typeNames.add("ADC");

    tbut = new UtilityButton("ADC",Font("Small Text", 9, Font::plain));
    tbut->setEnabledState(true);
    tbut->setCorners(false,false,false,false);
    tbut->addListener(this);
    tbut->setClickingTogglesState(true);
    tbut->setRadioGroupId(100,dontSendNotification);
    tbut->setToggleState(false,dontSendNotification);
    addAndMakeVisible(tbut);
    typeButtons.add(tbut);

    selectedVoltageRangeValues[DataChannel::HEADSTAGE_CHANNEL] = voltageRanges[DataChannel::HEADSTAGE_CHANNEL][selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] - 1];
    selectedVoltageRangeValues[DataChannel::AUX_CHANNEL] = voltageRanges[DataChannel::AUX_CHANNEL][selectedVoltageRange[DataChannel::AUX_CHANNEL] - 1];
    selectedVoltageRangeValues[DataChannel::ADC_CHANNEL] = voltageRanges[DataChannel::ADC_CHANNEL][selectedVoltageRange[DataChannel::ADC_CHANNEL] - 1];

    showHideOptionsButton = new ShowHideOptionsButton(this);
    showHideOptionsButton->addListener(this);
    addAndMakeVisible(showHideOptionsButton);

    timebases.add("0.25");
    timebases.add("0.5");
    timebases.add("1.0");
    timebases.add("2.0");
    timebases.add("3.0");
    timebases.add("4.0");
    timebases.add("5.0");
    timebases.add("10.0");
    timebases.add("20.0");
    selectedTimebase = 4;
    selectedTimebaseValue = timebases[selectedTimebase-1];

    spreads.add("10");
    spreads.add("20");
    spreads.add("30");
    spreads.add("40");
    spreads.add("50");
    spreads.add("60");
    spreads.add("70");
    spreads.add("80");
    spreads.add("90");
    spreads.add("100");
    selectedSpread = 5;
    selectedSpreadValue = spreads[selectedSpread-1];


    overlaps.add("0.5");
    overlaps.add("0.75");
    overlaps.add("1");
    overlaps.add("2");
    overlaps.add("3");
    overlaps.add("4");
    overlaps.add("5");
    selectedOverlap = 4;
    selectedOverlapValue = overlaps[selectedOverlap-1];

    saturationThresholds.add("0.5");
    saturationThresholds.add("100");
    saturationThresholds.add("1000");
    saturationThresholds.add("5000");
    saturationThresholds.add("6389");
    
    selectedSaturation = 5;
    selectedSaturationValue = saturationThresholds[selectedSaturation-1];
    
    
    colorGroupings.add("1");
    colorGroupings.add("2");
    colorGroupings.add("4");
    colorGroupings.add("8");
    colorGroupings.add("16");


    rangeSelection = new ComboBox("Voltage range");
    rangeSelection->addItemList(voltageRanges[DataChannel::HEADSTAGE_CHANNEL], 1);
    rangeSelection->setSelectedId(selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL], sendNotification);
    rangeSelection->setEditableText(true);
    rangeSelection->addListener(this);
    addAndMakeVisible(rangeSelection);


    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(selectedTimebase, sendNotification);
    timebaseSelection->setEditableText(true);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);


    spreadSelection = new ComboBox("Spread");
    spreadSelection->addItemList(spreads, 1);
    spreadSelection->setSelectedId(selectedSpread,sendNotification);
    spreadSelection->addListener(this);
    spreadSelection->setEditableText(true);
    addAndMakeVisible(spreadSelection);

    overlapSelection = new ComboBox("Overlap");
    overlapSelection->addItemList(overlaps, 1);
    overlapSelection->setSelectedId(selectedOverlap,sendNotification);
    overlapSelection->addListener(this);
    overlapSelection->setEditableText(true);
    addAndMakeVisible(overlapSelection);
    
    saturationWarningSelection = new ComboBox("Sat.Warn");
    saturationWarningSelection->addItemList(saturationThresholds, 1);
    saturationWarningSelection->setSelectedId(selectedSaturation,sendNotification);
    saturationWarningSelection->addListener(this);
    saturationWarningSelection->setEditableText(true);
    addAndMakeVisible(saturationWarningSelection);
    
    
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
    
    // two sliders for the two histogram components of the supersampled plotting mode
    // todo: rename these
    brightnessSliderA = new Slider;
    brightnessSliderA->setRange (0, 1);
    brightnessSliderA->setTextBoxStyle(Slider::NoTextBox, false, 50,30);
    brightnessSliderA->addListener(this);
    addAndMakeVisible (brightnessSliderA);
    
    brightnessSliderB = new Slider;
    brightnessSliderB->setRange (0, 1);
    brightnessSliderB->setTextBoxStyle(Slider::NoTextBox, false, 50,30);
    brightnessSliderB->addListener(this);
    addAndMakeVisible (brightnessSliderB);
    
    sliderALabel = new Label("Brightness","Brightness");
    sliderALabel->setFont(Font("Small Text", 13, Font::plain));
    sliderALabel->setColour(Label::textColourId,Colour(150,150,150));
    addAndMakeVisible(sliderALabel);
    
    sliderBLabel = new Label("Min. brightness","Min. brightness");
    sliderBLabel->setFont(Font("Small Text", 13, Font::plain));
    sliderBLabel->setColour(Label::textColourId,Colour(150,150,150));
    addAndMakeVisible(sliderBLabel);
    
    
    //ScopedPointer<UtilityButton> drawClipWarningButton; // optinally draw (subtle) warning if data is clipped in display
    drawClipWarningButton = new UtilityButton("0", Font("Small Text", 13, Font::plain));
    drawClipWarningButton->setRadius(5.0f);
    drawClipWarningButton->setEnabledState(true);
    drawClipWarningButton->setCorners(true, true, true, true);
    drawClipWarningButton->addListener(this);
    drawClipWarningButton->setClickingTogglesState(true);
    drawClipWarningButton->setToggleState(false, sendNotification);
    addAndMakeVisible(drawClipWarningButton);
    
    
    //ScopedPointer<UtilityButton> drawSaturateWarningButton; // optionally raise hell if the actual data is saturating
    drawSaturateWarningButton = new UtilityButton("0", Font("Small Text", 13, Font::plain));
    drawSaturateWarningButton->setRadius(5.0f);
    drawSaturateWarningButton->setEnabledState(true);
    drawSaturateWarningButton->setCorners(true, true, true, true);
    drawSaturateWarningButton->addListener(this);
    drawSaturateWarningButton->setClickingTogglesState(true);
    drawSaturateWarningButton->setToggleState(false, sendNotification);
    addAndMakeVisible(drawSaturateWarningButton);
    

    //button for pausing the display - works by skipping buffer updates. This way scrolling etc still works
    pauseButton = new UtilityButton("Pause", Font("Small Text", 13, Font::plain));
    pauseButton->setRadius(5.0f);
    pauseButton->setEnabledState(true);
    pauseButton->setCorners(true, true, true, true);
    pauseButton->addListener(this);
    pauseButton->setClickingTogglesState(true);
    pauseButton->setToggleState(false, sendNotification);
    addAndMakeVisible(pauseButton);

    // add event display-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 8; i++)
    {

        EventDisplayInterface* eventOptions = new EventDisplayInterface(lfpDisplay, canvas, i);
        eventDisplayInterfaces.add(eventOptions);
        addAndMakeVisible(eventOptions);
        eventOptions->setBounds(500+(floor(i/2)*20), getHeight()-20-(i%2)*20, 40, 20);

        lfpDisplay->setEventDisplayState(i,true);

    }

    lfpDisplay->setRange(voltageRanges[DataChannel::HEADSTAGE_CHANNEL][selectedVoltageRange[DataChannel::HEADSTAGE_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::HEADSTAGE_CHANNEL]
        , DataChannel::HEADSTAGE_CHANNEL);
    lfpDisplay->setRange(voltageRanges[DataChannel::ADC_CHANNEL][selectedVoltageRange[DataChannel::ADC_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::ADC_CHANNEL]
        , DataChannel::ADC_CHANNEL);
    lfpDisplay->setRange(voltageRanges[DataChannel::AUX_CHANNEL][selectedVoltageRange[DataChannel::AUX_CHANNEL] - 1].getFloatValue()*rangeGain[DataChannel::AUX_CHANNEL]
        , DataChannel::AUX_CHANNEL);
    
}

LfpDisplayOptions::~LfpDisplayOptions()
{

}

void LfpDisplayOptions::resized()
{
    rangeSelection->setBounds(5,getHeight()-30,80,25);
    timebaseSelection->setBounds(175,getHeight()-30,60,25);
    
    spreadSelection->setBounds(5,getHeight()-90,60,25);
    
    overlapSelection->setBounds(100,getHeight()-90,60,25);

    drawClipWarningButton->setBounds(175,getHeight()-89,20,20);
    drawSaturateWarningButton->setBounds(325, getHeight()-89, 20, 20);
    
    colorGroupingSelection->setBounds(400,getHeight()-90,60,25);

    invertInputButton->setBounds(35,getHeight()-180,100,22);
    drawMethodButton->setBounds(35,getHeight()-150,100,22);

    pauseButton->setBounds(450,getHeight()-50,50,44);

    saturationWarningSelection->setBounds(250, getHeight()-90, 60, 25);
    
    
    for (int i = 0; i < 8; i++)
    {
        eventDisplayInterfaces[i]->setBounds(300+(floor(i/2)*20), getHeight()-40+(i%2)*20, 40, 20); // arrange event channel buttons in two rows
        eventDisplayInterfaces[i]->repaint();
    }
    
    brightnessSliderA->setBounds(170,getHeight()-180,100,22);
    sliderALabel->setBounds(270, getHeight()-180, 180, 22);
    brightnessSliderA->setValue(0.9); //set default value
    
    brightnessSliderB->setBounds(170,getHeight()-150,100,22);
    sliderBLabel->setBounds(270, getHeight()-150, 180, 22);
    brightnessSliderB->setValue(0.1); //set default value

    showHideOptionsButton->setBounds (getWidth() - 28, getHeight() - 28, 20, 20);
    
    int bh = 25/typeButtons.size();
    for (int i = 0; i < typeButtons.size(); i++)
    {
        typeButtons[i]->setBounds(95,getHeight()-30+i*bh,50,bh);
    }
}

void LfpDisplayOptions::paint(Graphics& g)
{
    int row1 = 55;
    int row2 = 110;

    g.fillAll(Colours::black);
    g.setFont(Font("Default", 16, Font::plain));

    g.setColour(Colour(100,100,100));

    g.drawText("Range("+ rangeUnits[selectedChannelType] +")",5,getHeight()-row1,300,20,Justification::left, false);
    g.drawText("Timebase(s)",160,getHeight()-row1,300,20,Justification::left, false);
    g.drawText("Size(px)",5,getHeight()-row2,300,20,Justification::left, false);
    g.drawText("Clip",100,getHeight()-row2,300,20,Justification::left, false);
    g.drawText("Warn",168,getHeight()-row2,300,20,Justification::left, false);
    
    g.drawText("Sat. Warning",225,getHeight()-row2,300,20,Justification::left, false);

    g.drawText("Color grouping",365,getHeight()-row2,300,20,Justification::left, false);

    g.drawText("Event disp.",300,getHeight()-row1,300,20,Justification::left, false);

    if(canvas->drawClipWarning)
    {
        g.setColour(Colours::white);
        g.fillRoundedRectangle(173,getHeight()-90-1,24,24,6.0f);
    }
    
    if(canvas->drawSaturationWarning)
    {
        g.setColour(Colours::red);
        g.fillRoundedRectangle(323,getHeight()-90-1,24,24,6.0f);
    }
    
}

int LfpDisplayOptions::getChannelHeight()
{
    return (int)spreadSelection->getText().getIntValue();
}


bool LfpDisplayOptions::getDrawMethodState()
{
    
    return drawMethodButton->getToggleState();
}

bool LfpDisplayOptions::getInputInvertedState()
{
    return invertInputButton->getToggleState();
}

void LfpDisplayOptions::setRangeSelection(float range, bool canvasMustUpdate)
{
    if (canvasMustUpdate)
    {
        rangeSelection->setText(String(range/rangeGain[selectedChannelType]), sendNotification); 
    }
    else
    {
        rangeSelection->setText(String(range/rangeGain[selectedChannelType]),dontSendNotification);
        
        selectedVoltageRange[selectedChannelType] = rangeSelection->getSelectedId();
        selectedVoltageRangeValues[selectedChannelType] = rangeSelection->getText();

        canvas->repaint();
        canvas->refresh();
    }

}

void LfpDisplayOptions::setSpreadSelection(int spread, bool canvasMustUpdate)
{
    if (canvasMustUpdate)
    {
        spreadSelection->setText(String(spread),sendNotification);
    }
    else
    {
        spreadSelection->setText(String(spread),dontSendNotification);
        selectedSpread = spreadSelection->getSelectedId();
        selectedSpreadValue = spreadSelection->getText();

        canvas->repaint();
        canvas->refresh();
    }
}

void LfpDisplayOptions::togglePauseButton()
{
    pauseButton->setToggleState(!pauseButton->getToggleState(), sendNotification);
}

void LfpDisplayOptions::buttonClicked(Button* b)
{
    if (b == invertInputButton)
    {
        lfpDisplay->setInputInverted(b->getToggleState());
        return;
    }
    if (b == drawMethodButton)
    {
        lfpDisplay->setDrawMethod(b->getToggleState()); // this should be done the same way as drawClipWarning - or the other way around.
        return;
    }
    if (b == drawClipWarningButton)
    {
        canvas->drawClipWarning = b->getToggleState();
        canvas->redraw();
        return;
    }
    if (b == drawSaturateWarningButton)
    {
        canvas->drawSaturationWarning = b->getToggleState();
        canvas->redraw();
        return;
    }
    
    if (b == pauseButton)
    {
        lfpDisplay->isPaused = b->getToggleState();
        return;
    }

    if (b == showHideOptionsButton)
    {
        canvas->toggleOptionsDrawer(b->getToggleState());
    }

    int idx = typeButtons.indexOf((UtilityButton*)b);

    if ((idx >= 0) && (b->getToggleState()))
    {
        for (int i = 0; i < processor->getNumInputs(); i++)
        {
            if (lfpDisplay->channels[i]->getSelected())
            {
                lfpDisplay->channels[i]->deselect();
                lfpDisplay->channels[i]->repaint();
            }
        } 

        setSelectedType((DataChannel::DataChannelTypes) idx, false);
    }

}


void LfpDisplayOptions::comboBoxChanged(ComboBox* cb)
{

    if (cb == timebaseSelection)
    {
        if (cb->getSelectedId())
        {
            canvas->timebase = timebases[cb->getSelectedId()-1].getFloatValue();
        }
        else
        {
            canvas->timebase = cb->getText().getFloatValue();

            if (canvas->timebase)
            {
                if (canvas->timebase < timebases[0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    canvas->timebase = timebases[0].getFloatValue();
                }
                else if (canvas->timebase > timebases[timebases.size()-1].getFloatValue())
                {
                    cb->setSelectedId(timebases.size(),dontSendNotification);
                    canvas->timebase = timebases[timebases.size()-1].getFloatValue();
                }
                else
                    cb->setText(String(canvas->timebase,1), dontSendNotification);
            }
            else
            {
                if (selectedSpread == 0)
                {
                    cb->setText(selectedTimebaseValue, dontSendNotification);
                    canvas->timebase = selectedTimebaseValue.getFloatValue();
                }
                else
                {
                    cb->setSelectedId(selectedTimebase,dontSendNotification);
                    canvas->timebase = timebases[selectedTimebase-1].getFloatValue();
                }

            }
        }
    }
    else if (cb == rangeSelection)
    {
        if (cb->getSelectedId())
        {
        lfpDisplay->setRange(voltageRanges[selectedChannelType][cb->getSelectedId()-1].getFloatValue()*rangeGain[selectedChannelType]
            ,selectedChannelType);
        }
        else
        {
            float vRange = cb->getText().getFloatValue();
            if (vRange)
            {
                if (vRange < voltageRanges[selectedChannelType][0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    vRange = voltageRanges[selectedChannelType][0].getFloatValue();
                }
                else if (vRange > voltageRanges[selectedChannelType][voltageRanges[selectedChannelType].size()-1].getFloatValue())
                {
                   // cb->setSelectedId(voltageRanges[selectedChannelType].size(),dontSendNotification);
                   // vRange = voltageRanges[selectedChannelType][voltageRanges[selectedChannelType].size()-1].getFloatValue();
                }
                else
                {
                    if (rangeGain[selectedChannelType] > 1)
                        cb->setText(String(vRange,1),dontSendNotification);
                    else
                        cb->setText(String(vRange),dontSendNotification);
                }
                lfpDisplay->setRange(vRange*rangeGain[selectedChannelType],selectedChannelType);
            }
            else
            {
                if (selectedVoltageRange[selectedChannelType])
                    cb->setText(selectedVoltageRangeValues[selectedChannelType],dontSendNotification);
                else
                    cb->setSelectedId(selectedVoltageRange[selectedChannelType],dontSendNotification);
            }
        }
        selectedVoltageRange[selectedChannelType] = cb->getSelectedId();
        selectedVoltageRangeValues[selectedChannelType] = cb->getText();
        //std::cout << "Setting range to " << voltageRanges[cb->getSelectedId()-1].getFloatValue() << std::endl;
        canvas->redraw();
    }
    else if (cb == spreadSelection)
    {
        if (cb->getSelectedId())
        {
            lfpDisplay->setChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
            resized();
        }
        else
        {
            int spread = cb->getText().getIntValue();
            if (spread)
            {
                if (spread < spreads[0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    spread = spreads[0].getFloatValue();
                }
                else if (spread > spreads[spreads.size()-1].getFloatValue())
                {
                    cb->setSelectedId(spreads.size(),dontSendNotification);
                    spread = spreads[spreads.size()-1].getFloatValue();
                }
                else
                {
                    cb->setText(String(spread),dontSendNotification);
                }
                lfpDisplay->setChannelHeight(spread);
                canvas->resized();
            }
            else
            {
                if (selectedSpread == 0)
                    cb->setText(selectedSpreadValue,dontSendNotification);
                else
                    cb->setSelectedId(selectedSpread,dontSendNotification);
            }
        }
        selectedSpread = cb->getSelectedId();
        selectedSpreadValue = cb->getText();

        canvas->redraw();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }
    else if (cb == saturationWarningSelection)
    {
        if (cb->getSelectedId())
        {
            selectedSaturationValueFloat = (saturationThresholds[cb->getSelectedId()-1].getFloatValue());
        }
        else
        {
            selectedSaturationValueFloat = cb->getText().getFloatValue();
            if (selectedSaturationValueFloat)
            {
                 std::cout << "Setting saturation warning to to " << selectedSaturationValueFloat << std::endl;
                if (selectedSaturationValueFloat < 0)
                {
                    cb->setSelectedId(1,dontSendNotification);
                    selectedSaturationValueFloat = saturationThresholds[0].getFloatValue();
                }
                else
                {
                  //  cb->setText(String(selectedSaturationValueFloat),dontSendNotification);
                }
            }
            else
            {
               // cb->setSelectedId(1,dontSendNotification);
                //selectedSaturationValueFloat = saturationThresholds[0].getFloatValue();

            }
        }
        canvas->redraw();

        std::cout << "Setting saturation warning to to " << selectedSaturationValueFloat << std::endl;
    }
    else if (cb == overlapSelection)
    {
        if (cb->getSelectedId())
        {
            canvas->channelOverlapFactor = (overlaps[cb->getSelectedId()-1].getFloatValue());
            canvas->resized();
        }
        else
        {
            float overlap = cb->getText().getFloatValue();
            if (overlap)
            {
                if (overlap < overlaps[0].getFloatValue())
                {
                    cb->setSelectedId(1,dontSendNotification);
                    overlap = overlaps[0].getFloatValue();
                }
                else if (overlap > overlaps[overlaps.size()-1].getFloatValue())
                {
                    cb->setSelectedId(overlaps.size(),dontSendNotification);
                    overlap = overlaps[overlaps.size()-1].getFloatValue();
                }
                else
                {
                    cb->setText(String(overlap),dontSendNotification);
                }
                canvas->channelOverlapFactor= overlap;
                canvas->resized();
            }
            else
            {
                if (selectedSpread == 0)
                    cb->setText(selectedSpreadValue,dontSendNotification);
                else
                    cb->setSelectedId(selectedSpread,dontSendNotification);
            }
        }
        selectedSpread = cb->getSelectedId();
        selectedSpreadValue = cb->getText();
        lfpDisplay->setChannelHeight( lfpDisplay->getChannelHeight());
        canvas->redraw();
        //std::cout << "Setting spread to " << spreads[cb->getSelectedId()-1].getFloatValue() << std::endl;
    }

    else if (cb == colorGroupingSelection)
    {
        // set color grouping here
        lfpDisplay->setColorGrouping(colorGroupings[cb->getSelectedId()-1].getIntValue());// so that channel colors get re-assigned
        canvas->redraw();
    }

    timescale->setTimebase(canvas->timebase);
}


void LfpDisplayOptions::sliderValueChanged(Slider* sl)
{
    if (sl == brightnessSliderA)
        canvas->histogramParameterA = sl->getValue();

    if (sl == brightnessSliderB)
        canvas->histogramParameterB = sl->getValue();
    
    canvas->fullredraw=true;
    //repaint();
    canvas->refresh();

}

void LfpDisplayOptions::sliderEvent(Slider* sl) {}

DataChannel::DataChannelTypes LfpDisplayOptions::getChannelType(int n)
{
    if (n < processor->getNumInputs())
        return processor->getDataChannel(n)->getChannelType();
    else
        return DataChannel::HEADSTAGE_CHANNEL;
}

DataChannel::DataChannelTypes LfpDisplayOptions::getSelectedType()
{
    return selectedChannelType;
}

void LfpDisplayOptions::setSelectedType(DataChannel::DataChannelTypes type, bool toggleButton)
{
    if (selectedChannelType == type)
        return; //Nothing to do here
    selectedChannelType = type;
    rangeSelection->clear(dontSendNotification);
    rangeSelection->addItemList(voltageRanges[type],1);

    int id = selectedVoltageRange[type];
    if (id)
        rangeSelection->setSelectedId(id,sendNotification);
    else
        rangeSelection->setText(selectedVoltageRangeValues[selectedChannelType],dontSendNotification);
    
    repaint(5,getHeight()-55,300,100);

    if (toggleButton)
        typeButtons[type]->setToggleState(true,dontSendNotification);
}

String LfpDisplayOptions::getTypeName(DataChannel::DataChannelTypes type)
{
    return typeNames[type];
}

int LfpDisplayOptions::getRangeStep(DataChannel::DataChannelTypes type)
{
    return rangeSteps[type];
}



void LfpDisplayOptions::saveParameters(XmlElement* xml)
{

    std::cout << "Saving lfp display params" << std::endl;

    XmlElement* xmlNode = xml->createNewChildElement("LFPDISPLAY");

    lfpDisplay->reactivateChannels();

    xmlNode->setAttribute("Range",selectedVoltageRangeValues[0]+","+selectedVoltageRangeValues[1]+
        ","+selectedVoltageRangeValues[2]);
    xmlNode->setAttribute("Timebase",timebaseSelection->getText());
    xmlNode->setAttribute("Spread",spreadSelection->getText());
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

    lfpDisplay->reactivateChannels();

    xmlNode->setAttribute("EventButtonState", eventButtonState);

    String channelDisplayState = "";

    for (int i = 0; i < canvas->nChans; i++)
    {
        if (lfpDisplay->getEnabledState(i))
        {
            channelDisplayState += "1";
        }
        else
        {
            channelDisplayState += "0";
        }
        //std::cout << channelDisplayState;
    }

    //std::cout << std::endl;


    xmlNode->setAttribute("ChannelDisplayState", channelDisplayState);

    xmlNode->setAttribute("ScrollX",canvas->viewport->getViewPositionX());
    xmlNode->setAttribute("ScrollY",canvas->viewport->getViewPositionY());
}


void LfpDisplayOptions::loadParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("LFPDISPLAY"))
        {
            StringArray ranges;
            ranges.addTokens(xmlNode->getStringAttribute("Range"),",",String::empty);
            selectedVoltageRangeValues[0] = ranges[0];
            selectedVoltageRangeValues[1] = ranges[1];
            selectedVoltageRangeValues[2] = ranges[2];
            selectedVoltageRange[0] = voltageRanges[0].indexOf(ranges[0])+1;
            selectedVoltageRange[1] = voltageRanges[1].indexOf(ranges[1])+1;
            selectedVoltageRange[2] = voltageRanges[2].indexOf(ranges[2])+1;
            rangeSelection->setText(ranges[0]);

            timebaseSelection->setText(xmlNode->getStringAttribute("Timebase"));
            spreadSelection->setText(xmlNode->getStringAttribute("Spread"));
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

            canvas->viewport->setViewPosition(xmlNode->getIntAttribute("ScrollX"),
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
                    //lfpDisplay->enableChannel(true, i);
                    canvas->isChannelEnabled.set(i,true); //lfpDisplay->enableChannel(true, i);
                }
                else
                {
                    //lfpDisplay->enableChannel(false, i);
                    canvas->isChannelEnabled.set(i,false);
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

        labels.add(labelString.substring(0,6));
    }

    repaint();

}


// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v) :
    singleChan(-1), canvas(c), viewport(v)
{
    totalHeight = 0;
    colorGrouping=1;

    range[0] = 1000;
    range[1] = 500;
    range[2] = 500000;

    addMouseListener(this, true);

    // hue cycle
    //for (int i = 0; i < 15; i++)
    //{
    //    channelColours.add(Colour(float(sin((3.14/2)*(float(i)/15))),float(1.0),float(1),float(1.0)));
    //}

    backgroundColour = Colour(0,18,43);
    
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

        LfpChannelDisplay* lfpChan = new LfpChannelDisplay(canvas, this, options, i);

        //lfpChan->setColour(channelColours[i % channelColours.size()]);
        lfpChan->setRange(range[options->getChannelType(i)]);
        lfpChan->setChannelHeight(canvas->getChannelHeight());
        
        addAndMakeVisible(lfpChan);

        channels.add(lfpChan);

        LfpChannelDisplayInfo* lfpInfo = new LfpChannelDisplayInfo(canvas, this, options, i);

        //lfpInfo->setColour(channelColours[i % channelColours.size()]);
        lfpInfo->setRange(range[options->getChannelType(i)]);
        lfpInfo->setChannelHeight(canvas->getChannelHeight());

        addAndMakeVisible(lfpInfo);

        channelInfo.add(lfpInfo);

        savedChannelState.add(true);

        totalHeight += lfpChan->getChannelHeight();

    }

    setColors();
    

    //std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;

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
    
    //canvas->channelOverlapFactor

    int totalHeight = 0;

    for (int i = 0; i < channels.size(); i++)
    {

        LfpChannelDisplay* disp = channels[i];

        disp->setBounds(canvas->leftmargin,
                        totalHeight-(disp->getChannelOverlap()*canvas->channelOverlapFactor)/2,
                        getWidth(),
                        disp->getChannelHeight()+(disp->getChannelOverlap()*canvas->channelOverlapFactor));

        disp-> resized();
        
        LfpChannelDisplayInfo* info = channelInfo[i];

        info->setBounds(0,
                        totalHeight-disp->getChannelHeight()/4,
                        canvas->leftmargin + 50,
                        disp->getChannelHeight());

        totalHeight += disp->getChannelHeight();

    }

    canvas->fullredraw = true; //issue full redraw
    if (singleChan != -1)
        viewport->setViewPosition(Point<int>(0,singleChan*getChannelHeight()));

  

    lfpChannelBitmap = Image(Image::ARGB, getWidth(), getHeight(), false);
    
    //inititalize black background
    Graphics gLfpChannelBitmap(lfpChannelBitmap);
    gLfpChannelBitmap.setColour(Colour(0,0,0)); //background color
    gLfpChannelBitmap.fillRect(0,0, getWidth(), getHeight());

    
    canvas->fullredraw = true;
    
    refresh();
    // std::cout << "Total height: " << totalHeight << std::endl;

}

void LfpDisplay::paint(Graphics& g)
{

    g.drawImageAt(lfpChannelBitmap, canvas->leftmargin,0);
    
}


void LfpDisplay::refresh()
{
    // Ensure the lfpChannelBitmap has been initialized
    if (lfpChannelBitmap.isNull()) { resized(); }

    // X-bounds of this update
    int fillfrom = canvas->lastScreenBufferIndex[0];
    int fillto = (canvas->screenBufferIndex[0]);
    
    if (fillfrom<0){fillfrom=0;};
    if (fillto>lfpChannelBitmap.getWidth()){fillto=lfpChannelBitmap.getWidth();};
    
    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    // clear appropriate section of the bitmap --
    // we need to do this before each channel draws its new section of data into lfpChannelBitmap
    Graphics gLfpChannelBitmap(lfpChannelBitmap);
    gLfpChannelBitmap.setColour(backgroundColour); //background color

    if (canvas->fullredraw)
    {
        gLfpChannelBitmap.fillRect(0,0, getWidth(), getHeight());
    } else {
        gLfpChannelBitmap.setColour(backgroundColour); //background color

        gLfpChannelBitmap.fillRect(fillfrom,0, (fillto-fillfrom)+1, getHeight());
    };
    
    
    for (int i = 0; i < numChans; i++)
    {

        int componentTop = channels[i]->getY();
        int componentBottom = channels[i]->getHeight() + componentTop;

        if ((topBorder <= componentBottom && bottomBorder >= componentTop)) // only draw things that are visible
        {
            if (canvas->fullredraw)
            {
                channels[i]->fullredraw = true;
                
                channels[i]->pxPaint();
                channelInfo[i]->repaint();
                
            }
            else
            {
                 channels[i]->pxPaint(); // draws to lfpChannelBitmap
                
                 // it's not clear why, but apparently because the pxPaint() in a child component of LfpDisplay, we also need to issue repaint() calls for each channel, even though there's nothin to repaint there. Otherwise, the repaint call in LfpDisplay::refresh(), a few lines down, lags behind the update line by ~60 px. This could ahev something to do with teh reopaint message passing in juce. In any case, this seemingly redundant repaint here seems to fix the issue.
                
                 // we redraw from 0 to +2 (px) relative to the real redraw window, the +1 draws the vertical update line
                 channels[i]->repaint(fillfrom, 0, (fillto-fillfrom)+2, channels[i]->getHeight());
                
                
            }
            //std::cout << i << std::endl;
        }

    }

    if (fillfrom == 0 && singleChan != -1)
    {
        channelInfo[singleChan]->repaint();
    }
    
    
    if (canvas->fullredraw)
    {
        repaint(0,topBorder,getWidth(),bottomBorder-topBorder);
    }else{
        //repaint(fillfrom, topBorder, (fillto-fillfrom)+1, bottomBorder-topBorder); // doesntb seem to be needed and results in duplicate repaint calls
    }
    
    canvas->fullredraw = false;
    
}



void LfpDisplay::setRange(float r, DataChannel::DataChannelTypes type)
{
    range[type] = r;
    
    if (channels.size() > 0)
    {

        for (int i = 0; i < numChans; i++)
        {
            if (channels[i]->getType() == type)
                channels[i]->setRange(range[type]);
        }
        canvas->fullredraw = true; //issue full redraw
    }
}

int LfpDisplay::getRange()
{
    return getRange(options->getSelectedType());
}

int LfpDisplay::getRange(DataChannel::DataChannelTypes type)
{
    for (int i=0; i < numChans; i++)
    {
        if (channels[i]->getType() == type)
            return channels[i]->getRange();
    }
    return 0;
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
        //std::cout << "width " <<  getWidth() << " numchans  " << numChans << " height " << getChannelHeight() << std::endl;
        setSize(getWidth(),numChans*getChannelHeight());
        viewport->setScrollBarsShown(true,false);
        viewport->setViewPosition(Point<int>(0,singleChan*r));
        singleChan = -1;
        for (int n = 0; n < numChans; n++)
        {
            channelInfo[n]->setEnabledState(savedChannelState[n]);
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
    //TODO Changing ranges with the wheel is currently broken. With multiple ranges, most
    //of the wheel range code needs updating
    
    
    if (e.mods.isCommandDown() && singleChan == -1)  // CTRL + scroll wheel -> change channel spacing
    {
        int h = getChannelHeight();
        int hdiff=0;
        
        // std::cout << wheel.deltaY << std::endl;
        
        if (wheel.deltaY > 0)
        {
            hdiff = 2;
        }
        else
        {
            if (h > 5)
                hdiff = -2;
        }

        if (abs(h) > 100) // accelerate scrolling for large ranges
            hdiff *= 3;

        setChannelHeight(h+hdiff);
        int oldX=viewport->getViewPositionX();
        int oldY=viewport->getViewPositionY();

        setBounds(0,0,getWidth()-0, getChannelHeight()*canvas->nChans); // update height so that the scrollbar is correct

        int mouseY=e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
        int scrollBy = (mouseY/h)*hdiff*2;// compensate for motion of point under current mouse position
        viewport->setViewPosition(oldX,oldY+scrollBy); // set back to previous position plus offset

        options->setSpreadSelection(h+hdiff); // update combobox
        
        canvas->fullredraw = true;//issue full redraw - scrolling without modifier doesnt require a full redraw
    }
    else
    {
        if (e.mods.isAltDown())  // ALT + scroll wheel -> change channel range (was SHIFT but that clamps wheel.deltaY to 0 on OSX for some reason..)
        {
            int h = getRange();
            
            
            int step = options->getRangeStep(options->getSelectedType());
                       
            // std::cout << wheel.deltaY << std::endl;
            
            if (wheel.deltaY > 0)
            {
                setRange(h+step,options->getSelectedType());
            }
            else
            {
                if (h > step+1)
                    setRange(h-step,options->getSelectedType());
            }

            options->setRangeSelection(h); // update combobox
            canvas->fullredraw = true; //issue full redraw - scrolling without modifier doesnt require a full redraw
            
        }
        else    // just scroll
        {
            //  passes the event up to the viewport so the screen scrolls
            if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
                viewport->mouseWheelMove(e.getEventRelativeTo(canvas), wheel);

        }
      

    }
       //refresh(); // doesn't seem to be needed now that channels daraw to bitmap

}

void LfpDisplay::toggleSingleChannel(int chan)
{
    //std::cout << "Toggle channel " << chan << std::endl;

    if (chan != singleChan)
    {
        std::cout << "Single channel on" << std::endl;
        singleChan = chan;

        int newHeight = viewport->getHeight();
        channelInfo[chan]->setEnabledState(true);
        channelInfo[chan]->setSingleChannelState(true);
        setChannelHeight(newHeight, false);
        setSize(getWidth(), numChans*getChannelHeight());

        viewport->setScrollBarsShown(false,false);
        viewport->setViewPosition(Point<int>(0,chan*newHeight));

        for (int i = 0; i < channels.size(); i++)
        {
            if (i != chan)
                channels[i]->setEnabledState(false);
        }

    }
    else if (chan == singleChan || chan == -2)
    {
        std::cout << "Single channel off" << std::endl;
        for (int n = 0; n < numChans; n++)
        {

            channelInfo[chan]->setSingleChannelState(false);
        }
        setChannelHeight(canvas->getChannelHeight());

        reactivateChannels();
    }
}

void LfpDisplay::reactivateChannels()
{

    for (int n = 0; n < channels.size(); n++)
       setEnabledState(savedChannelState[n], n);

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
    int x = canvasevent.getMouseDownX();

    int dist = 0;
    int mindist = 10000;
    int closest = 5;
    for (int n = 0; n < numChans; n++) // select closest instead of relying on eventComponent
    {
        channels[n]->deselect();

        int cpos = (channels[n]->getY() + (channels[n]->getHeight()/2));
        dist = int(abs(y - cpos));

        //std::cout << "Mouse down at " << y << " pos is "<< cpos << "n:" << n << "  dist " << dist << std::endl;

        if (dist < mindist)
        {
            mindist = dist-1;
            closest = n;
        }
    }

    if (singleChan != -1)
    {
        //std::cout << y << " " << channels[singleChan]->getHeight() << " " << getRange() << std::endl;
        channelInfo[singleChan]->updateXY(
                float(x)/getWidth()*canvas->timebase, 
                (-(float(y)-viewport->getViewPositionY())/viewport->getViewHeight()*float(getRange()))+float(getRange()/2)
                );
    }

    channels[closest]->select();
    options->setSelectedType(channels[closest]->getType());

    if (event.getNumberOfClicks() == 2)
        toggleSingleChannel(closest);

    if (event.mods.isRightButtonDown())
    {
        PopupMenu channelMenu = channels[closest]->getOptions();
        const int result = channelMenu.show();
        channels[closest]->changeParameter(result);
    }

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


void LfpDisplay::setEnabledState(bool state, int chan, bool updateSaved)
{
    if (chan < numChans)
    {
        channels[chan]->setEnabledState(state);
        channelInfo[chan]->setEnabledState(state);

        if (updateSaved)
            savedChannelState.set(chan, state);

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

LfpChannelDisplay::LfpChannelDisplay(LfpDisplayCanvas* c, LfpDisplay* d, LfpDisplayOptions* o, int channelNumber) :
    canvas(c), display(d), options(o), isSelected(false), chan(channelNumber),
    channelOverlap(300), channelHeight(40), range(1000.0f),
    isEnabled(true), inputInverted(false), canBeInverted(true), drawMethod(false)
{


    name = String(channelNumber+1); // default is to make the channelNumber the name


    channelHeightFloat = (float) channelHeight;

    channelFont = Font("Default", channelHeight*0.6, Font::plain);

    lineColour = Colour(255,255,255);

    type = options->getChannelType(channelNumber);
    typeStr = options->getTypeName(type);

}

LfpChannelDisplay::~LfpChannelDisplay()
{

}

void LfpChannelDisplay::resized()
{
    // all of this will likely need to be moved into the sharedLfpDisplay image in the lfpDisplay, not here
    // now that the complete height is know, the sharedLfpDisplay image that we'll draw the pixel-wise lfp plot to needs to be resized
    //lfpChannelBitmap = Image(Image::ARGB, getWidth(), getHeight(), false);
}


void LfpChannelDisplay::updateType()
{
    type = options->getChannelType(chan);
    typeStr = options->getTypeName(type);
}

void LfpChannelDisplay::setEnabledState(bool state)
{

    //if (state)
    //std::cout << "Setting channel " << name << " to true." << std::endl;
    //else
    //std::cout << "Setting channel " << name << " to false." << std::endl;

    isEnabled = state;

}

void LfpChannelDisplay::pxPaint()
{
    if (isEnabled)
    {
        Image::BitmapData bdLfpChannelBitmap(display->lfpChannelBitmap, 0,0, display->lfpChannelBitmap.getWidth(), display->lfpChannelBitmap.getHeight());

        int center = getHeight()/2;
        
        // max and min of channel in absolute px coords for event displays etc - actual data might be drawn outside of this range
        int jfrom_wholechannel= (int) (getY()+center-channelHeight/2)+1 +0 ;
        int jto_wholechannel= (int) (getY()+center+channelHeight/2) -0;
    
        //int jfrom_wholechannel_almost= (int) (getY()+center-channelHeight/3)+1 +0 ; // a bit less tall, for saturation warnings
        //int jto_wholechannel_almost= (int) (getY()+center+channelHeight/3) -0;
        
        // max and min of channel, this is the range where actual data is drawn
        int jfrom_wholechannel_clip= (int) (getY()+center-(channelHeight)*canvas->channelOverlapFactor)+1  ;
        int jto_wholechannel_clip  = (int) (getY()+center+(channelHeight)*canvas->channelOverlapFactor) -0;

        if (jfrom_wholechannel<0) {jfrom_wholechannel=0;};
        if (jto_wholechannel >= display->lfpChannelBitmap.getHeight()) {jto_wholechannel=display->lfpChannelBitmap.getHeight()-1;};
    
        // draw most recent drawn sample position
        if (canvas->screenBufferIndex[chan]+1 <= display->lfpChannelBitmap.getWidth())
        for (int k=jfrom_wholechannel; k<=jto_wholechannel; k+=2) // draw line
            bdLfpChannelBitmap.setPixelColour(canvas->screenBufferIndex[chan]+1,k, Colours::yellow);
        
        
        bool clipWarningHi =false; // keep track if something clipped in the display, so we can draw warnings after the data pixels are done
        bool clipWarningLo =false;

        bool saturateWarningHi =false; // similar, but for saturating the amplifier, not just the display - make this warning very visible
        bool saturateWarningLo =false;
        
        // pre compute some colors for later so we dont do it once per pixel.
        Colour lineColourBright = lineColour.withMultipliedBrightness(2.0f);
        //Colour lineColourDark = lineColour.withMultipliedSaturation(0.5f).withMultipliedBrightness(0.3f);
        Colour lineColourDark = lineColour.withMultipliedSaturation(0.5f*canvas->histogramParameterB).withMultipliedBrightness(canvas->histogramParameterB);
        
    
        int stepSize = 1;
        int from = 0; // for vertical line drawing in the LFP data
        int to = 0;
        
        int ifrom = canvas->lastScreenBufferIndex[chan] - 1; // need to start drawing a bit before the actual redraw window for the interpolated line to join correctly
        
        if (ifrom < 0)
            ifrom = 0;
        
        int ito = canvas->screenBufferIndex[chan] +0;
        
        if (fullredraw)
        {
            ifrom = 0; //canvas->leftmargin;
            ito = getWidth()-stepSize;
            fullredraw = false;
        }
        
        
        for (int i = ifrom; i < ito ; i += stepSize) // redraw only changed portion
        {
            if (i < display->lfpChannelBitmap.getWidth())
            {
                //draw zero line
                int m = getY()+center;
                
                if(m > 0 & m < display->lfpChannelBitmap.getHeight())
                   {
                    if ( bdLfpChannelBitmap.getPixelColour(i,m) == display->backgroundColour ) { // make sure we're not drawing over an existing plot from another channel
                        bdLfpChannelBitmap.setPixelColour(i,m,Colour(50,50,50));
                    }
                }
                
                //draw range markers
                if (isSelected)
                {
                    int start = getY()+center -channelHeight/2;
                    int jump = channelHeight/4;

                    for (m = start; m <= start + jump*4; m += jump)
                    {
                        if (m > 0 & m < display->lfpChannelBitmap.getHeight())
                        {
                            if ( bdLfpChannelBitmap.getPixelColour(i,m) == display->backgroundColour ) // make sure we're not drawing over an existing plot from another channel
                                bdLfpChannelBitmap.setPixelColour(i, m, Colour(80,80,80));
                        }
                    }
                }
                
                // draw event markers
                int rawEventState = canvas->getYCoord(canvas->getNumChannels(), i);// get last channel+1 in buffer (represents events)
                
                //if (i == ifrom)
                //    std::cout << rawEventState << std::endl;
                
                for (int ev_ch = 0; ev_ch < 8 ; ev_ch++) // for all event channels
                {
                    if (display->getEventDisplayState(ev_ch))  // check if plotting for this channel is enabled
                    {
                        if (rawEventState & (1 << ev_ch))    // events are  representet by a bit code, so we have to extract the individual bits with a mask
                        {
                            //std::cout << "Drawing event." << std::endl;
                            Colour currentcolor=display->channelColours[ev_ch*2];
                            
                            for (int k=jfrom_wholechannel; k<=jto_wholechannel; k++) // draw line
                                bdLfpChannelBitmap.setPixelColour(i,k,bdLfpChannelBitmap.getPixelColour(i,k).interpolatedWith(currentcolor,0.3f));
                            
                        }
                    }
                }
                
                //std::cout << "e " << canvas->getYCoord(canvas->getNumChannels()-1, i) << std::endl;
                
                
                // set max-min range for plotting, used in all methods
                double a = (canvas->getYCoordMax(chan, i)/range*channelHeightFloat);
                double b = (canvas->getYCoordMin(chan, i)/range*channelHeightFloat);
                
                double a_raw = canvas->getYCoordMax(chan, i);
                double b_raw = canvas->getYCoordMin(chan, i);
                double from_raw=0; double to_raw=0;
                
                //double m = (canvas->getYCoordMean(chan, i)/range*channelHeightFloat)+getHeight()/2;
                if (a<b)
                {
                    from = (a); to = (b);
                    from_raw = (a_raw); to_raw = (b_raw);

                }
                else
                {
                    from = (b); to = (a);
                    from_raw = (b_raw); to_raw = (a_raw);
                }
                
                // start by clipping so that we're not populating pixels that we dont want to plot
                int lm= channelHeightFloat*canvas->channelOverlapFactor;
                if (lm>0)
                    lm=-lm;
            
                if (from > -lm) {from = -lm; clipWarningHi=true;};
                if (to > -lm) {to = -lm; clipWarningHi=true;};
                if (from < lm) {from = lm; clipWarningLo=true;};
                if (to < lm) {to = lm; clipWarningLo=true;};
                
                
                // test if raw data is clipped for displaying saturation warning
                if (from_raw > options->selectedSaturationValueFloat) { saturateWarningHi=true;};
                if (to_raw > options->selectedSaturationValueFloat) { saturateWarningHi=true;};
                if (from_raw < -options->selectedSaturationValueFloat) { saturateWarningLo=true;};
                if (to_raw < -options->selectedSaturationValueFloat) { saturateWarningLo=true;};
                
                
                from = from + getHeight()/2;       // so the plot is centered in the channeldisplay
                to = to + getHeight()/2;
                
                int samplerange = to - from;

                if (drawMethod) // switched between 'supersampled' drawing and simple pixel wise drawing
                { // histogram based supersampling method
                    
                    std::array<float, MAX_N_SAMP_PER_PIXEL> samplesThisPixel = canvas->getSamplesPerPixel(chan, i);
                    int sampleCountThisPixel = canvas->getSampleCountPerPixel(i);
                    
                    if (samplerange>0 & sampleCountThisPixel>0)
                    {
                        
                        //float localHist[samplerange]; // simple histogram
                        Array<float> rangeHist; // [samplerange]; // paired range histogram, same as plotting at higher res. and subsampling
                        
                        for (int k = 0; k <= samplerange; k++)
                            rangeHist.add(0);
                        
                        for (int k = 0; k <= sampleCountThisPixel; k++) // add up paired-range histogram per pixel - for each pair fill intermediate with uniform distr.
                        {
                            int cs_this = (((samplesThisPixel[k]/range*channelHeightFloat)+getHeight()/2)-from); // sample values -> pixel coordinates relative to from
                            int cs_next = (((samplesThisPixel[k+1]/range*channelHeightFloat)+getHeight()/2)-from);
                            
                            
                            if (cs_this<0) {cs_this=0;};                        //here we could clip the diaplay to the max/min, or ignore out of bound values, not sure which one is better
                            if (cs_this>samplerange) {cs_this=samplerange;};
                            if (cs_next<0) {cs_next=0;};
                            if (cs_next>samplerange) {cs_next=samplerange;};
                            
                            int hfrom=0;
                            int hto=0;
                            
                            if (cs_this<cs_next)
                            {
                                hfrom = (cs_this);  hto = (cs_next);
                            }
                            else
                            {
                                hfrom = (cs_next);  hto = (cs_this);
                            }
                            //float hrange=hto-hfrom;
                            float ha=1;
                            for (int l=hfrom; l<hto; l++)
                            {
                                rangeHist.set(l, rangeHist[l] + ha); //this emphasizes fast Y components
                                
                                //rangeHist[l]+=1/hrange; // this is like an oscilloscope, same energy depositetd per dx, not dy
                            }
                        }
                        
                        
                        for (int s = 0; s <= samplerange; s ++)  // plot histogram one pixel per bin
                        {
                            float a=15*((rangeHist[s])/(sampleCountThisPixel)) *(2*(0.2+canvas->histogramParameterA));
                            if (a>1.0f) {a=1.0f;};
                            if (a<0.0f) {a=0.0f;};
                            
                            
                            //Colour gradedColor = lineColour.withMultipliedBrightness(2.0f).interpolatedWith(lineColour.withMultipliedSaturation(0.6f).withMultipliedBrightness(0.3f),1-a) ;
                            Colour gradedColor =  lineColourBright.interpolatedWith(lineColourDark,1-a);
                            //Colour gradedColor =  Colour(0,255,0);
                            
                            int ploty = from+s+getY();
                            if(ploty>0 & ploty < display->lfpChannelBitmap.getHeight()) {
                                bdLfpChannelBitmap.setPixelColour(i,from+s+getY(),gradedColor);
                            }
                        }
                        
                    } else {
                        
                        int ploty = from+getY();
                        if(ploty>0 & ploty < display->lfpChannelBitmap.getHeight()) {
                            bdLfpChannelBitmap.setPixelColour(i,ploty,lineColour);
                        }
                    }
                    
                    
                    
                }
                else //drawmethod
                { // simple per-pixel min-max drawing, has no anti-aliasing, but runs faster
                    
                    int jfrom=from+getY();
                    int jto=to+getY();
                    
                    //if (yofs<0) {yofs=0;};
                    
                    if (i<0) {i=0;};
                    if (i >= display->lfpChannelBitmap.getWidth()) {i = display->lfpChannelBitmap.getWidth()-1;}; // this shouldnt happen, there must be some bug above - to replicate, run at max refresh rate where draws overlap the right margin by a lot
                    
                    if (jfrom<0) {jfrom=0;};
                    if (jto >= display->lfpChannelBitmap.getHeight()) {jto=display->lfpChannelBitmap.getHeight()-1;};
                    
                    
                    for (int j = jfrom; j <= jto; j += 1)
                    {
                        
                        //uint8* const pu8Pixel = bdSharedLfpDisplay.getPixelPointer(    (int)(i),(int)(j));
                        //*(pu8Pixel)        = 200;
                        //*(pu8Pixel+1)    = 200;
                        //*(pu8Pixel+2)    = 200;
                        
                        bdLfpChannelBitmap.setPixelColour(i,j,lineColour);
                        
                    }
                    
                }
                
                // now draw warnings, if needed
                if (canvas->drawClipWarning) // draw simple warning if display cuts off data
                {
                    
                    if(clipWarningHi) {
                        for (int j=0; j<=3; j++)
                        {
                            int clipmarker = jto_wholechannel_clip;
                        
                            if(clipmarker>0 & clipmarker<display->lfpChannelBitmap.getHeight()){
                                                      bdLfpChannelBitmap.setPixelColour(i,clipmarker-j,Colour(255,255,255));
                            }
                        }
                    }
                     
                    if(clipWarningLo) {
                            for (int j=0; j<=3; j++)
                            {
                               int clipmarker = jfrom_wholechannel_clip;
                                
                                if(clipmarker>0 & clipmarker<display->lfpChannelBitmap.getHeight()){
                                    bdLfpChannelBitmap.setPixelColour(i,clipmarker+j,Colour(255,255,255));
                                }
                            }
                    }

                clipWarningHi=false;
                clipWarningLo=false;
                }
                
                
                if (canvas->drawSaturationWarning) // draw bigger warning if actual data gets cuts off
                {
                    
                    if(saturateWarningHi || saturateWarningLo) {
                        
                        
                        for (int k=jfrom_wholechannel; k<=jto_wholechannel; k++){ // draw line
                            Colour thiscolour=Colour(255,0,0);
                            if (fmod((i+k),50)>25){
                                thiscolour=Colour(255,255,255);
                            }
                            if(k>0 & k<display->lfpChannelBitmap.getHeight()){
                                bdLfpChannelBitmap.setPixelColour(i,k,thiscolour);
                            }
                        };
                    }
                    
                    saturateWarningHi=false; // we likely just need one of this because for this warning we dont care if its saturating on the positive or negative side
                    saturateWarningLo=false;
                }
            } // if i < getWidth()
            
        } // for i (x pixels)
        
    } // isenabled

}

void LfpChannelDisplay::paint(Graphics& g) {}



PopupMenu LfpChannelDisplay::getOptions()
{

    PopupMenu menu;
    menu.addItem(1, "Invert signal", true, inputInverted);

    return menu;
}

void LfpChannelDisplay::changeParameter(int id)
{
    switch (id)
    {
        case 1:
            setInputInverted(!inputInverted);
        default:
            break;
    }
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

bool LfpChannelDisplay::getSelected()
{
   return isSelected;
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

    channelOverlap = channelHeight*2;
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

DataChannel::DataChannelTypes LfpChannelDisplay::getType()
{
    return type;
}

// -------------------------------

LfpChannelDisplayInfo::LfpChannelDisplayInfo(LfpDisplayCanvas* canvas_, LfpDisplay* display_, LfpDisplayOptions* options_, int ch)
    : LfpChannelDisplay(canvas_, display_, options_, ch)
{

    chan = ch;
    x = -1.0f;
    y = -1.0f;

    enableButton = new UtilityButton(String(ch+1), Font("Small Text", 13, Font::plain));
    enableButton->setRadius(5.0f);

    enableButton->setEnabledState(true);
    enableButton->setCorners(true, true, true, true);
    enableButton->addListener(this);
    enableButton->setClickingTogglesState(true);
    enableButton->setToggleState(true, dontSendNotification);

    addAndMakeVisible(enableButton);

}

void LfpChannelDisplayInfo::updateType()
{
    type = options->getChannelType(chan);
    typeStr = options->getTypeName(type);
    repaint();
}

void LfpChannelDisplayInfo::buttonClicked(Button* button)
{

    bool state = button->getToggleState();

    display->setEnabledState(state, chan);

    //UtilityButton* b = (UtilityButton*) button;

    // if (state)
    // {
    //  b->setLabel("ON");
    // } else {
    //  b->setLabel("OFF");
    // }

    //std::cout << "Turn channel " << chan << " to " << button->getToggleState() << std::endl;

}

void LfpChannelDisplayInfo::setEnabledState(bool state)
{
    enableButton->setToggleState(state, sendNotification);
}

void LfpChannelDisplayInfo::setSingleChannelState(bool state)
{
    isSingleChannel = state;
}

void LfpChannelDisplayInfo::paint(Graphics& g)
{

    int center = getHeight()/2;

    g.setColour(lineColour);

    //if (chan > 98)
    //  g.fillRoundedRectangle(5,center-8,51,22,8.0f);
    //else
    g.fillRoundedRectangle(5,center-8,41,22,8.0f);

    g.setFont(Font("Small Text", 13, Font::plain));
    g.drawText(typeStr,5,center+16,41,10,Justification::centred,false);
    // g.setFont(channelHeightFloat*0.3);
    g.setFont(Font("Small Text", 11, Font::plain));

    if (isSingleChannel)
    {
        g.setColour(Colours::darkgrey);
        g.drawText("STD:", 5, center+100,41,10,Justification::centred,false);
        g.drawText("MEAN:", 5, center+50,41,10,Justification::centred,false);
        
        if (x > 0)
        {
            g.drawText("uV:", 5, center+150,41,10,Justification::centred,false);
        }
        //g.drawText("Y:", 5, center+200,41,10,Justification::centred,false);

        g.setColour(Colours::grey);
        g.drawText(String(canvas->getStd(chan)), 5, center+120,41,10,Justification::centred,false);
        g.drawText(String(canvas->getMean(chan)), 5, center+70,41,10,Justification::centred,false);
        if (x > 0)
        {
            //g.drawText(String(x), 5, center+150,41,10,Justification::centred,false);
            g.drawText(String(y), 5, center+170,41,10,Justification::centred,false);
        }
        
    }

    //  g.drawText(name, 10, center-channelHeight/2, 200, channelHeight, Justification::left, false);

}

void LfpChannelDisplayInfo::updateXY(float x_, float y_)
{
    x = x_;
    y = y_;
}

void LfpChannelDisplayInfo::resized()
{

    int center = getHeight()/2;

    //if (chan > 98)
    //  enableButton->setBounds(8,center-5,45,16);
    //else
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
        g.fillRoundedRectangle(2,2,18,18,6.0f);
    }


    //g.drawText(String(channelNumber), 8, 2, 200, 15, Justification::left, false);

}

// Lfp Viewport -------------------------------------------

LfpViewport::LfpViewport(LfpDisplayCanvas *canvas)
    : Viewport()
{
    this->canvas = canvas;
}

void LfpViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{
    canvas->fullredraw = true;
    canvas->refresh();
}
