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


class AddProcessor : public UndoableAction
{
    
public:
    AddProcessor(ProcessorDescription description,
                 GenericProcessor* source,
                 GenericProcessor* dest,
                 EditorViewport*);
 
    ~AddProcessor();
    
    bool perform();
    bool undo();
    
    GenericProcessor* processor;
    
    XmlElement* settings;
    
private:

    int sourceNodeId;
    int destNodeId;
    
    int nodeId;
    
    ProcessorDescription description;

    EditorViewport* editorViewport;

};

class DeleteProcessor : public UndoableAction
{
    
public:
    DeleteProcessor(GenericProcessor* p, EditorViewport*);
 
    ~DeleteProcessor();
    
    bool perform();
    bool undo();
    
private:

    GenericProcessor* processor;
    
    int sourceNodeId;
    int destNodeId;
    int nodeId;
    
    XmlElement* settings;
    
    EditorViewport* editorViewport;

};



class MoveProcessor : public UndoableAction
{
    
public:
    MoveProcessor(GenericProcessor* p,
                  GenericProcessor* source,
                  GenericProcessor* dest,
                  bool moveDownstream);
 
    ~MoveProcessor();
    
    bool perform();
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

class ClearSignalChain : public UndoableAction
{
    
public:
    ClearSignalChain(EditorViewport*);
 
    ~ClearSignalChain();
    
    bool perform();
    bool undo();
    
private:
    
    XmlElement* settings;

    EditorViewport* editorViewport;

};

class LoadSignalChain : public UndoableAction
{
    
public:
    LoadSignalChain(EditorViewport*, XmlElement* newSettings);
 
    ~LoadSignalChain();
    
    bool perform();
    bool undo();
    
    const String getError();
    
private:
    
    XmlElement* oldSettings;
    XmlElement* newSettings;
    
    String error;

    EditorViewport* editorViewport;

};

class LoadPluginSettings : public UndoableAction
{
    
public:
    LoadPluginSettings(EditorViewport*, GenericProcessor* processor, XmlElement* newSettings);
 
    ~LoadPluginSettings();
    
    bool perform();
    bool undo();
    
    const String getError();
    
private:
    
    XmlElement* oldSettings;
    XmlElement* newSettings;
    
    String error;

    EditorViewport* editorViewport;
    int processorId;

};

class SwitchIO : public UndoableAction
{
    
public:
    SwitchIO(GenericProcessor* processor, int path);
 
    ~SwitchIO();
    
    bool perform();
    bool undo();

private:

    EditorViewport* editorViewport;
    int processorId;
    
    int originalPath;

};

#endif /* EditorViewportActions_h */
