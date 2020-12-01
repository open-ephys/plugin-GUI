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
 Represents an individual processor/plugin in the GraphViewer.
 
 @see GraphViewer
*/

class GraphNode : public Component
{
public:
    
    /** Constructor */
    GraphNode (GenericEditor* editor, GraphViewer* g);
    
    /** Destructor */
    ~GraphNode();
    
    /** Paint component */
    void paint (Graphics& g)    override;
    
    /** Behavior on start of mouse hover */
    void mouseEnter (const MouseEvent& event) override;
    
    /** Behavior on end of mouse hover */
    void mouseExit  (const MouseEvent& event) override;
    
    /** Behavior on mouse click */
    void mouseDown  (const MouseEvent& event) override;
    
    /** Indicates whether node has an editor component */
    bool hasEditor (GenericEditor* editor) const;
    
    /** Returns location of component center point */
    juce::Point<float> getCenterPoint() const;
    
    /** Returns editor of downstream node */
    GenericEditor* getDest()    const;
    
    /** Returns editor of upstream node */
    GenericEditor* getSource()  const;
    
    /** Returns array of editors for all connected nodes (splitter and merger only) */
    Array<GenericEditor*> getConnectedEditors() const;
    
    /** Returns true if node is a splitter */
    bool isSplitter() const;
    
    /** Returns true if node is a merger */
    bool isMerger()   const;
    
    /** Returns name of the underlying processor */
    const String getName() const;
    
    /** Returns level (y-position) of node in graph display */
    int getLevel()     const;
    
    /** Returns horizontal shift (x-position of node in graph display */
    int getHorzShift() const;
    
    /** Sets the level (y-position) of node in graph display */
    void setLevel (int newLevel);
    
    /** Sets the width of node in graph display */
    void setWidth (int newWidth);
    
    /** Sets the horizontal shift (x-position of node in graph display) */
    void setHorzShift (int newHorizontalShift);
    
    /** Not currently used (consider deleting) */
    //void switchIO (int path);
    
    void updateBoundaries();
    
private:
    GenericEditor* editor;
    GraphViewer* gv;
    
    
    
    String getInfoString();
    
    bool isMouseOver;
    int horzShift;
    int vertShift;
    int nodeWidth;
    
    int nodeId;
};

/**

 Displays the full processor graph for a given session.

 Inhabits a tab in the DataViewport, and allows the user to select processor editors by clicking on their icons inside the graph.

@see UIComponent, DataViewport, ProcessorGraph, EditorViewport

*/
class GraphViewer : public Component
{
public:
    
    /** Constructor */
    GraphViewer();
    
    /** Destructor */
    ~GraphViewer();
    
    /** Draws the GraphViewer.*/
    void paint (Graphics& g)    override;
    
    /** Adds a graph node for a particular processor */
    void updateNodes    (Array<GenericProcessor*> rootProcessors);
    
    /** Adds a graph node for a particular processor */
    void addNode    (GenericEditor* editor, int level, int offset);
    
    /** Clears the graph */
    void removeAllNodes();
    
    /** Returns the graph node for a particular processor editor */
    GraphNode* getNodeForEditor (GenericEditor* editor) const;
    
    int getIndexOfEditor(GenericEditor* editor) const;
    
    /** Checks if a node exists for a given processor*/
    bool nodeExists(GenericProcessor* processor);
    
private:
    void connectNodes (int, int, Graphics&);

    int rootNum;

    String currentVersionText;
    
    OwnedArray<GraphNode> availableNodes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphViewer);
};


#endif  // __GRAPHVIEWER_H_4E971BF9__
