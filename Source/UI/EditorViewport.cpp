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

    //MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    //Typeface::Ptr typeface = new CustomTypeface(mis);
    
    //font = Font("Small Text", 10, Font::plain);
    //font.setHeight(10);

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

}

void EditorViewport::resized()
{
  //  refreshEditors();
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
        g.drawLine(insertionX, (float) BORDER_SIZE,
                   insertionX, (float) getHeight()-(float) BORDER_SIZE*3, 3.0f);

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
        ProcessorDescription description;
        
        description.fromProcessorList = descr->getUnchecked(0);
        description.processorName = descr->getUnchecked(1);
        description.processorType = descr->getUnchecked(2);
        description.processorIndex = descr->getUnchecked(3);
        description.libName = descr->getUnchecked(4);
        description.nodeId = 0;

        message = "last filter dropped: " + description.processorName;

        LOGD("Item dropped at insertion point ", insertionPoint);
        
        addProcessor(description, insertionPoint);
        
        insertionPoint = -1; // make sure all editors are left-justified
        indexOfMovingComponent = -1;
        somethingIsBeingDraggedOver = false;
        
        refreshEditors();

    }
}

GenericProcessor* EditorViewport::addProcessor(ProcessorDescription description, int insertionPt)
{
    GenericProcessor* sourceProcessor = nullptr;
    GenericProcessor* destProcessor = nullptr;

    if (insertionPoint > 0)
    {
        sourceProcessor = editorArray[insertionPt-1]->getProcessor();
    }
    
    if (editorArray.size() > insertionPoint)
    {
        destProcessor = editorArray[insertionPoint]->getProcessor();
    }
    
    return AccessClass::getProcessorGraph()->createProcessor(description,
                                                      sourceProcessor,
                                                      destProcessor,
                                                      loadingConfig);
}

void EditorViewport::clearSignalChain()
{

    if (!CoreServices::getAcquisitionStatus())
    {
        AccessClass::getProcessorGraph()->clearSignalChain();
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

void EditorViewport::updateVisibleEditors(Array<GenericEditor*> visibleEditors,
                                          int numberOfTabs,
                                          int selectedTab)
{
    for (auto editor : editorArray)
        editor->setVisible(false);
    
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
        desiredWidth += editor->desiredWidth + BORDER_SIZE;
    }
    
    return desiredWidth;
}

void EditorViewport::refreshEditors()
{

    int lastBound = BORDER_SIZE;

LOGDD(insertionPoint);

    bool pastRightEdge = false;

    int rightEdge = getWidth();
    int numEditors = editorArray.size();

    for (int n = 0; n < editorArray.size(); n++)
    {
        GenericEditor* editor = editorArray[n];
        int componentWidth = editor->desiredWidth;

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

    // } else if (key.getKeyCode() == key.upKey) {

    //     // move one tab up
    // } else if (key.getKeyCode() == key.downKey) {

    //     // move one tab down
    // }
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

                Array<GenericProcessor*> processorsToRemove;

                for (auto editor : editorArray)
                {
                    if (editor->getSelectionState())
                        processorsToRemove.add(editor->getProcessor());
                }

                AccessClass::getProcessorGraph()->deleteNodes(processorsToRemove);

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

            lastEditorClicked->switchIO(0);
            AccessClass::getProcessorGraph()->updateSettings(lastEditorClicked->getProcessor());

            return true;
        }
        else if (key.getKeyCode() == key.downKey)
        {
            lastEditorClicked->switchIO(1);
            AccessClass::getProcessorGraph()->updateSettings(lastEditorClicked->getProcessor());
            return true;
        }
    }

    return false;

}

//void EditorViewport::modifierKeysChanged (const ModifierKeys & modifiers) {

/*     if (modifiers.isShiftDown()) {

LOGD("Shift key pressed.");
        shiftDown  = true;

    } else {


LOGD("Shift key released.");
        shiftDown = false;
    }*/

//}

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

            // || e.eventComponent->getParentComponent() == editorArray[i] ||
            //    e.eventComponent->getParentComponent()->getParentComponent() ==
            //            editorArray[i])
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

                const int result = m.show();

                if (result == 1)
                {
                    editorNamingLabel.setText("", dontSendNotification);

                    juce::Rectangle<int> rect1 = juce::Rectangle<int>(editorArray[i]->getScreenX()+20,editorArray[i]->getScreenY()+11,1,1);

                    CallOutBox callOut(editorNamingLabel, rect1, nullptr);
                    editorToUpdate = editorArray[i];
                    callOut.runModalLoop();
                    // editorNamingLabel.showEditor();
                    //CallOutBox& myBox = CallOutBox::launchAsynchronously(&editorNamingLabel, rect1, nullptr);

                    return;

                }
                else if (result == 2)
                {
                    
                    Array<GenericProcessor*> processorsToRemove;

                    processorsToRemove.add(editorArray[i]->getProcessor());

                    AccessClass::getProcessorGraph()->deleteNodes(processorsToRemove);
                    return;
                }
                else if (result == 3)
                {
                    editorArray[i]->switchCollapsedState();
                    refreshEditors();
                    return;
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


            //     Array<GenericEditor*> editorsToSelect;
            //     bool foundSelected = false;

            //     for (int j = i; j < editorArray.size(); j++)
            //     {
            //         editorsToSelect.add(editorArray[j]);

            //         if (editorArray[j]->getSelectionState())
            //         {
            //             foundSelected = true;
            //             break;
            //         }
            //     }

            //     if (!foundSelected)
            //         editorsToSelect.clear();

            //     for (int j = 0; j < editorsToSelect.size(); j++)
            //     {
            //         editorsToSelect[j]->select();
            //     }

            //     for (int j = i; j > -1; j--)
            //     {
            //         editorsToSelect.add(editorArray[j]);
            //         if (editorArray[j]->getSelectionState())
            //         {
            //             foundSelected = true;
            //             break;
            //         }
            //     }

            //     if (!foundSelected)
            //         editorsToSelect.clear();

            //     for (int j = 0; j < editorsToSelect.size(); j++)
            //     {
            //         editorsToSelect[j]->select();
            //     }

            //     break;

            // }

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

        if (indexOfMovingComponent != insertionPoint)
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
            
            AccessClass::getProcessorGraph()->moveProcessor(editorArray[indexOfMovingComponent]->getProcessor(),
                                                            newSource, newDest,
                                                            insertionPoint > indexOfMovingComponent);
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
        //refreshEditors();

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
////////////////SIGNAL CHAIN TAB BUTTON////////////////////////////
///////////////////////////////////////////////////////////////////

SignalChainTabButton::SignalChainTabButton(int index) : Button("Name"), num(index)
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
        LOGD("Tab button clicked: ", num);
        
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

    int b = 2; // border

    downButton->setBounds(b, getHeight()-25-b, TAB_SIZE-b, 15);
    upButton->setBounds(b, b, TAB_SIZE-b, 15);

    viewport->setBounds(TAB_SIZE, 0, getWidth()-TAB_SIZE, getHeight());
    
    int width = editorViewport->getDesiredWidth() < getWidth()-TAB_SIZE ? getWidth() -TAB_SIZE : editorViewport->getDesiredWidth();
    editorViewport->setBounds(0, 0, width, getHeight());
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
        LOGD("Up button pressed.");

        if (topTab > 0)
            topTab -= 1;
    }
    else if (button == downButton)
    {
        LOGD("Down button pressed.");
        
        if (numberOfTabs > 4)
        {
            if (topTab < (numberOfTabs-4))
                topTab += 1;
        }
    }

    refreshTabs(numberOfTabs, selectedTab, true);
}

// LOADING AND SAVING

XmlElement* EditorViewport::createNodeXml(GenericProcessor* source, bool isStartOfSignalChain)
{

    XmlElement* e = new XmlElement("PROCESSOR");

    String name = "";

    if (source->isSource())
        name += "Sources/";
    else if (source->isSink())
        name += "Sinks/";
    else if (source->isSplitter() || source->isMerger() || source->isUtility())
        name += "Utilities/";
    else
        name += "Filters/";

    name += source->getEditor()->getName();

LOGDD(name);

    e->setAttribute("name", name);
    if (isStartOfSignalChain)
        e->setAttribute("insertionPoint", 0);
    else
        e->setAttribute("insertionPoint", 1);
	e->setAttribute("pluginName", source->getPluginName());
	e->setAttribute("pluginType", (int)(source->getPluginType()));
	e->setAttribute("pluginIndex", source->getIndex());
	e->setAttribute("libraryName", source->getLibName());
	e->setAttribute("libraryVersion", source->getLibVersion());
	e->setAttribute("isSource", source->isSource());
	e->setAttribute("isSink", source->isSink());

    /**Saves individual processor parameters to XML */
    LOGDD("Create subnodes with parameters");
    source->saveToXml(e);

    return e;

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

    Array<GenericProcessor*> splitPoints;
    Array<GenericProcessor*> allSplitters;
    Array<int> splitterStates;
    /** Used to reset saveOrder at end, to allow saving the same processor multiple times*/
    Array<GenericProcessor*> allProcessors;

    int saveOrder = 0;

    XmlElement* xml = new XmlElement("SETTINGS");

    XmlElement* info = xml->createNewChildElement("INFO");

    XmlElement* version = info->createNewChildElement("VERSION");
    version->addTextElement(JUCEApplication::getInstance()->getApplicationVersion());

	XmlElement* pluginAPIVersion = info->createNewChildElement("PLUGIN_API_VERSION");
	pluginAPIVersion->addTextElement(String(PLUGIN_API_VER));

    Time currentTime = Time::getCurrentTime();

    XmlElement* date = info->createNewChildElement("DATE");
    date->addTextElement(currentTime.toString(true, true, true, true));

    XmlElement* operatingSystem = info->createNewChildElement("OS");
    operatingSystem->addTextElement(SystemStats::getOperatingSystemName());

    XmlElement* machineName = info->createNewChildElement("MACHINE");
    machineName->addTextElement(SystemStats::getComputerName());
    
    Array<GenericProcessor*> rootNodes = AccessClass::getProcessorGraph()->getRootNodes();
    
    for (auto p : rootNodes)
    {
        XmlElement* signalChain = new XmlElement("SIGNALCHAIN");
        
        bool isStartOfSignalChain = true;
        
        GenericProcessor* processor = p;

        while (processor != nullptr)
        {
            if (processor->saveOrder < 0)
            {
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

                // create a new XML element
                signalChain->addChildElement(createNodeXml(processor,  isStartOfSignalChain));
                processor->saveOrder = saveOrder;
                allProcessors.addIfNotAlreadyThere(processor);
                saveOrder++;

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

    XmlElement* audioSettings = new XmlElement("AUDIO");

    AccessClass::getAudioComponent()->saveStateToXml(audioSettings);
    xml->addChildElement(audioSettings);

    /*
	XmlElement* recordSettings = new XmlElement("RECORDING");
	recordSettings->setAttribute("isRecordThreadEnabled", AccessClass::getProcessorGraph()->getRecordNode()->getRecordThreadStatus());
	xml->addChildElement(recordSettings);
    */

	XmlElement* timestampSettings = new XmlElement("GLOBAL_TIMESTAMP");
	int tsID, tsSubID;
	AccessClass::getProcessorGraph()->getTimestampSources(tsID, tsSubID);
	timestampSettings->setAttribute("selected_index", tsID);
	timestampSettings->setAttribute("selected_sub_index", tsSubID);
	xml->addChildElement(timestampSettings);

    //Resets Save Order for processors, allowing them to be saved again without omitting themselves from the order.
    int allProcessorSize = allProcessors.size();
    for (int i = 0; i < allProcessorSize; i++)
    {
        allProcessors.operator[](i)->saveOrder = -1;
    }

    AccessClass::getControlPanel()->saveStateToXml(xml); // save the control panel settings
    AccessClass::getProcessorList()->saveStateToXml(xml);
    AccessClass::getUIComponent()->saveStateToXml(xml);  // save the UI settings

    if (! xml->writeToFile(currentFile, String::empty))
        error = "Couldn't write to file ";
    else
        error = "Saved configuration as ";

    error += currentFile.getFileName();

	if (xmlText != nullptr)
	{
		(*xmlText) = xml->createDocument(String::empty);
		if ((*xmlText).isEmpty())
			(*xmlText) = "Couldn't create configuration xml";
	}

    delete xml;

    return error;
}

const String EditorViewport::loadState(File fileToLoad)
{
    
    currentFile = fileToLoad;

    LOGD("Loading processor graph.");

    Array<GenericProcessor*> splitPoints;

    XmlDocument doc(currentFile);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName("SETTINGS"))
    {
        LOGD("File not found.");
        delete xml;
        return "Not a valid file.";
    }

    bool sameVersion = false;
	bool pluginAPI = false;
	bool rhythmNodePatch = false;
    String versionString;

    forEachXmlChildElement(*xml, element)
    {
        if (element->hasTagName("INFO"))
        {
            forEachXmlChildElement(*element, element2)
            {
                if (element2->hasTagName("VERSION"))
                {
                    versionString = element2->getAllSubText();
					StringArray tokens;
					tokens.addTokens(versionString, ".", String::empty);

					//Patch to correctly load saved chains from before 0.4.4
					if (tokens[0].getIntValue() == 0 && tokens[1].getIntValue() == 4 && tokens[2].getIntValue() < 4)
						rhythmNodePatch = true;

                    if (versionString.equalsIgnoreCase(JUCEApplication::getInstance()->getApplicationVersion()))
                        sameVersion = true;
                }
				else if (element2->hasTagName("PLUGIN_API_VERSION"))
				{
					//API version should be the same between the same binary release and, in any case, do not necessarily
					//change processor configurations. We simply check if the save file has been written from a plugin
					//capable build, as the save format itself is different.
					pluginAPI = true;
				}
            }
            break;
        }
    }
    if (!sameVersion)
    {
        String responseString = "Your configuration file was saved from a different version of the GUI than the one you're using. \n";
        responseString += "The current software is version ";
        responseString += JUCEApplication::getInstance()->getApplicationVersion();
        responseString += ", but the file you selected ";
        if (versionString.length() > 0)
        {
            responseString += " is version ";
            responseString += versionString;
        }
        else
        {
            responseString += "does not have a version number";
        }

        responseString += ".\n This file may not load properly. Continue?";

        bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
                                                     "Version mismatch", responseString,
                                                     "Yes", "No", 0, 0);
        if (!response)
        {
            delete xml;
            return "Failed To Open " + fileToLoad.getFileName();
        }
            

    }
	if (!pluginAPI)
	{
		String responseString = "Your configuration file was saved from a non-plugin version of the GUI.\n";
		responseString += "Save files from non-plugin versions are incompatible with the current load system.\n";
		responseString += "The chain file will not load.";
		AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Non-plugin save file", responseString);
		return "Failed To Open " + fileToLoad.getFileName();
	}
    clearSignalChain();
    
    loadingConfig = true; //Indicate config is being loaded into the GUI
    String description;// = " ";
    int loadOrder = 0;

    GenericProcessor* p;

    forEachXmlChildElement(*xml, element)
    {

        if (element->hasTagName("SIGNALCHAIN"))
        {

            forEachXmlChildElement(*element, processor)
            {

                if (processor->hasTagName("PROCESSOR"))
                {

                    int insertionPt = processor->getIntAttribute("insertionPoint");
                    
                    if (insertionPt == 1)
                    {
                        insertionPoint = editorArray.size();
                    }
                    else
                    {
                        insertionPoint = 0;
                    }

					ProcessorDescription description;
                    
                    description.fromProcessorList = false;
                    description.processorName = processor->getStringAttribute("pluginName");
					description.processorType = processor->getIntAttribute("pluginType");
					description.processorIndex = processor->getIntAttribute("pluginIndex");
					description.libName = processor->getStringAttribute("libraryName");
					description.libVersion = processor->getIntAttribute("libraryVersion");
					description.isSource = processor->getBoolAttribute("isSource");
					description.isSink = processor->getBoolAttribute("isSink");
                    description.nodeId = processor->getIntAttribute("NodeId");

					if (rhythmNodePatch) //old version, when rhythm was a plugin
					{
						if (description.processorType == -1) //if builtin
						{
							if (description.processorIndex == 0) //Rhythm node
							{
								description.processorType = 4; //DataThread
								description.processorIndex = 1;
								description.libName = "Rhythm FPGA";
							}
							else
								description.processorIndex = description.processorIndex - 1; //arrange old nodes to its current index
						}
					}
                    
                    

                    p = addProcessor(description, insertionPoint);
                    p->loadOrder = loadOrder++;
                    p->parametersAsXml = processor;

                    if (p->isSplitter()) //|| p->isMerger())
                    {
                        splitPoints.add(p);
                    }
                    
                    //if (p->isMerger())
                    //{
                   //     MergerEditor* editor = (MergerEditor*) p->getEditor();
                    //    editor->switchSource(1);
                    //    AccessClass::getProcessorGraph()->updateViews(p);
                   // }
                }
                else if (processor->hasTagName("SWITCH"))
                {
                    int processorNum = processor->getIntAttribute("number");

                    LOGD("SWITCHING number ", processorNum);

                    for (int n = 0; n < splitPoints.size(); n++)
                    {

                        LOGD("Trying split point ", ", load order: ", splitPoints[n]->loadOrder);

                        if (splitPoints[n]->loadOrder == processorNum)
                        {

                            //if (splitPoints[n]->isMerger())
                            //{
                            //    LOGD("Switching merger source.");
                             //   MergerEditor* editor = (MergerEditor*) splitPoints[n]->getEditor();
                            //    editor->switchSource(1);
                            //    AccessClass::getProcessorGraph()->updateViews(splitPoints[n]);
                            //}
                            //else
                            //{
                                LOGD("Switching splitter destination.");
                                SplitterEditor* editor = (SplitterEditor*) splitPoints[n]->getEditor();
                                editor->switchDest(1);
                                AccessClass::getProcessorGraph()->updateViews(splitPoints[n]);
                           // }

                            splitPoints.remove(n);
                        }
                    }
                }
            }
        }
        else if (element->hasTagName("AUDIO"))
        {
            AccessClass::getAudioComponent()->loadStateFromXml(element);
        }
		else if (element->hasTagName("RECORDING"))
		{
			bool recordThreadStatus = element->getBoolAttribute("isRecordThreadEnabled");

            /*
			if (recordThreadStatus)
				AccessClass::getProcessorGraph()->getRecordNode()->setParameter(3, 1.0f);
			else
				AccessClass::getProcessorGraph()->getRecordNode()->setParameter(3, 0.0f);
            */
		}
		else if (element->hasTagName("GLOBAL_TIMESTAMP"))
		{
			int tsID = element->getIntAttribute("selected_index", -1);
			int tsSubID = element->getIntAttribute("selected_sub_index");
			AccessClass::getProcessorGraph()->setTimestampSource(tsID, tsSubID);
		}

    }

    AccessClass::getProcessorGraph()->restoreParameters();

    AccessClass::getControlPanel()->loadStateFromXml(xml); // load the control panel settings
    AccessClass::getProcessorList()->loadStateFromXml(xml); // load the processor list settings
    AccessClass::getUIComponent()->loadStateFromXml(xml);  // load the UI settings

    String error = "Opened ";
    error += currentFile.getFileName();

    delete xml;

    loadingConfig = false;
    
    return error;
}
