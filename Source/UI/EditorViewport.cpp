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

EditorViewport::EditorViewport()
    : message ("Drag-and-drop some rows from the top-left box onto this component!"),
      somethingIsBeingDraggedOver (false), shiftDown(false),
       insertionPoint(0), componentWantsToMove(false), indexOfMovingComponent(-1), 
       borderSize(6), tabSize(30), tabButtonSize(15), canEdit(true), currentTab(-1)//, signalChainNeedsSource(true)
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

}

EditorViewport::~EditorViewport()
{
    deleteAndZero(signalChainManager);
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

    for (int n = 0; n < 5; n++)
    {
        g.drawEllipse(6,(tabSize-2)*n+8,tabSize-10,tabSize-10,1.0);
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
            addChildComponent(activeEditor);
            activeEditor->setUIComponent(getUIComponent());

            signalChainManager->updateVisibleEditors(activeEditor, indexOfMovingComponent, insertionPoint, ADD);

        } 
        
        insertionPoint = -1; // make sure all editors are left-justified
        indexOfMovingComponent = -1;
        refreshEditors();

        somethingIsBeingDraggedOver = false;

        repaint();
    }
}

void EditorViewport::makeEditorVisible(GenericEditor* editor)
{
    
    signalChainManager->updateVisibleEditors(editor, 0, 0, ACTIVATE);

    refreshEditors();
}



void EditorViewport::deleteNode (GenericEditor* editor) {

    if (canEdit) {
        indexOfMovingComponent = editorArray.indexOf(editor);
        editor->setVisible(false);
   
        signalChainManager->updateVisibleEditors(editor, indexOfMovingComponent, insertionPoint, REMOVE);
    
        refreshEditors();

        getProcessorGraph()->removeProcessor((GenericProcessor*) editor->getProcessor());
    }

    int64 t1 = Time::currentTimeMillis();
    int64 t2 = t1;

    // pause for 50 ms so multiple editors are not accidentally deleted
    while (t2 < t1+50)
    {
        t2 = Time::currentTimeMillis();
    }

}


void EditorViewport::refreshEditors () {
    
    int lastBound = borderSize+tabSize;

    for (int n = 0; n < editorArray.size(); n++)
    {
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

        int componentWidth = editorArray[n]->desiredWidth;
        editorArray[n]->setBounds(lastBound, borderSize, componentWidth, getHeight()-borderSize*2);
        lastBound+=(componentWidth + borderSize);
    }

}

void EditorViewport::moveSelection (const KeyPress &key) {
    
    if (key.getKeyCode() == key.leftKey) {
        
        for (int i = 0; i < editorArray.size(); i++) {
        
            if (editorArray[i]->getSelectionState() && i > 0) {
                
                editorArray[i-1]->select();
                editorArray[i]->deselect();
                break;
            }               
        }
    } else if (key.getKeyCode() == key.rightKey) {
         
         for (int i = 0; i < editorArray.size()-1; i++) {
        
            if (editorArray[i]->getSelectionState()) {
                
                // if (i == editorArray.size()-1)
                // {
                //     editorArray[i]->deselect();
                //     break;
                // } else {
                    editorArray[i+1]->select();
                    editorArray[i]->deselect();
                    break;
                // }
            }  
        }
    } else if (key.getKeyCode() == key.upKey) {
        
        // move one tab up
    } else if (key.getKeyCode() == key.downKey) {
        
        // move one tab down
    }
}

bool EditorViewport::keyPressed (const KeyPress &key) {
    
   //std::cout << key.getKeyCode() << std::endl;

   if (canEdit) {

    if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey) {
        
        for (int i = 0; i < editorArray.size(); i++) {
        
            if (editorArray[i]->getSelectionState()) {
#if !JUCE_MAC
                deleteNode(editorArray[i]);
                break;
#endif
            }               
        }

    } else if (key.getKeyCode() == key.leftKey || key.getKeyCode() == key.rightKey) {

        moveSelection(key);

    }
    }

   return true;

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
    
    for (int i = 0; i < editorArray.size(); i++) {
        
        if (e.eventComponent == editorArray[i]
             || e.eventComponent->getParentComponent() == editorArray[i]) {
            editorArray[i]->select();
        } else {
            editorArray[i]->deselect();
        }
    } 

   // selectEditor((GenericEditor*) e.eventComponent);

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

SignalChainTabButton::SignalChainTabButton() : Button("Name"),
        configurationChanged(true)
    {
        setRadioGroupId(99);
        //setToggleState(false,true);
        setClickingTogglesState(true);

        MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
        Typeface::Ptr typeface = new CustomTypeface(mis);
        buttonFont = Font(typeface);
        buttonFont.setHeight(14);
    }


void SignalChainTabButton::clicked() 
{
    {
    
        //std::cout << "Button clicked: " << firstEditor->getName() << std::endl;
        EditorViewport* ev = (EditorViewport*) getParentComponent();
    
        scm->updateVisibleEditors(firstEditor, 0, 0, ACTIVATE); 
        ev->refreshEditors();   
    }
    
}

void SignalChainTabButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
        if (getToggleState() == true)
            g.setColour(Colours::orange);
        else 
            g.setColour(Colours::darkgrey);

        if (isMouseOver)
            g.setColour(Colours::white);

        g.fillEllipse(0,0,getWidth(),getHeight());

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

    e->setAttribute (T("name"), name);
    e->setAttribute (T("insertionPoint"), insertionPt);
  
    GenericProcessor* dest = (GenericProcessor*) source->getDestNode();

    return e;

}

const String EditorViewport::saveState(const File& file) 
{

    XmlElement* xml = new XmlElement("PROCESSORGRAPH");

    for (int n = 0; n < signalChainArray.size(); n++)
    {
        
        XmlElement* signalChain = new XmlElement("SIGNALCHAIN");

        GenericEditor* editor = signalChainArray[n]->getEditor();

        int insertionPt = 0;

        while (editor != 0)
        {

            signalChain->addChildElement(createNodeXml(editor, insertionPt));
            
            GenericProcessor* source = (GenericProcessor*) editor->getProcessor();
            GenericProcessor* dest = (GenericProcessor*) source->getDestNode();
    
            if (dest != 0)
                editor = (GenericEditor*) dest->getEditor();
            else
                editor = 0;

            insertionPt++;
        }

        xml->addChildElement(signalChain);
    }
 
    String error;

    std::cout << "Saving processor graph." << std::endl;

    if (! xml->writeToFile (file, String::empty))
        error = "Couldn't write to file";
    
    delete xml;

    return error;
}

const String EditorViewport::loadState(const File& file) 
{
    std::cout << "Loading processor graph." << std::endl;
    
    XmlDocument doc (file);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName (T("PROCESSORGRAPH")))
    {
        std::cout << "File not found." << std::endl;
        delete xml;
        // don't do anything
        return "Not a valid file.";
    }

    String description;// = T(" ");

    forEachXmlChildElement (*xml, signalChain)
    {
        forEachXmlChildElement(*signalChain, processor)
        {
            insertionPoint = processor->getIntAttribute("insertionPoint");

            itemDropped(processor->getStringAttribute("name"),0,0,0);
        }

    }

    delete xml;
    return "Everything went ok.";

}