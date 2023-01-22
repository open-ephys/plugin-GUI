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
#include "../Processors/Visualization/Visualizer.h"

#include "../../JuceLibraryCode/JuceHeader.h"

class DataStream;
class GraphViewer;


/**
 Represents a DataStream handled by a given processor.

 @see GraphViewer, GraphNode
*/

class DataStreamInfo : public Component
{
public:

    /** Constructor */
    DataStreamInfo(const DataStream* stream);

    /** Destructor */
    ~DataStreamInfo();

    /** Paint component */
    void paint(Graphics& g);

private:

    const DataStream* stream;

};

/**
 Represents a DataStream handled by a given processor.

 @see GraphViewer, GraphNode
*/

class DataStreamButton : public Button
{
public:

    /** Constructor */
    DataStreamButton(Colour colour, const DataStream* stream, DataStreamInfo* info);

    /** Destructor */
    ~DataStreamButton();

    /** Paint component */
    void paintButton(Graphics& g, bool isHighlighted, bool isDown);

    Component* getComponent() const { return (Component*)info; }

private:

    const DataStream* stream;
    DataStreamInfo* info;
    Path pathOpen;
    Path pathClosed;
    Colour colour;

};

/**
 Represents an individual processor/plugin in the GraphViewer.
 
 @see GraphViewer
*/

class GraphNode : public Component,
    public Button::Listener
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

    /** To respond to clicks in the DataStreamPanel*/
    void buttonClicked(Button* button);
    
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
    void verticalShift(int pixels);
    
    /** Adjusts the boundaries of this node, based on its inputs and outputs*/
    void updateBoundaries();

    /** True if processor still exists */
    bool stillNeeded;
    
private:
    GenericEditor* editor;
    GenericProcessor* processor;
    GraphViewer* gv;
    

    String getInfoString();

    ConcertinaPanel dataStreamPanel;
    Array<DataStreamButton*> dataStreamButtons;
    
    bool isMouseOver;
    int horzShift;
    int vertShift;
    int nodeWidth;
    
    int nodeId;

    int previousHeight;

    int verticalOffset;
};


/**
    Allows the GraphViewer to be scrolled

 */
class GraphViewport : public Visualizer
{
public:
    /** Constructor */
    GraphViewport(GraphViewer* gv);

    /** Destructor */
    ~GraphViewport() { }

    /** Draws the Open Ephys Logo*/
    void paint(Graphics& g) override;

    /** Visualizer virtual functions */
    void refresh() { }
    void update() { }
    void refreshState() { }

    /** Sets viewport bounds*/
    void resized() override;

    /** Scroll area*/
    std::unique_ptr<Viewport> viewport;

    /** Holds the Open Ephys application version*/
    String currentVersionText;

    /** Logo to display*/
    Image bw_logo;
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
    ~GraphViewer() { }
    
    /** Draws the GraphViewer.*/
    void paint (Graphics& g)    override;

    /** Resizes the component, based on the bottom-most node*/
    void updateBoundaries();
    
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

    /** Returns a pointer to the top-level component */
    GraphViewport* getGraphViewport() { return graphViewport.get(); }
    
private:
    void connectNodes (int, int, Graphics&);

    int rootNum;

    OwnedArray<GraphNode> availableNodes;

    std::unique_ptr<GraphViewport> graphViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphViewer);
};





#endif  // __GRAPHVIEWER_H_4E971BF9__
