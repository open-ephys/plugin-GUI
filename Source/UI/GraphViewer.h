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
    GraphNode(GenericEditor* editor, GraphViewer* g);
    ~GraphNode();

    void mouseEnter(const MouseEvent& m);
    void mouseExit(const MouseEvent& m);
    void mouseDown(const MouseEvent& m);

    bool hasEditor(GenericEditor* editor);

    void paint(Graphics& g);

    void updateBoundaries();

    Point<float> getCenterPoint();
    GenericEditor* getDest();
    GenericEditor* getSource();
    Array<GenericEditor*> getConnectedEditors();
    void switchIO(int path);

    bool isSplitter();
    bool isMerger();

    const String getName();

    int getLevel();
    void setLevel(int);
    int getHorzShift();
    void setHorzShift(int);

    int horzShift;
    int vertShift;

private:

    GenericEditor* editor;

    Font labelFont;

    bool mouseOver;

    GraphViewer* gv;
};


class GraphViewer : public Component
{
public:
    GraphViewer();
    ~GraphViewer();

    /** Draws the GraphViewer.*/
    void paint(Graphics& g);

    void addNode(GenericEditor* editor);
    void removeNode(GenericEditor* editor);
    void removeAllNodes();
    void updateNodeLocations();

    int nodesAtLevel(int lvl);
    int getHorizontalShift(GraphNode*);
    GraphNode* getNodeForEditor(GenericEditor* editor);

private:

    void connectNodes(int, int, Graphics&);
    void checkLayout(GraphNode*);

    int indexOfEditor(GenericEditor* editor);

    Font labelFont;

    int rootNum;

    OwnedArray<GraphNode> availableNodes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphViewer);

};


#endif  // __GRAPHVIEWER_H_4E971BF9__
