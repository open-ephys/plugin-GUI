// LfpChannelDisplay.cpp

#include "LfpChannelDisplay.h"
#include "LfpDisplay.h"
#include "LfpDisplayOptions.h"

using namespace LfpDisplayNodeBeta;
    
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
