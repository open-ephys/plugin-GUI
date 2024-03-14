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

#include "GraphViewer.h"
#include "../Processors/Splitter/Splitter.h"
#include "../Processors/Splitter/SplitterEditor.h"
#include "../Processors/Merger/Merger.h"
#include "../Utils/Utils.h"

#include "../Processors/Settings/DataStream.h"

const int NODE_WIDTH = 165;
const int NODE_HEIGHT = 50;
const int BORDER_SIZE = 20;

GraphViewport::GraphViewport(GraphViewer* gv)
{
    viewport = std::make_unique<Viewport>();
    viewport->setViewedComponent(gv, false);
    gv->setVisible(true);
    addAndMakeVisible(viewport.get());

    JUCEApplication* app = JUCEApplication::getInstance();
    currentVersionText = "GUI version " + app->getApplicationVersion();

    bw_logo = ImageCache::getFromMemory(BinaryData::bw_logo72_png, BinaryData::bw_logo72_pngSize);

}

void GraphViewport::paint(Graphics& g)
{
    g.fillAll(Colour(20, 20, 20));
    g.setOpacity(0.6f);
    g.drawImageAt(bw_logo, getWidth() - 175, getHeight() - 115);
    

    g.setOpacity(1.0f);
    g.setColour(Colours::grey);

    g.setFont(Font("Silkscreen", "Regular", 14));
    g.drawFittedText(currentVersionText, 40, 40, getWidth() - 65, getHeight() - 60, Justification::bottomRight, 100);
}

void GraphViewport::resized()
{
    viewport->setBounds(0, 0, getWidth(), getHeight());
}

GraphViewer::GraphViewer()
{
    rootNum = 0;

    graphViewport = std::make_unique<GraphViewport>(this);
}

void GraphViewer::updateBoundaries()
{
    int maxHeight = 0;
    int maxWidth = graphViewport->getWidth() - 30;

    for (auto node : availableNodes)
    {
        if (node->getBottom() > maxHeight)
            maxHeight = node->getBottom();

        if (node->getRight() > maxWidth)
            maxWidth = node->getRight();
    }

    setBounds(0, 0, maxWidth + 10, maxHeight + 10);
}


void GraphViewer::updateNodes(Array<GenericProcessor*> rootProcessors)
{

    removeAllNodes();

    //for (auto node : availableNodes)
    //    node->stillNeeded = false;
            
    Array<Splitter*> splitters;

    int rootNum = -1;

    for (auto processor : rootProcessors)
    {
        rootNum++;
        int level = -1;
        
        while ((processor != nullptr) || (splitters.size() > 0))
        {
            if (processor != nullptr)
            {
                level++;
                
                if (!nodeExists(processor))
                {
                    addNode(processor->getEditor(), level, rootNum);
                }
                //else {
                //    GraphNode* node = getNodeForEditor(processor->getEditor());
                //    node->stillNeeded = true;
                //    node->setLevel(level);
                //    node->setHorzShift(rootNum);
                //}
                
                if (processor->isSplitter())
                {
                    splitters.add((Splitter*) processor);
                    processor = splitters.getLast()->getDestNode(0); // travel down chain 0 first
                } else {
                    processor = processor->getDestNode();
                }
            }
            else {
                Splitter* splitter = splitters.getFirst();
                processor = splitter->getDestNode(1); // then come back to chain 1
                GraphNode* gn = getNodeForEditor(splitter->getEditor());
                
                level = gn->getLevel();
                
                //check if 2 splitters are connected to 1 splitter
                if(splitter->getDestNode(0) && 
                   splitter->getDestNode(0)->isSplitter() &&
                   processor &&
                   processor->isSplitter())
                {
                    rootNum = gn->getHorzShift() + 2;
                }
                else
                {
                    rootNum = gn->getHorzShift() + 1;
                }

                splitters.remove(0);
            }
        }
    }

    /*Array<GraphNode*> nodesToDelete;

    for (auto node : availableNodes)
    {
        if (!node->stillNeeded)
            nodesToDelete.add(node);
    }

    for (auto node : nodesToDelete)
    {
        availableNodes.removeObject(node, true);
    }*/

    updateBoundaries();
    
    
}

bool GraphViewer::nodeExists(GenericProcessor* p)
{

    if (getNodeForEditor(p->getEditor()) != nullptr)
        return true;
    
    return false;
}

void GraphViewer::addNode (GenericEditor* editor, int level, int offset)
{
    GraphNode* gn = new GraphNode (editor, this);
    addAndMakeVisible (gn);
    availableNodes.add (gn);
    
    int thisNodeWidth = NODE_WIDTH;

    if (gn->getName().length() > 15)
    {
        thisNodeWidth += (gn->getName().length() - 15) * 10;
    }
    
    gn->setLevel(level);
    gn->setHorzShift(offset);
    gn->setWidth(thisNodeWidth);
    gn->updateBoundaries();

}



void GraphViewer::removeAllNodes()
{
    availableNodes.clear();
    
    repaint();
}


int GraphViewer::getIndexOfEditor (GenericEditor* editor) const
{
    int index = -1;
    
    const int numAvailableNodes = availableNodes.size();
    
    for (int i = 0; i < numAvailableNodes; ++i)
    {
        if (availableNodes[i]->hasEditor (editor))
        {
            return i;
        }
    }
    
    return index;
}


GraphNode* GraphViewer::getNodeForEditor (GenericEditor* editor) const
{
    int indexOfEditor = getIndexOfEditor (editor);
    
    if (indexOfEditor > -1)
        return availableNodes[indexOfEditor];
    else
        return nullptr;
}



void GraphViewer::paint (Graphics& g)
{
    
    // Draw connections
    const int numAvailableNodes = availableNodes.size();

    for (int i = 0; i < numAvailableNodes; ++i)
    {
        if (! availableNodes[i]->isSplitter())
        {
            if (availableNodes[i]->getDest() != nullptr)
            {
                int indexOfDest = getIndexOfEditor (availableNodes[i]->getDest());
                
                if (indexOfDest > -1)
                    connectNodes (i, indexOfDest, g);
            }
        }
        else
        {
            Array<GenericEditor*> editors = availableNodes[i]->getConnectedEditors();
            
            for (int path = 0; path < 2; ++path)
            {
                int indexOfDest = getIndexOfEditor (editors[path]);
                
                if (indexOfDest > -1)
                    connectNodes (i, indexOfDest, g);
            }
        }
    }
}


void GraphViewer::connectNodes (int node1, int node2, Graphics& g)
{
    
    juce::Point<float> start  = availableNodes[node1]->getCenterPoint();
    juce::Point<float> end    = availableNodes[node2]->getCenterPoint();
    
    Path linePath;
    float x1 = start.getX();
    float y1 = start.getY();
    float x2 = end.getX();
    float y2 = end.getY();
    
    linePath.startNewSubPath (x1, y1);
    linePath.cubicTo (x1, y1 + (y2 - y1) * 0.9f,
                      x2, y1 + (y2 - y1) * 0.1f,
                      x2, y2);
    
    
    g.setColour (Colour(30,30,30));
    PathStrokeType stroke3 (3.5f);
    g.strokePath (linePath, stroke3);
    
    g.setColour (Colours::grey);
    PathStrokeType stroke2 (2.0f);
    g.strokePath (linePath, stroke2);
}

/// ------------------------------------------------------


DataStreamInfo::DataStreamInfo(const DataStream* stream_)
    :  stream(stream_)
{
}

DataStreamInfo::~DataStreamInfo()
{

}

void DataStreamInfo::paint(Graphics& g)
{
    g.setFont(Font("Fira Sans", "SemiBold", 14));
    g.setColour(Colour(30, 30, 30));
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
    g.setColour(Colours::white.withAlpha(0.3f));
    g.fillRect(1, 0, getWidth() - 2, getHeight() - 1);
    g.fillRect(1, 0, 24, getHeight() - 1);

    int numEventChannels = stream->getEventChannels().size();
    int numSpikeChannels = stream->getSpikeChannels().size();

    String ttlText = numEventChannels == 1 ? "TTL Channel" : "TTL Channels";
    String spikeText = numSpikeChannels == 1 ? "Spike Channel" : "Spike Channels";

    g.setColour(Colours::black);
    g.drawText("@ " + String(stream->getSampleRate()) + " Hz", 30, 0, getWidth() - 30, 20, Justification::left);
    g.drawText(ttlText, 30, 20, getWidth() - 30, 20, Justification::left);
    g.drawText(spikeText, 30, 40, getWidth() - 30, 20, Justification::left);

    g.drawText(String(stream->getChannelCount()), 0, 0, 25, 20, Justification::centred);
    g.drawText(String(numEventChannels), 0, 20, 25, 20, Justification::centred);
    g.drawText(String(numSpikeChannels), 0, 40, 25, 20, Justification::centred);

}


DataStreamButton::DataStreamButton(Colour colour_, const DataStream* stream_, DataStreamInfo* info_)
    : Button(stream_->getName())
    , colour(colour_.withAlpha(0.5f))
    , stream(stream_)
    , info(info_)
{
    setClickingTogglesState(true);
    setToggleState(false, dontSendNotification);

    pathOpen.addTriangle(7.0f, 5.0f, 10.5f, 12.0f, 14.0f, 5.0f);
    pathOpen.applyTransform(AffineTransform::scale(1.2f));

    pathClosed.addTriangle(8.0f, 4.0f, 8.0f, 11.0f, 15.0f, 7.5f);
    pathClosed.applyTransform(AffineTransform::scale(1.2f));
    
}


DataStreamButton::~DataStreamButton()
{

}

void DataStreamButton::paintButton(Graphics& g, bool isHighlighted, bool isDown)
{

    g.setColour(Colour(30, 30, 30));
    g.fillAll();

    g.setColour(Colours::lightgrey);
    g.fillRect(1, 0, 24, getHeight() - 1);

    g.setColour(colour);
    g.fillRect(25, 0, getWidth() - 26, getHeight() - 1);

    g.setColour(Colour(30, 30, 30));

    if (getToggleState())
        g.fillPath(pathOpen);
    else
        g.fillPath(pathClosed);

    g.setColour(Colours::white);
    g.drawText(stream->getName(), 30, 0, getWidth()-30, 20, Justification::left);

}

/// ------------------------------------------------------

GraphNode::GraphNode (GenericEditor* ed, GraphViewer* g)
: editor        (ed)
, processor     (ed->getProcessor())
, gv            (g)
, isMouseOver   (false)
, stillNeeded   (true)
{
    nodeId = processor->getNodeId();
    horzShift = 0;
    vertShift = 0;

    
    if (!processor->isMerger() && !processor->isSplitter())
    {
        for (auto stream : processor->getDataStreams())
        {

            DataStreamInfo* info = new DataStreamInfo(stream);
            dataStreamPanel.addPanel(-1, info, true);

            DataStreamButton* button = new DataStreamButton(editor->getBackgroundColor(), stream, info);
            button->addListener(this);
            dataStreamPanel.setCustomPanelHeader(info, button, true);
            dataStreamPanel.setMaximumPanelSize(info, 60);

            dataStreamButtons.add(button);
        }

        addAndMakeVisible(dataStreamPanel);
    }
   
    setBounds(BORDER_SIZE + getHorzShift() * NODE_WIDTH,
        BORDER_SIZE + getLevel() * NODE_HEIGHT,
        nodeWidth,
        40);

    previousHeight = 0;
    verticalOffset = 0;

}


GraphNode::~GraphNode()
{
}


int GraphNode::getLevel() const
{
    return vertShift;
}


void GraphNode::setLevel (int level)
{
    vertShift = level;
    
}


int GraphNode::getHorzShift() const
{
    return horzShift;
}


void GraphNode::setHorzShift (int shift)
{
    horzShift = shift;
    
}

void GraphNode::setWidth(int width)
{
    nodeWidth = width;
}

void GraphNode::mouseEnter (const MouseEvent& m)
{
    if (m.getEventRelativeTo(this).y < 20)
        isMouseOver = true;
    
    repaint();
}


void GraphNode::mouseExit (const MouseEvent& m)
{
    isMouseOver = false;
    
    repaint();
}


void GraphNode::mouseDown (const MouseEvent& m)
{
    editor->makeVisible();
}

void GraphNode::buttonClicked(Button* button)
{

    updateBoundaries();

    DataStreamButton* dsb = (DataStreamButton*)button;

    if (button->getToggleState())
        dataStreamPanel.setPanelSize(dsb->getComponent(), 60, false);
    else
        dataStreamPanel.setPanelSize(dsb->getComponent(), 0, false);

    gv->updateBoundaries();
    gv->repaint();
}


bool GraphNode::hasEditor (GenericEditor* ed) const
{
    if (ed == editor)
        return true;
    else
        return false;
}


bool GraphNode::isSplitter() const
{
    return editor->isSplitter();
}


bool GraphNode::isMerger() const
{
    return editor->isMerger();
}


GenericEditor* GraphNode::getDest() const
{
    return editor->getDestEditor();
}


GenericEditor* GraphNode::getSource() const
{
    GenericProcessor* sourceNode = editor->getProcessor()->getSourceNode();
    
    if (sourceNode != nullptr)
        return sourceNode->getEditor();
    else
        return nullptr;
}


Array<GenericEditor*> GraphNode::getConnectedEditors() const
{
    return editor->getConnectedEditors();
}


const String GraphNode::getName() const
{
    return editor->getDisplayName();
}


juce::Point<float> GraphNode::getCenterPoint() const
{
    juce::Point<float> center = juce::Point<float> (getX() + 11, getY() + 10);
    
    return center;
}


void GraphNode::updateBoundaries()
{

    int panelHeight = 0;

    for (auto dsb : dataStreamButtons)
    {
        if (dsb->getToggleState())
            panelHeight += 80;
        else
            panelHeight += 20;
    }

    dataStreamPanel.setBounds(23, 20, NODE_WIDTH - 23, panelHeight);

    int yshift = BORDER_SIZE;

    if (processor->sourceNode != nullptr && !processor->isMerger())
    {
        GraphNode* node = gv->getNodeForEditor(processor->sourceNode->getEditor());
        
        if (node != nullptr)
            yshift += node->getBottom();
        else
            std::cout << "NULLPTR" << std::endl;
    }

    if (processor->isMerger())
    {
        Merger* merger = (Merger*)processor;

        int shift1 = 0;
        int shift2 = 0;
        int shift3 = 0;

        if (merger->sourceNodeA != nullptr)
        {
            GraphNode* node = gv->getNodeForEditor(merger->sourceNodeA->getEditor());
            if (node != nullptr)
                shift1 = node->getBottom();
        }
            

        if (merger->sourceNodeB != nullptr)
        {
            GraphNode* node = gv->getNodeForEditor(merger->sourceNodeB->getEditor());
            if (node != nullptr)
                shift2 = node->getBottom();
        }

        shift3 = jmax(shift1, shift2);

        yshift += shift3;

    }

    // Adjust vertical shift if there are consecutive splitters.
    if(processor->sourceNode != nullptr && processor->sourceNode->isSplitter())
    {
        Splitter* parenSplitter = (Splitter*)processor->sourceNode;
        if(parenSplitter->sourceNode != nullptr && parenSplitter->sourceNode->isSplitter())
        {
            GraphNode* node = gv->getNodeForEditor(parenSplitter->getSourceNode()->getEditor());
            for(auto ed : node->getConnectedEditors())
            {
                if(ed != nullptr && !ed->isSplitter())
                {
                    auto nodeEditor = ed;
                    while(nodeEditor)
                    {
                        GraphNode* subNode = gv->getNodeForEditor(nodeEditor);
                        if(subNode)
                        {
                            yshift = BORDER_SIZE + subNode->getBottom();
                            nodeEditor = subNode->getDest();
                        }
                        else
                        {
                            nodeEditor = nullptr;
                        }
                    }

                    for(auto sibling : getSource()->getConnectedEditors())
                    {
                        GraphNode* siblingNode = gv->getNodeForEditor(sibling);
                        if(siblingNode && siblingNode != this)
                            gv->getNodeForEditor(sibling)->verticalShift(yshift - siblingNode->getY());
                    }
                }
            }
            
        }
    }


    if (previousHeight > 0 && previousHeight != panelHeight)
    {
        setBounds(getX(), getY(), getWidth(),
            panelHeight + 20);
    }
    else {
        setBounds(BORDER_SIZE + getHorzShift() * NODE_WIDTH,
            yshift, //BORDER_SIZE + getLevel() * NODE_HEIGHT + verticalOffset,
            nodeWidth,
            panelHeight + 20);
    }
    
    
    if (previousHeight > 0)
    {
        if (processor->destNode != nullptr)
        {
            gv->getNodeForEditor(processor->destNode->getEditor())->verticalShift(panelHeight - previousHeight);
        }
    }
    

    previousHeight = panelHeight;
}

void GraphNode::verticalShift(int pixels)
{
 
    setBounds(getX(), getY() + pixels, getWidth(), getHeight());

    if (!processor->isSplitter())
    {
        if (processor->destNode != nullptr)
        {
            GraphNode* node = gv->getNodeForEditor(processor->destNode->getEditor());
            
            if (node != nullptr)
                node->verticalShift(pixels);
        }
            
    }
    else {
        SplitterEditor* editor = (SplitterEditor*)processor->getEditor();

        for (auto ed : editor->getConnectedEditors())
        {
            GraphNode* node = gv->getNodeForEditor(ed);
            
            if (node != nullptr)
                node->verticalShift(pixels);
        }

    }

    verticalOffset = pixels;

}

String GraphNode::getInfoString()
{
    GenericProcessor* processor = (GenericProcessor*) editor->getProcessor();
    
    int ch1 = processor->getTotalContinuousChannels();
    int ch2 = processor->getTotalEventChannels();
    int ch3 = processor->getTotalSpikeChannels();
    
    String info = "Data channels: ";
    info += String(ch1);
    
    info += "\nEvent channels: ";
    info += String(ch2);
    
    info += "\nSpike channels: ";
    info += String(ch3);
    
    return info;
}


void GraphNode::paint (Graphics& g)
{
    //if (isMouseOver)
    //{
    //    g.setColour (Colours::yellow);
   // } else {
   //     
   // }

    Path linePath;
    float x1 = 8;
    float y1 = 11;
    float x2 = 40;
    float y2 = 11;

    linePath.startNewSubPath(x1, y1);
    linePath.lineTo(x2, y2);

    g.setColour(Colour(30, 30, 30));
    PathStrokeType stroke1(10.0f);
    g.strokePath(linePath, stroke1);

    g.setColour(Colour(90, 90, 90));
    PathStrokeType stroke2(7.5f);
    g.strokePath(linePath, stroke2);

    g.setColour(Colour(150, 150, 150));
    PathStrokeType stroke3(4.5f);
    g.strokePath(linePath, stroke3);
    
    g.setColour(Colour(30, 30, 30));
    g.fillRect(23, 0, getWidth() - 23, 20);

    g.setColour(Colours::lightgrey);
    g.fillRect(24, 1, 24, 18);
    g.setColour(editor->getBackgroundColor());
    g.fillRect(48, 1, getWidth() - 49, 18);
    
    if (isMouseOver)
    {
        g.setColour(Colours::yellow);
        g.drawEllipse(5,5,12,12,1.5);
        g.setGradientFill(ColourGradient(Colours::yellow,
                                    11,8,
                                    Colours::orange,
                                    20,20,
                                    true));
    } else {
        g.setColour(Colour(30,30,30));
        g.drawEllipse(5,5,12,12,1.2);
        g.setGradientFill(ColourGradient(Colours::lightgrey,
        11,8,
        Colours::grey,
        20,20,
        true));
    }
    g.fillEllipse (5.5, 5.5, 11, 11);

    g.setFont(Font("Fira Code", "SemiBold", 14));
    g.setColour (Colours::black); // : editor->getBackgroundColor());
    g.drawText (String(nodeId), 24, 0, 23, 20, Justification::centred, true);
    g.setColour(Colours::white); // : editor->getBackgroundColor());
    g.drawText(getName(), 52, 0, getWidth() - 52, 20, Justification::left, true);

}
