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
#include "../Utils/Utils.h"

const int NODE_WIDTH = 150;
const int NODE_HEIGHT = 100;
const int BORDER_SIZE = 20;


GraphViewer::GraphViewer()
{
    JUCEApplication* app = JUCEApplication::getInstance();
    currentVersionText = "GUI version " + app->getApplicationVersion();
    
    rootNum = 0;
}


GraphViewer::~GraphViewer()
{
}

void GraphViewer::updateNodes(Array<GenericProcessor*> rootProcessors)
{
    removeAllNodes(); // clear the current nodes
            
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
                rootNum = gn->getHorzShift() + 1;
                splitters.remove(0);
            }
        }
    }
    
    //updateNodeLocations();
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
    g.fillAll (Colours::darkgrey);
    
    g.setFont (Font("Paragraph",  50, Font::plain));
    
    g.setColour (Colours::grey);
    
    g.drawFittedText ("open ephys", 40, 40, getWidth()-50, getHeight()-60, Justification::bottomRight, 100);
    
    g.setFont (Font("Small Text", 14, Font::plain));
    g.drawFittedText (currentVersionText, 40, 40, getWidth()-50, getHeight()-45, Justification::bottomRight, 100);
    
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

GraphNode::GraphNode (GenericEditor* ed, GraphViewer* g)
: editor        (ed)
, gv            (g)
, isMouseOver   (false)
{
    nodeId = ed->getProcessor()->getNodeId();
    horzShift = 0;
    vertShift = 0;
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

    setBounds (BORDER_SIZE + getHorzShift() * NODE_WIDTH,
               BORDER_SIZE + getLevel() * NODE_HEIGHT,
               nodeWidth,
               NODE_HEIGHT);
}

String GraphNode::getInfoString()
{
    GenericProcessor* processor = (GenericProcessor*) editor->getProcessor();
    
    int ch1 = processor->getTotalDataChannels();
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
    if (isMouseOver)
    {
        g.setColour (Colours::yellow);
        g.fillRoundedRectangle (0, 0, getWidth()-23, NODE_HEIGHT-23, 4);
    } else {
        g.setColour (Colour(30,30,30));
        g.fillRoundedRectangle (0, 0, getWidth()-23, NODE_HEIGHT-23, 4);
    }
    
    g.setColour(editor->getBackgroundColor());
    g.fillRoundedRectangle    (1, 1, getWidth()-25, NODE_HEIGHT-25, 3);
    
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
    
    g.setColour (Colours::white); // : editor->getBackgroundColor());
    g.drawText (String(nodeId) + " " + getName(), 23, 1, getWidth() - 25, 20, Justification::left, true);
    
    g.setColour (Colours::black); // : editor->getBackgroundColor());
    g.drawFittedText (getInfoString(), 10, 25, getWidth() - 5, 70, Justification::left, true);
}
