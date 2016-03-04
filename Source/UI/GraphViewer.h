/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2016 Open Ephys
 
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
    GraphNode (GenericEditor* editor, GraphViewer* g);
    ~GraphNode();
    
    void paint (Graphics& g)    override;
    
    void mouseEnter (const MouseEvent& event) override;
    void mouseExit  (const MouseEvent& event) override;
    void mouseDown  (const MouseEvent& event) override;
    
    bool hasEditor (GenericEditor* editor) const;
    
    Point<float> getCenterPoint() const;
    GenericEditor* getDest()    const;
    GenericEditor* getSource()  const;
    Array<GenericEditor*> getConnectedEditors() const;
    
    bool isSplitter() const;
    bool isMerger()   const;
    
    const String getName() const;
    
    int getLevel()     const;
    int getHorzShift() const;
    
    void setLevel (int newLevel);
    void setHorzShift (int newHorizontalShift);
    
    void updateBoundaries();
    void switchIO (int path);
    
    int horzShift;
    int vertShift;
    
private:
    GenericEditor* editor;
    GraphViewer* gv;
    
    bool isMouseOver;
};


class GraphViewer : public Component
{
public:
    GraphViewer();
    ~GraphViewer();
    
    /** Draws the GraphViewer.*/
    void paint (Graphics& g)    override;
    
    void addNode    (GenericEditor* editor);
    void removeNode (GenericEditor* editor);
    void removeAllNodes();
    void updateNodeLocations();
    
    int nodesAtLevel (int lvl) const;
    int getHorizontalShift (GraphNode*) const;
    GraphNode* getNodeForEditor (GenericEditor* editor) const;
    
    
private:
    void connectNodes (int, int, Graphics&);
    void checkLayout (GraphNode*);
    
    int getIndexOfEditor (GenericEditor* editor) const;
    
    int rootNum;
    
    String currentVersionText;
    
    OwnedArray<GraphNode> availableNodes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphViewer);
};


#endif  // __GRAPHVIEWER_H_4E971BF9__
