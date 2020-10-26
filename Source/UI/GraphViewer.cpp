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
    removeAllNodes();
            
    Array<Splitter*> splitters;

    for (auto processor : rootProcessors)
    {
        while ((processor != nullptr) || (splitters.size() > 0))
        {
            if (processor != nullptr)
            {
                if (!nodeExists(processor))
                {
                    addNode(processor->getEditor());
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
                splitters.remove(0);
            }
        }
    }
    
    updateNodeLocations();
}

bool GraphViewer::nodeExists(GenericProcessor* p)
{

    if (getNodeForEditor(p->getEditor()) != nullptr)
        return true;
    
    return false;
}

void GraphViewer::addNode (GenericEditor* editor)
{
    GraphNode* gn = new GraphNode (editor, this);
    addAndMakeVisible (gn);
    availableNodes.add (gn);

    int thisNodeWidth = NODE_WIDTH;

    if (gn->getName().length() > 15)
    {
        thisNodeWidth += (gn->getName().length() - 15) * 10;
    }
    
    gn->setBounds (BORDER_SIZE, BORDER_SIZE, thisNodeWidth, NODE_HEIGHT);

}

void GraphViewer::removeAllNodes()
{
    availableNodes.clear();
    
    updateNodeLocations();
}

int GraphViewer::getLevel(GraphNode* node) {
    
    int level = 0;
    
    while (node->getSource() != nullptr)
    {
        node = getNodeForEditor(node->getSource());
        level += 1;
    }
    
    return level;
}

void GraphViewer::adjustBranchLayout(GraphNode* rootNode, int startLevel)
{
    if (rootNode->getLevel() != -1 && rootNode->isSplitter())
    {
        return; // we've already adjusted this splitter node
    }
    
    int level = startLevel + 1;
    
    if (rootNode->isMerger())
    {
        Array<GenericEditor*> upstreamEditors = rootNode->getConnectedEditors();
        
        int level1 = level;
        int level2 = level;
        
        if (upstreamEditors[0] != nullptr)
        {
            GraphNode* node = getNodeForEditor(upstreamEditors[0]);
            if (node != nullptr)
                level1 = node->getLevel() + 1;
            std::cout << "Merger input 1 at " << level1 << std::endl;
        }
        
        if (upstreamEditors[1] != nullptr)
        {
            GraphNode* node = getNodeForEditor(upstreamEditors[1]);
            if (node != nullptr)
                level1 = node->getLevel() + 1;
            std::cout << "Merger input 2 at " << level1 << std::endl;
        }
        
        level = (level1 > level2) ? level1 : level2;
    }
    
    rootNode->setLevel(level);
    rootNode->setHorzShift(rootNum);
    
    if (rootNode->getDest() != nullptr || rootNode->isSplitter())
    {
        if (!rootNode->isSplitter())
        {
            adjustBranchLayout(getNodeForEditor(rootNode->getDest()), level);
            
        } else {
            
          Array<GenericEditor*> downstreamEditors = rootNode->getConnectedEditors();
            
            if (downstreamEditors[0] != nullptr && downstreamEditors[1] != nullptr)
            {
                adjustBranchLayout(getNodeForEditor(downstreamEditors[0]), level);
                rootNum += 1;
                adjustBranchLayout(getNodeForEditor(downstreamEditors[1]), level);
            } else {
                if (downstreamEditors[0] != nullptr)
                {
                    adjustBranchLayout(getNodeForEditor(downstreamEditors[0]), level);
                } else if (downstreamEditors[1] != nullptr ){
                    adjustBranchLayout(getNodeForEditor(downstreamEditors[1]), level);
                }
            }
        }
    }
}

void GraphViewer::updateNodeLocations()
{
    const int numAvailableNodes = availableNodes.size();
    
    rootNum = 0;
    
    // reset the positions
    for (auto& node : availableNodes)
    {
        node->setLevel(-1);
    }
    
    Array<Splitter*> splitters;
    
    // update the location of each branch
    for (auto& node : availableNodes)
    {
        if (node->getSource() == nullptr)
        {
            //if (node->isMerger())
           //{
           //     rootNum -= 1;
           // }
            
            adjustBranchLayout(node, -1);
            rootNum += 1;
            
        }
    }
    
    // remove extra horizontal shift
    //for (auto& node : availableNodes)
    //{
    //    if (node->getHorzShift() > 0)
    //    {
    //        for (int i = 0; i < 3; i++)
     //       {
     //           if (isEmptySpace(node->getLevel(), node->getHorzShift()-1))
     //           {
     //               node->setHorzShift(node->getHorzShift()-1);
     //           }
     //       }
     //   }
    //}
    
    repaint();
}

bool GraphViewer::isEmptySpace(int level, int horzShift)
{
    if (horzShift < 0)
        return false;
    
    for (auto& node : availableNodes)
    {
        if (node->getHorzShift() == horzShift && node->getLevel() == level)
        {
            return false;
        }
    }
    
    return true;
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


int GraphViewer::getHorizontalShift (GraphNode* gn) const
{
    int shift = 0;
    
    for (auto& node : availableNodes)
    {
        if (node == gn)
            break;
        else
            if (node->getLevel() == gn->getLevel())
                ++shift;
    }
    
    return shift;
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
    setBounds (getX(), BORDER_SIZE + level * NODE_HEIGHT, getWidth(), getHeight());
    
    vertShift = level;
}


int GraphNode::getHorzShift() const
{
    return horzShift;
}


void GraphNode::setHorzShift (int shift)
{
    setBounds (BORDER_SIZE + shift * NODE_WIDTH, getY(), getWidth(), getHeight());
    
    horzShift = shift;
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
    int horzShift = gv->getHorizontalShift (this);
    
    setBounds (BORDER_SIZE + horzShift * NODE_WIDTH,
               BORDER_SIZE + getLevel() * NODE_HEIGHT,
               NODE_WIDTH,
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
    g.drawText (getName(), 23, 1, getWidth() - 25, 20, Justification::left, true);
    
    g.setColour (Colours::black); // : editor->getBackgroundColor());
    g.drawFittedText (getInfoString(), 10, 25, getWidth() - 5, 70, Justification::left, true);
}
