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

GraphViewer::GraphViewer()
{
    JUCEApplication* app = JUCEApplication::getInstance();
    currentVersionText = "GUI version " + app->getApplicationVersion();
    
    rootNum = 0;
}


GraphViewer::~GraphViewer()
{
}


void GraphViewer::addNode (GenericEditor* editor)
{
    GraphNode* gn = new GraphNode (editor, this);
    addAndMakeVisible (gn);
    availableNodes.add (gn);
    
    gn->setBounds (20, 20, 150, 50);
    updateNodeLocations();
}


void GraphViewer::removeNode (GenericEditor* editor)
{
    availableNodes.remove (getIndexOfEditor (editor));
    
    updateNodeLocations();
}


void GraphViewer::removeAllNodes()
{
    availableNodes.clear();
    
    updateNodeLocations();
}


// void GenericEditor::updateNodeName(GenericEditor* editor)
// {
//     GraphNode* n = getNodeForEditor(editor);
//     n->repaint();
// }


void GraphViewer::updateNodeLocations()
{
    const int numAvailableNodes = availableNodes.size();
    
    // If this was made just for future purpose it would be better to execute this only in debug mode
#if JUCE_DEBUG
    // set the initial locations
    for (int i = 0; i < numAvailableNodes; ++i)
    {
    }
#endif
    
    rootNum = 0;
    
    // do initial layout
    for (int i = 0; i < numAvailableNodes; ++i)
    {
        checkLayout (availableNodes[i]);
    }
    
    // check for overlap
    for (int i = 0; i < numAvailableNodes; ++i)
    {
        for (int j = 0; j < numAvailableNodes; ++j)
        {
            if (j != i)
            {
                if (availableNodes[j]->getLevel() == availableNodes[i]->getLevel()
                    && availableNodes[j]->getHorzShift() == availableNodes[i]->getHorzShift())
                {
                    availableNodes[j]->setHorzShift (availableNodes[j]->getHorzShift() + 1);
                }
            }
        }
    }
    
    repaint();
}


void GraphViewer::checkLayout (GraphNode* gn)
{
    if (gn != nullptr)
    {
        GraphNode* sourceNode = nullptr;
        
        if (gn->isMerger())
        {
            Array<GenericEditor*> editors = gn->getConnectedEditors();
            
            int level1 = 0;
            int level2 = 0;
            
            if (editors[0] != nullptr)
            {
                level1 = getNodeForEditor (editors[0])->getLevel();
            }
            
            if (editors[1] != nullptr)
            {
                level2 = getNodeForEditor (editors[1])->getLevel();
            }
            
            // std::cout << "LEVEL1 = " << level1 << " LEVEL2 = " << level2 << std::endl;
            
            sourceNode = level1 > level2 ? getNodeForEditor (editors[0])
            : getNodeForEditor (editors[1]); // choose the higher source
        }
        else
        {
            sourceNode = getNodeForEditor (gn->getSource());
        }
        
        if (! sourceNode)
        {
            gn->setLevel (0);
            gn->setHorzShift (rootNum);
            
            ++rootNum;
        }
        else if (sourceNode->isSplitter())
        {
            Array<GenericEditor*> editors = sourceNode->getConnectedEditors();
            
            const int newHorizontalShift = gn->hasEditor (editors[1])
            ? sourceNode->getHorzShift() + 1
            : sourceNode->getHorzShift();
            
            // increase level
            gn->setLevel (sourceNode->getLevel() + 1);
            // increase horizontal shift
            gn->setHorzShift (newHorizontalShift);
        }
        else
        {
            // increase level
            gn->setLevel (sourceNode->getLevel() + 1);
            // use the same horizontal shift
            gn->setHorzShift (sourceNode->getHorzShift());
        }
        
        checkLayout (getNodeForEditor (gn->getDest()));
    }
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


int GraphViewer::nodesAtLevel (int level) const
{
    int numNodes = 0;
    
    for (auto& node : availableNodes)
    {
        // Level value is not used.
        // It seems we should check if current level of each node is equal to the passed as param level
        // TODO : research this question and either delete this comment or fix the bug
        if (node->getLevel() == numNodes)
            ++numNodes;
    }
    
    return numNodes;
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
    
    Point<float> start  = availableNodes[node1]->getCenterPoint();
    Point<float> end    = availableNodes[node2]->getCenterPoint();
    
    Path linePath;
    float x1 = start.getX();
    float y1 = start.getY();
    float x2 = end.getX();
    float y2 = end.getY();
    
    linePath.startNewSubPath (x1, y1);
    linePath.cubicTo (x1, y1 + (y2 - y1) * 0.9f,
                      x2, y1 + (y2 - y1) * 0.1f,
                      x2, y2);
    
    PathStrokeType stroke (2.0f);
    g.strokePath (linePath, stroke);
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
    // int level = -1;
    
    // GenericEditor* ed = editor;
    
    // while (ed != nullptr)
    // {
    //     level += 1;
    //     ed = ed->getSourceEditor();
    // }
    
    return vertShift;
}


void GraphNode::setLevel (int level)
{
    setBounds (getX(), 20 + level * 40, getWidth(), getHeight());
    
    vertShift = level;
}


int GraphNode::getHorzShift() const
{
    return horzShift; //gv->getHorizontalShift(this);
}


void GraphNode::setHorzShift (int shift)
{
    setBounds (20 + shift * 140, getY(), getWidth(), getHeight());
    
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
    return editor->getSourceEditor();
}


Array<GenericEditor*> GraphNode::getConnectedEditors() const
{
    return editor->getConnectedEditors();
}


const String GraphNode::getName() const
{
    return editor->getDisplayName();
}


Point<float> GraphNode::getCenterPoint() const
{
    Point<float> center = Point<float> (getX() + 10, getY() + 10);
    
    return center;
}


void GraphNode::switchIO (int path)
{
}


void GraphNode::updateBoundaries()
{
    int horzShift = gv->getHorizontalShift (this);
    
    setBounds (20 + horzShift * 140, 20 + getLevel() * 40, 150, 50);
}


void GraphNode::paint (Graphics& g)
{
    Array<bool> recordStatuses = editor->getRecordStatusArray();
    
    Path recordPath;
    
    const int numRecordStatuses = recordStatuses.size();
    for (int i = 0; i < numRecordStatuses; ++i)
    {
        float stepSize      = 2 * float_Pi / numRecordStatuses;
        float startRadians  = stepSize * i;
        float endRadians    = startRadians + stepSize;
        
        if (recordStatuses[i])
            recordPath.addPieSegment (0, 0, 20, 20, startRadians, endRadians, 0.5);
    }
    
    g.setColour (Colours::red);
    g.fillPath (recordPath);
    
    g.setColour (isMouseOver ? Colours::yellow : editor->getBackgroundColor());
    g.fillEllipse (2, 2, 16, 16);
    
    g.drawText (getName(), 25, 0, getWidth() - 25, 20, Justification::left, true);
}
