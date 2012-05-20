/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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
#include "../Processors/ProcessorGraph.h"
#include "../Processors/Editors/GenericEditor.h"
#include "../Processors/Editors/SplitterEditor.h"
#include "../Processors/Editors/MergerEditor.h"

#include "../AccessClass.h"
#include "DataViewport.h"

/**
  
  Allows the user to view and edit the signal chain.

  The EditorViewport is one of the most important classes in the GUI application.

  @see UIComponent, ProcessorGraph

*/

class GenericEditor;
class SignalChainTabButton;
class SignalChainManager;
class EditorScrollButton;
class SignalChainScrollButton;

class EditorViewport  : public Component,
                        public DragAndDropTarget,
                        public AccessClass,
                        public Button::Listener

{
public:

    EditorViewport();//ProcessorGraph* pgraph, DataViewport* tabComp);
    ~EditorViewport();

    void paint (Graphics& g);

    // Creating and deleting editors:
    void deleteNode(GenericEditor* editor);
    void selectEditor(GenericEditor* e);

    void makeEditorVisible(GenericEditor* e, bool highlight = true);
    void makeEditorVisibleAndUpdateSettings(GenericEditor* e);
    void refreshEditors();

    void clearSignalChain();

    void signalChainCanBeEdited(bool t);

    // DragAndDropTarget methods:
    bool isInterestedInDragSource (const String& /*sourceDescription*/, Component* /*sourceComponent*/);
    void itemDragEnter (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int /*x*/, int /*y*/);
    void itemDragMove (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int /*x*/, int /*y*/);
    void itemDragExit (const String& /*sourceDescription*/, Component* /*sourceComponent*/);
    void itemDropped (const String& sourceDescription, Component* /*sourceComponent*/, int /*x*/, int /*y*/);

    // mouse and keypress methods:
    void mouseDown(const MouseEvent &e);
    void mouseDrag(const MouseEvent &e);
    void mouseUp(const MouseEvent &e);
    void mouseExit(const MouseEvent &e);
    //void mouseEnter(const MouseEvent &e);
    //void mouseExit
    //void modifierKeysChanged (const ModifierKeys & modifiers);
    bool keyPressed (const KeyPress &key);
    void moveSelection( const KeyPress &key);
    void buttonClicked(Button* button);

    Array<SignalChainTabButton*, CriticalSection> requestSignalChain() {return signalChainArray;}

    // loading and saving!
    const String saveState();
    const String loadState();

    XmlElement* createNodeXml(GenericEditor*, int);
    XmlElement* switchNodeXml(GenericProcessor*);

    void checkScrollButtons(int topTab);

    int leftmostEditor;

    File currentFile;
    
private:

    String message;
    bool somethingIsBeingDraggedOver;
    bool shiftDown;

    bool canEdit;
    GenericEditor* lastEditor;
    GenericEditor* lastEditorClicked;

    int selectionIndex;

    Array<GenericEditor*, CriticalSection> editorArray;
    Array<SignalChainTabButton*, CriticalSection> signalChainArray;

    ScopedPointer<SignalChainManager> signalChainManager;

    Font font;
    Image sourceDropImage;

    void createNewTab(GenericEditor* editor);
    void removeTab(int tabIndex);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorViewport);  

};

class SignalChainTabButton : public Button
{
public:
    SignalChainTabButton();
    ~SignalChainTabButton() {}

    void setEditor(GenericEditor* p) {firstEditor = p;}
    void setManager(SignalChainManager* scm_) {scm = scm_;}
    GenericEditor* getEditor() {return firstEditor;}

    void setNumber(int n) {num = n;}

    bool hasNewConnections() {return configurationChanged;}
    void hasNewConnections(bool t) {configurationChanged = t;}

    int offset;

private:

    GenericEditor* firstEditor;

    SignalChainManager* scm;

    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown);

    void clicked();
    
    enum actions {ADD, MOVE, REMOVE, ACTIVATE};
    
    int num;
    bool configurationChanged;

    Font buttonFont;

};

#endif  // __EDITORVIEWPORT_H_80260F3F__
