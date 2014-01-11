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

#ifndef __GRAPHVIEWER_H_4E971BF9__
#define __GRAPHVIEWER_H_4E971BF9__

#include "../AccessClass.h"
#include "../Processors/Editors/GenericEditor.h"

#include "../../JuceLibraryCode/JuceHeader.h"

/**
 
 Displays the full processor graph for a given session.
 
 Inhabits a tab in the DataViewport.
 
 @see UIComponent, DataViewport, ProcessorGraph, EditorViewport
 
 */


class GraphNode : public Component
{
public:
    GraphNode(GenericEditor* editor);
    ~GraphNode();
    
    void mouseEnter(const MouseEvent& m);
    void mouseExit(const MouseEvent& m);
    void mouseDown(const MouseEvent& m);
    
    bool hasEditor(GenericEditor* editor);
    
    void paint(Graphics& g);
    
    const String getName();
    
private:
    GenericEditor* editor;
    
    Font labelFont;
    
    bool mouseOver;
};


class GraphViewer : public Component, public AccessClass

{
public:
    GraphViewer();
    ~GraphViewer();
    
    /** Draws the GraphViewer.*/
    void paint(Graphics& g);
    
    void addNode(GenericEditor* editor);
    void removeNode(GenericEditor* editor);
    void removeAllNodes();

    
private:
    
    void updateNodeLocations();
    
    Font labelFont;
    
    OwnedArray<GraphNode> availableNodes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphViewer);
    
};


#endif  // __GRAPHVIEWER_H_4E971BF9__
