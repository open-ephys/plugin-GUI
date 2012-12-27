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

#include "EditorViewport.h"

#include "SignalChainManager.h"
#include "EditorViewportButtons.h"

EditorViewport::EditorViewport()
    : message ("Drag-and-drop some rows from the top-left box onto this component!"),
      somethingIsBeingDraggedOver(false), shiftDown(false), selectionIndex(0), leftmostEditor(0), lastEditorClicked(0),
       insertionPoint(0), componentWantsToMove(false), indexOfMovingComponent(-1), 
       borderSize(6), tabSize(30), tabButtonSize(15), canEdit(true), currentTab(-1)
{

    addMouseListener(this, true);

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);
    font.setHeight(10);

    sourceDropImage = ImageCache::getFromMemory (BinaryData::SourceDrop_png, 
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

}

EditorViewport::~EditorViewport()
{
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

void EditorViewport::paint (Graphics& g)
{

    if (somethingIsBeingDraggedOver)
    {
         g.setColour (Colours::yellow);

    } else {
        g.setColour (Colour(48,48,48));
    }

    g.drawRect (0, 0, getWidth(), getHeight(), 2.0);
    g.drawVerticalLine(tabSize, 0, getHeight());
    g.drawVerticalLine(getWidth()-tabSize, 0, getHeight());
   // g.drawHorizontalLine(getHeight()/2, getWidth()-tabSize, tabSize);

    for (int n = 0; n < 4; n++)
    {
        g.drawEllipse(7,(tabSize-2)*n+24,tabSize-12,tabSize-12,1.0);
    }

    if (somethingIsBeingDraggedOver)
    {
        float insertionX = (float) (borderSize) * 2.5 + (float) tabSize;

        int n;
        for (n = 0; n < insertionPoint; n++)
        {
            insertionX += editorArray[n]->getWidth();
            
        }

        if (n > 1)
            insertionX += borderSize*(n-1);

        g.setColour(Colours::yellow);
        g.drawLine(insertionX, (float) borderSize,
                   insertionX, (float) getHeight()-(float) borderSize, 3.0f);

    }

    int insertionX = tabSize + borderSize;
    g.setColour(Colours::darkgrey);

    int x = insertionX + 19;
    int y = borderSize + 2;
    int w = 30;
    int h = getHeight() - 2*(borderSize+2);

    g.drawImageAt(sourceDropImage, x, y);

}

bool EditorViewport::isInterestedInDragSource (const String& description, Component* component)
{

    if (canEdit && description.startsWith("Processors")) {
        return false;
    } else {
        return true;
    }

}

void EditorViewport::itemDragEnter (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int /*x*/, int /*y*/)
{
    if (canEdit) {
        somethingIsBeingDraggedOver = true;
        repaint();
    }   
}

void EditorViewport::itemDragMove (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int x, int /*y*/)
{

    if (canEdit) {
        bool foundInsertionPoint = false;

        int lastCenterPoint = -1;
        int leftEdge;
        int centerPoint;

        for (int n = 0; n < editorArray.size(); n++)
        {
            leftEdge = editorArray[n]->getX();
            centerPoint = leftEdge + (editorArray[n]->getWidth())/2;
            
            if (x < centerPoint && x > lastCenterPoint) {
                insertionPoint = n;
                foundInsertionPoint = true;
            }

            lastCenterPoint = centerPoint;
        }

        if (!foundInsertionPoint) {
            insertionPoint = editorArray.size();
        }

        repaint();
        refreshEditors();
    }

}

void EditorViewport::itemDragExit (const String& /*sourceDescription*/, Component* /*sourceComponent*/)
{
    somethingIsBeingDraggedOver = false;

    repaint();

    refreshEditors();

}

void EditorViewport::itemDropped (const String& sourceDescription, Component* /*sourceComponent*/, int /*x*/, int /*y*/)
{

    if (canEdit) {

        message = "last filter dropped: " + sourceDescription;

        std::cout << "Item dropped at insertion point " << insertionPoint << std::endl;

        /// needed to remove const cast --> should be a better way to do this
        String description = sourceDescription.substring(0);

        GenericEditor* activeEditor = (GenericEditor*) getProcessorGraph()->createNewProcessor(description);//, source, dest);

        std::cout << "Active editor: " << activeEditor << std::endl;

        if (activeEditor != 0)
        {
            activeEditor->setUIComponent(getUIComponent());
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

        } 
        
        insertionPoint = -1; // make sure all editors are left-justified
        indexOfMovingComponent = -1;
        refreshEditors();

        somethingIsBeingDraggedOver = false;

        repaint();
    }
}

void EditorViewport::clearSignalChain()
{
    if (canEdit)
    {
        std::cout << "Clearing signal chain." << std::endl;
        signalChainManager->clearSignalChain();
        getProcessorGraph()->clearSignalChain();

    } else {

        sendActionMessage("Cannot clear signal chain while acquisition is active.");
    
    }

    repaint();
}

void EditorViewport::makeEditorVisible(GenericEditor* editor, bool highlight, bool updateSettings)
{
    
    if (editor == 0)
        return;

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

}

void EditorViewport::deleteNode (GenericEditor* editor) 
{

    if (canEdit) {
        indexOfMovingComponent = editorArray.indexOf(editor);
        editor->setVisible(false);
   
        signalChainManager->updateVisibleEditors(editor, indexOfMovingComponent, insertionPoint, REMOVE);
    
        refreshEditors();

        getProcessorGraph()->removeProcessor((GenericProcessor*) editor->getProcessor());
    }
}


void EditorViewport::refreshEditors () {
    
    int lastBound = borderSize+tabSize;
    int totalWidth = 0;

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

        if (lastBound + componentWidth < getWidth()-tabSize && n >= leftmostEditor) {

            if (n == 0)
            {
                 if (!editorArray[n]->getEnabledState()) 
                 {
                    GenericProcessor* p = (GenericProcessor*) editorArray[n]->getProcessor();
                    if (!p->isSource())
                        lastBound += borderSize*10;
                   // signalChainNeedsSource = true;
                } else {
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
                } else {
                    if (n == 0)
                        lastBound += borderSize*3;
                    else
                        lastBound += borderSize*2;
                }

            }

            editorArray[n]->setVisible(true);
          //   std::cout << "setting visible." << std::endl;
            editorArray[n]->setBounds(lastBound, borderSize, componentWidth, getHeight()-borderSize*2);
            lastBound+=(componentWidth + borderSize);

            tooLong = false;

            totalWidth = lastBound;

        } else {
            editorArray[n]->setVisible(false);

            totalWidth += componentWidth + borderSize;

            // std::cout << "setting invisible." << std::endl;

           if (lastBound + componentWidth > getWidth()-tabSize)
                tooLong = true;

        }
    }

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

void EditorViewport::moveSelection (const KeyPress &key) {
    
    ModifierKeys mk = key.getModifiers();

    if (key.getKeyCode() == key.leftKey) {

        if (mk.isShiftDown())
        {
            selectionIndex--;
        } else {
            
            selectionIndex = 0;

            for (int i = 0; i < editorArray.size(); i++) {
        
                if (editorArray[i]->getSelectionState() && i > 0) {
                    editorArray[i-1]->select();
                    lastEditorClicked = editorArray[i-1];
                    editorArray[i]->deselect();
                }               
            }

        }
        
    } else if (key.getKeyCode() == key.rightKey) {
         
        if (mk.isShiftDown())
        {
            selectionIndex++;
        } else {
            
            selectionIndex = 0;

            bool stopSelection = false;
            int i = 0;

            while (i < editorArray.size()-1)
            {

                if (editorArray[i]->getSelectionState()) {

                  //  if (!stopSelection)
                    // {
                    lastEditorClicked = editorArray[i+1];
                    editorArray[i+1]->select();
                    stopSelection = true;
                  //  }

                    editorArray[i]->deselect();
                    i += 2;
                } else {
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

bool EditorViewport::keyPressed (const KeyPress &key) {
    
   //std::cout << "Editor viewport received " << key.getKeyCode() << std::endl;

  if (canEdit && editorArray.size() > 0) 
   {

        ModifierKeys mk = key.getModifiers();

        if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey) {

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

        } else if (key.getKeyCode() == key.leftKey || 
                   key.getKeyCode() == key.rightKey) {

            moveSelection(key);

            return true;

        } else if (key.getKeyCode() == key.upKey)
        {

            lastEditorClicked->switchIO(0);
            
            return true;
        } else if (key.getKeyCode() == key.downKey)
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
    for (int i = 0; i < editorArray.size(); i++) {
        
        if (editor == editorArray[i]
             || editor->getParentComponent() == editorArray[i]) {
            editorArray[i]->select();
        } else {
            editorArray[i]->deselect();
        }
    } 
}

void EditorViewport::mouseDown(const MouseEvent &e) {
    

   // std::cout << "Mouse click at " << e.x << " " << e.y << std::endl;

    bool clickInEditor = false;

    for (int i = 0; i < editorArray.size(); i++) {
        
        if (e.eventComponent == editorArray[i] && e.y < 22)
            // event must take place along title bar
             // || e.eventComponent->getParentComponent() == editorArray[i] ||
             //    e.eventComponent->getParentComponent()->getParentComponent() ==
             //            editorArray[i]) 
        {

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

                    } else {
                        for (int j = i-1; j >= index; j-- )
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

        } else {

            if (!e.mods.isCtrlDown() && !e.mods.isShiftDown())
                editorArray[i]->deselect();

        }
    } 

    if (!clickInEditor)
        lastEditorClicked = 0;

}

void EditorViewport::mouseDrag(const MouseEvent &e) {
    

    if (editorArray.contains((GenericEditor*) e.originalComponent) 
        && e.y < 15 
        && canEdit
        && editorArray.size() > 1) {

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

        if (!foundInsertionPoint && indexOfMovingComponent != editorArray.size()-1) {
            insertionPoint = editorArray.size();
        }

        refreshEditors();
        repaint();
    }

}

void EditorViewport::mouseUp(const MouseEvent &e) {


    if (componentWantsToMove) {
        
        somethingIsBeingDraggedOver = false;
        componentWantsToMove = false;

        GenericEditor* editor = editorArray[indexOfMovingComponent];

        signalChainManager->updateVisibleEditors(editor, indexOfMovingComponent,
                                                 insertionPoint, MOVE);
        refreshEditors();
        repaint();

    }


}

void EditorViewport::mouseExit(const MouseEvent &e) {

    if (componentWantsToMove) {
        
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
    } else {
        downButton->setActive(false);
    }

    if (topTab > 0)
    {
        upButton->setActive(true);
    } else {
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

void EditorViewport::resized() {

    int b = 2; // border

    downButton->setBounds(b, getHeight()-15-b, tabSize-b, 15);
    upButton->setBounds(b, b, tabSize-b, 15);
    leftButton->setBounds(getWidth()-25, getHeight()/2+b, 20, getHeight()/2-2*b);
    rightButton->setBounds(getWidth()-25, b, 20, getHeight()/2-b*2);

    refreshEditors();
}

void EditorViewport::buttonClicked (Button* button)
{
    if (button == upButton)
    {
        std::cout << "Up button pressed." << std::endl;

        if (upButton->isActive)
            signalChainManager->scrollUp();

    } else if (button == downButton)
    {
        if (downButton->isActive)
            signalChainManager->scrollDown();

    } else if (button == leftButton)
    {
        if (leftButton->isActive)
        {
            leftmostEditor -= 1;
            refreshEditors();
        }
        
    } else if (button == rightButton)
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

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    buttonFont = Font(typeface);
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

void SignalChainTabButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{

    ColourGradient grad1, grad2;

    if (getToggleState() == true) {

        grad1 = ColourGradient(Colour(255, 136, 34), 0.0f, 0.0f, 
                               Colour(230, 193, 32), 0.0f, 20.0f,
                               false);

        grad2 = ColourGradient(Colour(255, 136, 34), 0.0f, 20.0f, 
                               Colour(230, 193, 32), 0.0f, 0.0f,
                               false);
    }
    else { 
         grad2 = ColourGradient(Colour(80, 80, 80), 0.0f, 20.0f, 
                               Colour(120, 120, 120), 0.0f, 0.0f,
                               false);

        grad1 =  ColourGradient(Colour(80, 80, 80), 0.0f, 0.0f, 
                               Colour(120, 120, 120), 0.0f, 20.0f,
                               false);
    }

    if (isMouseOver) {
        
        grad1.multiplyOpacity(0.7f);
        grad2.multiplyOpacity(0.7f);
        //  grad1 = ColourGradient(Colour(255, 255, 255), 0.0f, 20.0f, 
        //                         Colour(180, 180, 180), 0.0f, 0.0f,
        //                        false);

        // grad2 = ColourGradient(Colour(255, 255, 255), 0.0f, 0.0f, 
        //                         Colour(180, 180, 180), 0.0f, 20.0f,
        //                        false);
    }

    if (isButtonDown) {

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

XmlElement* EditorViewport::createNodeXml (GenericEditor* editor,
                                           int insertionPt)
{

    XmlElement* e = new XmlElement("PROCESSOR");

    GenericProcessor* source = (GenericProcessor*) editor->getProcessor();
    
    String name = "";

    if (source->isSource())
        name += "Sources/";
    else if (source->isSink())
        name += "Sinks/";
    else if (source->isSplitter() || source->isMerger())
        name += "Utilities/";
    else
        name += "Filters/";

    name += editor->getName();

    std::cout << name << std::endl;

    e->setAttribute ("name", name);
    e->setAttribute ("insertionPoint", insertionPt);

   // source->stateSaved = true;
  
    //GenericProcessor* dest = (GenericProcessor*) source->getDestNode();

    return e;

}


XmlElement* EditorViewport::switchNodeXml (GenericProcessor* processor)
{

    XmlElement* e = new XmlElement("SWITCH");

    e->setAttribute ("number", processor->saveOrder);

    return e;

}

const String EditorViewport::saveState() 
{

     String error;

    FileChooser fc ("Choose the file to save...",
                        File::getCurrentWorkingDirectory(),
                        "*",
                        true);

    if (fc.browseForFileToSave(true))
    {
        currentFile = fc.getResult();
        std::cout << currentFile.getFileName() << std::endl;
    } else {
        error = "No file chosen.";
        std::cout << "no file chosen." << std::endl;
        return error;
    }

    Array<GenericProcessor*> splitPoints;

    bool moveForward;
    int saveOrder = 0;

    XmlElement* xml = new XmlElement("PROCESSORGRAPH");

    for (int n = 0; n < signalChainArray.size(); n++)
    {

        moveForward = true;
        
        XmlElement* signalChain = new XmlElement("SIGNALCHAIN");

        GenericEditor* editor = signalChainArray[n]->getEditor();

        int insertionPt = 1;
        
        while (editor != 0)
        {

            GenericProcessor* currentProcessor = (GenericProcessor*) editor->getProcessor();
            GenericProcessor* nextProcessor;

            if (currentProcessor->saveOrder < 0) { // create a new XML element

                signalChain->addChildElement(createNodeXml(editor, insertionPt));
                currentProcessor->saveOrder = saveOrder;
                saveOrder++;

            } else {
                std::cout << "   Processor already saved as number " << currentProcessor->saveOrder << std::endl;
            }
            
            if (moveForward) {
                std::cout << "  Moving forward along signal chain." << std::endl;
                nextProcessor = currentProcessor->getDestNode();
            } else {
                std::cout << "  Moving backward along signal chain." << std::endl;
                nextProcessor = currentProcessor->getSourceNode();
            }

    
            if (nextProcessor != 0) { // continue until the end of the chain

                editor = (GenericEditor*) nextProcessor->getEditor();

                if ((nextProcessor->isSplitter() || nextProcessor->isMerger()) 
                    && nextProcessor->saveOrder < 0)
                {
                    splitPoints.add(nextProcessor);

                    nextProcessor->switchIO(0);
                }

            } else {

                std::cout << "  No processor found." << std::endl;

                if (splitPoints.size() > 0) {

                    nextProcessor = splitPoints.getFirst();
                    splitPoints.remove(0);

                    nextProcessor->switchIO(1);
                    signalChain->addChildElement(switchNodeXml(nextProcessor));
                    
                    if (nextProcessor->isMerger())
                    {
                        insertionPt = 0;
                        moveForward = false;
                    } else { 
                        insertionPt = 1;
                        moveForward = true;
                    }

                    editor = nextProcessor->getEditor();

                } else {

                    std::cout << "  End of chain." << std::endl;

                    editor = 0;
                }
            }

            //insertionPt++;
        }

        xml->addChildElement(signalChain);
    }

    std::cout << "Saving processor graph." << std::endl;

    if (! xml->writeToFile (currentFile, String::empty))
        error = "Couldn't write to file ";
    else 
        error = "Saved configuration as ";

    error += currentFile.getFileName();
    
    delete xml;

    return error;
}

const String EditorViewport::loadState() 
{  

    FileChooser fc ("Choose a file to load...",
                    File::getCurrentWorkingDirectory(),
                    "*.xml",
                    true);

    if (fc.browseForFileToOpen())
    {
        currentFile = fc.getResult();
    } else {
        return "No configuration selected.";
    }

    std::cout << "Loading processor graph." << std::endl;

     Array<GenericProcessor*> splitPoints;
    
    XmlDocument doc (currentFile);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName ("PROCESSORGRAPH"))
    {
        std::cout << "File not found." << std::endl;
        delete xml;
        return "Not a valid file.";
    }

    clearSignalChain();

    String description;// = " ";
    int loadOrder = 0;

    forEachXmlChildElement (*xml, signalChain)
    {
        forEachXmlChildElement(*signalChain, processor)
        {

            if (processor->hasTagName("PROCESSOR"))
            {

                int insertionPt = processor->getIntAttribute("insertionPoint");

                if (insertionPt == 1)
                {
                    insertionPoint = editorArray.size();
                } else {
                    insertionPoint = 0;
                }

                itemDropped(processor->getStringAttribute("name"),0,0,0);

                GenericProcessor* p = (GenericProcessor*) lastEditor->getProcessor();
                p->loadOrder = loadOrder;
                
                loadOrder++;

                if (p->isSplitter() || p->isMerger())
                {
                    splitPoints.add(p);
                }

            } else if (processor->hasTagName("SWITCH"))
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
                        } else {
                            std::cout << "Switching splitter destination." << std::endl;
                            SplitterEditor* editor = (SplitterEditor*) splitPoints[n]->getEditor();
                            editor->switchDest(1);
                        }

                        splitPoints.remove(n);
                    }
                }

            }

        }

    }

    for (int i = 0; i < editorArray.size(); i++)
    {
        // deselect everything initially
        editorArray[i]->deselect();
    }

    String error = "Opened ";
    error += currentFile.getFileName();

    delete xml;
    return error;
}

