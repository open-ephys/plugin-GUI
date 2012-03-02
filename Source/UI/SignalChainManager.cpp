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

#include "SignalChainManager.h"

#include "EditorViewport.h"

SignalChainManager::SignalChainManager (EditorViewport* ev_, Array<GenericEditor*, CriticalSection>& editorArray_,
	 				   Array<SignalChainTabButton*, CriticalSection>& signalChainArray_) :

	 				   ev(ev_), editorArray(editorArray_), signalChainArray(signalChainArray_)
{
	
	

}

SignalChainManager::~SignalChainManager()
{
	
}

void SignalChainManager::createNewTab(GenericEditor* editor)
{
	int tabSize = 30;
    
    int index = signalChainArray.size();

    SignalChainTabButton* t = new SignalChainTabButton();
    t->setManager(this);
    t->setEditor(editor);
    
    t->setBounds(6,(tabSize-2)*(index)+8,tabSize-10,tabSize-10);

    ev->addAndMakeVisible(t);
    signalChainArray.add(t);

    editor->tabNumber(signalChainArray.size()-1);
    t->setToggleState(true,false);
    t->setNumber(index);

}

void SignalChainManager::removeTab(int tabIndex)
{

	int tabSize = 30; // also set in EditorViewport

    SignalChainTabButton* t = signalChainArray.remove(tabIndex);
    deleteAndZero(t);

    for (int n = 0; n < signalChainArray.size(); n++) 
    {
        signalChainArray[n]->setBounds(6,(tabSize-2)*n+8,tabSize-10,tabSize-10);
        
        int tNum = signalChainArray[n]->getEditor()->tabNumber();
        
        if (tNum > tabIndex) {
            signalChainArray[n]->getEditor()->tabNumber(tNum-1);
            signalChainArray[n]->setNumber(tNum-1);
        }

    }
}


void SignalChainManager::updateVisibleEditors(GenericEditor* activeEditor,
											  int index, int insertionPoint, int action)

{

	enum actions {ADD, MOVE, REMOVE, ACTIVATE };

    // Step 1: update the editor array
    if (action == ADD) /// add
    {
        std::cout << "    Adding editor." << std::endl;
        editorArray.insert(insertionPoint, activeEditor);
        //activeEditor->select();
    } else if (action == MOVE) {  /// move
        std::cout << "    Moving editors." << std::endl;
        if (insertionPoint < index)
           editorArray.move(index, insertionPoint);
        else if (insertionPoint > index)
           editorArray.move(index, insertionPoint-1);

        //activeEditor->select();
    } else if (action == REMOVE) {/// remove
        std::cout << "    Removing editor." << std::endl;

        editorArray.remove(index);

        int t = activeEditor->tabNumber();

       // std::cout << editorArray.size() << " " << t << std::endl;

        if (editorArray.size() > 0) // if there are still editors in this chain
        {
            if (t > -1) {// pass on tab
          //      std::cout << "passing on the tab." << std::endl;
                int nextEditor = jmax(0,0);//index-1);
                editorArray[nextEditor]->tabNumber(t); 
                signalChainArray[t]->setEditor(editorArray[nextEditor]);
            }

            // int nextEditor;
            // if (index > editorArray.size())
            //     nextEditor = index -1;
            // else if (index == editorArray.size())
            //     nextEditor = 

            int nextEditor = jmin(index,editorArray.size()-1);
            activeEditor = editorArray[nextEditor];
            activeEditor->select();
            activeEditor->grabKeyboardFocus();
            
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

           //std::cout << dest->getName() << "::";
        }

        dest->setDestNode(0); // set last dest as 0

      // std::cout << std::endl;
    }//


    // Step 3: check for new tabs
   if (action < 4) {

        std::cout << "Checking for new tabs." << std::endl;

        for (int n = 0; n < editorArray.size(); n++)
        {
            GenericProcessor* p = (GenericProcessor*) editorArray[n]->getProcessor();

      //      std::cout << editorArray[n]->tabNumber() << std::endl;

            if (p->getSourceNode() == 0)// && editorArray[n]->tabNumber() == -1)
            {
               
                if (editorArray[n]->tabNumber() == -1) 

                {
                     std::cout << p->getName() << " has no source node. Creating a new tab." << std::endl;
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
    std::cout << "Cleared editor array." << std::endl;

    GenericEditor* editorToAdd = activeEditor;

    while (editorToAdd != 0) 
    {
        std::cout << "Inserting " << editorToAdd->getName() << " at point 0." << std::endl;

        editorArray.insert(0,editorToAdd);
        GenericProcessor* currentProcessor = (GenericProcessor*) editorToAdd->getProcessor();
        GenericProcessor* source = currentProcessor->getSourceNode();

        if (source != 0)
        {
            std::cout << "Source: " << source->getName() << std::endl;
            editorToAdd = (GenericEditor*) source->getEditor();
        } else {
            std::cout << "No source found." << std::endl;
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

            std::cout << "Destination: " << dest->getName() << std::endl;
            editorToAdd = (GenericEditor*) dest->getEditor();
            editorArray.add(editorToAdd);
            std::cout << "Inserting " << editorToAdd->getName() << " at the end." << std::endl;


        } else {
           std::cout << "No dest found." << std::endl;
            editorToAdd = 0;
        }
    }

    //std::cout << "OK1." << std::endl;

    // Step 5: check the validity of the signal chain
    if (action < 5) {
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
    }

    // Step 6: inform the tabs that something has changed
    for (int n = 0; n < signalChainArray.size(); n++)
    {
        if (signalChainArray[n]->getToggleState())
        {
            signalChainArray[n]->hasNewConnections(true);
        }
    }

    // Step 7: update all settings
    if (action < 4) {
        std::cout << "Updating settings." << std::endl;
        for (int n = 0; n < signalChainArray.size(); n++)
        {
            
            GenericEditor* source = (GenericEditor*) signalChainArray[n]->getEditor();
            GenericProcessor* p = (GenericProcessor*) source->getProcessor();

            p->updateSettings();

            GenericProcessor* dest = p->getDestNode();

            while (dest != 0)
            {
                dest->updateSettings();
                dest = dest->getDestNode();
            }
        }
    }

   // std::cout << "OK2." << std::endl;

    // Step 8: make sure all editors are visible, and refresh
    for (int n = 0; n < editorArray.size(); n++)
    {
       // std::cout << "Editor " << n << ": " << editorArray[n]->getName() << std::endl;
        editorArray[n]->setVisible(true);
    }


   std::cout << "Finished adding new editor." << std::endl << std::endl << std::endl;
    
}