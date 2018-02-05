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

#include "SignalChainManager.h"
#include "GraphViewer.h"
#include "EditorViewportButtons.h"
#include "../AccessClass.h"
#include "../Processors/MessageCenter/MessageCenterEditor.h"
#include "ProcessorList.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"

EditorViewport::EditorViewport()
    : leftmostEditor(0),
      message("Drag-and-drop some rows from the top-left box onto this component!"),
      somethingIsBeingDraggedOver(false), shiftDown(false), canEdit(true),
      lastEditorClicked(0), selectionIndex(0), borderSize(6), tabSize(30),
      tabButtonSize(15), insertionPoint(0), componentWantsToMove(false),
      indexOfMovingComponent(-1), currentTab(-1)
{

    addMouseListener(this, true);

    //MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    //Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font("Small Text", 10, Font::plain);
    font.setHeight(10);

    sourceDropImage = ImageCache::getFromMemory(BinaryData::SourceDrop_png,
                                                BinaryData::SourceDrop_pngSize);

    sourceDropImage = sourceDropImage.rescaled(25, 135,
                                               Graphics::highResamplingQuality);

    signalChainManager = new SignalChainManager(this, editorArray,
                                                signalChainArray);

    upButton = new SignalChainScrollButton(UP);
    downButton = new SignalChainScrollButton(DOWN);
    leftButton = new EditorScrollButton(LEFT);
    rightButton = new EditorScrollButton(RIGHT);

    upButton->addListener(this);
    downButton->addListener(this);
    leftButton->addListener(this);
    rightButton->addListener(this);

    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);
    addAndMakeVisible(rightButton);
    addAndMakeVisible(leftButton);

    currentId = 100;
    maxId = 100;

    editorNamingLabel.setEditable(true);
    editorNamingLabel.setBounds(0,0,100,20);
    editorNamingLabel.setColour(Label::textColourId, Colours::white);
    editorNamingLabel.addListener(this);

}

EditorViewport::~EditorViewport()
{
	signalChainManager = nullptr;
    deleteAllChildren();
}

void EditorViewport::signalChainCanBeEdited(bool t)
{
    canEdit = t;

    if (!canEdit)
        std::cout << "Filter Viewport disabled." << std::endl;
    else
        std::cout << "Filter Viewport enabled." << std::endl;

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

    g.drawRect(0, 0, getWidth(), getHeight(), 2.0);
    g.drawVerticalLine(tabSize, 0, getHeight());
    g.drawVerticalLine(getWidth()-tabSize, 0, getHeight());
    // g.drawHorizontalLine(getHeight()/2, getWidth()-tabSize, tabSize);

    for (int n = 0; n < 4; n++)
    {
        g.drawEllipse(7,(tabSize-2)*n+24,tabSize-12,tabSize-12,1.0);
    }

    if (somethingIsBeingDraggedOver)
    {
        float insertionX = (float)(borderSize) * 2.5 + (float) tabSize;

        int n;
        for (n = leftmostEditor; n < insertionPoint; n++)
        {
            insertionX += editorArray[n]->getWidth();

        }

        if (n - leftmostEditor > 1)
            insertionX += borderSize*(n-leftmostEditor-1);

        g.setColour(Colours::yellow);
        g.drawLine(insertionX, (float) borderSize,
                   insertionX, (float) getHeight()-(float) borderSize, 3.0f);

    }

    int insertionX = tabSize + borderSize;
    g.setColour(Colours::darkgrey);

    int x = insertionX + 15;
    int y = borderSize + 2;
    //int w = 30;
    //int h = getHeight() - 2*(borderSize+2);get

    //if (editorArray.size() > 0)
    //{
    //if (!editorArray[0]->getProcessor()->isSource())
    //    g.drawImageAt(sourceDropImage, x, y);
    //} else {
    g.drawImageAt(sourceDropImage, x, y);
    //}
}

bool EditorViewport::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    if (canEdit && dragSourceDetails.description.toString().startsWith("Processors"))
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
    if (canEdit)
    {
        somethingIsBeingDraggedOver = true;
        repaint();
    }
}

void EditorViewport::itemDragMove(const SourceDetails& dragSourceDetails)
{

    int x = dragSourceDetails.localPosition.getX();
    // int y = dragSourceDetails.localPosition.getY();

    if (canEdit)
    {
        bool foundInsertionPoint = false;

        int lastCenterPoint = -1;
        int leftEdge;
        int centerPoint;

        for (int n = leftmostEditor; n < editorArray.size(); n++)
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

	var descr = dragSourceDetails.description;
	Array<var>* description = descr.getArray();

    if (canEdit)
    {

        message = "last filter dropped: " + (*description)[1].toString();

        std::cout << "Item dropped at insertion point " << insertionPoint << std::endl;

        /// needed to remove const cast --> should be a better way to do this
        //String description = sourceDescription.substring(0);

        GenericEditor* activeEditor = (GenericEditor*)AccessClass::getProcessorGraph()->createNewProcessor(*description, currentId);//, source, dest);

        //std::cout << "Active editor: " << activeEditor << std::endl;

        if (activeEditor != 0)
        {
            //activeEditor->setUIComponent(getUIComponent());
            activeEditor->refreshColors();
            addChildComponent(activeEditor);

            lastEditor = activeEditor;

            signalChainManager->updateVisibleEditors(activeEditor, indexOfMovingComponent, insertionPoint, ADD);

            for (int i = 0; i < editorArray.size(); i++)
            {
                if (editorArray[i] == activeEditor)
                    editorArray[i]->select();
                else
                    editorArray[i]->deselect();
            }

            // Instructions below were enclosed into the if block by Michael Borisov
            // To allow for errors during creation of editors, in which case activeEditor will be ==0

            insertionPoint = -1; // make sure all editors are left-justified
            indexOfMovingComponent = -1;
            refreshEditors();

            somethingIsBeingDraggedOver = false;

            AccessClass::getGraphViewer()->addNode(activeEditor);

            repaint();

            currentId++;
        }

    }
}

void EditorViewport::clearSignalChain()
{

    if (canEdit)
    {
        editorArray.clear();
        //const MessageManagerLock mmLock; // prevent redraw while deleting
        std::cout << "Clearing signal chain." << std::endl;
        signalChainManager->clearSignalChain();
        AccessClass::getProcessorGraph()->clearSignalChain();
        AccessClass::getGraphViewer()->removeAllNodes();

    }
    else
    {

        CoreServices::sendStatusMessage("Cannot clear signal chain while acquisition is active.");

    }

    repaint();
}

void EditorViewport::makeEditorVisible(GenericEditor* editor, bool highlight, bool updateSettings)
{

	if (editor == 0)
	{
		if (updateSettings)
			signalChainManager->updateProcessorSettings();
		return;
	}

    if (!updateSettings)
        signalChainManager->updateVisibleEditors(editor, 0, 0, ACTIVATE);
    else
        signalChainManager->updateVisibleEditors(editor, 0, 0, UPDATE);

    refreshEditors();

    for (int i = 0; i < editorArray.size(); i++)
    {
        editorArray[i]->deselect();
    }

    if (highlight)
        editor->highlight();

    while (!editor->isVisible())
    {
        if (leftmostEditor < editorArray.indexOf(editor))
            leftmostEditor++;
        else
            leftmostEditor--;

        refreshEditors();
    }

    repaint();

}

void EditorViewport::deleteNode(GenericEditor* editor)
{

    if (canEdit)
    {
        indexOfMovingComponent = editorArray.indexOf(editor);
        editor->setVisible(false);

        signalChainManager->updateVisibleEditors(editor, indexOfMovingComponent, insertionPoint, REMOVE);

        AccessClass::getGraphViewer()->removeNode(editor);

        refreshEditors();

        AccessClass::getProcessorGraph()->removeProcessor((GenericProcessor*)editor->getProcessor());

        insertionPoint = -1; // make sure all editors are left-justified
        indexOfMovingComponent = -1;

        somethingIsBeingDraggedOver = false;

        repaint();

    }
}


void EditorViewport::refreshEditors()
{

    int lastBound = borderSize+tabSize;
    int totalWidth = 0;

    //std::cout << insertionPoint << std::endl;

    bool tooLong;

    for (int n = 0; n < signalChainArray.size(); n++)
    {
        if (signalChainArray[n]->getToggleState())
        {
            signalChainArray[n]->offset = leftmostEditor;
        }
    }

    for (int n = 0; n < editorArray.size(); n++)
    {

        //   std::cout << "Refreshing editor number" << n << std::endl;

        int componentWidth = editorArray[n]->desiredWidth;

        if (lastBound + componentWidth < getWidth() - tabSize && n >= leftmostEditor)
        {

            if (n == 0)
            {
                if (!editorArray[n]->getEnabledState())
                {
                    GenericProcessor* p = (GenericProcessor*) editorArray[n]->getProcessor();
                    if (!p->isSource())
                        lastBound += borderSize*10;
                    // signalChainNeedsSource = true;
                }
                else
                {
                    //  signalChainNeedsSource = false;
                }
            }

            if (somethingIsBeingDraggedOver && n == insertionPoint)
            {
                if (indexOfMovingComponent > -1)
                {
                    if (n != indexOfMovingComponent && n != indexOfMovingComponent+1)
                    {
                        if (n == 0)
                            lastBound += borderSize*3;
                        else
                            lastBound += borderSize*2;
                    }
                }
                else
                {
                    if (n == 0)
                        lastBound += borderSize*3;
                    else
                        lastBound += borderSize*2;
                }

            }

            editorArray[n]->setVisible(true);
            //   std::cout << "setting visible." << std::endl;
            editorArray[n]->setBounds(lastBound, borderSize, componentWidth, getHeight()-borderSize*2);
            lastBound += (componentWidth + borderSize);

            tooLong = false;

            totalWidth = lastBound;

        }
        else
        {
            editorArray[n]->setVisible(false);

            totalWidth += componentWidth + borderSize;

            // std::cout << "setting invisible." << std::endl;

            if (lastBound + componentWidth > getWidth()-tabSize)
                tooLong = true;

        }
    }

    // BUG: variable is used without being initialized
    if (tooLong && editorArray.size() > 0)
        rightButton->setActive(true);
    else
        rightButton->setActive(false);

    if (leftmostEditor == 0 || editorArray.size() == 0)
        leftButton->setActive(false);
    else
        leftButton->setActive(true);

    // std::cout << totalWidth << " " << getWidth() - tabSize << std::endl;

    // if (totalWidth < getWidth()-tabSize && leftButton->isActive)
    // {
    //     leftmostEditor -= 1;
    //     refreshEditors();
    // }

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

                    //  if (!stopSelection)
                    // {
                    lastEditorClicked = editorArray[i+1];
                    editorArray[i+1]->select();
                    // stopSelection = true;
                    //  }

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

        // std::cout << "Selection index: " << selectionIndex << std::endl;

        // int startIndex = editorArray.indexOf(lastEditorClicked);

        // if (selectionIndex < 0)
        // {

        //     for (int i = startIndex-1; i >= startIndex + selectionIndex; i--)
        //     {
        //         editorArray[i]->select();
        //     }

        // } else if (selectionIndex > 0)
        // {
        //     for (int i = startIndex+1; i <= startIndex + selectionIndex; i++)
        //     {
        //         editorArray[i]->select();
        //     }

        // }

    }

    // } else if (key.getKeyCode() == key.upKey) {

    //     // move one tab up
    // } else if (key.getKeyCode() == key.downKey) {

    //     // move one tab down
    // }
}

bool EditorViewport::keyPressed(const KeyPress& key)
{

    //std::cout << "Editor viewport received " << key.getKeyCode() << std::endl;

    if (canEdit && editorArray.size() > 0)
    {

        ModifierKeys mk = key.getModifiers();

        if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey)
        {

            if (!mk.isAnyModifierKeyDown())
            {

                Array<GenericEditor*> editorsToRemove;

                for (int i = 0; i < editorArray.size(); i++)
                {
                    if (editorArray[i]->getSelectionState())
                        editorsToRemove.add(editorArray[i]);
                }

                for (int i = 0; i < editorsToRemove.size(); i++)
                    deleteNode(editorsToRemove[i]);

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

            return true;
        }
        else if (key.getKeyCode() == key.downKey)
        {
            lastEditorClicked->switchIO(1);
            return true;
        }
    }

    return false;

}

//void EditorViewport::modifierKeysChanged (const ModifierKeys & modifiers) {

/*     if (modifiers.isShiftDown()) {

        std::cout << "Shift key pressed." << std::endl;
        shiftDown  = true;

    } else {


        std::cout << "Shift key released." << std::endl;
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


    // std::cout << "Mouse click at " << e.x << " " << e.y << std::endl;

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

                if (canEdit)
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
                    deleteNode(editorArray[i]);
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
        && canEdit
        && editorArray.size() > 1)
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

        GenericEditor* editor = editorArray[indexOfMovingComponent];

        signalChainManager->updateVisibleEditors(editor, indexOfMovingComponent,
                                                 insertionPoint, MOVE);
        refreshEditors();
        repaint();

    }


}

void EditorViewport::mouseExit(const MouseEvent& e)
{

    if (componentWantsToMove)
    {

        somethingIsBeingDraggedOver = false;
        componentWantsToMove = false;

        repaint();
        refreshEditors();

    }


}

void EditorViewport::checkScrollButtons(int topTab)
{

    if (signalChainArray.size() - topTab > 4)
    {
        downButton->setActive(true);
    }
    else
    {
        downButton->setActive(false);
    }

    if (topTab > 0)
    {
        upButton->setActive(true);
    }
    else
    {
        upButton->setActive(false);
    }

}

bool EditorViewport::isSignalChainEmpty()
{

    if (editorArray.size() == 0)
        return true;
    else
        return false;

}

void EditorViewport::resized()
{

    int b = 2; // border

    downButton->setBounds(b, getHeight()-15-b, tabSize-b, 15);
    upButton->setBounds(b, b, tabSize-b, 15);
    leftButton->setBounds(getWidth()-25, getHeight()/2+b, 20, getHeight()/2-2*b);
    rightButton->setBounds(getWidth()-25, b, 20, getHeight()/2-b*2);

    refreshEditors();
}

void EditorViewport::buttonClicked(Button* button)
{
    if (button == upButton)
    {
        std::cout << "Up button pressed." << std::endl;

        if (upButton->isActive)
            signalChainManager->scrollUp();

    }
    else if (button == downButton)
    {
        if (downButton->isActive)
            signalChainManager->scrollDown();

    }
    else if (button == leftButton)
    {
        if (leftButton->isActive)
        {
            leftmostEditor -= 1;
            refreshEditors();
        }

    }
    else if (button == rightButton)
    {
        if (rightButton->isActive)
        {
            leftmostEditor += 1;
            refreshEditors();
        }
    }
}

///////////////////////////////////////////////////////////////////
////////////////SIGNAL CHAIN TAB BUTTON////////////////////////////
///////////////////////////////////////////////////////////////////

SignalChainTabButton::SignalChainTabButton() : Button("Name"),
    configurationChanged(true)
{
    setRadioGroupId(99);
    setClickingTogglesState(true);

    // MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    //Typeface::Ptr typeface = new CustomTypeface(mis);
    buttonFont = Font("Small Text", 10, Font::plain);
    buttonFont.setHeight(14);

    offset = 0;
}


void SignalChainTabButton::clicked()
{

    //std::cout << "Button clicked: " << firstEditor->getName() << std::endl;
    EditorViewport* ev = (EditorViewport*) getParentComponent();

    scm->updateVisibleEditors(firstEditor, 0, 0, ACTIVATE);
    ev->leftmostEditor = offset;
    ev->refreshEditors();


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
        //  grad1 = ColourGradient(Colour(255, 255, 255), 0.0f, 20.0f,
        //                         Colour(180, 180, 180), 0.0f, 0.0f,
        //                        false);

        // grad2 = ColourGradient(Colour(255, 255, 255), 0.0f, 0.0f,
        //                         Colour(180, 180, 180), 0.0f, 20.0f,
        //                        false);
    }

    if (isButtonDown)
    {

        // ColourGradient grad3 = grad1;
        // grad1 = grad2;
        // grad2 = grad3;
        // grad1.multiplyOpacity(0.7f);
        // grad2.multiplyOpacity(0.7f);

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



// how about some loading and saving?

XmlElement* EditorViewport::createNodeXml(GenericEditor* editor,
                                          int insertionPt)
{

    XmlElement* e = new XmlElement("PROCESSOR");

    GenericProcessor* source = (GenericProcessor*) editor->getProcessor();

    String name = "";

    if (source->isSource())
        name += "Sources/";
    else if (source->isSink())
        name += "Sinks/";
    else if (source->isSplitter() || source->isMerger() || source->isUtility())
        name += "Utilities/";
    else
        name += "Filters/";

    name += editor->getName();

    std::cout << name << std::endl;

    e->setAttribute("name", name);
    e->setAttribute("insertionPoint", insertionPt);
	e->setAttribute("pluginName", source->getPluginName());
	e->setAttribute("pluginType", (int)(source->getPluginType()));
	e->setAttribute("pluginIndex", source->getIndex());
	e->setAttribute("libraryName", source->getLibName());
	e->setAttribute("libraryVersion", source->getLibVersion());
	e->setAttribute("isSource", source->isSource());
	e->setAttribute("isSink", source->isSink());

    /**Saves individual processor parameters to XML */
    std::cout << "Create subnodes with parameters" << std::endl;
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

    // FileChooser fc("Choose the file to save...",
    //                CoreServices::getDefaultUserSaveDirectory(),
    //                "*",
    //                true);

    // if (fc.browseForFileToSave(true))
    // {
    //     currentFile = fc.getResult();
    //     std::cout << currentFile.getFileName() << std::endl;
    // }
    // else
    // {
    //     error = "No file chosen.";
    //     std::cout << "no file chosen." << std::endl;
    //     return error;
    // }

    Array<GenericProcessor*> splitPoints;
    /** Used to reset saveOrder at end, to allow saving the same processor multiple times*/
    Array<GenericProcessor*> allProcessors;

    bool moveForward;
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

    GenericEditor* editor;

    for (int n = 0; n < signalChainArray.size(); n++)
    {

        moveForward = true;

        XmlElement* signalChain = new XmlElement("SIGNALCHAIN");

        editor = signalChainArray[n]->getEditor();

        int insertionPt = 1;

        while (editor != 0)
        {

            GenericProcessor* currentProcessor = (GenericProcessor*) editor->getProcessor();
            GenericProcessor* nextProcessor;

            if (currentProcessor->saveOrder < 0)   // create a new XML element
            {

                signalChain->addChildElement(createNodeXml(editor, insertionPt));
                currentProcessor->saveOrder = saveOrder;
                allProcessors.addIfNotAlreadyThere(currentProcessor);
                saveOrder++;

            }
            else
            {
                std::cout << "   Processor already saved as number " << currentProcessor->saveOrder << std::endl;
            }

            if (moveForward)
            {
                std::cout << "  Moving forward along signal chain." << std::endl;
                nextProcessor = currentProcessor->getDestNode();
            }
            else
            {
                std::cout << "  Moving backward along signal chain." << std::endl;
                nextProcessor = currentProcessor->getSourceNode();
            }


            if (nextProcessor != 0)   // continue until the end of the chain
            {

                editor = (GenericEditor*) nextProcessor->getEditor();

                if ((nextProcessor->isSplitter())// || nextProcessor->isMerger())
                    && nextProcessor->saveOrder < 0)
                {
                    splitPoints.add(nextProcessor);

                    nextProcessor->switchIO(0);
                }

            }
            else
            {

                std::cout << "  No processor found." << std::endl;

                if (splitPoints.size() > 0)
                {

                    nextProcessor = splitPoints.getFirst();
                    splitPoints.remove(0);

                    nextProcessor->switchIO(1);
                    signalChain->addChildElement(switchNodeXml(nextProcessor));

                    if (nextProcessor->isMerger())
                    {
                        insertionPt = 0;
                        moveForward = false;
                    }
                    else
                    {
                        insertionPt = 1;
                        moveForward = true;
                    }

                    editor = nextProcessor->getEditor();

                }
                else
                {

                    std::cout << "  End of chain." << std::endl;

                    editor = 0;
                }
            }

            //insertionPt++;
        }

        xml->addChildElement(signalChain);
    }

    XmlElement* audioSettings = new XmlElement("AUDIO");

    audioSettings->setAttribute("bufferSize", AccessClass::getAudioComponent()->getBufferSize());
    xml->addChildElement(audioSettings);

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

    // FileChooser fc("Choose a file to load...",
    //                CoreServices::getDefaultUserSaveDirectory(),
    //                "*.xml",
    //                true);

    // if (fc.browseForFileToOpen())
    // {
    //     currentFile = fc.getResult();
    // }
    // else
    // {
    //     return "No configuration selected.";
    // }
    int maxID = 100;
    currentFile = fileToLoad;

    std::cout << "Loading processor graph." << std::endl;

    Array<GenericProcessor*> splitPoints;

    XmlDocument doc(currentFile);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName("SETTINGS"))
    {
        std::cout << "File not found." << std::endl;
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
            return "Failed To Open " + fileToLoad.getFileName();

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
                    currentId = processor->getIntAttribute("NodeId");

                    maxID= (maxID > currentId) ? maxID  : currentId ;

                    if (insertionPt == 1)
                    {
                        insertionPoint = editorArray.size();
                    }
                    else
                    {
                        insertionPoint = 0;
                    }

                    //See ProcessorGraph::createProcessorFromDescription for description info
					Array<var> procDesc;
					procDesc.add(false);
					procDesc.add(processor->getStringAttribute("pluginName"));
					procDesc.add(processor->getIntAttribute("pluginType"));
					procDesc.add(processor->getIntAttribute("pluginIndex"));
					procDesc.add(processor->getStringAttribute("libraryName"));
					procDesc.add(processor->getIntAttribute("libraryVersion"));
					procDesc.add(processor->getBoolAttribute("isSource"));
					procDesc.add(processor->getBoolAttribute("isSink"));

					if (rhythmNodePatch) //old version, when rhythm was a plugin
					{
						if (int(procDesc[2]) == -1) //if builtin
						{
							if (int(procDesc[3]) == 0) //Rhythm node
							{
								procDesc.set(2, 4); //DataThread
								procDesc.set(3, 1); //index
								procDesc.set(4, "Rhytm FPGA"); //libraryName
							}
							else
								procDesc.set(3, int(procDesc[3]) - 1); //arrange old nodes to its current index
						}
					}

                    SourceDetails sd = SourceDetails(procDesc,
                                                     0,
                                                     Point<int>(0,0));

                    itemDropped(sd);

                    p = (GenericProcessor*) lastEditor->getProcessor();
                    p->loadOrder = loadOrder;
                    p->parametersAsXml = processor;

                    //Sets parameters based on XML files
                    setParametersByXML(p, processor);
                    loadOrder++;

                    if (p->isSplitter() || p->isMerger())
                    {
                        splitPoints.add(p);
                    }

                    signalChainManager->updateVisibleEditors(editorArray[0], 0, 0, UPDATE);

                }
                else if (processor->hasTagName("SWITCH"))
                {
                    int processorNum = processor->getIntAttribute("number");

                    std::cout << "SWITCHING number " << processorNum << std::endl;

                    for (int n = 0; n < splitPoints.size(); n++)
                    {

                        std::cout << "Trying split point " << n
                                  << ", load order: " << splitPoints[n]->loadOrder << std::endl;

                        if (splitPoints[n]->loadOrder == processorNum)
                        {

                            if (splitPoints[n]->isMerger())
                            {
                                std::cout << "Switching merger source." << std::endl;
                                MergerEditor* editor = (MergerEditor*) splitPoints[n]->getEditor();
                                editor->switchSource(1);
                            }
                            else
                            {
                                std::cout << "Switching splitter destination." << std::endl;
                                SplitterEditor* editor = (SplitterEditor*) splitPoints[n]->getEditor();
                                editor->switchDest(1);
                            }

                            splitPoints.remove(n);
                        }
                    }

                    signalChainManager->updateVisibleEditors(editorArray[0], 0, 0, UPDATE);

                }

            }

        }
        else if (element->hasTagName("AUDIO"))
        {
            int bufferSize = element->getIntAttribute("bufferSize");
            AccessClass::getAudioComponent()->setBufferSize(bufferSize);
        }
		else if (element->hasTagName("GLOBAL_TIMESTAMP"))
		{
			int tsID = element->getIntAttribute("selected_index", -1);
			int tsSubID = element->getIntAttribute("selected_sub_index");
			AccessClass::getProcessorGraph()->setTimestampSource(tsID, tsSubID);
		}

    }

    for (int i = 0; i < editorArray.size(); i++)
    {
        // deselect everything initially
        editorArray[i]->deselect();
    }

    AccessClass::getProcessorGraph()->restoreParameters();

    AccessClass::getControlPanel()->loadStateFromXml(xml); // save the control panel settings
    AccessClass::getProcessorList()->loadStateFromXml(xml);
    AccessClass::getUIComponent()->loadStateFromXml(xml);  // save the UI settings

    if (editorArray.size() > 0)
        signalChainManager->updateVisibleEditors(editorArray[0], 0, 0, UPDATE);

    refreshEditors();

    AccessClass::getProcessorGraph()->restoreParameters();


    String error = "Opened ";
    error += currentFile.getFileName();

    delete xml;

    currentId=maxID+1; // make sure future processors don't have overlapping id numbers

    return error;
}
/* Set parameters based on XML.*/
void EditorViewport::setParametersByXML(GenericProcessor* targetProcessor, XmlElement* processorXML)
{
    // Should probably do some error checking to make sure XML is valid, depending on how it treats errors (will likely just not update parameters, but error message could be nice.)
    int numberParameters = targetProcessor->getNumParameters();
    // Ditto channels. Not sure how to handle different channel sizes when variable sources (file reader etc. change). Maybe I should check number of channels vs source, but that requires hardcoding when source matters.
    //int numChannels=(targetProcessor->channels).size();
    //int numEventChannels=(targetProcessor->eventChannels).size();

    // Sets channel in for loop
    int currentChannel;

    // What the parameter name to change is.
    String parameterNameForXML;
    String parameterValue;
    float parameterFloat;
    //float testGrab;


    forEachXmlChildElementWithTagName(*processorXML, channelXML, "CHANNEL")
    {
        currentChannel=channelXML->getIntAttribute("name");

        // std::cout <<"currentChannel:"<< currentChannel  << std::endl;
        // Sets channel to change parameter on
        targetProcessor->setCurrentChannel(currentChannel-1);

        forEachXmlChildElement(*channelXML, parameterXML)
        {

            for (int j = 0; j < numberParameters; ++j)
            {
                parameterNameForXML = targetProcessor->getParameterName(j);

                if (parameterXML->getStringAttribute("name")==parameterNameForXML)
                {
                    parameterValue=parameterXML->getAllSubText();
                    parameterFloat=parameterValue.getFloatValue();
                    targetProcessor->setParameter(j, parameterFloat);
                    // testGrab=targetProcessor->getParameterVar(j, currentChannel);
                    std::cout <<"Channel:" <<currentChannel<<"Parameter:" << parameterNameForXML << "Intended Value:" << parameterFloat << std::endl;
                }

            }


        }
    }
}
