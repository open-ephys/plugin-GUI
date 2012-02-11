/*
==============================================================================

FilterViewport.cpp
Created: 1 May 2011 4:13:45pm
Author:  jsiegle

==============================================================================
*/

#include "FilterViewport.h"

FilterViewport::FilterViewport(ProcessorGraph* pgraph, DataViewport* tcomp)
    : message ("Drag-and-drop some rows from the top-left box onto this component!"),
      somethingIsBeingDraggedOver (false), graph(pgraph), tabComponent(tcomp), shiftDown(false),
       insertionPoint(0), componentWantsToMove(false), indexOfMovingComponent(-1), 
       borderSize(6), tabSize(20), tabButtonSize(15), canEdit(true)//, signalChainNeedsSource(true)
{

  addMouseListener(this, true);

  //File file = File("./savedState.xml");
  //loadState(file);

}

FilterViewport::~FilterViewport()
{
    deleteAllChildren();
}

void FilterViewport::signalChainCanBeEdited(bool t)
{
    canEdit = t;
    if (!canEdit)
        std::cout << "Filter Viewport disabled.";
    else
        std::cout << "Filter Viewport enabled.";

}

void FilterViewport::paint (Graphics& g)
{

    if (somethingIsBeingDraggedOver)
    {
         g.setColour (Colours::magenta);

    } else {
        g.setColour (Colour(127,137,147));
    }

    g.fillRoundedRectangle (tabSize, 0, getWidth(), getHeight(), 8);

    g.setColour (Colour(170,178,183));
    g.fillRect (tabSize+borderSize,borderSize,
                getWidth()-borderSize*2-tabSize,
                getHeight()-borderSize*2);

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

        g.setColour(Colours::orange);
        g.drawLine(insertionX, (float) borderSize,
                   insertionX, (float) getHeight()-(float) borderSize, 3.0f);

    }

    //if (signalChainNeedsSource)
   // {
    // draw the signal chain reminders
  //  if (!(somethingIsBeingDraggedOver && insertionPoint == 0))
        float insertionX = (float) tabSize + (float) borderSize;
        g.setColour(Colours::teal);
        //g.drawLine(insertionX, (float) borderSize,
        //           insertionX, (float) getHeight()-(float) borderSize, 3.0f);
        g.drawLine(insertionX*1.1, (float) (borderSize+10),
                   insertionX*1.1, (float) getHeight()-(float) (borderSize+10), 3.0f);
   // }

}

bool FilterViewport::isInterestedInDragSource (const String& description, Component* component)
{

    if (canEdit && description.startsWith("Processors")) {
        return false;
    } else {
        return true;
    }

}

void FilterViewport::itemDragEnter (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int /*x*/, int /*y*/)
{
    if (canEdit) {
        somethingIsBeingDraggedOver = true;
        repaint();
    }   
}

void FilterViewport::itemDragMove (const String& /*sourceDescription*/, Component* /*sourceComponent*/, int x, int /*y*/)
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
                //std::cout << insertionPoint << std::endl;
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

void FilterViewport::itemDragExit (const String& /*sourceDescription*/, Component* /*sourceComponent*/)
{
    somethingIsBeingDraggedOver = false;

    repaint();

    refreshEditors();

}

void FilterViewport::itemDropped (const String& sourceDescription, Component* /*sourceComponent*/, int /*x*/, int /*y*/)
{

    if (canEdit) {

        message = "last filter dropped: " + sourceDescription;

        std::cout << "Item dropped at insertion point " << insertionPoint << std::endl;

        /// needed to remove const cast --> should be a better way to do this
        String description = sourceDescription.substring(0);

        GenericEditor* activeEditor = (GenericEditor*) graph->createNewProcessor(description);//, source, dest);

        std::cout << "Active editor: " << activeEditor << std::endl;

        if (activeEditor != 0)
        {
            addChildComponent(activeEditor);

            updateVisibleEditors(activeEditor, 1);

        }

        somethingIsBeingDraggedOver = false;

        repaint();
    }
}



void FilterViewport::addEditor (GenericEditor* editor) 
{
    
 

}

void FilterViewport::deleteNode (GenericEditor* editor) {

    if (canEdit) {
        indexOfMovingComponent = editorArray.indexOf(editor);
        editor->setVisible(false);
   
        updateVisibleEditors(editor, 3);
    
        graph->removeProcessor((GenericProcessor*) editor->getProcessor());
    }

}

void FilterViewport::createNewTab(GenericEditor* editor)
{
    
    int index = signalChainArray.size();

    SignalChainTabButton* t = new SignalChainTabButton();
    t->setEditor(editor);
    
    t->setBounds(0,(tabButtonSize+5)*(index),
                 tabButtonSize,tabButtonSize);
    addAndMakeVisible(t);
    signalChainArray.add(t);
    editor->tabNumber(signalChainArray.size()-1);
    t->setToggleState(true,false);
    t->setNumber(index);

}

void FilterViewport::removeTab(int tabIndex)
{
    SignalChainTabButton* t = signalChainArray.remove(tabIndex);
    deleteAndZero(t);

    for (int n = 0; n < signalChainArray.size(); n++) 
    {
        signalChainArray[n]->setBounds(0,(tabButtonSize+5)*n,
                 tabButtonSize,tabButtonSize);
        
        int tNum = signalChainArray[n]->getEditor()->tabNumber();
        
        if (tNum > tabIndex) {
            signalChainArray[n]->getEditor()->tabNumber(tNum-1);
            signalChainArray[n]->setNumber(tNum-1);
        }

    }

}

void FilterViewport::updateVisibleEditors(GenericEditor* activeEditor, int action)

{
    // 1 = add
    // 2 = move
    // 3 = remove

    // Step 1: update the editor array
    if (action == 1) /// add
    {
        std::cout << "    Adding editor." << std::endl;
        editorArray.insert(insertionPoint, activeEditor);
        //activeEditor->select();
    } else if (action == 2) {  /// move
        std::cout << "    Moving editors." << std::endl;
        if (insertionPoint < indexOfMovingComponent)
           editorArray.move(indexOfMovingComponent, insertionPoint);
        else if (insertionPoint > indexOfMovingComponent)
           editorArray.move(indexOfMovingComponent, insertionPoint-1);

        //activeEditor->select();
    } else if (action == 3) {/// remove
        std::cout << "    Removing editor." << std::endl;

        editorArray.remove(indexOfMovingComponent);

        int t = activeEditor->tabNumber();

       // std::cout << editorArray.size() << " " << t << std::endl;

        if (editorArray.size() > 0) // if there are still editors in this chain
        {
            if (t > -1) {// pass on tab
          //      std::cout << "passing on the tab." << std::endl;
                int nextEditor = jmax(0,0);//indexOfMovingComponent-1);
                editorArray[nextEditor]->tabNumber(t); 
                signalChainArray[t]->setEditor(editorArray[nextEditor]);
            }

            // int nextEditor;
            // if (indexOfMovingComponent > editorArray.size())
            //     nextEditor = indexOfMovingComponent -1;
            // else if (indexOfMovingComponent == editorArray.size())
            //     nextEditor = 

            int nextEditor = jmin(indexOfMovingComponent,editorArray.size()-1);
            activeEditor = editorArray[nextEditor];
            activeEditor->select();
            
        } else {

            removeTab(t);

            if (signalChainArray.size() > 0) // if there are other chains
            {
                int nextTab = jmin(t,signalChainArray.size()-1);
                activeEditor = signalChainArray[nextTab]->getEditor(); 
                activeEditor->select();
                signalChainArray[nextTab]->setToggleState(true,false); // send it back to update connections   
            } else {
                activeEditor = 0; // nothing is active
              //  signalChainNeedsSource = true;
            }
        }

    } else { //no change
        ;
    }

    // Step 2: update connections
    if (action < 4 && editorArray.size() > 0) {

        GenericProcessor* source = 0;
        GenericProcessor* dest = (GenericProcessor*) editorArray[0]->getProcessor();

        dest->setSourceNode(source); // set first source as 0

      //  std::cout << "        " << dest->getName() << "::";

        for (int n = 1; n < editorArray.size(); n++)
        {

            dest = (GenericProcessor*) editorArray[n]->getProcessor();
            source = (GenericProcessor*) editorArray[n-1]->getProcessor();

            dest->setSourceNode(source);

         //  std::cout << dest->getName() << "::";
        }

        dest->setDestNode(0); // set last dest as 0

      // std::cout << std::endl;
    }//


    // Step 3: check for new tabs
   if (action < 4) {

        for (int n = 0; n < editorArray.size(); n++)
        {
            GenericProcessor* p = (GenericProcessor*) editorArray[n]->getProcessor();

      //      std::cout << editorArray[n]->tabNumber() << std::endl;

            if (p->getSourceNode() == 0)// && editorArray[n]->tabNumber() == -1)
            {
                if (editorArray[n]->tabNumber() == -1) 
                {
                    createNewTab(editorArray[n]);
                }

            } else {
                if (editorArray[n]->tabNumber() > -1) 
                {
                    removeTab(editorArray[n]->tabNumber());
                }

                editorArray[n]->tabNumber(-1); // reset tab status
            }
        }
    }



    
    // Step 4: Refresh editors in editor array, based on active editor
    for (int n = 0; n < editorArray.size(); n++)
    {
        editorArray[n]->setVisible(false);
    }

    editorArray.clear();

    GenericEditor* editorToAdd = activeEditor;

    while (editorToAdd != 0) 
    {

        editorArray.insert(0,editorToAdd);
        GenericProcessor* currentProcessor = (GenericProcessor*) editorToAdd->getProcessor();
        GenericProcessor* source = currentProcessor->getSourceNode();

        if (source != 0)
        {
          //  std::cout << "Source: " << source->getName() << std::endl;
            editorToAdd = (GenericEditor*) source->getEditor();
        } else {

          //  std::cout << "No source found." << std::endl;
            editorToAdd = 0;
        }
    }

    editorToAdd = activeEditor;

    while (editorToAdd != 0)
    {

        GenericProcessor* currentProcessor = (GenericProcessor*) editorToAdd->getProcessor();
        GenericProcessor* dest = currentProcessor->getDestNode();

        if (dest != 0)
        {
         //   std::cout << "Destination: " << dest->getName() << std::endl;
            editorToAdd = (GenericEditor*) dest->getEditor();
            editorArray.add(editorToAdd);

        } else {
          // std::cout << "No dest found." << std::endl;
            editorToAdd = 0;
        }
    }

    //std::cout << "OK1." << std::endl;

    // Step 5: check the validity of the signal chain
    bool enable = true;

    if (editorArray.size() == 1) {
        
         GenericProcessor* source = (GenericProcessor*) editorArray[0]->getProcessor();
         if (source->isSource())
            editorArray[0]->setEnabledState(true);
         else
            editorArray[0]->setEnabledState(false);

    } else {

        //bool sourceIsInChain = true;

        for (int n = 0; n < editorArray.size()-1; n++)
        {
            GenericProcessor* source = (GenericProcessor*) editorArray[n]->getProcessor();
            GenericProcessor* dest = (GenericProcessor*) editorArray[n+1]->getProcessor();

            if (n == 0 && !source->isSource())
                enable = false;

            editorArray[n]->setEnabledState(enable);
            
            if (source->canSendSignalTo(dest) && source->enabledState())
                enable = true;
            else 
                enable = false;

            if (enable)
                std::cout << "Enabling node." << std::endl;
            else
                std::cout << "Not enabling node." << std::endl;
            
            editorArray[n+1]->setEnabledState(enable);

        }
    }

    // Step 6: inform the tabs that something has changed
    for (int n = 0; n < signalChainArray.size(); n++)
    {
        if (signalChainArray[n]->getToggleState())
        {
            signalChainArray[n]->hasNewConnections(true);
        }
    }

   // std::cout << "OK2." << std::endl;

    // Step 7: make sure all editors are visible, and refresh
    for (int n = 0; n < editorArray.size(); n++)
    {
       // std::cout << "Editor " << n << ": " << editorArray[n]->getName() << std::endl;
        editorArray[n]->setVisible(true);
    }

    insertionPoint = -1; // make sure all editors are left-justified
    indexOfMovingComponent = -1;

   // std::cout << "OK3." << std::endl;
    refreshEditors();

   // std::cout << "OK4." << std::endl;
    grabKeyboardFocus();
   // std::cout << "OK5." << std::endl;
    
}

void FilterViewport::refreshEditors () {
    
    int lastBound = borderSize+tabSize;

    for (int n = 0; n < editorArray.size(); n++)
    {
        if (n == 0)
        {
             if (!editorArray[n]->getEnabledState()) 
             {
                lastBound += borderSize*3;
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

void FilterViewport::moveSelection (const KeyPress &key) {
    
    if (key.getKeyCode() == key.leftKey) {
        
        for (int i = 0; i < editorArray.size(); i++) {
        
            if (editorArray[i]->getSelectionState() && i > 0) {
                
                editorArray[i-1]->select();
                editorArray[i]->deselect();
                break;
            }               
        }
    } else if (key.getKeyCode() == key.rightKey) {
         
         for (int i = 0; i < editorArray.size(); i++) {
        
            if (editorArray[i]->getSelectionState()) {
                
                if (i == editorArray.size()-1)
                {
                    editorArray[i]->deselect();
                    break;
                } else {
                    editorArray[i+1]->select();
                    editorArray[i]->deselect();
                    break;
                }
            }  
        }
    }
}

bool FilterViewport::keyPressed (const KeyPress &key) {
    
   //std::cout << key.getKeyCode() << std::endl;

   if (canEdit) {

    if (key.getKeyCode() == key.deleteKey || key.getKeyCode() == key.backspaceKey) {
        
        for (int i = 0; i < editorArray.size(); i++) {
        
            if (editorArray[i]->getSelectionState()) {
                deleteNode(editorArray[i]);
                break;
            }               
        }

    } else if (key.getKeyCode() == key.leftKey || key.getKeyCode() == key.rightKey) {

        moveSelection(key);

    }
    }

}

//void FilterViewport::modifierKeysChanged (const ModifierKeys & modifiers) {
    
/*     if (modifiers.isShiftDown()) {
        
        std::cout << "Shift key pressed." << std::endl;
        shiftDown  = true;

    } else {


        std::cout << "Shift key released." << std::endl;
        shiftDown = false;
    }*/

//}

void FilterViewport::selectEditor(GenericEditor* editor)
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

void FilterViewport::mouseDown(const MouseEvent &e) {
    
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

void FilterViewport::mouseDrag(const MouseEvent &e) {
    

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

void FilterViewport::mouseUp(const MouseEvent &e) {


    if (componentWantsToMove) {
        
        somethingIsBeingDraggedOver = false;
        componentWantsToMove = false;

        GenericEditor* editor = editorArray[indexOfMovingComponent];

        updateVisibleEditors(editor, 2);
        refreshEditors();
        repaint();

    }


}

void FilterViewport::mouseExit(const MouseEvent &e) {

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

        MemoryInputStream mis(BinaryData::misoserialized, BinaryData::misoserializedSize, false);
        Typeface::Ptr typeface = new CustomTypeface(mis);
        buttonFont = Font(typeface);
        buttonFont.setHeight(14);
    }


void SignalChainTabButton::clicked() 
{
    {
    
        //std::cout << "Button clicked: " << firstEditor->getName() << std::endl;
        FilterViewport* fv = (FilterViewport*) getParentComponent();
    
        fv->updateVisibleEditors(firstEditor,4);    
    }
    
}

void SignalChainTabButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
        if (getToggleState() == true)
            g.setColour(Colours::teal);
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

// void FilterViewport::loadSignalChain()
// {
//     insertionPoint = 0;

//     itemDropped ("Sources/Intan Demo Board", 0, 0, 0);

//     insertionPoint = 1;

//     itemDropped ("Filters/Bandpass Filter", 0, 0, 0);
// }

// void FilterViewport::saveSignalChain()
// {
    


// }




XmlElement* FilterViewport::createNodeXml (GenericEditor* editor,
                                           int insertionPt)
{

   // if (editor == 0)
   //     return 0;
    
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
    
    //if (dest != 0)
    //    editor = (GenericEditor*) dest->getEditor();
    //else
     //   editor = 0;
//
    return e;

    // if (dest != 0)
    //     return (GenericEditor*) dest->getEditor();
    // else 
    //     return 0; 

    // int sourceId = -1;
    // int destId = -1;

    // if (processor->getSourceNode() != 0)
    //     sourceId = processor->getSourceNode()->getNodeId();
    
    // if (processor->getDestNode() != 0)
    //     destId = processor->getDestNode()->getNodeId();

    // e->setAttribute (T("dest"), destId);
    // e->setAttribute (T("source"), sourceId);

    // XmlElement* state = new XmlElement ("STATE");

 //    MemoryBlock m;
 //    node->getProcessor()->getStateInformation (m);
 //    state->addTextElement (m.toBase64Encoding());
 //    e->addChildElement (state);

   /// return e;

}

const String FilterViewport::saveState(const File& file) 
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

const String FilterViewport::loadState(const File& file) 
{
    std::cout << "Loading processor graph." << std::endl;
    
    XmlDocument doc (file);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName (T("PROCESSORGRAPH")))
    {
        delete xml;
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