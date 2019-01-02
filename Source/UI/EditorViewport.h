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

#ifndef __EDITORVIEWPORT_H_80260F3F__
#define __EDITORVIEWPORT_H_80260F3F__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "../Processors/Editors/GenericEditor.h"
#include "../Processors/Splitter/SplitterEditor.h"
#include "../Processors/Merger/MergerEditor.h"

#include "ControlPanel.h"
#include "UIComponent.h"
#include "DataViewport.h"

class GenericEditor;
class SignalChainTabButton;
class SignalChainManager;
class EditorScrollButton;
class SignalChainScrollButton;
class ControlPanel;
class UIComponent;

/**

  Allows the user to view and edit the signal chain.

  The EditorViewport is one of the most important classes in the GUI application.
  Dragging processors from the ProcessorList into the EditorViewport adds them to the signal chain. The
  newly added processors appear an editors in the EditorViewport. Deleting the editor from the
  EditorViewport removes its associated processor from the signal chain. Moving an editor (by dragging
  and dropping within the EditorViewport) rearranges the order of processing.

  The EditorViewport can be used to browse through multiple parallel signal chains
  (by clicking the buttons on the far left), or to navigate around branching
  signal chains.

  @see UIComponent, ProcessorGraph, SignalChainManager

*/

class EditorViewport  : public Component,
    public DragAndDropTarget,
    public Button::Listener,
    public Label::Listener
{
public:

    /** Constructor. Adds the buttons for browsing through the signal chains.*/
    EditorViewport();

    /** Destructor. */
    ~EditorViewport();

    /** Draws the background of the EditorViewport. */
    void paint(Graphics& g);

    /** Removes the processor associated with a given editor. */
    void deleteNode(GenericEditor* editor);

    /** Removes the processor associated with a given editor. */
    void selectEditor(GenericEditor* editor);

    /** Ensures that the user can see the requested editor. */
    void makeEditorVisible(GenericEditor* editor, bool highlight = true, bool updateSettings = false);

    /** Updates the boundaries and visibility of all the editors in the signal chain. */
    void refreshEditors();

    /** Removes all processors from the signal chain(s).*/
    void clearSignalChain();

    /** Used to enable and disable drag-and-drop signal chain editing. Called by the
    ProcessorGraph when data acquisition begins and ends. */
    void signalChainCanBeEdited(bool canEdit);

    /** Determines whether or not the EditorViewport should respond to
    the component that is currently being dragged. */
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails);

    /** Called when a dragged item (usually a name from the ProcessorList) enters the
       boundaries of the EditorViewport. Causes the background of the EditorViewport to change color.*/
    void itemDragEnter(const SourceDetails& dragSourceDetails);

    /** Called when a dragged item (usually a name from the ProcessorList) moves within the
       boundaries of the EditorViewport. Causes existing editors (if any) to shift their position
       to make room for the new processor that could be dropped.*/
    void itemDragMove(const SourceDetails& dragSourceDetails);

    /** Called when a dragged item (usually a name from the ProcessorList) leaves the
       boundaries of the EditorViewport. Causes the background of the EditorViewport to change color.*/
    void itemDragExit(const SourceDetails& dragSourceDetails);

    /** Called when a dragged item (usually a name from the ProcessorList) is released within the
       boundaries of the EditorViewport. Adds the dropped processor to the signal chain.*/
    void itemDropped(const SourceDetails& dragSourceDetails);

    /** Called when a mouse click begins within the EditorViewport. Usually used to select editors.*/
    void mouseDown(const MouseEvent& e);

    /** Called when a mouse drag occurs within the EditorViewport. Usually used to move editors around in the signal chain.*/
    void mouseDrag(const MouseEvent& e);

    /** Called when a mouse click ends within the EditorViewport. Usually used to indicate that a moving editor has been dropped.*/
    void mouseUp(const MouseEvent& e);

    /** Called when the mouse leaves the boundaries of the EditorViewport.*/
    void mouseExit(const MouseEvent& e);

    /** Called when a key is pressed an the EditorViewport has keyboard focus.*/
    bool keyPressed(const KeyPress& key);

    /** Changes which editor is selected, depending on the keypress (and modifier keys).*/
    void moveSelection(const KeyPress& key);

    /** Called when one of the buttons the EditorViewport listens to has been clicked.*/
    void buttonClicked(Button* button);

    /** Called when a label is changed.*/
    void labelTextChanged(Label* label);

    /** Returns an array of pointers to SignalChainTabButtons (which themselves hold pointers to the sources of each signal chain). */
    Array<SignalChainTabButton*, CriticalSection> requestSignalChain()
    {
        return signalChainArray;
    }

    /** Save the current configuration as an XML file. */
    const String saveState(File filename, String* xmlText = nullptr);

	/** Save the current configuration as an XML file. Reference wrapper*/
	const String saveState(File filename, String& xmlText);

    /** Load a saved configuration from an XML file. */
    const String loadState(File filename);

    /** Converts information about a given editor to XML. */
    XmlElement* createNodeXml(GenericProcessor*);

    /** Converts information about a splitter or merge to XML. */
    XmlElement* switchNodeXml(GenericProcessor*);

    /** Sets the parameters of a given processor via XML save files*/
    void setParametersByXML(GenericProcessor*, XmlElement*);

    /** Checks whether or not the signal chain scroll buttons need to be activated. */
    void checkScrollButtons(int topTab);

    /** Returns a boolean indicating whether or not the signal chain is empty. */
    bool isSignalChainEmpty();

    /** The index of the left-most editor (used for scrolling purposes). */
    int leftmostEditor;

    File currentFile;

private:

    String message;
    bool somethingIsBeingDraggedOver;
    bool shiftDown;

    bool canEdit;
    GenericEditor* lastEditor;
    GenericEditor* lastEditorClicked;
    GenericEditor* editorToUpdate;

    int selectionIndex;

    Array<GenericEditor*, CriticalSection> editorArray;
    Array<SignalChainTabButton*, CriticalSection> signalChainArray;

    ScopedPointer<SignalChainManager> signalChainManager;

    Font font;
    Image sourceDropImage;

    int borderSize, tabSize, tabButtonSize;

    int insertionPoint;
    bool componentWantsToMove;
    int indexOfMovingComponent;

    int currentTab;

    enum actions {ADD, MOVE, REMOVE, ACTIVATE, UPDATE};
    enum directions1 {LEFT, RIGHT};
    enum directions2 {UP, DOWN};

    EditorScrollButton* leftButton;
    EditorScrollButton* rightButton;
    SignalChainScrollButton* upButton;
    SignalChainScrollButton* downButton;

    void resized();

    int currentId;
    int maxId;

    Label editorNamingLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorViewport);

};

/**

  Allows the user to navigate between multiple parallel signal chains.

  Each SignalChainTabButton sits on the left-hand side of the EditorViewport
  and is associated with a given signal chain. Clicking the tab button makes
  the editors for its signal chain visible.

  @see EditorViewport

*/

class SignalChainTabButton : public Button
{
public:
    SignalChainTabButton();
    ~SignalChainTabButton() {}

    /** Determines the first editor in the signal chain associated with a SignalChainTabButton.*/
    void setEditor(GenericEditor* p)
    {
        firstEditor = p;
    }

    /** Sets the SignalChainManager for this SignalChainTabButton.*/
    void setManager(SignalChainManager* scm_)
    {
        scm = scm_;
    }

    /** Returns the editor associated with this SignalChainTabButton.*/
    GenericEditor* getEditor()
    {
        return firstEditor;
    }

    /** Sets the number of this SignalChainTabButton.*/
    void setNumber(int n)
    {
        num = n;
    }

    /** Returns the state of the configurationChanged variable.*/
    bool hasNewConnections()
    {
        return configurationChanged;
    }

    /** Sets the state of the configurationChanged variable.*/
    void hasNewConnections(bool t)
    {
        configurationChanged = t;
    }

    int offset;

private:

    GenericEditor* firstEditor;

    SignalChainManager* scm;

    /** Draws the SignalChainTabButton.*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    /** Called when a mouse click occurs inside a SignalChainTabButton.*/
    void clicked();

    enum actions {ADD, MOVE, REMOVE, ACTIVATE};

    int num;
    bool configurationChanged;

    Font buttonFont;

};

#endif  // __EDITORVIEWPORT_H_80260F3F__
