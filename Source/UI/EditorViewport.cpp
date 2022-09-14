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

#include "EditorViewport.h"

#include "GraphViewer.h"
#include "EditorViewportButtons.h"
#include "../AccessClass.h"
#include "../Processors/MessageCenter/MessageCenterEditor.h"
#include "ProcessorList.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "EditorViewportActions.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"

const int BORDER_SIZE = 6;
const int TAB_SIZE = 30;

EditorViewport::EditorViewport(SignalChainTabComponent* s_)
    : message("Drag-and-drop some rows from the top-left box onto this component!"),
      somethingIsBeingDraggedOver(false),
      shiftDown(false),
      lastEditorClicked(0),
      selectionIndex(0),
      insertionPoint(0),
      componentWantsToMove(false),
      indexOfMovingComponent(-1),
      loadingConfig(false),
      signalChainTabComponent(s_)
{

    addMouseListener(this, true);

    sourceDropImage = ImageCache::getFromMemory(BinaryData::SourceDrop_png,
                                                BinaryData::SourceDrop_pngSize);

    sourceDropImage = sourceDropImage.rescaled(25, 135,
                                               Graphics::highResamplingQuality);
    
    signalChainTabComponent->setEditorViewport(this);
    
    editorNamingLabel.setEditable(true);
    editorNamingLabel.setBounds(0,0,100,20);
    editorNamingLabel.setColour(Label::textColourId, Colours::white);
    editorNamingLabel.addListener(this);

}

EditorViewport::~EditorViewport()
{
    copyBuffer.clear();
}

void EditorViewport::paint(Graphics& g)
{

    if (somethingIsBeingDraggedOver)
    {
        g.setColour(Colours::yellow);
    }
    else
    {
        g.setColour(Colour(48,48,48));
    }

    g.drawRect(0, 0, getWidth(), getHeight()-15);
    
    if (somethingIsBeingDraggedOver)
    {
        float insertionX = (float)(BORDER_SIZE) * 2.5;

        int n;
        for (n = 0; n < insertionPoint; n++)
        {
            insertionX += editorArray[n]->getWidth();
        }

        if (n > 1)
            insertionX += BORDER_SIZE*(n-1);

        g.setColour(Colours::yellow);
        g.drawLine(insertionX, (float) BORDER_SIZE + 5,
                   insertionX, (float) getHeight()-(float) BORDER_SIZE*3 - 5, 3.0f);

    }
    
    int insertionX = BORDER_SIZE;
    g.setColour(Colours::darkgrey);

    int x = insertionX + 15;
    int y = BORDER_SIZE;

    g.drawImageAt(sourceDropImage, x, y);
    
}

bool EditorViewport::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    if (!CoreServices::getAcquisitionStatus() && dragSourceDetails.description.toString().startsWith("Processors"))
    {
        return false;
    }
    else if (dragSourceDetails.description.toString().startsWith("EditorDrag"))
    {
        return false;
    }
    else
    {
        return true;
    }

}

void EditorViewport::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    if (!CoreServices::getAcquisitionStatus())
    {
        somethingIsBeingDraggedOver = true;
        repaint();
    }
}

void EditorViewport::itemDragMove(const SourceDetails& dragSourceDetails)
{

    int x = dragSourceDetails.localPosition.getX();

    if (!CoreServices::getAcquisitionStatus())
    {
        bool foundInsertionPoint = false;

        int lastCenterPoint = -1;
        int leftEdge;
        int centerPoint;

        for (int n = 0; n < editorArray.size(); n++)
        {
            leftEdge = editorArray[n]->getX();
            centerPoint = leftEdge + (editorArray[n]->getWidth())/2;

            if (x < centerPoint && x > lastCenterPoint)
            {
                insertionPoint = n;
                foundInsertionPoint = true;
            }

            lastCenterPoint = centerPoint;
        }

        if (!foundInsertionPoint)
        {
            insertionPoint = editorArray.size();
        }
        
        repaint();
        
        refreshEditors();
    }
}

void EditorViewport::itemDragExit(const SourceDetails& dragSourceDetails)
{
    somethingIsBeingDraggedOver = false;

    repaint();

    refreshEditors();

}

void EditorViewport::itemDropped(const SourceDetails& dragSourceDetails)
{

    if (!CoreServices::getAcquisitionStatus())
    {
        Array<var>* descr = dragSourceDetails.description.getArray();
        
        Plugin::Description description;
        
        description.fromProcessorList = descr->getUnchecked(0);
        description.name = descr->getUnchecked(1);
        description.index = descr->getUnchecked(2);
        description.type = (Plugin::Type) int(descr->getUnchecked(3));
        description.processorType = (Plugin::Processor::Type) int(descr->getUnchecked(4));
        description.nodeId = 0;

        LOGD("Item dropped at insertion point ", insertionPoint);
        
        addProcessor(description, insertionPoint);
        
        insertionPoint = -1; // make sure all editors are left-justified
        indexOfMovingComponent = -1;
        somethingIsBeingDraggedOver = false;
        
        refreshEditors();

    }
}

GenericProcessor* EditorViewport::addProcessor(Plugin::Description description, int insertionPt)
{
    
    GenericProcessor* source = nullptr;
    GenericProcessor* dest = nullptr;

    if (insertionPoint > 0)
    {
        source = editorArray[insertionPoint-1]->getProcessor();
    }
    
    if (editorArray.size() > insertionPoint)
    {
        dest = editorArray[insertionPoint]->getProcessor();
    }
    
    AddProcessor* action = new AddProcessor(description, source, dest, this, loadingConfig);
    
    if (!loadingConfig)
    {
        undoManager.beginNewTransaction();
        undoManager.perform(action);
        return action->processor;
    }
    else
    {
        action->perform();
        orphanedActions.add(action);
        return action->processor;
    }
    
}

void EditorViewport::clearSignalChain()
{

    if (!CoreServices::getAcquisitionStatus())
    {
        LOGD("Clearing signal chain.");
        
        undoManager.beginNewTransaction();
        ClearSignalChain* action = new ClearSignalChain(this);
        undoManager.perform(action);
    }
    else
    {
        CoreServices::sendStatusMessage("Cannot clear signal chain while acquisition is active.");
    }
}

void EditorViewport::makeEditorVisible(GenericEditor* editor, bool highlight, bool updateSettings)
{

	if (updateSettings)
        AccessClass::getProcessorGraph()->updateSettings(editor->getProcessor());
    else
        AccessClass::getProcessorGraph()->updateViews(editor->getProcessor());
    
    if (highlight)
    {
        for (auto ed : editorArray)
        {
            if (ed == editor )
            {
                ed->select();
            } else {
                ed->deselect();
            }
        }
    }
}

void EditorViewport::removeEditor(GenericEditor* editor)
{
    int matchingIndex = -1;

    for (int i = 0; i < editorArray.size(); i++)
    {
        if (editorArray[i] == editor)
            matchingIndex = i;
    }

    if (matchingIndex > -1)
        editorArray.remove(matchingIndex);
}

void EditorViewport::updateVisibleEditors(Array<GenericEditor*> visibleEditors,
                                          int numberOfTabs,
                                          int selectedTab)
{

    if (visibleEditors.size() > 0)
    {
        for (auto editor : editorArray)
        {
            LOGD("Updating ", editor->getNameAndId());
            editor->setVisible(false);
        }
    }

    editorArray.clear();
    
    for (auto editor : visibleEditors)
    {
        editorArray.add(editor);
        addChildComponent(editor);
        editor->setVisible(true);
    }
        
    refreshEditors();
    signalChainTabComponent->refreshTabs(numberOfTabs, selectedTab);
    repaint();
}

int EditorViewport::getDesiredWidth()
{
    int desiredWidth = 0;
    
    for (auto editor : editorArray)
    {
        desiredWidth += editor->getTotalWidth() + BORDER_SIZE;
    }
    
    return desiredWidth;
}

void EditorViewport::refreshEditors()
{

    int lastBound = BORDER_SIZE;

    bool pastRightEdge = false;

    int rightEdge = getWidth();
    int numEditors = editorArray.size();

    for (int n = 0; n < editorArray.size(); n++)
    {
        
        GenericEditor* editor = editorArray[n];

        int componentWidth = editor->getTotalWidth();

        if (n == 0 && !editor->getProcessor()->isSource())
        {
            // leave room to drop a source node
            lastBound += BORDER_SIZE * 10;
        }

        if (somethingIsBeingDraggedOver && n == insertionPoint)
        {
            if (indexOfMovingComponent == -1 // adding new processor
                || (n != indexOfMovingComponent && n != indexOfMovingComponent + 1))
            {
                if (n == 0)
                    lastBound += BORDER_SIZE*3;
                else
                    lastBound += BORDER_SIZE*2;
            }
        }

        editor->setVisible(true);
        editor->setBounds(lastBound, BORDER_SIZE, componentWidth, getHeight() - BORDER_SIZE*4);
        lastBound += (componentWidth + BORDER_SIZE);
    }
    
    signalChainTabComponent->resized();
}

void EditorViewport::moveSelection(const KeyPress& key)
{

    ModifierKeys mk = key.getModifiers();

    if (key.getKeyCode() == key.leftKey)
    {

        if (mk.isShiftDown())
        {
            selectionIndex--;
        }
        else
        {
            selectionIndex = 0;

            for (int i = 0; i < editorArray.size(); i++)
            {

                if (editorArray[i]->getSelectionState() && i > 0)
                {
                    editorArray[i-1]->select();
                    lastEditorClicked = editorArray[i-1];
                    editorArray[i]->deselect();
                }
            }
        }
    }
    else if (key.getKeyCode() == key.rightKey)
    {

        if (mk.isShiftDown())
        {
            selectionIndex++;
        }
        else
        {

            selectionIndex = 0;

            // bool stopSelection = false;
            int i = 0;

            while (i < editorArray.size()-1)
            {

                if (editorArray[i]->getSelectionState())
                {

                    lastEditorClicked = editorArray[i+1];
                    editorArray[i+1]->select();
                    editorArray[i]->deselect();
                    i += 2;
                }
                else
                {
                    editorArray[i]->deselect();
                    i++;
                }
            }
        }
    }

    if (mk.isShiftDown() && lastEditorClicked != 0 && editorArray.contains(lastEditorClicked))
    {

        LOGDD("Selection index: ", selectionIndex);

        int startIndex = editorArray.indexOf(lastEditorClicked);

        if (selectionIndex < 0)
        {

             for (int i = startIndex-1; i >= startIndex + selectionIndex; i--)
             {
                 editorArray[i]->select();
             }

        } else if (selectionIndex > 0)
        {
            for (int i = startIndex+1; i <= startIndex + selectionIndex; i++)
             {
                 editorArray[i]->select();
             }
         }
    }
}

bool EditorViewport::keyPressed(const KeyPress& key)
{

    LOGDD("Editor viewport received ", key.getKeyCode());

    if (!CoreServices::getAcquisitionStatus() && editorArray.size() > 0)
    {

        ModifierKeys mk = key.getModifiers();

        if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey)
        {

            if (!mk.isAnyModifierKeyDown())
            {

                deleteSelectedProcessors();

                return true;
            }

        }
        else if (key.getKeyCode() == key.leftKey ||
                 key.getKeyCode() == key.rightKey)
        {

            moveSelection(key);

            return true;

        }
        else if (key.getKeyCode() == key.upKey)
        {

            if(lastEditorClicked)
            {
                if (lastEditorClicked->isMerger() || lastEditorClicked->isSplitter())
                {
                    lastEditorClicked->switchIO(0);
                    AccessClass::getProcessorGraph()->updateViews(lastEditorClicked->getProcessor());
                    this->grabKeyboardFocus();
                }
            }
            else
            {
                lastEditorClicked = editorArray.getFirst();
                lastEditorClicked->select();
            }
        
            return true;
        }
        else if (key.getKeyCode() == key.downKey)
        {
            if(lastEditorClicked)
            {
                if (lastEditorClicked->isMerger() || lastEditorClicked->isSplitter())
                {
                    lastEditorClicked->switchIO(1);
                    AccessClass::getProcessorGraph()->updateViews(lastEditorClicked->getProcessor());
                    this->grabKeyboardFocus();
                }
            }
            else
            {
                lastEditorClicked = editorArray.getFirst();
                lastEditorClicked->select();
            }
            return true;
        }
    }

    return false;

}

void EditorViewport::switchIO(GenericProcessor* processor, int path)
{
    
    undoManager.beginNewTransaction();
    
    SwitchIO* switchIO = new SwitchIO(processor, path);
    
    undoManager.perform(switchIO);
}


void EditorViewport::copySelectedEditors()
{

    LOGDD("Editor viewport received copy signal");

    if (!CoreServices::getAcquisitionStatus())
    {

        Array<XmlElement*> copyInfo;

        for (auto editor : editorArray)
        {
            if (editor->getSelectionState())
                copyInfo.add( createNodeXml(editor->getProcessor(), false) );
        }

        if (copyInfo.size() > 0)
        {
            copy(copyInfo);
        } else {
            CoreServices::sendStatusMessage("No processors selected.");
        }
        
    } else {
        
        CoreServices::sendStatusMessage("Cannot copy while acquisition is active.");
    }
            
}

bool EditorViewport::editorIsSelected()
{
    for (auto editor : editorArray)
    {
        if (editor->getSelectionState())
            return true;
    }
    
    return false;
}

bool EditorViewport::canPaste()
{
    if (copyBuffer.size() > 0 && editorIsSelected())
        return true;
    else
        return false;
}


void EditorViewport::copy(Array<XmlElement*> copyInfo)
{

    copyBuffer.clear();
    copyBuffer.addArray(copyInfo);
    
}

void EditorViewport::paste()
{
    LOGDD("Editor viewport received paste signal");

    if (!CoreServices::getAcquisitionStatus())
    {

        int insertionPoint;
        bool foundSelected = false;

        for (int i = 0; i < editorArray.size(); i++)
        {
            if (editorArray[i]->getSelectionState())
            {
                insertionPoint = i + 1;
                foundSelected = true;
            }
        }
        
        LOGDD("Insertion point: ", insertionPoint);

        if (foundSelected)
        {
           
            Array<XmlElement*> processorInfo;

            for (auto xml : copyBuffer)
            {
                for (auto* element : xml->getChildWithTagNameIterator("EDITOR"))
                {
                    for (auto* subelement : element->getChildWithTagNameIterator("WINDOW"))
                    {
                        subelement->setAttribute("Active", 0);
                        subelement->setAttribute("Index", -1);
                    }
                    
                    for (auto* subelement : element->getChildWithTagNameIterator("TAB"))
                    {
                        subelement->setAttribute("Active", 0);
                    }
                }
                processorInfo.add(xml);
            }

            undoManager.beginNewTransaction();

            PasteProcessors* action =
                new PasteProcessors(
                    processorInfo, insertionPoint,
                    this);

            undoManager.perform(action);

        } else {
            CoreServices::sendStatusMessage("Select an insertion point to paste.");
        }
        
    } else {
        
        CoreServices::sendStatusMessage("Cannot paste while acquisition is active.");
    }
}

void EditorViewport::undo()
{
    undoManager.undo();
}

void EditorViewport::redo()
{
    undoManager.redo();
}

void EditorViewport::selectEditor(GenericEditor* editor)
{
    for (int i = 0; i < editorArray.size(); i++)
    {

        if (editor == editorArray[i]
            || editor->getParentComponent() == editorArray[i])
        {
            editorArray[i]->select();
        }
        else
        {
            editorArray[i]->deselect();
        }
    }
}

void EditorViewport::labelTextChanged(Label* label)
{
    editorToUpdate->setDisplayName(label->getText());
}

void EditorViewport::mouseDown(const MouseEvent& e)
{

    bool clickInEditor = false;

    for (int i = 0; i < editorArray.size(); i++)
    {

        if (e.eventComponent == editorArray[i])
        {

            if (e.getNumberOfClicks() == 2) // double-clicks toggle collapse state
            {
                if (editorArray[i]->getCollapsedState())
                {
                    editorArray[i]->switchCollapsedState();
                }
                else
                {
                    if (e.y < 22)
                    {
                        editorArray[i]->switchCollapsedState();
                    }
                }
                return;
            }

            if (e.mods.isRightButtonDown())
            {

                if (!editorArray[i]->getCollapsedState() && e.y > 22)
                    return;

                if (editorArray[i]->isMerger() || editorArray[i]->isSplitter())
                    return;

                PopupMenu m;
                
                
                if (editorArray[i]->getCollapsedState())
                    m.addItem(3, "Uncollapse", true);
                else
                    m.addItem(3, "Collapse", true);

                if (!CoreServices::getAcquisitionStatus())
                    m.addItem(2, "Delete", true);
                else
                    m.addItem(2, "Delete", false);

                m.addItem(1, "Rename", true);
                
                m.addSeparator();

                m.addItem(4, "Save settings...", true);
                
                if (!CoreServices::getAcquisitionStatus())
                    m.addItem(5, "Load settings...", true);
                else
                    m.addItem(5, "Load settings...", false);
                
                m.addSeparator();
                
                m.addItem(6, "Save image...", true);


                const int result = m.show();

                if (result == 1)
                {
                    editorNamingLabel.setText("", dontSendNotification);

                    juce::Rectangle<int> rect1 = juce::Rectangle<int>(editorArray[i]->getScreenX()+20,editorArray[i]->getScreenY()+11,1,1);

                    CallOutBox callOut(editorNamingLabel, rect1, nullptr);
                    editorToUpdate = editorArray[i];
                    callOut.runModalLoop();

                    return;

                }
                else if (result == 2)
                {
                    deleteSelectedProcessors();
                    
                    return;
                }
                else if (result == 3)
                {
                    editorArray[i]->switchCollapsedState();
                    refreshEditors();
                    return;
                    
                } else if (result == 4)
                {
                    FileChooser fc("Choose the file name...",
                            CoreServices::getDefaultUserSaveDirectory(),
                            "*",
                            true);

                    if (fc.browseForFileToSave(true))
                    {
                        savePluginState(fc.getResult(), editorArray[i]);
                    }
                    else
                    {
                        CoreServices::sendStatusMessage("No file chosen.");
                    }
                } else if (result == 5)
                {
                    FileChooser fc("Choose a settings file to load...",
                            CoreServices::getDefaultUserSaveDirectory(),
                            "*",
                            true);

                    if (fc.browseForFileToOpen())
                    {
                        currentFile = fc.getResult();
                        loadPluginState(currentFile, editorArray[i]);
                    }
                    else
                    {
                        CoreServices::sendStatusMessage("No file selected.");
                    }
                } else if (result == 6)
                {
                    
                    File picturesDirectory = File::getSpecialLocation(File::SpecialLocationType::userPicturesDirectory);
                    
                    File outputFile = picturesDirectory.getChildFile(editorArray[i]->getNameAndId() + ".png");
                    
                    Rectangle<int> bounds = Rectangle<int>(3, 3, editorArray[i]->getWidth()-6, editorArray[i]->getHeight()-6);
                    
                    Image componentImage = editorArray[i]->createComponentSnapshot(
                                                                                   bounds,
                                            true, 1.5f);
                    
                    FileOutputStream stream (outputFile);
                    PNGImageFormat pngWriter;
                    pngWriter.writeImageToStream(componentImage, stream);
                    
                    CoreServices::sendStatusMessage("Saved image to " + outputFile.getFullPathName());
                    
                }
            }

            // make sure uncollapsed editors don't accept clicks outside their title bar
            if (!editorArray[i]->getCollapsedState() && e.y > 22)
                return;

            clickInEditor = true;
            editorArray[i]->select();

            if (e.mods.isShiftDown())
            {
                if (editorArray.contains(lastEditorClicked))
                {

                    int index = editorArray.indexOf(lastEditorClicked);

                    if (index > i)
                    {
                        for (int j = i+1; j <= index; j++)
                        {
                            editorArray[j]->select();
                        }

                    }
                    else
                    {
                        for (int j = i-1; j >= index; j--)
                        {
                            editorArray[j]->select();
                        }

                    }
                }

                lastEditorClicked = editorArray[i];
                break;
            }

            lastEditorClicked = editorArray[i];
        }
        else
        {

            if (!e.mods.isCtrlDown() && !e.mods.isShiftDown())
                editorArray[i]->deselect();

        }
    }

    if (!clickInEditor)
        lastEditorClicked = 0;

}

void EditorViewport::mouseDrag(const MouseEvent& e)
{


    if (editorArray.contains((GenericEditor*) e.originalComponent)
        && e.y < 15
        && !CoreServices::getAcquisitionStatus()
        && editorArray.size() > 1
        && e.getDistanceFromDragStart() > 10
        )
    {

        componentWantsToMove = true;
        indexOfMovingComponent = editorArray.indexOf((GenericEditor*) e.originalComponent);

    }

    if (componentWantsToMove)
    {

        somethingIsBeingDraggedOver = true;

        bool foundInsertionPoint = false;

        int lastCenterPoint = 0;
        int leftEdge;
        int centerPoint;

        const MouseEvent event = e.getEventRelativeTo(this);

        for (int n = 0; n < editorArray.size(); n++)
        {
            leftEdge = editorArray[n]->getX();
            centerPoint = leftEdge + (editorArray[n]->getWidth())/2;

            if (event.x < centerPoint && event.x > lastCenterPoint)
            {
                insertionPoint = n;
                foundInsertionPoint = true;
            }

            lastCenterPoint = centerPoint;
        }

        if (!foundInsertionPoint && indexOfMovingComponent != editorArray.size()-1)
        {
            insertionPoint = editorArray.size();
        }
        
        refreshEditors();
        repaint();
    }

}

void EditorViewport::mouseUp(const MouseEvent& e)
{
    
    if (componentWantsToMove)
    {
        
        somethingIsBeingDraggedOver = false;
        componentWantsToMove = false;

        if (!getScreenBounds().contains(e.getScreenPosition()))
        {
            //undoManager.beginNewTransaction();
            
             //DeleteProcessor* action =
             //       new DeleteProcessor(
             //           editorArray[indexOfMovingComponent]->getProcessor(),
              //          this);
             
             //undoManager.perform(action);
            
            repaint();
            
            refreshEditors();
        }
        else
        {
            if (indexOfMovingComponent != insertionPoint
             && indexOfMovingComponent != insertionPoint - 1)
            {
                
                GenericProcessor* newSource;
                GenericProcessor* newDest;
                
                if (insertionPoint == editorArray.size())
                {
                    newDest = nullptr;
                    newSource = editorArray.getLast()->getProcessor();
                } else if (insertionPoint == 0)
                {
                    newDest = editorArray.getFirst()->getProcessor();
                    newSource = nullptr;
                } else {
                    newSource = editorArray[insertionPoint-1]->getProcessor();
                    newDest = editorArray[insertionPoint]->getProcessor();
                }
                
                undoManager.beginNewTransaction();
               
                MoveProcessor* action = new MoveProcessor(
                                                          editorArray           [indexOfMovingComponent]->getProcessor(),
                                                          newSource,
                                                          newDest,
                                                          insertionPoint > indexOfMovingComponent);
                
                undoManager.perform(action);
                                                          
            } else {
                repaint();
            }
        }
    }

}

void EditorViewport::mouseExit(const MouseEvent& e)
{

    if (componentWantsToMove)
    {
        somethingIsBeingDraggedOver = false;
        componentWantsToMove = false;

        repaint();
    }
}


bool EditorViewport::isSignalChainEmpty()
{

    if (editorArray.size() == 0)
        return true;
    else
        return false;

}

///////////////////////////////////////////////////////////////////
////////////////SIGNAL CHAIN TAB BUTTON///////////
///////////////////////////////////////////////////////////////////

SignalChainTabButton::SignalChainTabButton(int index) : 
    Button("Signal Chain Tab Button " + String(index)), 
    num(index)
{
    setRadioGroupId(99);
    setClickingTogglesState(true);

    buttonFont = Font("Small Text", 10, Font::plain);
    buttonFont.setHeight(14);

    offset = 0;
}


void SignalChainTabButton::clicked()
{
    if (getToggleState())
    {
        LOGDD("Tab button clicked: ", num);
        
        AccessClass::getProcessorGraph()->viewSignalChain(num);
    }
}

void SignalChainTabButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    ColourGradient grad1, grad2;

    if (getToggleState() == true)
    {

        grad1 = ColourGradient(Colour(255, 136, 34), 0.0f, 0.0f,
                               Colour(230, 193, 32), 0.0f, 20.0f,
                               false);

        grad2 = ColourGradient(Colour(255, 136, 34), 0.0f, 20.0f,
                               Colour(230, 193, 32), 0.0f, 0.0f,
                               false);
    }
    else
    {
        grad2 = ColourGradient(Colour(80, 80, 80), 0.0f, 20.0f,
                               Colour(120, 120, 120), 0.0f, 0.0f,
                               false);

        grad1 =  ColourGradient(Colour(80, 80, 80), 0.0f, 0.0f,
                                Colour(120, 120, 120), 0.0f, 20.0f,
                                false);
    }

    if (isMouseOver)
    {

        grad1.multiplyOpacity(0.7f);
        grad2.multiplyOpacity(0.7f);
    }

    g.setGradientFill(grad2);
    g.fillEllipse(0,0,getWidth(),getHeight());

    g.setGradientFill(grad1);
    g.fillEllipse(2,2,getWidth()-4,getHeight()-4);

    g.setFont(buttonFont);
    g.setColour(Colours::black);

    String n;

    if (num == 0)
        n = "A";
    else if (num == 1)
        n = "B";
    else if (num == 2)
        n = "C";
    else if (num == 3)
        n = "D";
    else if (num == 4)
        n = "E";
    else if (num == 5)
        n = "F";
    else if (num == 6)
        n = "G";
    else if (num == 7)
        n = "H";
    else if (num == 8)
        n = "I";
    else
        n = "-";

    g.drawText(n,0,0,getWidth(),getHeight(),Justification::centred,true);
}

// SignalChainTabComponent

SignalChainTabComponent::SignalChainTabComponent()
{
    topTab = 0;
    
    upButton = new SignalChainScrollButton(UP);
    downButton = new SignalChainScrollButton(DOWN);

    upButton->addListener(this);
    downButton->addListener(this);
    
    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);

    viewport = new Viewport();
    viewport->setScrollBarsShown(false, true);
    viewport->setScrollBarThickness(12);
    addAndMakeVisible(viewport);

    for (int i = 0; i < 8; i++)
    {
        SignalChainTabButton* button = new SignalChainTabButton(i);
        signalChainTabButtonArray.add(button);
        addChildComponent(button);
    }
}

SignalChainTabComponent::~SignalChainTabComponent()
{
    deleteAllChildren();
}

void SignalChainTabComponent::setEditorViewport(EditorViewport* ev)
{
    editorViewport = ev;
    viewport->setViewedComponent(ev, true);
}

int SignalChainTabComponent::getScrollOffset()
{
    return viewport->getViewPositionX();
}

void SignalChainTabComponent::setScrollOffset(int offset)
{
    viewport->setViewPosition(offset, 0);
}

void SignalChainTabComponent::paint(Graphics& g)
{
    g.setColour(Colours::darkgrey);
    
    for (int n = 0; n < 4; n++)
    {
        g.drawEllipse(7,
                      (TAB_SIZE-2)*n+24,
                      TAB_SIZE-12,
                      TAB_SIZE-12,
                      1.0);
    }
}


void SignalChainTabComponent::resized()
{

    int scrollOffset = getScrollOffset();
    
    int b = 2; // border

    downButton->setBounds(b, getHeight()-25-b, TAB_SIZE-b, 15);
    upButton->setBounds(b, b, TAB_SIZE-b, 15);

    viewport->setBounds(TAB_SIZE, 0, getWidth()-TAB_SIZE, getHeight());
    
    int width = editorViewport->getDesiredWidth() < getWidth()-TAB_SIZE ? getWidth() -TAB_SIZE : editorViewport->getDesiredWidth();
    editorViewport->setBounds(0, 0, width, getHeight());
    
    setScrollOffset(scrollOffset);
}


void SignalChainTabComponent::refreshTabs(int numberOfTabs_, int selectedTab_, bool internal)
{
    numberOfTabs = numberOfTabs_;
    selectedTab = selectedTab_;
    
    if (!internal)
    {
        if (topTab < (selectedTab - 3))
            topTab = selectedTab - 3;
        else if (topTab > selectedTab && selectedTab != -1)
            topTab = selectedTab;
    }
    
    for (int i = 0; i < signalChainTabButtonArray.size(); i++)
    {
        signalChainTabButtonArray[i]->setBounds(6,
                                                (TAB_SIZE-2) * (i-topTab) + 23,
                                                TAB_SIZE-10,
                                                TAB_SIZE-10);
        
        if (i < numberOfTabs && i >= topTab && i < topTab + 4)
        {
            signalChainTabButtonArray[i]->setVisible(true);
        } else {
            signalChainTabButtonArray[i]->setVisible(false);
        }
        
        if (i == selectedTab)
        {
            signalChainTabButtonArray[i]->setToggleState(true, NotificationType::dontSendNotification);
        } else {
            signalChainTabButtonArray[i]->setToggleState(false, NotificationType::dontSendNotification);
        }
    }

}

void SignalChainTabComponent::buttonClicked(Button* button)
{
    if (button == upButton)
    {
        LOGDD("Up button pressed.");

        if (topTab > 0)
            topTab -= 1;
    }
    else if (button == downButton)
    {
        LOGDD("Down button pressed.");
        
        if (numberOfTabs > 4)
        {
            if (topTab < (numberOfTabs-4))
                topTab += 1;
        }
    }

    refreshTabs(numberOfTabs, selectedTab, true);
}

// LOADING AND SAVING

XmlElement* EditorViewport::createNodeXml(GenericProcessor* processor, bool isStartOfSignalChain)
{

    XmlElement* xml = new XmlElement("PROCESSOR");

    xml->setAttribute("name", processor->getEditor()->getName());

    if (isStartOfSignalChain)
        xml->setAttribute("insertionPoint", 0);
    else
        xml->setAttribute("insertionPoint", 1);
	xml->setAttribute("pluginName", processor->getName());
	xml->setAttribute("type", (int)(processor->getPluginType()));
	xml->setAttribute("index", processor->getIndex());
	xml->setAttribute("libraryName", processor->getLibName());
    xml->setAttribute("libraryVersion", processor->getLibVersion());
    xml->setAttribute("processorType", (int) processor->getProcessorType());

    /**Saves individual processor parameters to XML */
    processor->saveToXml(xml);

    return xml;

}


XmlElement* EditorViewport::switchNodeXml(GenericProcessor* processor)
{

    XmlElement* e = new XmlElement("SWITCH");

    e->setAttribute("number", processor->saveOrder);

    return e;

}

const String EditorViewport::saveState(File fileToUse, String& xmlText)
{
	return saveState(fileToUse, &xmlText);
}

const String EditorViewport::saveState(File fileToUse, String* xmlText)
{

    String error;

    currentFile = fileToUse;
    
    std::unique_ptr<XmlElement> xml = createSettingsXml();

    if (! xml->writeTo(currentFile))
        error = "Couldn't write to file ";
    else
        error = "Saved configuration as ";

    error += currentFile.getFileName();
    
    LOGD("Editor viewport saved state.");

    return error;
    
}
    
std::unique_ptr<XmlElement> EditorViewport::createSettingsXml()
{
    
    Array<GenericProcessor*> splitPoints;
    Array<GenericProcessor*> allSplitters;
    Array<int> splitterStates;
    /** Used to reset saveOrder at end, to allow saving the same processor multiple times*/
    Array<GenericProcessor*> allProcessors;

    int saveOrder = 0;
    
    std::unique_ptr<XmlElement> xml = std::unique_ptr<XmlElement>(new XmlElement("SETTINGS"));

    XmlElement* info = xml->createNewChildElement("INFO");

    XmlElement* version = info->createNewChildElement("VERSION");
    version->addTextElement(JUCEApplication::getInstance()->getApplicationVersion());

	XmlElement* pluginAPIVersion = info->createNewChildElement("PLUGIN_API_VERSION");
	pluginAPIVersion->addTextElement(String(PLUGIN_API_VER));

    Time currentTime = Time::getCurrentTime();

    info->createNewChildElement("DATE")->addTextElement(currentTime.toString(true, true, true, true));
    info->createNewChildElement("OS")->addTextElement(SystemStats::getOperatingSystemName());
    
    XmlElement* machine = info->createNewChildElement("MACHINE");
    machine->setAttribute("name", SystemStats::getComputerName());
    machine->setAttribute("cpu_model", SystemStats::getCpuModel());
    machine->setAttribute("cpu_num_cores", SystemStats::getNumCpus());

    Array<GenericProcessor*> rootNodes = AccessClass::getProcessorGraph()->getRootNodes();
    
    for (int i = 0; i < rootNodes.size(); i++)
    {
        XmlElement* signalChain = new XmlElement("SIGNALCHAIN");
        
        bool isStartOfSignalChain = true;
        
        GenericProcessor* processor = rootNodes[i];

        while (processor != nullptr)
        {
            if (processor->saveOrder < 0)
            {
                
                // create a new XML element
                signalChain->addChildElement(createNodeXml(processor,  isStartOfSignalChain));
                processor->saveOrder = saveOrder;
                allProcessors.addIfNotAlreadyThere(processor);
                saveOrder++;
                
                if (processor->isSplitter())
                {
                    // add to list of splitters to come back to
                    splitPoints.add(processor);

                    //keep track of all splitters and their inital states
                    allSplitters.add(processor);
                    Splitter* sp = (Splitter*)processor;
                    splitterStates.add(sp->getPath());
                    
                    processor->switchIO(0);
                }

            }

            // continue until the end of the chain
            LOGDD("  Moving forward along signal chain.");
            processor = processor->getDestNode();
            isStartOfSignalChain = false;

            if (processor == nullptr)
            {
                if (splitPoints.size() > 0)
                {
                    LOGDD("  Going back to first unswitched splitter.");

                    processor = splitPoints.getFirst();
                    splitPoints.remove(0);

                    processor->switchIO(1);
                    signalChain->addChildElement(switchNodeXml(processor));
                }
                else
                {
                    LOGDD("  End of chain.");
                }
            }
            
        }

        xml->addChildElement(signalChain);
    }

    // Loop through all splitters and reset their states to original values
    for (int i = 0; i < allSplitters.size(); i++) {
        allSplitters[i]->switchIO(splitterStates[i]);
    }

    XmlElement* editorViewportSettings = new XmlElement("EDITORVIEWPORT");
    editorViewportSettings->setAttribute("scroll", signalChainTabComponent->getScrollOffset());
    
    for (auto editor : editorArray)
    {
        XmlElement* visibleEditorXml = new XmlElement(editor->getName().replaceCharacters(" ","_").toUpperCase());
        visibleEditorXml->setAttribute("ID", editor->getProcessor()->getNodeId());
        editorViewportSettings->addChildElement(visibleEditorXml);
    }
    
    xml->addChildElement(editorViewportSettings);

    AccessClass::getDataViewport()->saveStateToXml(xml.get()); // save the data viewport settings
    
    XmlElement* audioSettings = new XmlElement("AUDIO");
    AccessClass::getAudioComponent()->saveStateToXml(audioSettings);
    xml->addChildElement(audioSettings);

    //Resets Save Order for processors, allowing them to be saved again without omitting themselves from the order.
    int allProcessorSize = allProcessors.size();
    for (int i = 0; i < allProcessorSize; i++)
    {
        allProcessors.operator[](i)->saveOrder = -1;
    }

    AccessClass::getControlPanel()->saveStateToXml(xml.get()); // save the control panel settings
    AccessClass::getProcessorList()->saveStateToXml(xml.get());
    AccessClass::getUIComponent()->saveStateToXml(xml.get());  // save the UI settings

    return xml;
    
}

const String EditorViewport::loadPluginState(File fileToLoad, GenericEditor* selectedEditor)
{

    int numSelected = 0;
    
    if (selectedEditor == nullptr)
    {
        for (auto editor : editorArray)
        {
            if (editor->getSelectionState())
            {
                selectedEditor = editor;
                numSelected++;
            }
        }
    } else {
        numSelected = 1;
    }
    
    if (numSelected == 0)
    {
        return("No editors selected.");
        
    } else if (numSelected > 1)
    {
        return("Multiple editors selected.");
        
    } else {
        
        XmlDocument doc(fileToLoad);
        std::unique_ptr<XmlElement> xml = doc.getDocumentElement();
        
        if (xml == 0 || ! xml->hasTagName("PROCESSOR"))
        {
            LOGC("Not a valid file.");
            return "Not a valid file.";
        } else {
            
            undoManager.beginNewTransaction();
            
            LoadPluginSettings* action = new LoadPluginSettings(this,
                                                             selectedEditor->getProcessor(),
                                                             xml.get());
            undoManager.perform(action);
        }
    }
    
    return "Success";
}
    
const String EditorViewport::savePluginState(File fileToSave, GenericEditor* selectedEditor)
{
    
    int numSelected = 0;
    
    if (selectedEditor == nullptr)
    {
        for (auto editor : editorArray)
        {
            if (editor->getSelectionState())
            {
                selectedEditor = editor;
                numSelected++;
            }
        }
    } else {
        numSelected = 1;
    }
    
    if (numSelected == 0)
    {
        return("No editors selected.");
        
    } else if (numSelected > 1)
    {
        return("Multiple editors selected.");
    } else {
        
        String error;
        
        XmlElement* settings = createNodeXml(selectedEditor->getProcessor(), false);
        
        if (! settings->writeTo(fileToSave))
            error = "Couldn't write to file ";
        else
            error = "Saved plugin settings to ";

        error += fileToSave.getFileName();

        delete settings;
        
        return error;
        
    }
    
    
}

const String EditorViewport::loadState(File fileToLoad)
{
    
    currentFile = fileToLoad;

    LOGC("Loading configuration from ", fileToLoad.getFullPathName());
   
    XmlDocument doc(currentFile);
    std::unique_ptr<XmlElement> xml = doc.getDocumentElement();
    
    if (xml == 0 || ! xml->hasTagName("SETTINGS"))
    {
        LOGC("Not a valid configuration file.");
        return "Not a valid file.";
    }

    undoManager.beginNewTransaction();
    
    LoadSignalChain* action = new LoadSignalChain(this, xml);
    undoManager.perform(action);

    CoreServices::sendStatusMessage("Loaded " + fileToLoad.getFileName());
    
    return "Loaded signal chain.";
    
}

const String EditorViewport::loadStateFromXml(XmlElement* xml)
{

    Array<GenericProcessor*> splitPoints;

    bool sameVersion = false;
    bool compatibleVersion = false;
    int scrollOffset;

    String versionString;

    for (auto* element : xml->getChildIterator())
    {
        if (element->hasTagName("INFO"))
        {
            for (auto* element2 : element->getChildIterator())
            {
                if (element2->hasTagName("VERSION"))
                {
                    versionString = element2->getAllSubText();
					
                    if (versionString.equalsIgnoreCase(JUCEApplication::getInstance()->getApplicationVersion()))
                    {
                        sameVersion = true;
                        compatibleVersion = true;
                        break;
                    }
                    
                    StringArray tokens;
                    tokens.addTokens(versionString, ".", "");
                    
                    LOGD("Version string: ", versionString);

                    if (tokens.size() > 1)
                    {
                        if (tokens[0].getIntValue() >= 0 && tokens[1].getIntValue() > 5)
                            compatibleVersion = true;
                    }
                }
            }
        }
    }
    
    if (!compatibleVersion)
    {
        String responseString = "Your configuration file was saved by an older version of the GUI (";
        responseString += versionString;
        responseString += "), and is not compatible with the version you're currently running. \n\n";
        responseString += "In order to replicate the signal chain you'll have to re-build it from scratch.";

        AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Incompatible configuration file", responseString);
        
        return "Failed To Open " + currentFile.getFileName();
    }
    
    if (!sameVersion)
    {
        String responseString = "Your configuration file was saved from a different version of the GUI than the one you're using. \n";
        responseString += "The current software is version ";
        responseString += JUCEApplication::getInstance()->getApplicationVersion();
        responseString += ", but the file you selected ";
        if (versionString.length() > 0)
        {
            responseString += "was saved by version ";
            responseString += versionString;
        }
        else
        {
            responseString += "does not have a version number";
        }

        responseString += ".\n This file may not load properly. Continue?";

        bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
            "Configuration file version mismatch", responseString);

        if (!response)
        {
            return "Failed To Open " + currentFile.getFileName();
        }
    }
    
    MouseCursor::showWaitCursor();

    AccessClass::getProcessorList()->loadStateFromXml(xml); // load the processor list settings
    AccessClass::getUIComponent()->loadStateFromXml(xml);   // load the UI settings

    AccessClass::getProcessorGraph()->clearSignalChain();

    loadingConfig = true; //Indicate config is being loaded into the GUI
    String description;// = " ";
    int loadOrder = 0;

    GenericProcessor* p;

    for (auto* element : xml->getChildIterator())
    {
        if (element->hasTagName("SIGNALCHAIN"))
        {
            for (auto* processor : element->getChildIterator())
            {
                if (processor->hasTagName("PROCESSOR"))
                {

                    auto loadedPlugins = AccessClass::getProcessorList()->getItemList();
                    String pName = processor->getStringAttribute("pluginName");

                    if(!loadedPlugins.contains(pName))
                    {
                        LOGC(pName, " plugin not found in Processor List! Looking for it on Artifactory...");

                        String libName = processor->getStringAttribute("libraryName");
                        String libVer = processor->getStringAttribute("libraryVersion");
                        libVer = libVer.isEmpty() ? "" : libVer + "-API" + String(PLUGIN_API_VER);
                        
                        CoreServices::PluginInstaller::installPlugin(libName, libVer);
                    }
                    
                    int insertionPt = processor->getIntAttribute("insertionPoint");
                    
                    LOGD("Creating processor: ", pName);
                    p = createProcessorAtInsertionPoint(processor, insertionPt, false);
                    p->loadOrder = loadOrder++;
                    
                    if (p->isSplitter())
                    {
                        splitPoints.add(p);
                    }
                }
                else if (processor->hasTagName("SWITCH"))
                {
                    int processorNum = processor->getIntAttribute("number");

                    LOGDD("SWITCHING number ", processorNum);

                    for (int n = 0; n < splitPoints.size(); n++)
                    {
                        LOGDD("Trying split point ", n,  ", load order: ", splitPoints[n]->loadOrder);

                        if (splitPoints[n]->loadOrder == processorNum)
                        {
                            LOGDD("Switching splitter destination.");
                            SplitterEditor* editor = (SplitterEditor*) splitPoints[n]->getEditor();
                            editor->switchDest(1);
                            AccessClass::getProcessorGraph()->updateViews(splitPoints[n]);
                            
                            splitPoints.remove(n);
                        }
                    }
                }
            }
        }
        else if (element->hasTagName("AUDIO"))
        {
            AccessClass::getAudioComponent()->loadStateFromXml(element);
            AccessClass::getControlPanel()->loadStateFromXml(xml);  // load the control panel settings after the audio settings
        }
        else if (element->hasTagName("EDITORVIEWPORT"))
        {
            
            for (auto editor : editorArray)
            {
                editor->setVisible(false);
            }
            editorArray.clear();
            
            for (auto* visibleEditor : element->getChildIterator())
            {
                int nodeId = visibleEditor->getIntAttribute("ID");
                editorArray.add(AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId)->getEditor());
            }
            
            refreshEditors();
            
            scrollOffset = element->getIntAttribute("scroll", 0);
            
        }

    }

    AccessClass::getProcessorGraph()->restoreParameters();  // loads the processor graph settings

    AccessClass::getDataViewport()->loadStateFromXml(xml);

    String error = "Opened ";
    error += currentFile.getFileName();

    loadingConfig = false;

    MouseCursor::hideWaitCursor();
    
    signalChainTabComponent->setScrollOffset(scrollOffset);
    
    return error;
}

void EditorViewport::deleteSelectedProcessors()
{
    undoManager.beginNewTransaction();

    Array<GenericEditor*> editors = Array(editorArray);
    
    for (auto editor : editors)
    {
        if (editor->getSelectionState())
        {
            editorArray.remove(editorArray.indexOf(editor));
            DeleteProcessor* action = new DeleteProcessor(editor->getProcessor(), this);
            undoManager.perform(action);
        }
    }

}

Plugin::Description EditorViewport::getDescriptionFromXml(XmlElement* settings, bool ignoreNodeId)
{
    Plugin::Description description;
    
    description.fromProcessorList = false;
    description.name = settings->getStringAttribute("pluginName");
    description.type = (Plugin::Type) settings->getIntAttribute("type");
    description.processorType = (Plugin::Processor::Type) settings->getIntAttribute("processorType");
    description.index = settings->getIntAttribute("index");
    description.libName = settings->getStringAttribute("libraryName");
    description.libVersion = settings->getStringAttribute("libraryVersion");
    
    if (!ignoreNodeId)
        description.nodeId = settings->getIntAttribute("nodeId");
    else
        description.nodeId = -1;
    
    return description;
}

GenericProcessor* EditorViewport::createProcessorAtInsertionPoint(XmlElement* parametersAsXml,
                                                                  int insertionPt,
                                                                  bool ignoreNodeId)
{
    if (loadingConfig)
    {
        if (insertionPt == 1)
        {
            insertionPoint = editorArray.size();
        }
        else
        {
            insertionPoint = 0;
        }

    } else {
        insertionPoint = insertionPt;
    }
    
    Plugin::Description description = getDescriptionFromXml(parametersAsXml, ignoreNodeId);
    
    GenericProcessor* p = addProcessor(description, insertionPoint);
    p->parametersAsXml = parametersAsXml;
    
    return p;
}

