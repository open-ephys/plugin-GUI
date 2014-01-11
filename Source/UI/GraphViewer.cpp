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
    
    GraphNode* gn = new GraphNode(editor);
    addAndMakeVisible(gn);
    availableNodes.add(gn);
    
    updateNodeLocations();
    
}


void GraphViewer::removeNode(GenericEditor* editor)
{
    
    for (int i = 0; i < availableNodes.size(); i++)
    {
        if (availableNodes[i]->hasEditor(editor))
        {
            availableNodes.remove(i);
        }
    }
    
    updateNodeLocations();

}

void GraphViewer::updateNodeLocations()
{
    for (int i = 0; i < availableNodes.size(); i++)
    {
        availableNodes[i]->setBounds(25,10+40*i,200,20);
    }
}

void GraphViewer::removeAllNodes()
{
    availableNodes.clear();
    
}


void GraphViewer::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
    
    g.setFont(labelFont);
    
    g.setColour(Colours::grey);
    
    g.drawFittedText("open ephys", 40, 40, getWidth()-50, getHeight()-50, Justification::bottomRight, 100);
    
//    for (int i = 0; i < availableNodes.size(); i++)
//    {
//        g.drawText(availableNodes[i]->getName(), 20, 20*i, getWidth()-20, 40, Justification::left, true);
//    }
}

/// ------------------------------------------------------

GraphNode::GraphNode(GenericEditor* ed)
{
    editor = ed;
    mouseOver = false;
    labelFont = Font("Paragraph", 14, Font::plain);
}

GraphNode::~GraphNode()
{
    
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

const String GraphNode::getName()
{
    return editor->getName();
}

void GraphNode::paint(Graphics& g)
{
    if (mouseOver)
    {
        g.setColour(Colours::yellow);
        g.fillEllipse(0,0,getHeight(),getHeight());
    } else {
        g.setColour(Colours::lightgrey);
        g.fillEllipse(1,1,getHeight()-2,getHeight()-2);
    
    }
    
    g.drawText(getName(), getHeight()+5, 0, getWidth()-getHeight(), getHeight(), Justification::left, true);
    
}