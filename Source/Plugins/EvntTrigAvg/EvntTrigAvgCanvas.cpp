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

/*
 */
 #include "EvntTrigAvgCanvas.h"


EvntTrigAvgCanvas::EvntTrigAvgCanvas(EvntTrigAvg* n) :
    processor(n)
{
    
    clearHisto = new UtilityButton("CLEAR", Font("Default", 12, Font::plain));
    clearHisto->addListener(this);
    clearHisto->setRadius(3.0f);
    clearHisto->setBounds(80,5,65,15);
    clearHisto->setClickingTogglesState(false);
    addAndMakeVisible(clearHisto);
    setWantsKeyboardFocus(true);
    
    viewport = new Viewport();
    viewport->setScrollBarsShown(true,true);
    scrollBarThickness = viewport->getScrollBarThickness();
    
    int yOffset = 50;
    processor = n;
    display = new EvntTrigAvgDisplay(this, viewport, n);
    display->setBounds(0,100,getWidth()-scrollBarThickness, getHeight()-yOffset-40);
    //removeChildComponent(scale);
    scale = new Timescale(processor->getWindowSize(),processor->getSampleRate(),data,bin,binSize);
    //holder = new EvntTrigAvgCanvasHolder(processor,processor->getWindowSize(),processor->getSampleRate(),data,bin,binSize);
    viewport->setViewedComponent(display,false);
    addAndMakeVisible(viewport);
    viewport->setBounds(0,100,getWidth(), getHeight()-yOffset-40);
    //holder->setBounds(0, getHeight(), getWidth(), 40);
    scale->setBounds(0, getHeight()-40, getWidth()-scrollBarThickness, 40);
    //addAndMakeVisible(scale,false);
    addAndMakeVisible(scale,true);
    
    update();
}

EvntTrigAvgCanvas::~EvntTrigAvgCanvas()
{
    //delete scale;
    //deleteAllChildren();
}

void EvntTrigAvgCanvas::beginAnimation()
{
    std::cout << "EvntTrigAvgCanvas beginning animation." << std::endl;

    startCallbacks();
}

void EvntTrigAvgCanvas::endAnimation()
{
    std::cout << "EvntTrigAvgCanvas ending animation." << std::endl;

    stopCallbacks();
}

void EvntTrigAvgCanvas::update()
{
    repaint();
}


void EvntTrigAvgCanvas::refreshState()
{
    resized();
}

void EvntTrigAvgCanvas::resized()
{

    int yOffset = 50;
    viewport->setBounds(0,yOffset,getWidth(),getHeight()-yOffset-40);
    if (display->getNumGraphs()>0)
        display->setBounds(0,yOffset,getWidth()-scrollBarThickness,display->getNumGraphs()*40);
    else
        display->setBounds(0,100,getWidth()-scrollBarThickness, getHeight()-yOffset-40);
    scale->setBounds(0, getHeight()-40, getWidth()-scrollBarThickness, 40);
    repaint();
}

void EvntTrigAvgCanvas::paint(Graphics& g)
{
    g.fillAll(Colour(0,18,43));
    
    int width=getWidth();
    int height=getHeight();
    int drawWidth = width-20-width/4;
    int xOffset= 20;
    int yOffset = 50;
    g.setColour(Colours::lightgrey);
    g.fillRoundedRectangle(getWidth()-scrollBarThickness, yOffset, scrollBarThickness, height-2*yOffset, 4.0);

    g.setColour(Colours::snow);
    
    g.drawText("Electrode",5, 5, width/8, 20, juce::Justification::left);
    g.drawText("Trials: " + String(processor->getLastTTLCalculated()),(xOffset+drawWidth)/2-50,5,100,20,Justification::centred);
    g.drawText("Min.", width-180-scrollBarThickness, 5, 60, 20, Justification::right);
    g.drawText("Max", width-120-scrollBarThickness, 5, 60, 20, Justification::right);
    g.drawText("Mean", width-60-scrollBarThickness, 5, 60, 20, Justification::right);
    //delete scale;
    //scale = new Timescale(processor->getWindowSize(),processor->getSampleRate(),data,bin,processor->getBinSize());
    //removeChildComponent(scale);
    scale = new Timescale(processor->getWindowSize(),processor->getSampleRate(),data,bin,processor->getBinSize());
    scale->setBounds(0, getHeight()-40, width-scrollBarThickness, 40);
    addAndMakeVisible(scale,true);
    repaint();
}

void EvntTrigAvgCanvas::repaintDisplay(){
    display->repaint();
}

void EvntTrigAvgCanvas::refresh()
{
    // called every 10 Hz
    display->refresh(); // dont know if this ever gets called
    repaint();
}

bool EvntTrigAvgCanvas::keyPressed(const KeyPress& key)
{
    return false;
}

void EvntTrigAvgCanvas::buttonClicked(Button* button)
{
    if (button == clearHisto){
        histoData.clear();
        minMaxMean.clear();
        processor->setParameter(4,0);
    }
     repaint();
}

void EvntTrigAvgCanvas::setBin(int bin_){
    bin = bin_;
}

void EvntTrigAvgCanvas::setBinSize(int binSize_){
    binSize = binSize_;
}

void EvntTrigAvgCanvas::setData(int data_){
    data=data_;
}

//--------------------------------------------------------------------


EvntTrigAvgDisplay::EvntTrigAvgDisplay(EvntTrigAvgCanvas* c, Viewport* v, EvntTrigAvg* p){
    processor=p;
    canvas=c;
    viewport=v;
    addMouseListener(this, true);
    channelColours[0]=Colour(224,185,36);
    channelColours[1]=Colour(214,210,182);
    channelColours[2]=Colour(243,119,33);
    channelColours[3]=Colour(186,157,168);
    channelColours[4]=Colour(237,37,36);
    channelColours[5]=Colour(179,122,79);
    channelColours[6]=Colour(217,46,171);
    channelColours[7]=Colour(217, 139,196);
    channelColours[8]=Colour(101,31,255);
    channelColours[9]=Colour(141,111,181);
    channelColours[10]=Colour(48,117,255);
    channelColours[11]=Colour(184,198,224);
    channelColours[12]=Colour(116,227,156);
    channelColours[13]=Colour(150,158,155);
    channelColours[14]=Colour(82,173,0);
    channelColours[15]=Colour(125,99,32);
}

EvntTrigAvgDisplay::~EvntTrigAvgDisplay(){
    deleteAllChildren();
}

void EvntTrigAvgDisplay::visibleAreaChanged(const Rectangle<int>& newVisibleArea){
    
}

void EvntTrigAvgDisplay::viewedComponentChanged(Component* newComponent){
    
}

void EvntTrigAvgDisplay::resized()
{
    int width = getWidth();
    for(int i = 0 ; i < graphs.size() ; i++){
        graphs[i]->setBounds(20, 40*(i+1), width-20-width/4, 40);
        graphs[i]->resized();
    }
}

void EvntTrigAvgDisplay::paint(Graphics &g)
{

    histoData.clear();
    histoData = processor->getHistoData();
    minMaxMean.clear();
    minMaxMean = processor->getMinMaxMean();
    int width=getWidth();
    g.setColour(Colours::snow);
    std::vector<String> labels = processor->getElectrodeLabels();
    deleteAllChildren();
    graphs.clear();
    int graphCount = 0;
    
    for (int i = 0 ; i < histoData.size() ; i++){
        GraphUnit* graph;
        ScopedLock myScopedLock(*processor->getMutex());
        if(histoData[i][1]==0){ // if sortedId == 0
                graph = new GraphUnit(processor,canvas,channelColours[(histoData[i][0])%16],labels[histoData[i][0]],&minMaxMean[i][2],&histoData[i][2]); // pass &histoData[i][2] instead of 3 to pass on how many bins are used
        }
            else{
                graph = new GraphUnit(processor,canvas,channelColours[(histoData[i][0])%16],"ID "+String(histoData[i][1]),&minMaxMean[i][2],&histoData[i][2]);
            }
            graphs.push_back(graph);
            graph->setBounds(0, 40*(graphCount), width-20, 40);
            addAndMakeVisible(graph,true);
            graphCount += 1;
    }
    repaint(); // ideally find better method than this
}

void EvntTrigAvgDisplay::refresh()
{
    for (int i = 0 ; i < graphs.size() ; i++){
        graphs[i]->repaint();
    }
}

int EvntTrigAvgDisplay::getNumGraphs()
{
    return graphs.size();
}

//--------------------------------------------------------------------

Timescale::Timescale(int windowSize_, uint64 sampleRate_, int data_, int bin_,int binSize_)
{
    windowSize = windowSize_;
    sampleRate = sampleRate_;
    data = data_;
    bin = bin_;
    binSize = binSize_;
}
Timescale::~Timescale()
{
    deleteAllChildren();
}

void Timescale::paint(Graphics& g)
{
    g.setColour(Colours::snow);
    int histogramLen = getWidth()-230;
    int vertLineLen = 20;
    int textStart = vertLineLen+5;
    g.drawHorizontalLine(0, 30, histogramLen+30);

    g.drawVerticalLine(30, 0, vertLineLen);
    g.drawText(String(-1000.0*float(windowSize/2)/float(sampleRate)) + " ms", 0, textStart, 60, 10, Justification::centred);
    
    g.drawVerticalLine(histogramLen/4+30, 0, vertLineLen);
    g.drawText(String(-1000.0*float(windowSize/2)/2.0/float(sampleRate)) + " ms", histogramLen/4, textStart, 60, 10, Justification::centred);
    
    g.drawVerticalLine(histogramLen/2+30, 0, vertLineLen);
    g.drawText(" 0 ms",histogramLen/2, textStart, 60, 10, Justification::centred);
    
    g.drawVerticalLine(3*histogramLen/4+30, 0, vertLineLen);
    g.drawText(String(1000.0*float(windowSize/2)/2.0/float(sampleRate)) + " ms", 3*histogramLen/4, textStart, 60, 10, Justification::centred);
    
    g.drawVerticalLine(histogramLen+30, 0, vertLineLen);
    g.drawText(String(1000.0*float(windowSize/2)/float(sampleRate)) + " ms", histogramLen, textStart, 60, 10, Justification::centred);
    
    g.drawText(String(1000*float(bin*binSize)/float(sampleRate)) + "-" + String(1000*(float(bin+1)*binSize)/float(sampleRate)) + "ms, Spikes: " + String(data),histogramLen+30, 5, getWidth()-(histogramLen+30), getHeight(), Justification::right);
}

void Timescale::resized()
{
    
}

void Timescale::update(int windowSize_, uint64 sampleRate_)
{
    windowSize=windowSize_;
    sampleRate=sampleRate_;
}

void inline Timescale::setBin(int bin_)
{
    bin = bin_;
}
void inline Timescale::setData(int data_)
{
    data = data_;
}
void inline Timescale::setBinSize(int binSize_)
{
    binSize = binSize_;
}

//--------------------------------------------------------------------


GraphUnit::GraphUnit(EvntTrigAvg* processor_, EvntTrigAvgCanvas* canvas_,juce::Colour color_, String name_, float  * stats_,uint64 * data_){
    ScopedLock myScopedLock(*processor_->getMutex());
    color = color_;
    LD = new LabelDisplay(color_,name_);
    LD->setBounds(0,0,30,40);
    addAndMakeVisible(LD,false);
    
    HG = new HistoGraph(processor_,canvas_,color_,data_[0], stats_[1], &data_[1]);
    HG->setBounds(30,0,getWidth()-210,40);
    addAndMakeVisible(HG,false);
    SD = new StatDisplay(processor_,color_,stats_);
    SD->setBounds(getWidth()-180,0,180,40);
    addAndMakeVisible(SD,false);
}
GraphUnit::~GraphUnit()
{
    deleteAllChildren();
}
void GraphUnit::paint(Graphics& g)
{
        //g.setOpacity(1);
}
void GraphUnit::resized()
{
    LD->setBounds(0,0,30,40);
    SD->setBounds(getWidth()-180,0,180,40);
    HG->setBounds(30,0,getWidth()-210,40);
}

//----------------

LabelDisplay::LabelDisplay(juce::Colour color_, String name_)
{
    color = color_;
    name = name_;
}
LabelDisplay::~LabelDisplay()
{
    deleteAllChildren();
}
void LabelDisplay::paint(Graphics& g)
{
    g.setColour(color);
    g.drawText(name,0, 0, 30, 40, juce::Justification::left);
}
void LabelDisplay::resized()
{
    
}

//----------------

HistoGraph::HistoGraph(EvntTrigAvg* processor_,EvntTrigAvgCanvas* canvas_, juce::Colour color_, uint64 bins_, float max_, uint64 * histoData_)
{
    color = color_;
    histoData = histoData_;
    bins = bins_;
    max = uint64(max_);
    processor=processor_;
    canvas = canvas_;
}

HistoGraph::~HistoGraph()
{
    deleteAllChildren();
}

void HistoGraph::paint(Graphics& g)
{
    
    g.setColour(Colours::snow);
    g.setOpacity(0.5);
    g.drawVerticalLine(getWidth()/2,5, getHeight());
    g.setColour(color);
    for (int i = 1 ; i < bins ; i++){
        ScopedLock myScopedLock(*processor->getMutex());
        if(max!=0){
            g.drawLine(float(i-1)*float(getWidth())/float(bins),getHeight()-(histoData[i-1]*getHeight()/max),float(i)*float(getWidth())/float(bins),getHeight()-(histoData[i]*getHeight()/max));
        }
        else
            g.drawLine(float(i-1)*float(getWidth())/float(bins),getHeight()-(histoData[i-1]*getHeight()),float(i)*float(getWidth())/float(bins),getHeight()-(histoData[i]*getHeight()));
    }
}

void HistoGraph::resized()
{
    repaint();
}

void HistoGraph::select()
{
    
}

void HistoGraph::deselect()
{
    
}

void HistoGraph::clear()
{
    
}

void HistoGraph::mouseMove(const MouseEvent &event)
{
    if(bins>0){
        int posX = event.x;
        ScopedLock myScopedLock(*processor->getMutex());
        int valueY = histoData[int(float(posX)/float(getWidth())*float(bins))];
        canvas->setData(valueY);
        canvas->setBin(int(float(posX)/float(getWidth())*float(bins))-(bins/2));
        canvas->repaint();
    }
}

//----------------

StatDisplay::StatDisplay(EvntTrigAvg* processor_, juce::Colour c, float * s)
{
    processor=processor_;
    color = c;
    stats = s;
}

StatDisplay::~StatDisplay()
{
    deleteAllChildren();
}

void StatDisplay::paint(Graphics& g)
{
    ScopedLock myScopedLock(*processor->getMutex());
    g.setColour(color);
    g.drawText(String(stats[0]),0, 0, 60, 40, juce::Justification::right);
    g.drawText(String(stats[1]),60, 0, 60, 40, juce::Justification::right);
    g.drawText(String(stats[2]),120, 0, 60, 40, juce::Justification::right);
    }

void StatDisplay::resized()
{
    
}




