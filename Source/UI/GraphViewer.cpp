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
    
    labelFont = Font("Paragraph", 50, Font::plain);
    
}

GraphViewer::~GraphViewer()
{
    
}

void GraphViewer::addNode(GenericEditor* editor)
{
    
    GraphNode* gn = new GraphNode(editor, this);
    addAndMakeVisible(gn);
    availableNodes.add(gn);
    
    updateNodeLocations();
    
}


void GraphViewer::removeNode(GenericEditor* editor)
{
    
    availableNodes.remove(indexOfEditor(editor));
    
    updateNodeLocations();

}

void GraphViewer::removeAllNodes()
{
    availableNodes.clear();

    updateNodeLocations();
    
}

void GraphViewer::updateNodeLocations()
{
    for (int i = 0; i < availableNodes.size(); i++)
    {
        availableNodes[i]->updateBoundaries();
    }
    
    repaint();
}

int GraphViewer::indexOfEditor(GenericEditor* editor)
{
    int index = -1;
    
    for (int i = 0; i < availableNodes.size(); i++)
    {
        if (availableNodes[i]->hasEditor(editor))
        {
            return i;
        }
    }
    
    return index;
}

int GraphViewer::nodesAtLevel(int level)
{

    int numNodes;

    for (int i = 0; i < availableNodes.size(); i++)
    {
        if (availableNodes[i]->getLevel() == numNodes)
        {
            numNodes++;
        }
    }

    return numNodes;

}

int GraphViewer::getHorizontalShift(GraphNode* gn)
{
    int shift = 0;

    for (int i = 0; i < availableNodes.size(); i++)
    {
        if (availableNodes[i] == gn)
        {
            break;
        } else
        {
            if (availableNodes[i]->getLevel() == gn->getLevel())
            {
                shift++;
            }
        }
    }

    return shift;

}


void GraphViewer::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
    
    g.setFont(labelFont);
    
    g.setColour(Colours::grey);
    
    g.drawFittedText("open ephys", 40, 40, getWidth()-50, getHeight()-50, Justification::bottomRight, 100);
    
    // draw connections

    for (int i = 0; i < availableNodes.size(); i++)
    {

        if (!availableNodes[i]->isSplitter())
        {
            if (availableNodes[i]->getDest() != nullptr)
            {
                int indexOfDest = indexOfEditor(availableNodes[i]->getDest());
                
                if (indexOfDest > -1)
                    connectNodes(i, indexOfDest, g);
            }
        } else {

            Array<GenericEditor*> editors = availableNodes[i]->getConnectedEditors();

            for (int path = 0; path < 2; path++)
            {
                int indexOfDest = indexOfEditor(editors[path]);
                    
                if (indexOfDest > -1)    
                    connectNodes(i, indexOfDest, g);
            }
            

        }

    }
}

void GraphViewer::connectNodes(int node1, int node2, Graphics& g)
{

    Point<float> start = availableNodes[node1]->getCenterPoint();
    Point<float> end = availableNodes[node2]->getCenterPoint();

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
    g.strokePath(linePath, stroke);
}

/// ------------------------------------------------------

GraphNode::GraphNode(GenericEditor* ed, GraphViewer* g)
{
    editor = ed;
    mouseOver = false;
    labelFont = Font("Paragraph", 14, Font::plain);

    gv = g;
}

GraphNode::~GraphNode()
{
    
}

int GraphNode::getLevel()
{
    int level = -1;

    GenericEditor* ed = editor;

    while (ed != nullptr)
    {
        level += 1;
        ed = ed->getSourceEditor();
    }

    return level;
}
    
void GraphNode::mouseEnter(const MouseEvent& m)
{
    mouseOver = true;
    repaint();
}
void GraphNode::mouseExit(const MouseEvent& m)
{
    mouseOver = false;
    repaint();
}
void GraphNode::mouseDown(const MouseEvent& m)
{
    editor->makeVisible();
}
    
bool GraphNode::hasEditor(GenericEditor* ed)
{
    if (ed == editor)
        return true;
    else
        return false;
}

bool GraphNode::isSplitter()
{
    return editor->isSplitter();
}

bool GraphNode::isMerger()
{
    return editor->isMerger();
}


GenericEditor* GraphNode::getDest()
{
    return editor->getDestEditor();
}

Array<GenericEditor*> GraphNode::getConnectedEditors()
{
    return editor->getConnectedEditors();
}

const String GraphNode::getName()
{
    return editor->getName();
}

Point<float> GraphNode::getCenterPoint()
{
    Point<float> center = Point<float>(getX()+10, getY()+10);

    return center;
}

void GraphNode::switchIO(int path)
{

}

void GraphNode::updateBoundaries()
{
    // float vertShift = -1;
    // float horzShift = 0;
    
    // GenericEditor* ed = editor;
    
    // while (ed != nullptr)
    // {
    //     vertShift += 1;

    //     GenericEditor* sourceEditor = ed->getSourceEditor();
        
    //     if (sourceEditor != nullptr)
    //     {
    //         if (sourceEditor->isSplitter())
    //         {
    //             if (sourceEditor->getPathForEditor(ed) == 1)
    //                 horzShift += 1;
    //             //else
    //             //    horzShift -= 0.5;
    //         }
    //     }
    //     ed = sourceEditor;
    
    // }

    // ed = editor;

    // if (ed->isSplitter())
    // {
    //     horzShift += 0.5;
    // }
    
    // while (ed != nullptr)
    // {

    //     GenericEditor* destEditor = ed->getDestEditor();
        
    //     if (destEditor != nullptr)
    //     {
    //         if (destEditor->isSplitter())
    //         {
    //             horzShift += 0.5;
    //         } else if (destEditor->isMerger())
    //         {
    //             if (destEditor->getPathForEditor(ed) == 1)
    //                 horzShift += 1;
    //         }
    //     }
    //     ed = destEditor;
    
    // }

    int level = getLevel();

    int horzShift = gv->getHorizontalShift(this);
    
    setBounds(20+horzShift*140, 20+getLevel()*40, 150, 50);
}

void GraphNode::paint(Graphics& g)
{

    if (mouseOver)
    {
        g.setColour(Colours::yellow);
        g.fillEllipse(0,0,20,20);
    } else {
        g.setColour(Colours::lightgrey);
        g.fillEllipse(1,1,18,18);
    
    }
    
    g.drawText(getName(), 25, 0, getWidth()-25, 20, Justification::left, true);
    
}