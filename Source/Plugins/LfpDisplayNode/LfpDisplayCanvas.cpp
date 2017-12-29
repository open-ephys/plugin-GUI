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

using namespace LfpViewer;



#pragma mark - LfpDisplayCanvas -

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
    nChans = jmax(processor->getNumInputs(), 0);

    resizeSamplesPerPixelBuffer(nChans);

    sampleRate.clear();
    screenBufferIndex.clear();
    lastScreenBufferIndex.clear();
    displayBufferIndex.clear();
    
    options->setEnabled(nChans != 0);
    // must manually ensure that overlapSelection propagates up to canvas
    channelOverlapFactor = options->selectedOverlapValue.getFloatValue();

    for (int i = 0; i <= nChans; i++) // extra channel for events
    {
		if (processor->getNumInputs() > 0)
		{
			if (i < nChans)
				sampleRate.add(processor->getDataChannel(i)->getSampleRate());
			else
			{
				//Since for now the canvas only supports one event channel, find the first TTL one and use that as sampleRate.
				//This is a bit hackish and should be fixed for proper multi-ttl-channel support
				for (int c = 0; c < processor->getTotalEventChannels(); c++)
				{
					if (processor->getEventChannel(c)->getChannelType() == EventChannel::TTL)
					{
						sampleRate.add(processor->getEventChannel(c)->getSampleRate());
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

        refreshScreenBuffer();

        lfpDisplay->setNumChannels(nChans); // add an extra channel for events

        // update channel names
        for (int i = 0; i < processor->getNumInputs(); i++)
        {

            String chName = processor->getDataChannel(i)->getName();

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
        for (int i = 0; i < processor->getNumInputs(); i++)
        {
            lfpDisplay->channels[i]->updateType();
            lfpDisplay->channelInfo[i]->updateType();
        }
        
        if (nChans > 0)
            lfpDisplay->rebuildDrawableChannelsList();
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

    for (int i = 0; i <= displayBufferIndex.size(); i++) // include event channel
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
        
        lastScreenBufferIndex.set(channel,sbi);

        int index = processor->getDisplayBufferIndex(channel);

        int nSamples =  index - dbi; // N new samples (not pixels) to be added to displayBufferIndex

        if (nSamples < 0) // buffer has reset to 0 -- xxx 2do bug: this shouldnt happen because it makes the range/histogram display not work properly/look off for one pixel
        {
            nSamples = (displayBufferSize - dbi) + index +1;
           //  std::cout << "nsamples 0 " ;
        }

        //if (channel == 15 || channel == 16)
        //     std::cout << channel << " " << sbi << " " << dbi << " " << nSamples << std::endl;


        float ratio = sampleRate[channel] * timebase / float(getWidth() - leftmargin - scrollBarThickness); // samples / pixel
        // this number is crucial: converting from samples to values (in px) for the screen buffer
        int valuesNeeded = (int) float(nSamples) / ratio; // N pixels needed for this update

        if (sbi + valuesNeeded > maxSamples)  // crop number of samples to fit canvas width
        {
            valuesNeeded = maxSamples - sbi;
        }
        float subSampleOffset = 0.0;

        dbi %= displayBufferSize; // make sure we're not overshooting
        int nextPos = (dbi + 1) % displayBufferSize; //  position next to displayBufferIndex in display buffer to copy from

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
                    float alpha = (float) subSampleOffset;
                    float invAlpha = 1.0f - alpha;

                    screenBuffer->clear(channel, sbi, 1);
                    screenBufferMean->clear(channel, sbi, 1);
                    screenBufferMin->clear(channel, sbi, 1);
                    screenBufferMax->clear(channel, sbi, 1);

                    dbi %= displayBufferSize; // just to be sure
                    
                    // update continuous data channels
                    if (channel != nChans)
                    {
                        // interpolate between two samples with invAlpha and alpha
                        screenBuffer->addFrom(channel, // destChannel
                                              sbi, // destStartSample
                                              displayBuffer->getReadPointer(channel, dbi), // source
                                              1, // numSamples
                                              invAlpha*gain); // gain


                        screenBuffer->addFrom(channel, // destChannel
                                              sbi, // destStartSample
                                              displayBuffer->getReadPointer(channel, nextPos), // source
                                              1, // numSamples
                                              alpha*gain); // gain
                    }

                    // same thing again, but this time add the min,mean, and max of all samples in current pixel
                    float sample_min   =  10000000;
                    float sample_max   = -10000000;
                    float sample_mean  =  0;
                    
                    int nextpix = (dbi +(int)ratio +1) % (displayBufferSize+1); //  position to next pixels index
                    
                    if (nextpix <= dbi) { // at the end of the displaybuffer, this can occur and it causes the display to miss one pixel woth of sample - this circumvents that
                    //    std::cout << "np " ;
                        nextpix=dbi;
                    }
                   
                    for (int j = dbi; j < nextpix; j++)
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
                       
                    }
                    
                    // update event channel
                    if (channel == nChans)
                    {
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
                                samplesPerPixel[channel][sbi][c]=sample_current;
                                c++;
                            }
                            if (c>0){
                                sampleCountPerPixel[sbi]=c-1; // save count of samples for this pixel
                            }else{
                                sampleCountPerPixel[sbi]=0;
                            }
                            sample_mean = sample_mean/c;
                            screenBufferMean->addSample(channel, sbi, sample_mean*gain);
                            
                            screenBufferMin->addSample(channel, sbi, sample_min*gain);
                            screenBufferMax->addSample(channel, sbi, sample_max*gain);
                        }
                    sbi++;
                }
            
                subSampleOffset += ratio;
                
                while (subSampleOffset >= 1.0)
                {
                    if (++dbi > displayBufferSize)
                        dbi = 0;
                    
                    nextPos = (dbi + 1) % displayBufferSize;
                    subSampleOffset -= 1.0;
                }
                
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

int LfpDisplayCanvas::getChannelSampleRate(int channel)
{
    return sampleRate[channel];
}

void LfpDisplayCanvas::setDrawableSampleRate(float samplerate)
{
//    std::cout << "setting the drawable sample rate in the canvas" << std::endl;
    lfpDisplay->setDisplayedSampleRate(samplerate);
}

void LfpDisplayCanvas::setDrawableSubprocessor(int idx)
{
    lfpDisplay->setDisplayedSubprocessor(idx);
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



#pragma mark - ShowHideOptionsButton -
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



#pragma mark - LfpDisplayOptions -
// -------------------------------------------------------------

LfpDisplayOptions::LfpDisplayOptions(LfpDisplayCanvas* canvas_, LfpTimescale* timescale_, 
                                     LfpDisplay* lfpDisplay_, LfpDisplayNode* processor_)
    : canvas(canvas_),
      lfpDisplay(lfpDisplay_),
      timescale(timescale_),
      processor(processor_),
      selectedChannelType(DataChannel::HEADSTAGE_CHANNEL),
      labelFont("Default", 13.0f, Font::plain),
      labelColour(100, 100, 100)
{
    // draw the colour scheme options
    // TODO: (kelly) this might be better as a modal window
    colourSchemeOptionLabel = new Label("colorSchemeOptionLabel", "Color Scheme");
    colourSchemeOptionLabel->setFont(labelFont);
    colourSchemeOptionLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(colourSchemeOptionLabel);
    
    StringArray colourSchemeNames = lfpDisplay->getColourSchemeNameArray();
    colourSchemeOptionSelection = new ComboBox("colorSchemeOptionSelection");
    colourSchemeOptionSelection->addItemList(colourSchemeNames, 1);
    colourSchemeOptionSelection->setEditableText(false);
    colourSchemeOptionSelection->addListener(this);
    colourSchemeOptionSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(colourSchemeOptionSelection);
    
    if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
        addAndMakeVisible(lfpDisplay->getColourSchemePtr());
    
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
    
    
    
    // init channel display skipping options
    channelDisplaySkipOptions.add("All");
    channelDisplaySkipOptions.add("2");
    channelDisplaySkipOptions.add("4");
    channelDisplaySkipOptions.add("8");
    channelDisplaySkipOptions.add("16");
    channelDisplaySkipOptions.add("32");
    channelDisplaySkipOptions.add("64");
    selectedChannelDisplaySkip = 1;
    selectedChannelDisplaySkipValue = channelDisplaySkipOptions[selectedChannelDisplaySkip - 1];
    
    channelDisplaySkipSelection = new ComboBox("Channel Skip");
    channelDisplaySkipSelection->addItemList(channelDisplaySkipOptions, 1);
    channelDisplaySkipSelection->setSelectedId(selectedChannelDisplaySkip, sendNotification);
    channelDisplaySkipSelection->setEditableText(false);
    channelDisplaySkipSelection->addListener(this);
    addAndMakeVisible(channelDisplaySkipSelection);
    
    channelDisplaySkipLabel = new Label("Channel Display Skip", "Ch. Skip");
    channelDisplaySkipLabel->setFont(labelFont);
    channelDisplaySkipLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(channelDisplaySkipLabel);
    
    
    
    // init spike raster options
    spikeRasterSelectionOptions = {"Off", "-50", "-100", "-150", "-200", "-300", "-400", "-500"};
    selectedSpikeRasterThreshold = 1;
    selectedSpikeRasterThresholdValue = spikeRasterSelectionOptions[selectedSpikeRasterThreshold - 1];
    
    spikeRasterSelection = new ComboBox("spikeRasterSelection");
    spikeRasterSelection->addItemList(spikeRasterSelectionOptions, 1);
    spikeRasterSelection->setSelectedId(selectedSpikeRasterThreshold, dontSendNotification);
    spikeRasterSelection->setEditableText(true);
    spikeRasterSelection->addListener(this);
    addAndMakeVisible(spikeRasterSelection);
    
    spikeRasterLabel = new Label("spikeRasterLabel", "Spike Raster Thresh.");
    spikeRasterLabel->setFont(labelFont);
    spikeRasterLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(spikeRasterLabel);
    
    
    
    // init median offset plotting
    medianOffsetPlottingLabel = new Label("Median Offset Correction", "Median Offset Correction");
    medianOffsetPlottingLabel->setFont(labelFont);
    medianOffsetPlottingLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(medianOffsetPlottingLabel);
    
    medianOffsetPlottingButton = new UtilityButton("0", labelFont);
    medianOffsetPlottingButton->setRadius(5.0f);
    medianOffsetPlottingButton->setEnabledState(true);
    medianOffsetPlottingButton->setCorners(true, true, true, true);
    medianOffsetPlottingButton->addListener(this);
    medianOffsetPlottingButton->setClickingTogglesState(true);
    medianOffsetPlottingButton->setToggleState(false, sendNotification);
    addAndMakeVisible(medianOffsetPlottingButton);

    
    
    // init show/hide options button
    showHideOptionsButton = new ShowHideOptionsButton(this);
    showHideOptionsButton->addListener(this);
    addAndMakeVisible(showHideOptionsButton);

    // init timebases options
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
    
    // toggle button to reverse the order of channels
    reverseChannelsDisplayButton = new UtilityButton("0", labelFont);
    reverseChannelsDisplayButton->setRadius(5.0f);
    reverseChannelsDisplayButton->setEnabledState(true);
    reverseChannelsDisplayButton->setCorners(true, true, true, true);
    reverseChannelsDisplayButton->addListener(this);
    reverseChannelsDisplayButton->setClickingTogglesState(true);
    reverseChannelsDisplayButton->setToggleState(lfpDisplay->getChannelsReversed(), sendNotification);
    addAndMakeVisible(reverseChannelsDisplayButton);
    
    reverseChannelsDisplayLabel = new Label("Rev. Channels", "Rev. Channels");
    reverseChannelsDisplayLabel->setFont(labelFont);
    reverseChannelsDisplayLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(reverseChannelsDisplayLabel);
    

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
    brightnessSliderA = new Slider();
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

    invertInputButton->setBounds(35,getHeight()-190,100,22);
    drawMethodButton->setBounds(35,getHeight()-160,100,22);

    pauseButton->setBounds(450,getHeight()-50,50,44);
    
    
    // Reverse Channels Display
    reverseChannelsDisplayButton->setBounds(pauseButton->getRight() + 5,
                                 getHeight() - 50,
                                 20,
                                 20);
    reverseChannelsDisplayLabel->setBounds(reverseChannelsDisplayButton->getRight(),
                                           reverseChannelsDisplayButton->getY(),
                                           120,
                                           22);
    
    // Channel Display Skip Selector
    channelDisplaySkipSelection->setBounds(reverseChannelsDisplayButton->getX(),
                                           reverseChannelsDisplayButton->getBottom(),
                                           60,
                                           25);
    channelDisplaySkipLabel->setBounds(channelDisplaySkipSelection->getRight(),
                                       channelDisplaySkipSelection->getY() + 2,
                                       100,
                                       22);
    
    // Median Offset Plotting Button
    medianOffsetPlottingButton->setBounds(reverseChannelsDisplayLabel->getRight() + 5,
                                          reverseChannelsDisplayButton->getY(),
                                          20,
                                          20);
    medianOffsetPlottingLabel->setBounds(medianOffsetPlottingButton->getRight(),
                                         medianOffsetPlottingButton->getY(),
                                         150,
                                         22);
    
    
    // Spike raster plotting button
    spikeRasterSelection->setBounds(medianOffsetPlottingButton->getX(),
                                    medianOffsetPlottingButton->getBottom(),
                                    60,
                                    25);
    spikeRasterLabel->setBounds(spikeRasterSelection->getRight(),
                                spikeRasterSelection->getY(),
                                120,
                                22);
    
    
    // Saturation Warning Selection
    saturationWarningSelection->setBounds(250, getHeight()-90, 60, 25);
    
    
    for (int i = 0; i < 8; i++)
    {
        eventDisplayInterfaces[i]->setBounds(300+(floor(i/2)*20), getHeight()-40+(i%2)*20, 40, 20); // arrange event channel buttons in two rows
        eventDisplayInterfaces[i]->repaint();
    }
    
    brightnessSliderA->setBounds(170,getHeight()-190,100,22);
    sliderALabel->setBounds(270, getHeight()-190, 180, 22);
    brightnessSliderA->setValue(0.9); //set default value
    
    brightnessSliderB->setBounds(170,getHeight()-160,100,22);
    sliderBLabel->setBounds(270, getHeight()-160, 180, 22);
    brightnessSliderB->setValue(0.1); //set default value

    showHideOptionsButton->setBounds (getWidth() - 28, getHeight() - 28, 20, 20);
    
    int bh = 25/typeButtons.size();
    for (int i = 0; i < typeButtons.size(); i++)
    {
        typeButtons[i]->setBounds(95,getHeight()-30+i*bh,50,bh);
    }
    
    
    colourSchemeOptionLabel->setBounds(medianOffsetPlottingButton->getX(),
                                       getHeight()-190,
                                       100,
                                       22);
    colourSchemeOptionSelection->setBounds(colourSchemeOptionLabel->getRight(),
                                           colourSchemeOptionLabel->getY(),
                                           80,
                                           25);
    
    // set the size of the active colour scheme's options, if it has configurable options
    if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
    {
        lfpDisplay->getColourSchemePtr()->setBounds(colourSchemeOptionLabel->getX(),
                                                    colourSchemeOptionLabel->getBottom(),
                                                    200,
                                                    110);
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

bool LfpDisplayOptions::getDisplaySpikeRasterizerState()
{
//    return spikeRasterButton->getToggleState();
    return false;
}

void LfpDisplayOptions::setDisplaySpikeRasterizerState(bool isEnabled)
{
//    spikeRasterButton->setToggleState(isEnabled, dontSendNotification);
    
//    if (isEnabled) medianOffsetPlottingButton->setToggleState(true, sendNotification);
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

void LfpDisplayOptions::setSpreadSelection(int spread, bool canvasMustUpdate, bool deferDisplayRefresh)
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

        if (!deferDisplayRefresh)
        {
            canvas->repaint();
            canvas->refresh();
        }
    }
}

void LfpDisplayOptions::togglePauseButton(bool sendUpdate)
{
    pauseButton->setToggleState(!pauseButton->getToggleState(), sendUpdate ? sendNotification : dontSendNotification);
}

void LfpDisplayOptions::buttonClicked(Button* b)
{
    if (b == invertInputButton)
    {
        lfpDisplay->setInputInverted(b->getToggleState());
        return;
    }
    if (b == reverseChannelsDisplayButton)
    {
        lfpDisplay->setChannelsReversed(b->getToggleState());
        return;
    }
    if (b == medianOffsetPlottingButton)
    {
        if (lfpDisplay->getSpikeRasterPlotting())
        {
            medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
        }
        else
        {
            lfpDisplay->setMedianOffsetPlotting(b->getToggleState());
        }
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

void LfpDisplayOptions::setTimebaseAndSelectionText(float timebase)
{
    canvas->timebase = timebase;
    
    if (canvas->timebase) // if timebase != 0
    {
        if (canvas->timebase < timebases[0].getFloatValue())
        {
            timebaseSelection->setSelectedId(1, dontSendNotification);
            canvas->timebase = timebases[0].getFloatValue();
        }
        else if (canvas->timebase > timebases[timebases.size()-1].getFloatValue())
        {
            timebaseSelection->setSelectedId(timebases.size(), dontSendNotification);
            canvas->timebase = timebases[timebases.size()-1].getFloatValue();
        }
        else{
            timebaseSelection->setText(String(canvas->timebase, 1), dontSendNotification);
        }
    }
    else
    {
        if (selectedSpread == 0)
        {
            timebaseSelection->setText(selectedTimebaseValue, dontSendNotification);
            canvas->timebase = selectedTimebaseValue.getFloatValue();
        }
        else
        {
            timebaseSelection->setSelectedId(selectedTimebase,dontSendNotification);
            canvas->timebase = timebases[selectedTimebase-1].getFloatValue();
        }
        
    }
}


void LfpDisplayOptions::comboBoxChanged(ComboBox* cb)
{
    if (canvas->getNumChannels() == 0) return;
    
    if (cb == channelDisplaySkipSelection)
    {
        const int skipAmt = pow(2, cb->getSelectedId() - 1);
        lfpDisplay->setChannelDisplaySkipAmount(skipAmt);
    }
    else if (cb == spikeRasterSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val == 0) // if value is zero, just disable plotting and set text to "Off"
            {
                cb->setSelectedItemIndex(0, dontSendNotification);
                lfpDisplay->setSpikeRasterPlotting(false);
                return;
            }
            
            if (val > 500)
            {
                val = 500;
            }
            
            val *= -1;
            
            spikeRasterSelection->setText(String(val), dontSendNotification);
            lfpDisplay->setSpikeRasterThreshold(val);
            medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
            lfpDisplay->setMedianOffsetPlotting(true);
            lfpDisplay->setSpikeRasterPlotting(true);
        }
        else if (cb->getSelectedItemIndex() == 0) // if "Off"
        {
            lfpDisplay->setSpikeRasterPlotting(false);
            return;
        }
        else
        {
            auto val = cb->getText().getFloatValue();
            
            lfpDisplay->setSpikeRasterThreshold(val);
            medianOffsetPlottingButton->setToggleState(true, dontSendNotification);
            lfpDisplay->setMedianOffsetPlotting(true);
            lfpDisplay->setSpikeRasterPlotting(true);
        }
    }
    else if (cb == colourSchemeOptionSelection)
    {
        // hide the old colour scheme config options if they are displayed
        if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
            removeChildComponent(lfpDisplay->getColourSchemePtr());
        
        // change the active colour scheme ptr
        lfpDisplay->setActiveColourSchemeIdx(cb->getSelectedId()-1);
        
        // show the new colour scheme's config options if has any
        
        if (lfpDisplay->getColourSchemePtr()->hasConfigurableElements())
        {
            lfpDisplay->getColourSchemePtr()->setBounds(colourSchemeOptionLabel->getX(),
                                                        colourSchemeOptionLabel->getBottom(),
                                                        200,
                                                        110);
            addAndMakeVisible(lfpDisplay->getColourSchemePtr());
        }
        
        // update the lfpDisplay's colors and redraw
        lfpDisplay->setColors();
        canvas->redraw();
    }
    else if (cb == timebaseSelection)
    {
        if (cb->getSelectedId())
        {
            canvas->timebase = timebases[cb->getSelectedId()-1].getFloatValue();
        }
        else
        {
            setTimebaseAndSelectionText(cb->getText().getFloatValue());
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
            if (lfpDisplay->getSingleChannelState())
            {
                lfpDisplay->cacheNewChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
            }
            else
            {
                lfpDisplay->setChannelHeight(spreads[cb->getSelectedId()-1].getIntValue());
                resized();
            }
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
                
                // if single channel focus is on, cache the value
                if (lfpDisplay->getSingleChannelState())
                {
                    lfpDisplay->cacheNewChannelHeight(spread);
                }
                else
                {
                    lfpDisplay->setChannelHeight(spread);
                    canvas->resized();
                }
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

        if (!lfpDisplay->getSingleChannelState()) canvas->redraw();
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
    // TODO: (kelly) add savers for:
    //      - channel reverse
    //      - channel zoom slider
    //      - channel display skip
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
    // TODO: (kelly) add loaders for:
    //      - channel reverse
    //      - channel zoom slider
    //      - channel display skip
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


#pragma mark - LfpTimescale -
// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplayCanvas* c, LfpDisplay* lfpDisplay)
    : canvas(c)
    , lfpDisplay(lfpDisplay)
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

    const String timeScaleUnitLabel = (timebase >= 2)?("s:"):("ms:");
    g.drawText(timeScaleUnitLabel,5,0,100,getHeight(),Justification::left, false);

    const int steps = labels.size() + 1;
    for (int i = 0; i < steps; i++)
    {
        
        // TODO: (kelly) added an extra spatial dimension to the timeline ticks, may be overkill
        if (i == 0)
        {
            g.drawLine(1,
                       0,
                       1,
                       getHeight(),
                       3.0f);
        }
        if (i != 0 && i % 4 == 0)
        {
            g.drawLine(getWidth()/steps*i,
                       0,
                       getWidth()/steps*i,
                       getHeight(),
                       3.0f);
        }
        else if (i != 0 && i % 2 == 0)
        {
            g.drawLine(getWidth()/steps*i,
                       getHeight(),
                       getWidth()/steps*i,
                       getHeight() / 2,
                       3.0f);
        }
        else
        {
            g.drawLine(getWidth()/steps*i,
                       getHeight(),
                       getWidth()/steps*i,
                       3 * getHeight()/4,
                       2.0f);
        }
        

        if (i != 0 && i % 2 == 0)
            g.drawText(labels[i-1],getWidth()/steps*i+3,0,100,getHeight(),Justification::left, false);

    }
}

void LfpTimescale::mouseUp(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown())
    {
        lfpDisplay->trackZoomInfo.isScrollingX = false;
    }
}

void LfpTimescale::resized()
{
    setTimebase(timebase);
}

void LfpTimescale::mouseDrag(const juce::MouseEvent &e)
{
    if (e.mods.isLeftButtonDown()) // double check that we initiate only for left click and hold
    {
        if (e.mods.isCommandDown())  // CTRL + drag -> change channel spacing
        {
            // init state in our track zooming info struct
            if (!lfpDisplay->trackZoomInfo.isScrollingX)
            {
                lfpDisplay->trackZoomInfo.isScrollingX = true;
                lfpDisplay->trackZoomInfo.timescaleStartScale = timebase;
            }

            float timescale = lfpDisplay->trackZoomInfo.timescaleStartScale;
            float dTimescale=0;
            int dragDeltaX = (e.getScreenPosition().getX() - e.getMouseDownScreenX()); // invert so drag up -> scale up

//            std::cout << dragDeltaX << std::endl;
            if (dragDeltaX > 0)
            {
                dTimescale = 0.01 * dragDeltaX;
            }
            else
            {
                // TODO: (kelly) change this to scale appropriately for -dragDeltaX
                if (timescale > 0.25)
                    dTimescale = 0.01 * dragDeltaX;
            }
            
            if (timescale >= 1) // accelerate scrolling for large ranges
                dTimescale *= 4;
            
            if (timescale >= 5)
                dTimescale *= 4;
            
            if (timescale >= 10)
                dTimescale *= 4;
            
            // round dTimescale to the nearest 0.005 sec
            dTimescale = ((dTimescale + (0.005/2)) / 0.005) * 0.005;
            
            float newTimescale = timescale+dTimescale;
            
            if (newTimescale < 0.25) newTimescale = 0.250;
            if (newTimescale > 20) newTimescale = 20;
            
            // don't bother updating if the new timebase is the same as the old (if clipped, for example)
            if (timescale != newTimescale)
            {
                lfpDisplay->options->setTimebaseAndSelectionText(newTimescale);
                setTimebase(canvas->timebase);
            }
        }
    }
}

void LfpTimescale::setTimebase(float t)
{
    timebase = t;

    labels.clear();
    
    const int minWidth = 60;
    labelIncrement = 0.025f;
    
    
    while (getWidth() != 0 &&                                   // setTimebase can be called before LfpTimescale has width
           getWidth() / (timebase / labelIncrement) < minWidth) // so, if width is 0 then don't iterate for scale factor
    {
//        std::cout << getWidth() / (timebase / labelIncrement) << " is smaller than minimum width, calculating new step size" << std::endl;
        if (labelIncrement < 0.2)
            labelIncrement *= 2;
        else
            labelIncrement += 0.2;
    }
    
    for (float i = labelIncrement; i < timebase; i += labelIncrement)
    {
        String labelString = String(i * ((timebase >= 2)?(1):(1000.0f)));
        labels.add(labelString.substring(0,6));
    }

    repaint();

}



#pragma mark - LfpDisplay -
// ---------------------------------------------------------------

LfpDisplay::LfpDisplay(LfpDisplayCanvas* c, Viewport* v)
    : singleChan(-1)
    , canvas(c)
    , viewport(v)
    , channelsReversed(false)
    , displaySkipAmt(0)
    , m_SpikeRasterPlottingFlag(false)
{
    perPixelPlotter = new PerPixelBitmapPlotter(this);
    supersampledPlotter = new SupersampledBitmapPlotter(this);
    
//    colorScheme = new LfpDefaultColourScheme();
    colourSchemeList.add(new LfpDefaultColourScheme(this, canvas));
    colourSchemeList.add(new LfpMonochromaticColourScheme(this, canvas));
    colourSchemeList.add(new LfpGradientColourScheme(this, canvas));
    
    activeColourScheme = 0;
    
    
    plotter = perPixelPlotter;
    m_MedianOffsetPlottingFlag = false;
    
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
    
//    setBufferedToImage(true); // TODO: (kelly) test

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
//    deleteAllChildren();
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
    getColourSchemePtr()->setColourGrouping(i);
    setColors(); // so that channel colors get re-assigned

}

LfpChannelColourScheme * LfpDisplay::getColourSchemePtr()
{
    return colourSchemeList[activeColourScheme];
}

void LfpDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;
    

//    deleteAllChildren();
    removeAllChildren();

    channels.clear();
    channelInfo.clear();
    drawableChannels.clear();

    totalHeight = 0;
    cachedDisplayChannelHeight = canvas->getChannelHeight();

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
        lfpInfo->setSubprocessorIdx(canvas->getChannelSubprocessorIdx(i));

        addAndMakeVisible(lfpInfo);

        channelInfo.add(lfpInfo);
        
        drawableChannels.add(LfpChannelTrack{
            lfpChan,
            lfpInfo
        });

		savedChannelState.add(true);

        totalHeight += lfpChan->getChannelHeight();

    }

    setColors();
    

    std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;

}

void LfpDisplay::setColors()
{
    for (int i = 0; i < drawableChannels.size(); i++)
    {

//        channels[i]->setColour(channelColours[(int(i/colorGrouping)+1) % channelColours.size()]);
//        channelInfo[i]->setColour(channelColours[(int(i/colorGrouping)+1)  % channelColours.size()]);
        drawableChannels[i].channel->setColour(getColourSchemePtr()->getColourForIndex(i));
        drawableChannels[i].channelInfo->setColour(getColourSchemePtr()->getColourForIndex(i));
    }

}

void LfpDisplay::setActiveColourSchemeIdx(int index)
{
    activeColourScheme = index;
}

int LfpDisplay::getActiveColourSchemeIdx()
{
    return activeColourScheme;
}

int LfpDisplay::getNumColourSchemes()
{
    return colourSchemeList.size();
}

StringArray LfpDisplay::getColourSchemeNameArray()
{
    StringArray nameList;
    for (auto scheme : colourSchemeList)
        nameList.add(scheme->getName());
    
    return nameList;
}

int LfpDisplay::getTotalHeight()
{
    return totalHeight;
}

void LfpDisplay::resized()
{
    int totalHeight = 0;
    
    for (int i = 0; i < drawableChannels.size(); i++)
    {
        
        LfpChannelDisplay* disp = drawableChannels[i].channel;
        
        if (disp->getHidden()) continue;
        
        disp->setBounds(canvas->leftmargin,
                        totalHeight-(disp->getChannelOverlap()*canvas->channelOverlapFactor)/2,
                        getWidth(),
                        disp->getChannelHeight()+(disp->getChannelOverlap()*canvas->channelOverlapFactor));
        
        disp-> resized();
        
        LfpChannelDisplayInfo* info = drawableChannels[i].channelInfo;
        
        info->setBounds(0,
                        totalHeight-disp->getChannelHeight() + (disp->getChannelOverlap()*canvas->channelOverlapFactor)/4.0,
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
//    for (int i = 0; i < drawableChannels.size(); ++i)
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
    if (!getSingleChannelState()) cachedDisplayChannelHeight = r;
    
    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight(r);
        channelInfo[i]->setChannelHeight(r);
    }
    if (resetSingle && singleChan != -1)
    {
        //std::cout << "width " <<  getWidth() << " numchans  " << numChans << " height " << getChannelHeight() << std::endl;
        setSize(getWidth(),drawableChannels.size()*getChannelHeight());
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
    
    if (isDrawMethod)
    {
        plotter = supersampledPlotter;
    }
    else
    {
        plotter = perPixelPlotter;
    }
    
    resized();

}


int LfpDisplay::getChannelHeight()
{
//    return cachedDisplayChannelHeight;
    return drawableChannels[0].channel->getChannelHeight();
//    return channels[0]->getChannelHeight();
}

void LfpDisplay::cacheNewChannelHeight(int r)
{
    cachedDisplayChannelHeight = r;
}

float LfpDisplay::getDisplayedSampleRate()
{
    return drawableSampleRate;
}

// Must manually call rebuildDrawableChannelsList after this is set, typically will happen
// already as a result of some other procedure
void LfpDisplay::setDisplayedSampleRate(float samplerate)
{
//    std::cout << "Setting the displayed samplerate for LfpDisplayCanvas to " << samplerate << std::endl;
    drawableSampleRate = samplerate;
}

int LfpDisplay::getDisplayedSubprocessor()
{
    return drawableSubprocessorIdx;
}

void LfpDisplay::setDisplayedSubprocessor(int subProcessorIdx)
{
    drawableSubprocessorIdx = subProcessorIdx;
}

bool LfpDisplay::getChannelsReversed()
{
    return channelsReversed;
}

void LfpDisplay::setChannelsReversed(bool state)
{
    if (state == channelsReversed) return; // bail early, in case bookkeeping error
    
    channelsReversed = state;
    
    if (getSingleChannelState()) return; // don't reverse if single channel
    
    // reverse channels that are currently in drawableChannels
    for (size_t i = 0, j = drawableChannels.size() - 1, len = drawableChannels.size()/2;
         i < len;
         i++, j--)
    {
        // remove channel and info components from front and back
        // moving toward middle
        removeChildComponent(drawableChannels[i].channel);
        removeChildComponent(drawableChannels[j].channel);
        removeChildComponent(drawableChannels[i].channelInfo);
        removeChildComponent(drawableChannels[j].channelInfo);
        
        // swap front and back, moving towards middle
        drawableChannels.swap(i, j);
        
        // also swap coords
        {
            const auto channelBoundsA = drawableChannels[i].channel->getBounds();
            const auto channelInfoBoundsA = drawableChannels[i].channelInfo->getBounds();
            
            drawableChannels[i].channel->setBounds(drawableChannels[j].channel->getBounds());
            drawableChannels[i].channelInfo->setBounds(drawableChannels[j].channelInfo->getBounds());
            drawableChannels[j].channel->setBounds(channelBoundsA);
            drawableChannels[j].channelInfo->setBounds(channelInfoBoundsA);
        }
    }
    
    
    // remove middle component if odd number of channels
    if (drawableChannels.size() % 2 != 0)
    {
        removeChildComponent(drawableChannels[drawableChannels.size()/2+1].channel);
        removeChildComponent(drawableChannels[drawableChannels.size()/2+1].channelInfo);
    }
    
    // add the channels and channel info again
    for (size_t i = 0, len = drawableChannels.size(); i < len; i++)
    {
        
        if (!drawableChannels[i].channel->getHidden())
        {
            addAndMakeVisible(drawableChannels[i].channel);
            addAndMakeVisible(drawableChannels[i].channelInfo);
        }
        
        // flag this to update the waveforms
        drawableChannels[i].channel->fullredraw = true;
    }
    
    // necessary to overwrite lfpChannelBitmap's display
    refresh();
}

int LfpDisplay::getChannelDisplaySkipAmount()
{
    return displaySkipAmt;
}

void LfpDisplay::setChannelDisplaySkipAmount(int skipAmt)
{
    displaySkipAmt = skipAmt;
    
    if (!getSingleChannelState())
        rebuildDrawableChannelsList();
    
    canvas->redraw();
}

bool LfpDisplay::getMedianOffsetPlotting()
{
    return m_MedianOffsetPlottingFlag;
}

void LfpDisplay::setMedianOffsetPlotting(bool isEnabled)
{
    m_MedianOffsetPlottingFlag = isEnabled;
}

bool LfpDisplay::getSpikeRasterPlotting()
{
    return m_SpikeRasterPlottingFlag;
}

void LfpDisplay::setSpikeRasterPlotting(bool isEnabled)
{
    m_SpikeRasterPlottingFlag = isEnabled;
}

float LfpDisplay::getSpikeRasterThreshold()
{
    return m_SpikeRasterThreshold;
}

void LfpDisplay::setSpikeRasterThreshold(float thresh)
{
    m_SpikeRasterThreshold = thresh;
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
        
        int newHeight = h+hdiff;
        
        // constrain the spread resizing to max and min values;
        if (newHeight < trackZoomInfo.minZoomHeight)
        {
            newHeight = trackZoomInfo.minZoomHeight;
            hdiff = 0;
        }
        else if (newHeight > trackZoomInfo.maxZoomHeight)
        {
            newHeight = trackZoomInfo.maxZoomHeight;
            hdiff = 0;
        }

        setChannelHeight(newHeight);
        int oldX=viewport->getViewPositionX();
        int oldY=viewport->getViewPositionY();

        setBounds(0,0,getWidth()-0, getChannelHeight()*drawableChannels.size()); // update height so that the scrollbar is correct

        int mouseY=e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
        int scrollBy = (mouseY/h)*hdiff*2;// compensate for motion of point under current mouse position
        viewport->setViewPosition(oldX,oldY+scrollBy); // set back to previous position plus offset

        options->setSpreadSelection(newHeight); // update combobox
        
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
    if (!getSingleChannelState())
    {
        
        std::cout << "Single channel on (" << chan << ")" << std::endl;
        singleChan = chan;
        
        int newHeight = viewport->getHeight();
        LfpChannelTrack lfpChannelTrack{drawableChannels[chan].channel, drawableChannels[chan].channelInfo};
        lfpChannelTrack.channelInfo->setEnabledState(true);
        lfpChannelTrack.channelInfo->setSingleChannelState(true);
        
        removeAllChildren();
        
        // disable unused channels
        for (int i = 0; i < getNumChannels(); i++)
        {
            if (i != chan)
            {
                drawableChannels[i].channel->setEnabledState(false);
            }
        }
        
        // update drawableChannels, give only the single channel to focus on
        Array<LfpChannelTrack> channelsToDraw{lfpChannelTrack};
        drawableChannels = channelsToDraw;
        
        addAndMakeVisible(lfpChannelTrack.channel);
        addAndMakeVisible(lfpChannelTrack.channelInfo);
        
        // set channel height and position (so that we allocate the smallest
        // necessary image size for drawing)
        setChannelHeight(newHeight, false);
        
        lfpChannelTrack.channel->setTopLeftPosition(canvas->leftmargin, 0);
        lfpChannelTrack.channelInfo->setTopLeftPosition(0, 0);
        setSize(getWidth(), getChannelHeight());
        
        viewport->setViewPosition(0, 0);

    }
//    else if (chan == singleChan || chan == -2)
    else
    {
        std::cout << "Single channel off" << std::endl;
        for (int n = 0; n < numChans; n++)
        {
            channelInfo[n]->setSingleChannelState(false);
        }
        
        setChannelHeight(cachedDisplayChannelHeight);

        reactivateChannels();
        rebuildDrawableChannelsList();
    }
}

void LfpDisplay::reactivateChannels()
{

    for (int n = 0; n < channels.size(); n++)
       setEnabledState(savedChannelState[n], n);

}

void LfpDisplay::rebuildDrawableChannelsList()
{
    
    if (displaySkipAmt != 0) removeAllChildren(); // start with clean slate
    
    Array<LfpChannelTrack> channelsToDraw;
    drawableChannels = Array<LfpDisplay::LfpChannelTrack>();
    
    // iterate over all channels and select drawable ones
    for (size_t i = 0, drawableChannelNum = 0; i < channels.size(); i++)
    {
//        std::cout << "\tchannel " << i << " has subprocessor index of "  << channelInfo[i]->getSubprocessorIdx() << std::endl;
        // if channel[i] is not sourced from the correct subprocessor, then hide it and continue
        if (channelInfo[i]->getSubprocessorIdx() != getDisplayedSubprocessor())
        {
            channels[i]->setHidden(true);
            channelInfo[i]->setHidden(true);
            continue;
        }
        
        if (displaySkipAmt == 0 || (i % displaySkipAmt == 0)) // no skips, add all channels
        {
            channels[i]->setHidden(false);
            channelInfo[i]->setHidden(false);
            
            channelInfo[i]->setDrawableChannelNumber(drawableChannelNum++);
            channelInfo[i]->resized(); // to update the conditional drawing of enableButton and channel num
            
            channelsToDraw.add(LfpDisplay::LfpChannelTrack{
                channels[i],
                channelInfo[i]
            });
            
            addAndMakeVisible(channels[i]);
            addAndMakeVisible(channelInfo[i]);
        }
        else // skip some channels
        {
//            if (i % (displaySkipAmt) == 0) // add these channels
//            {
//                channels[i]->setHidden(false);
//                channelInfo[i]->setHidden(false);
//                
//                channelsToDraw.add(LfpDisplay::LfpChannelTrack{
//                    channels[i],
//                    channelInfo[i]
//                });
//                
//                addAndMakeVisible(channels[i]);
//                addAndMakeVisible(channelInfo[i]);
//            }
//            else // but not these
//            {
                channels[i]->setHidden(true);
                channelInfo[i]->setHidden(true);
                
                removeChildComponent(channels[i]);
                removeChildComponent(channelInfo[i]);
//            }
        }
    }
    
    // check if channels should be added to drawableChannels in reverse
    if (getChannelsReversed())
    {
        for (int i = channelsToDraw.size() - 1; i >= 0; --i)
        {
            drawableChannels.add(channelsToDraw[i]);
        }
    }
    else
    {
        for (int i = 0; i < channelsToDraw.size(); ++i)
        {
            drawableChannels.add(channelsToDraw[i]);
        }
    }
    
    // this guards against an exception where the editor sets the drawable samplerate
    // before the lfpDisplay is fully initialized
    if (getHeight() > 0 && getWidth() > 0)
    {
        canvas->resizeToChannels();
    }
    
    setColors();
}

LfpBitmapPlotter * const LfpDisplay::getPlotterPtr() const
{
    return plotter;
}

bool LfpDisplay::getSingleChannelState()
{
    //if (singleChan < 0) return false;
    //else return true;
    return singleChan >= 0;
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
    for (int n = 0; n < drawableChannels.size(); n++) // select closest instead of relying on eventComponent
    {
        drawableChannels[n].channel->deselect();

        int cpos = (drawableChannels[n].channel->getY() + (drawableChannels[n].channel->getHeight()/2));
        dist = int(abs(y - cpos));

//        std::cout << "Mouse down at " << y << " pos is "<< cpos << " n: " << n << "  dist " << dist << std::endl;

        if (dist < mindist)
        {
            mindist = dist-1;
            closest = n;
        }
    }

    drawableChannels[closest].channel->select();
    options->setSelectedType(drawableChannels[closest].channel->getType());

    if (event.mods.isRightButtonDown()) { // if right click
        PopupMenu channelMenu = channels[closest]->getOptions();
        const int result = channelMenu.show();
        drawableChannels[closest].channel->changeParameter(result);
    }
    else // if left click
    {
//    if (singleChan != -1)
        if (event.getNumberOfClicks() == 2) {
            toggleSingleChannel(closest);
        }
        
        if (getSingleChannelState())
        {
            
            //        std::cout << "singleChan = " << singleChan << " " << y << " " << drawableChannels[0].channel->getHeight() << " " << getRange() << std::endl;
            //channelInfo[singleChan]->updateXY(
            drawableChannels[0].channelInfo->updateXY(
                                                      float(x)/getWidth()*canvas->timebase,
                                                      (-(float(y)-viewport->getViewPositionY())/viewport->getViewHeight()*float(getRange()))+float(getRange()/2)
                                                      );
        }
    }

//    canvas->fullredraw = true;//issue full redraw

//    refresh();

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



#pragma mark - LfpChannelDisplay -
// ------------------------------------------------------------------

LfpChannelDisplay::LfpChannelDisplay(LfpDisplayCanvas* c, LfpDisplay* d, LfpDisplayOptions* o, int channelNumber)
    : canvas(c)
    , display(d)
    , options(o)
    , isSelected(false)
    , chan(channelNumber)
    , drawableChan(channelNumber)
    , channelOverlap(300)
    , channelHeight(40)
    , range(1000.0f)
    , isEnabled(true)
    , inputInverted(false)
    , canBeInverted(true)
    , drawMethod(false)
    , isHidden(false)
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

void LfpChannelDisplay::setHidden(bool isHidden_)
{
    isHidden = isHidden_;
    isEnabled = !isHidden;
}

void LfpChannelDisplay::pxPaint()
{
    if (!isEnabled) return; // return early if THIS display is not enabled
    
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
    
    bool drawWithOffsetCorrection = display->getMedianOffsetPlotting();
    
    LfpBitmapPlotterInfo plotterInfo; // hold and pass plotting info for each plotting method class
    
    
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
            
            for (int ev_ch = 0; ev_ch < 8 ; ev_ch++) // for all event channels
            {
                if (display->getEventDisplayState(ev_ch))  // check if plotting for this channel is enabled
                {
                    if (rawEventState & (1 << ev_ch))    // events are  representet by a bit code, so we have to extract the individual bits with a mask
                    {
//                        std::cout << "Drawing event." << std::endl;
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
            
            double mean = (canvas->getMean(chan)/range*channelHeightFloat);
            
            if (drawWithOffsetCorrection)
            {
                a -= mean;
                b -= mean;
            }
            
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
            
            bool spikeFlag = display->getSpikeRasterPlotting()
                && !(saturateWarningHi || saturateWarningLo)
                && (from_raw - canvas->getYCoordMean(chan, i) < display->getSpikeRasterThreshold()
                        || to_raw - canvas->getYCoordMean(chan, i) < display->getSpikeRasterThreshold());
            
            from = from + getHeight()/2;       // so the plot is centered in the channeldisplay
            to = to + getHeight()/2;
            
            int samplerange = to - from;
            
            if (drawMethod) // switched between 'supersampled' drawing and simple pixel wise drawing
            { // histogram based supersampling method
                plotterInfo.channelID = chan;
                plotterInfo.samp = i;
                plotterInfo.y = getY();
                plotterInfo.from = from;
                plotterInfo.height = getHeight();
                plotterInfo.lineColourBright = lineColourBright;
                plotterInfo.lineColourDark = lineColourDark;
                plotterInfo.range = range;
                plotterInfo.channelHeightFloat = channelHeightFloat;
                plotterInfo.sampleCountPerPixel = canvas->getSampleCountPerPixel(i);
                plotterInfo.samplesPerPixel = canvas->getSamplesPerPixel(chan, i);
                plotterInfo.histogramParameterA = canvas->histogramParameterA;
                plotterInfo.samplerange = samplerange;
                
                // TODO: (kelly) complete transition toward plotter class encapsulation
//                display->getPlotterPtr()->plot(bdLfpChannelBitmap, plotterInfo);
                
            }
            else //drawmethod
            { // simple per-pixel min-max drawing, has no anti-aliasing, but runs faster
                
                plotterInfo.channelID = chan;
                plotterInfo.y = getY();
                plotterInfo.from = from;
                plotterInfo.to = to;
                plotterInfo.samp = i;
                plotterInfo.lineColour = lineColour;
                
                // TODO: (kelly) complete transition toward plotter class encapsulation
//                display->getPlotterPtr()->plot(bdLfpChannelBitmap, plotterInfo); // plotterInfo is prepared above
                
            }
            
            // Do the actual plotting for the selected plotting method
            if (!display->getSpikeRasterPlotting())
                display->getPlotterPtr()->plot(bdLfpChannelBitmap, plotterInfo);
            
                
                
                
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
            
            if (spikeFlag) // draw spikes
            {
                for (int k=jfrom_wholechannel; k<=jto_wholechannel; k++){ // draw line
                    if(k>0 & k<display->lfpChannelBitmap.getHeight()){
                        bdLfpChannelBitmap.setPixelColour(i,k,lineColour);
                    }
                };
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

int LfpChannelDisplay::getChannelNumber()
{
    return chan;
}

int LfpChannelDisplay::getDrawableChannelNumber()
{
    return drawableChan;
}

void LfpChannelDisplay::setDrawableChannelNumber(int channelId)
{
    drawableChan = channelId;
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



#pragma mark - LfpChannelDisplayInfo -
// -------------------------------

LfpChannelDisplayInfo::LfpChannelDisplayInfo(LfpDisplayCanvas* canvas_, LfpDisplay* display_, LfpDisplayOptions* options_, int ch)
    : LfpChannelDisplay(canvas_, display_, options_, ch)
{

    chan = ch;
    x = -1.0f;
    y = -1.0f;

//    enableButton = new UtilityButton(String(ch+1), Font("Small Text", 13, Font::plain));
    enableButton = new UtilityButton("*", Font("Small Text", 13, Font::plain));
    enableButton->setRadius(5.0f);

    enableButton->setEnabledState(true);
    enableButton->setCorners(true, true, true, true);
    enableButton->addListener(this);
    enableButton->setClickingTogglesState(true);
    enableButton->setToggleState(true, dontSendNotification);
    
    isSingleChannel = false;

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

int LfpChannelDisplayInfo::getChannelSampleRate()
{
    return samplerate;
}

void LfpChannelDisplayInfo::setChannelSampleRate(int samplerate_)
{
    samplerate = samplerate_;
}

void LfpChannelDisplayInfo::mouseDrag(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown()) // double check that we initiate only for left click and hold
    {
        if (e.mods.isCommandDown() && !display->getSingleChannelState())  // CTRL + drag -> change channel spacing
        {
            
            // init state in our track zooming info struct
            if (!display->trackZoomInfo.isScrollingY)
            {
                auto & zoomInfo = display->trackZoomInfo;
                
                zoomInfo.isScrollingY = true;
                zoomInfo.componentStartHeight = getChannelHeight();
                zoomInfo.zoomPivotRatioY = (getY() + e.getMouseDownY())/(float)display->getHeight();
                zoomInfo.zoomPivotRatioX = (getX() + e.getMouseDownX())/(float)display->getWidth();
                zoomInfo.zoomPivotViewportOffset = getPosition() + e.getMouseDownPosition() - canvas->viewport->getViewPosition();
                
                zoomInfo.unpauseOnScrollEnd = !display->isPaused;
                if (!display->isPaused) display->options->togglePauseButton(true);
            }
            
            int h = display->trackZoomInfo.componentStartHeight;
            int hdiff=0;
            int dragDeltaY = -0.1 * (e.getScreenPosition().getY() - e.getMouseDownScreenY()); // invert so drag up -> scale up
            
//             std::cout << dragDeltaY << std::endl;
            if (dragDeltaY > 0)
            {
                hdiff = 2 * dragDeltaY;
            }
            else
            {
                if (h > 5)
                    hdiff = 2 * dragDeltaY;
            }
            
            if (abs(h) > 100) // accelerate scrolling for large ranges
                hdiff *= 3;
            
            int newHeight = h+hdiff;
            
            // constrain the spread resizing to max and min values;
            if (newHeight < display->trackZoomInfo.minZoomHeight)
            {
                newHeight = display->trackZoomInfo.minZoomHeight;
            }
            else if (newHeight > display->trackZoomInfo.maxZoomHeight)
            {
                newHeight = display->trackZoomInfo.maxZoomHeight;
            }
            
            // return early if there is nothing to update
            if (newHeight == getChannelHeight())
            {
                return;
            }
            
            // set channel heights for all channel
//            display->setChannelHeight(newHeight);
            for (int i = 0; i < display->getNumChannels(); ++i)
            {
                display->channels[i]->setChannelHeight(newHeight);
                display->channelInfo[i]->setChannelHeight(newHeight);
            }
            
            options->setSpreadSelection(newHeight, false, true); // update combobox
            
            canvas->fullredraw = true;//issue full redraw - scrolling without modifier doesnt require a full redraw
            
            display->setBounds(0,0,display->getWidth()-0, display->getChannelHeight()*display->drawableChannels.size()); // update height so that the scrollbar is correct
            
            int newViewportY = display->trackZoomInfo.zoomPivotRatioY * display->getHeight() - display->trackZoomInfo.zoomPivotViewportOffset.getY();
            if (newViewportY < 0) newViewportY = 0; // make sure we don't adjust beyond the edge of the actual view
            
            canvas->viewport->setViewPosition(0, newViewportY);
        }
    }
}

void LfpChannelDisplayInfo::mouseUp(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown() && display->trackZoomInfo.isScrollingY)
    {
        display->trackZoomInfo.isScrollingY = false;
        if (display->trackZoomInfo.unpauseOnScrollEnd)
        {
            display->isPaused = false;
            display->options->togglePauseButton(false);
        }
    }
}

void LfpChannelDisplayInfo::paint(Graphics& g)
{

    int center = getHeight()/2 - (isSingleChannel?(75):(0));

//    g.setColour(lineColour);
    //if (chan > 98)
    //  g.fillRoundedRectangle(5,center-8,51,22,8.0f);
    //else
    
//    g.fillRoundedRectangle(5,center-8,41,22,8.0f);
    
    // Draw the channel numbers
    g.setColour(Colours::grey);
    const String channelString = (isChannelNumberHidden() ? ("--") : String(getChannelNumber() + 1));
    bool isCentered = !getEnabledButtonVisibility();
    
    g.drawText(channelString,
               2,
               center-4,
               isCentered ? (getWidth()/2-4) : (getWidth()/4),
               10,
               isCentered ? Justification::centred : Justification::centredRight,
               false);
    
    g.setColour(lineColour);
    g.fillRect(0, 0, 2, getHeight());
    
    if (getChannelTypeStringVisibility())
    {
        g.setFont(Font("Small Text", 13, Font::plain));
        g.drawText(typeStr,5,center+10,41,10,Justification::centred,false);
    }
    // g.setFont(channelHeightFloat*0.3);
    g.setFont(Font("Small Text", 11, Font::plain));

    if (isSingleChannel)
    {
        g.setColour(Colours::darkgrey);
        g.drawText("STD:", 5, center+90,41,10,Justification::centred,false);
        g.drawText("MEAN:", 5, center+40,41,10,Justification::centred,false);
        
        if (x > 0)
        {
            g.drawText("uV:", 5, center+140,41,10,Justification::centred,false);
        }
        //g.drawText("Y:", 5, center+200,41,10,Justification::centred,false);

        g.setColour(Colours::grey);
        g.drawText(String(canvas->getStd(chan)), 5, center+110,41,10,Justification::centred,false);
        g.drawText(String(canvas->getMean(chan)), 5, center+60,41,10,Justification::centred,false);
        if (x > 0)
        {
            //g.drawText(String(x), 5, center+150,41,10,Justification::centred,false);
            g.drawText(String(y), 5, center+160,41,10,Justification::centred,false);
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

    int center = getHeight()/2 - (isSingleChannel?(75):(0));

    //if (chan > 98)
    //  enableButton->setBounds(8,center-5,45,16);
    //else
//    enableButton->setBounds(8,center-5,35,16);
    
    setEnabledButtonVisibility(getHeight() >= 16);
    
    if (getEnabledButtonVisibility())
    {
        enableButton->setBounds(getWidth()/4 + 5, (center) - 7, 15, 15);
    }
    
    setChannelNumberIsHidden(getHeight() < 16 && (getDrawableChannelNumber() + 1) % 10 != 0);
    
    setChannelTypeStringVisibility(getHeight() > 34);
}

void LfpChannelDisplayInfo::setEnabledButtonVisibility(bool shouldBeVisible)
{
    if (shouldBeVisible)
    {
        addAndMakeVisible(enableButton);
    }
    else if (enableButton->isVisible())
    {
        removeChildComponent(enableButton);
        enableButton->setVisible(false);
    }
    
}

bool LfpChannelDisplayInfo::getEnabledButtonVisibility()
{
    return enableButton->isVisible();
}

void LfpChannelDisplayInfo::setChannelTypeStringVisibility(bool shouldBeVisible)
{
    channelTypeStringIsVisible = shouldBeVisible;
}

bool LfpChannelDisplayInfo::getChannelTypeStringVisibility()
{
    return channelTypeStringIsVisible || isSingleChannel;
}

void LfpChannelDisplayInfo::setChannelNumberIsHidden(bool shouldBeHidden)
{
    channelNumberHidden = shouldBeHidden;
}

bool LfpChannelDisplayInfo::isChannelNumberHidden()
{
    return channelNumberHidden;
}




#pragma mark - EventDisplayInterface -
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



#pragma mark - LfpViewport -
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



#pragma mark - PerPixelBitmapPlotter -

PerPixelBitmapPlotter::PerPixelBitmapPlotter(LfpDisplay * lfpDisplay)
    : LfpBitmapPlotter(lfpDisplay)
{ }

void PerPixelBitmapPlotter::plot(Image::BitmapData &bitmapData, LfpBitmapPlotterInfo &pInfo)
{
    int jfrom = pInfo.from + pInfo.y;
    int jto = pInfo.to + pInfo.y;
    
    //if (yofs<0) {yofs=0;};
    
    if (pInfo.samp < 0) {pInfo.samp = 0;};
    if (pInfo.samp >= display->lfpChannelBitmap.getWidth()) {pInfo.samp = display->lfpChannelBitmap.getWidth()-1;}; // this shouldnt happen, there must be some bug above - to replicate, run at max refresh rate where draws overlap the right margin by a lot
    
    if (jfrom<0) {jfrom=0;};
    if (jto >= display->lfpChannelBitmap.getHeight()) {jto=display->lfpChannelBitmap.getHeight()-1;};
    
    
    for (int j = jfrom; j <= jto; j += 1)
    {
        
        //uint8* const pu8Pixel = bdSharedLfpDisplay.getPixelPointer(	(int)(i),(int)(j));
        //*(pu8Pixel)		= 200;
        //*(pu8Pixel+1)	= 200;
        //*(pu8Pixel+2)	= 200;
        
        bitmapData.setPixelColour(pInfo.samp,j,pInfo.lineColour);
        
    }
}



#pragma mark - LfpSupersampledBitmapPlotter -

SupersampledBitmapPlotter::SupersampledBitmapPlotter(LfpDisplay * lfpDisplay)
    : LfpBitmapPlotter(lfpDisplay)
{ }

void SupersampledBitmapPlotter::plot(Image::BitmapData &bdLfpChannelBitmap, LfpBitmapPlotterInfo &pInfo)
{
    std::array<float, MAX_N_SAMP_PER_PIXEL> samplesThisPixel = pInfo.samplesPerPixel;
//    int sampleCountThisPixel = lfpDisplay->canvas->getSampleCountPerPixel(pInfo.samp);
    int sampleCountThisPixel = pInfo.sampleCountPerPixel;
    
    if (pInfo.samplerange>0 & sampleCountThisPixel>0)
    {
        
        //float localHist[samplerange]; // simple histogram
        Array<float> rangeHist; // [samplerange]; // paired range histogram, same as plotting at higher res. and subsampling
        
        for (int k = 0; k <= pInfo.samplerange; k++)
            rangeHist.add(0);
        
        for (int k = 0; k <= sampleCountThisPixel; k++) // add up paired-range histogram per pixel - for each pair fill intermediate with uniform distr.
        {
            int cs_this = (((samplesThisPixel[k]/pInfo.range*pInfo.channelHeightFloat)+pInfo.height/2)-pInfo.from); // sample values -> pixel coordinates relative to from
            int cs_next = (((samplesThisPixel[k+1]/pInfo.range*pInfo.channelHeightFloat)+pInfo.height/2)-pInfo.from);
            
            
            if (cs_this<0) {cs_this=0;};                        //here we could clip the diaplay to the max/min, or ignore out of bound values, not sure which one is better
            if (cs_this>pInfo.samplerange) {cs_this=pInfo.samplerange;};
            if (cs_next<0) {cs_next=0;};
            if (cs_next>pInfo.samplerange) {cs_next=pInfo.samplerange;};
            
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
        
        
        for (int s = 0; s <= pInfo.samplerange; s ++)  // plot histogram one pixel per bin
        {
            float a=15*((rangeHist[s])/(sampleCountThisPixel)) * (2*(0.2+pInfo.histogramParameterA));
            if (a>1.0f) {a=1.0f;};
            if (a<0.0f) {a=0.0f;};
            
            
            //Colour gradedColor = lineColour.withMultipliedBrightness(2.0f).interpolatedWith(lineColour.withMultipliedSaturation(0.6f).withMultipliedBrightness(0.3f),1-a) ;
            Colour gradedColor =  pInfo.lineColourBright.interpolatedWith(pInfo.lineColourDark,1-a);
            //Colour gradedColor =  Colour(0,255,0);
            
            int ploty = pInfo.from + s + pInfo.y;
            if(ploty>0 & ploty < display->lfpChannelBitmap.getHeight()) {
                bdLfpChannelBitmap.setPixelColour(pInfo.samp, pInfo.from + s + pInfo.y, gradedColor);
            }
        }
        
    } else {
        
        int ploty = pInfo.from + pInfo.y;
        if(ploty>0 && ploty < display->lfpChannelBitmap.getHeight()) {
            bdLfpChannelBitmap.setPixelColour(pInfo.samp, ploty, pInfo.lineColour);
        }
    }
}



#pragma mark - LfpChannelColourScheme -

int LfpChannelColourScheme::colourGrouping = 1;

void LfpChannelColourScheme::setColourGrouping(int grouping)
{
    colourGrouping = grouping;
}

int LfpChannelColourScheme::getColourGrouping()
{
    return colourGrouping;
}



#pragma mark - LfpDefaultColourScheme -

Array<Colour> LfpDefaultColourScheme::colourList = []() -> Array<Colour> {
    Array<Colour> colours;
    colours.add(Colour(224,185,36));
    colours.add(Colour(214,210,182));
    colours.add(Colour(243,119,33));
    colours.add(Colour(186,157,168));
    colours.add(Colour(237,37,36));
    colours.add(Colour(179,122,79));
    colours.add(Colour(217,46,171));
    colours.add(Colour(217, 139,196));
    colours.add(Colour(101,31,255));
    colours.add(Colour(141,111,181));
    colours.add(Colour(48,117,255));
    colours.add(Colour(184,198,224));
    colours.add(Colour(116,227,156));
    colours.add(Colour(150,158,155));
    colours.add(Colour(82,173,0));
    colours.add(Colour(125,99,32));
    return colours;
}();

LfpDefaultColourScheme::LfpDefaultColourScheme(LfpDisplay* display, LfpDisplayCanvas* canvas)
	: LfpViewer::LfpChannelColourScheme(LfpDefaultColourScheme::colourList.size(), display, canvas)
{
    setName("Default");
}

void LfpDefaultColourScheme::paint(Graphics &g)
{
    
}

void LfpDefaultColourScheme::resized()
{
    
}

const Colour LfpDefaultColourScheme::getColourForIndex(int index) const
{
//    return colourList[index % colourList.size()];
    return colourList[(int(index/colourGrouping)) % colourList.size()];
}



#pragma mark - LfpMonochromaticColorScheme

LfpMonochromaticColourScheme::LfpMonochromaticColourScheme(LfpDisplay* display, LfpDisplayCanvas* canvas)
    : LfpChannelColourScheme (8, display, canvas)
    , isBlackAndWhite (false)
    , colourPattern (DOWN_UP)
{
    setName("Monochromatic");
    
    numChannelsLabel = new Label("numChannelsLabel", "Num Color Steps");
    numChannelsLabel->setFont(Font("Default", 13.0f, Font::plain));
    numChannelsLabel->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(numChannelsLabel);
    
    StringArray numChannelsSelectionOptions = {"4", "8", "16"};
    numChannelsSelection = new ComboBox("numChannelsSelection");
    numChannelsSelection->addItemList(numChannelsSelectionOptions, 1);
    numChannelsSelection->setEditableText(true);
    numChannelsSelection->addListener(this);
    numChannelsSelection->setSelectedId(2, dontSendNotification);
    addAndMakeVisible(numChannelsSelection);
    
    
    baseHueLabel = new Label("baseHue", "Hue");
    baseHueLabel->setFont(Font("Default", 13.0f, Font::plain));
    baseHueLabel->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(baseHueLabel);
    
    baseHueSlider = new Slider;
    baseHueSlider->setRange(0, 1);
    baseHueSlider->setValue(0);
    baseHueSlider->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    baseHueSlider->addListener(this);
    addAndMakeVisible(baseHueSlider);
    
    baseHueSlider->addMouseListener(this, true);
    
    
    colourPatternLabel = new Label("colourPatternLabel", "Pattern");
    colourPatternLabel->setFont(Font("Default", 13.0f, Font::plain));
    colourPatternLabel->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(colourPatternLabel);
    
    StringArray colourPatternSelectionOptions = {"Down", "Up", "Down-Up", "Up-Down"};
    colourPatternSelection = new ComboBox("colourPatternSelection");
    colourPatternSelection->addItemList(colourPatternSelectionOptions, 1);
    colourPatternSelection->setEditableText(false);
    colourPatternSelection->addListener(this);
    colourPatternSelection->setSelectedId(colourPattern + 1, dontSendNotification);
    addAndMakeVisible(colourPatternSelection);
    
    baseHue = Colour::fromHSV(0, 1, 1, 1);
    swatchHue = baseHue;
    
    calculateColourSeriesFromBaseHue();
}

void LfpMonochromaticColourScheme::paint(Graphics &g)
{
    g.setColour(swatchHue);
    g.fillRect(colourSwatchRect);
}

void LfpMonochromaticColourScheme::resized()
{
    numChannelsLabel->setBounds(0, 5, 120, 25);
    numChannelsSelection->setBounds(numChannelsLabel->getRight(),
                                    numChannelsLabel->getY(),
                                    60,
                                    25);
    
    baseHueLabel->setBounds(0, numChannelsLabel->getBottom(), 35, 25);
    baseHueSlider->setBounds(baseHueLabel->getRight(),
                             baseHueLabel->getY(),
                             numChannelsSelection->getRight() - baseHueLabel->getRight() - 20,
                             25);
    
    colourSwatchRect.setBounds(baseHueSlider->getRight() + 5, baseHueSlider->getY() + 5, 15, baseHueSlider->getHeight() - 10);
    
    colourPatternLabel->setBounds(0, baseHueLabel->getBottom(), 80, 25);
    colourPatternSelection->setBounds(colourPatternLabel->getRight(),
                                      colourPatternLabel->getY(),
                                      numChannelsSelection->getRight() - colourPatternLabel->getRight(),
                                      25);
    
}

void LfpMonochromaticColourScheme::sliderValueChanged(Slider *sl)
{
    swatchHue = Colour::fromHSV(sl->getValue(), 1, 1, 1);
    repaint(colourSwatchRect);
}

void LfpMonochromaticColourScheme::comboBoxChanged(ComboBox *cb)
{
    if (cb == numChannelsSelection)
    {
        int numChannelsColourSpread = 0;
        if (cb->getSelectedId())
        {
            numChannelsColourSpread = cb->getText().getIntValue();
        }
        else
        {
            numChannelsColourSpread = cb->getText().getIntValue();
            if (numChannelsColourSpread < 1) numChannelsColourSpread = 1;
            else if (numChannelsColourSpread > 16) numChannelsColourSpread = 16;
            
            cb->setText(String(numChannelsColourSpread), dontSendNotification);
        }
        
        setNumColourSeriesSteps(numChannelsColourSpread);
    }
    else if (cb == colourPatternSelection)
    {
        setColourPattern((ColourPattern)(cb->getSelectedId() - 1));
    }
    calculateColourSeriesFromBaseHue();
    lfpDisplay->setColors();
//    canvas->fullredraw = true;
    canvas->redraw();
}

void LfpMonochromaticColourScheme::mouseUp(const MouseEvent &e)
{
    if (swatchHue.getARGB() != baseHue.getARGB())
    {
        baseHue = swatchHue;
        calculateColourSeriesFromBaseHue();
        lfpDisplay->setColors();
        canvas->redraw();
    }
}

void LfpMonochromaticColourScheme::setBaseHue(Colour base)
{
    baseHue = base;
    calculateColourSeriesFromBaseHue();
}

const Colour LfpMonochromaticColourScheme::getBaseHue() const
{
    return baseHue;
}

void LfpMonochromaticColourScheme::setNumColourSeriesSteps(int numSteps)
{
    numColourChannels = numSteps;
}

int LfpMonochromaticColourScheme::getNumColourSeriesSteps()
{
    return numColourChannels;
}

const Colour LfpMonochromaticColourScheme::getColourForIndex(int index) const
{
    int colourIdx = (int(index/colourGrouping) % numColourChannels);
    
    // adjust for oscillating patterns
    if (colourPattern == DOWN_UP || colourPattern == UP_DOWN)
    {
        int mid = numColourChannels / 2;
        if (colourIdx > mid)
        {
            if (numColourChannels % 2 == 0)
                colourIdx = numColourChannels - colourIdx;
            else
                colourIdx = (numColourChannels - colourIdx) * 2 - 1;
        }
        else if (numColourChannels % 2 != 0)
        {
            colourIdx *= 2;
        }
    }
    
    // invert if the pattern is UP or UP_DOWN
    if (colourPattern == UP)
        colourIdx = (numColourChannels - 1) - colourIdx;
    else if (colourPattern == UP_DOWN)
        colourIdx = (colourList.size() - 1) - colourIdx;
    
    return colourList[colourIdx];
}

void LfpMonochromaticColourScheme::calculateColourSeriesFromBaseHue()
{
    colourList.clear();
    
    int coloursToCalculate = numColourChannels;
    
    if (numColourChannels % 2 == 0 && (colourPattern == DOWN_UP || colourPattern == UP_DOWN))
    {
        coloursToCalculate = coloursToCalculate / 2 + 1;
    }
    
    for (int i = 0; i < coloursToCalculate; ++i)
    {
        float saturation = 1 - (i / float(coloursToCalculate + 1));
        colourList.add(baseHue.withMultipliedSaturation(saturation));
    }
}



#pragma mark - LfpGradientColourScheme

LfpGradientColourScheme::LfpGradientColourScheme(LfpDisplay * display, LfpDisplayCanvas * canvas)
    : LfpMonochromaticColourScheme(display, canvas)
{
    setName("Gradient");
    
    baseHueLabel->setName("baseHueA");
    baseHueLabel->setText("Hue A", dontSendNotification);
    
    baseHueLabelB = new Label("baseHueB", "Hue B");
    baseHueLabelB->setFont(Font("Default", 13.0f, Font::plain));
    baseHueLabelB->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(baseHueLabelB);
    
    baseHueSliderB = new Slider;
    baseHueSliderB->setRange(0, 1);
    baseHueSliderB->setValue(0.5);
    baseHueSliderB->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    baseHueSliderB->addListener(this);
    addAndMakeVisible(baseHueSliderB);
    
    baseHueSliderB->addMouseListener(this, true);
    
    baseHueB = Colour::fromHSV(0.5, 1.0, 1.0, 1.0);
    swatchHueB = baseHueB;
    
    calculateColourSeriesFromBaseHue();
}

void LfpGradientColourScheme::paint(Graphics &g)
{
    g.setColour(swatchHue);
    g.fillRect(colourSwatchRect);
    
    g.setColour(swatchHueB);
    g.fillRect(colourSwatchRectB);
}

void LfpGradientColourScheme::resized()
{
    numChannelsLabel->setBounds(0, 5, 120, 25);
    numChannelsSelection->setBounds(numChannelsLabel->getRight(),
                                    numChannelsLabel->getY(),
                                    60,
                                    25);
    
    baseHueLabel->setBounds(0, numChannelsLabel->getBottom(), 35, 25);
    baseHueSlider->setBounds(baseHueLabel->getRight(),
                             baseHueLabel->getY(),
                             numChannelsSelection->getRight() - baseHueLabel->getRight() - 20,
                             25);
    
    colourSwatchRect.setBounds(baseHueSlider->getRight() + 5, baseHueSlider->getY() + 5, 15, baseHueSlider->getHeight() - 10);
    
    baseHueLabelB->setBounds(0, baseHueLabel->getBottom(), 35, 25);
    baseHueSliderB->setBounds(baseHueLabelB->getRight(),
                             baseHueLabelB->getY(),
                             numChannelsSelection->getRight() - baseHueLabelB->getRight() - 20,
                             25);
    
    colourSwatchRectB.setBounds(baseHueSliderB->getRight() + 5, baseHueSliderB->getY() + 5, 15, baseHueSliderB->getHeight() - 10);
    
    colourPatternLabel->setBounds(0, baseHueLabelB->getBottom(), 80, 25);
    colourPatternSelection->setBounds(colourPatternLabel->getRight(),
                                      colourPatternLabel->getY(),
                                      numChannelsSelection->getRight() - colourPatternLabel->getRight(),
                                      25);
}

void LfpGradientColourScheme::sliderValueChanged(Slider *sl)
{
    if (sl == baseHueSlider)
    {
        swatchHue = Colour::fromHSV(sl->getValue(), 1, 1, 1);
        repaint(colourSwatchRect);
    }
    else
    {
        swatchHueB = Colour::fromHSV(sl->getValue(), 1, 1, 1);
        repaint(colourSwatchRectB);
    }
}

void LfpGradientColourScheme::mouseUp(const MouseEvent &e)
{
    if (e.originalComponent == baseHueSlider)
    {
        if (swatchHue.getARGB() != baseHue.getARGB())
        {
            baseHue = swatchHue;
            calculateColourSeriesFromBaseHue();
            lfpDisplay->setColors();
            canvas->redraw();
        }
    }
    else
    {
        if (swatchHueB.getARGB() != baseHueB.getARGB())
        {
            baseHueB = swatchHueB;
            calculateColourSeriesFromBaseHue();
            lfpDisplay->setColors();
            canvas->redraw();
        }
    }
}

void LfpGradientColourScheme::calculateColourSeriesFromBaseHue()
{
    colourList.clear();
    
    int coloursToCalculate = numColourChannels;
    
    if (numColourChannels % 2 == 0 && (colourPattern == DOWN_UP || colourPattern == UP_DOWN))
    {
        coloursToCalculate = coloursToCalculate / 2 + 1;
    }
    
    for (int i = 0; i < coloursToCalculate; ++i)
    {
        float hue = (baseHueB.getHue() - baseHue.getHue()) * i / float(coloursToCalculate - 1);
        colourList.add(baseHue.withRotatedHue(hue));
    }
}
