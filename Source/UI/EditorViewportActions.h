/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#ifndef EditorViewportActions_h
#define EditorViewportActions_h

#include "EditorViewport.h"

/** 
    Adds a processor to the signal chain, 
    based on the description.

    Undo: remove the processor from the 
    signal chain.
*/
class AddProcessor : public UndoableAction
{
    
public:

    /** Constructor*/
    AddProcessor(Plugin::Description description,
                 GenericProcessor* source,
                 GenericProcessor* dest,
                 EditorViewport*,
                 bool signalChainIsLoading = false);
 
    /** Destructor */
    ~AddProcessor();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();
    
    GenericProcessor* processor;
    
    XmlElement* settings;
    
private:

    int sourceNodeId;
    int destNodeId;

    bool signalChainIsLoading;
    
    int nodeId;
    
    Plugin::Description description;

    EditorViewport* editorViewport;

};


/**
    Adds a processor to the signal chain,
    based on the description.

    Undo: remove the processor from the
    signal chain.
*/
class PasteProcessors : public UndoableAction
{

public:

    /** Constructor*/
    PasteProcessors(Array<XmlElement*> copyBuffer,
        int insertionPoint,
        EditorViewport* editorViewport);

    /** Destructor */
    ~PasteProcessors();

    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

    
private:

    Array<XmlElement*> copyBuffer;

    Array<int> nodeIds;

    int insertionPoint;

    EditorViewport* editorViewport;

};


/**
    Deletes a processor to the signal chain,
    based on a pointer to the GenericProcessor object

    Undo: adds the processor back to the signal chain
*/
class DeleteProcessor : public UndoableAction
{
    
public:

    /** Constructor */
    DeleteProcessor(GenericProcessor* p, EditorViewport*);
 
    /** Destructor */
    ~DeleteProcessor();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();
    
private:

    GenericProcessor* processor;
    
    int sourceNodeId;
    int destNodeId;
    int nodeId;
    
    XmlElement* settings;
    
    EditorViewport* editorViewport;

};


/**
    Moves a processor to a new location in the signal chain

    Undo: moves the processor back to its 
    original location
*/
class MoveProcessor : public UndoableAction
{
    
public:

    /** Constructor */
    MoveProcessor(GenericProcessor* p,
                  GenericProcessor* source,
                  GenericProcessor* dest,
                  bool moveDownstream);
 
    /** Destructor */
    ~MoveProcessor();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();
    
private:

    int nodeId;
    
    int originalSourceNodeId;
    int originalDestNodeId;
    int newSourceNodeId;
    int newDestNodeId;
    
    int originalDestNodeDestNodeId;
    
    bool moveDownstream;
    
};

/**
    
    Clears the signal chain

    Undo: restores the signal chain back to its
    state prior to being cleared.
*/
class ClearSignalChain : public UndoableAction
{
    
public:

    /** Constructor */
    ClearSignalChain(EditorViewport*);
 
    /** Destructor */
    ~ClearSignalChain();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();
    
private:
    
    std::unique_ptr<XmlElement> settings;

    EditorViewport* editorViewport;

};

/**
    Loads a signal chain from an XML settings object

    Undo: restores the signal chain to its previous state
*/
class LoadSignalChain : public UndoableAction
{
    
public:
    /** Constructor */
    LoadSignalChain(EditorViewport*, std::unique_ptr<XmlElement>& newSettings);
 
    /** Destructor */
    ~LoadSignalChain();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();
    
    const String getError();
    
private:
    
    std::unique_ptr<XmlElement> oldSettings;
    std::unique_ptr<XmlElement> newSettings;
    
    String error;

    EditorViewport* editorViewport;

};

/**
    Loads the settings for an individual plugin

    Undo: restores the plugin back to its previous state
*/
class LoadPluginSettings : public UndoableAction
{
    
public:
    /** Constructor */
    LoadPluginSettings(EditorViewport*, GenericProcessor* processor, XmlElement* newSettings);
 
    /** Destructor */
    ~LoadPluginSettings();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();
    
    const String getError();
    
private:
    
    XmlElement* oldSettings;
    XmlElement* newSettings;
    
    String error;

    EditorViewport* editorViewport;
    int processorId;

};

/**
    Changes the path displayed by a merger or splitter

    Undo: restores the merger or splitter back to its
    original state

*/
class SwitchIO : public UndoableAction
{
    
public:
    /** Constructor */
    SwitchIO(GenericProcessor* processor, int path);
 
    /** Destructor */
    ~SwitchIO();
    
    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

private:

    EditorViewport* editorViewport;
    int processorId;
    
    int originalPath;

};

#endif /* EditorViewportActions_h */
