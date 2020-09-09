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

#include "SignalChainManager.h"

#include "EditorViewport.h"

#include "../AccessClass.h"

#include <iostream>

SignalChainManager::SignalChainManager
(EditorViewport* ev_,
 Array<GenericEditor*, CriticalSection>& editorArray_,
 Array<SignalChainTabButton*, CriticalSection>& signalChainArray_)
    : editorArray(editorArray_), signalChainArray(signalChainArray_),
      ev(ev_), tabSize(30)
{
    topTab = 0;
}

SignalChainManager::~SignalChainManager()
{


}

void SignalChainManager::scrollUp()
{

    //std::cout << "Scrolling up." << std::endl;

    if (topTab > 0)
    {
        topTab -= 1;
    }

    refreshTabs();

}

void SignalChainManager::scrollDown()
{

    //std::cout << "Scrolling down." << std::endl;

    if (topTab < signalChainArray.size()-4)
    {
        topTab += 1;
    }

    refreshTabs();

}

void SignalChainManager::clearSignalChain()
{
    editorArray.clear();

    while (signalChainArray.size() > 0)
    {
        SignalChainTabButton* t = signalChainArray.remove(signalChainArray.size()-1);
        deleteAndZero(t);
    }

}

void SignalChainManager::createNewTab(GenericEditor* editor)
{

    int index = signalChainArray.size();

    SignalChainTabButton* t = new SignalChainTabButton();
    t->setManager(this);
    t->setEditor(editor);

    ev->addChildComponent(t);
    signalChainArray.add(t);

    editor->tabNumber(signalChainArray.size()-1);
    t->setToggleState(true, dontSendNotification);
    t->setNumber(index);

    index -= topTab;
    ev->leftmostEditor = 0;

    if (signalChainArray.size()-topTab > 4)
    {
        scrollDown();
    }

    refreshTabs();

}

void SignalChainManager::removeTab(int tabIndex)
{

    SignalChainTabButton* t = signalChainArray.remove(tabIndex);
    deleteAndZero(t);

    for (int n = 0; n < signalChainArray.size(); n++)
    {
        int tNum = signalChainArray[n]->getEditor()->tabNumber();

        if (tNum > tabIndex)
        {
            signalChainArray[n]->getEditor()->tabNumber(tNum-1);
            signalChainArray[n]->setNumber(tNum-1);
        }

    }

    if (signalChainArray.size()-topTab < 4)
    {
        scrollUp();
    }

    refreshTabs();

}

void SignalChainManager::refreshTabs()
{
    for (int n = 0; n < signalChainArray.size(); n++)
    {
        if (n >= topTab && n < topTab + 4)
        {
            signalChainArray[n]->setVisible(true);
            signalChainArray[n]->setBounds(6,(tabSize-2)*(n-topTab)+23,tabSize-10,tabSize-10);
        }
        else
        {
            signalChainArray[n]->setVisible(false);
        }
    }

    ev->checkScrollButtons(topTab);

}


void SignalChainManager::updateVisibleEditors(GenericEditor* activeEditor,
                                              int index, int insertionPoint, int action)

{

    enum actions {ADD, MOVE, REMOVE, ACTIVATE, UPDATE};

    // Step 1: update the editor array
    if (action == ADD)
    {
        std::cout << "    Adding editor." << std::endl;
        editorArray.insert(insertionPoint, activeEditor);

    }
    else if (action == MOVE)
    {
        std::cout << "    Moving editors." << std::endl;
        if (insertionPoint < index)
            editorArray.move(index, insertionPoint);
        else if (insertionPoint > index)
            editorArray.move(index, insertionPoint-1);

    }
    else if (action == REMOVE)
    {

        std::cout << "    Removing editor." << std::endl;

        GenericProcessor* p = (GenericProcessor*) editorArray[index]->getProcessor();

        if (p->getSourceNode() != nullptr)
            if (p->getSourceNode()->isSplitter())
                p->getSourceNode()->setSplitterDestNode(nullptr);

        // if the processor to be removed is a merger,
        // we need to inform the other source that its merger has disappeared
        if (p->isMerger())
        {
            p->switchIO();
            if (p->getSourceNode() != nullptr)
                p->getSourceNode()->setDestNode(nullptr);
        }

        // if the processor to be removed is a splitter, we need to make sure
        // there aren't any orphaned processors
        if (p->isSplitter())
        {
            p->switchIO(0);
            if (p->getDestNode() != nullptr)
            {
                //   std::cout << "Found an orphaned signal chain" << std::endl;
                p->getDestNode()->setSourceNode(nullptr);
                createNewTab(p->getDestNode()->getEditor());
            }

            p->switchIO(1);
            if (p->getDestNode() != nullptr)
            {
                //   std::cout << "Found an orphaned signal chain" << std::endl;
                p->getDestNode()->setSourceNode(nullptr);
                createNewTab(p->getDestNode()->getEditor());
            }
        }

        editorArray.remove(index);

        int t = activeEditor->tabNumber();

        // std::cout << editorArray.size() << " " << t << std::endl;

		bool merger = false;

        if (editorArray.size() > 0)
        {
            // take the next processor in the array
            GenericProcessor* p2 = (GenericProcessor*) editorArray[0]->getProcessor();
            merger = (p2->isMerger() && p2->stillHasSource());
            if (p2->isMerger())
            {
                //  std::cout << "We've got a merger!" << std::endl;
                //p2->switchIO(0);
                p2->setMergerSourceNode(p->getSourceNode());
				if (p2->stillHasSource())
				{
					MergerEditor* me = static_cast<MergerEditor*>(p2->getEditor());
					me->switchSource();
				}
                // p2->setMergerSourceNode(nullptr);
            }
        }

        if (editorArray.size() > 0 && !merger) // if there are still editors in this chain
        {
            if (t > -1)  // pass on tab
            {
                //      std::cout << "passing on the tab." << std::endl;
                int nextEditor = jmax(0,0);//index-1);
                editorArray[nextEditor]->tabNumber(t);
                signalChainArray[t]->setEditor(editorArray[nextEditor]);
            }

            int nextEditor = jmin(index,editorArray.size()-1);
            activeEditor = editorArray[nextEditor];
            activeEditor->select();
            //activeEditor->grabKeyboardFocus();

        }
        else
        {

            // std::cout << "Tab number " << t << std::endl;

            removeTab(t);

            if (signalChainArray.size() > 0) // if there are other chains
            {
                int nextTab = jmin(t,signalChainArray.size()-1);
                activeEditor = signalChainArray[nextTab]->getEditor();
                activeEditor->select();
                signalChainArray[nextTab]->setToggleState(true, dontSendNotification); // send it back to update connections
            }
            else
            {
                activeEditor = 0; // nothing is active
                //  signalChainNeedsSource = true;
            }
        }

    }
    else     //no change
    {

        // std::cout << "Activating editor" << std::endl;
    }

    // Step 2: update connections
    if (action != ACTIVATE && action != UPDATE && editorArray.size() > 0)
    {

        //std::cout << "Updating connections." << std::endl;

        GenericProcessor* source = 0;
        GenericProcessor* dest = (GenericProcessor*) editorArray[0]->getProcessor();

        if (!dest->isMerger())
        {
            dest->setSourceNode(source);
        } else {
            dest->setMergerSourceNode(source);
        }
        
        for (int n = 1; n < editorArray.size(); n++)
        {

            dest = (GenericProcessor*) editorArray[n]->getProcessor();
            source = (GenericProcessor*) editorArray[n-1]->getProcessor();

            if (!dest->isMerger())
            {
                dest->setSourceNode(source);
            } else {
                dest->setMergerSourceNode(source);
            }
            
        }

        if (!dest->isSplitter())
        {
            dest->setDestNode(0);
        } else {
            dest->setSplitterDestNode(0);
        }
        

    }

    // Step 3: check for new tabs
    if (action != ACTIVATE && action != UPDATE)
    {

        //std::cout << "Checking for new tabs." << std::endl;

        for (int n = 0; n < editorArray.size(); n++)
        {
            GenericProcessor* p = (GenericProcessor*) editorArray[n]->getProcessor();

            if (p->getSourceNode() == 0)// && editorArray[n]->tabNumber() == -1)
            {

                if (editorArray[n]->tabNumber() == -1)

                {
                    if (!p->isMerger())
                    {
                        //   std::cout << p->getName() << " has no source node. Creating a new tab." << std::endl;
                        createNewTab(editorArray[n]);
                    }
                }

            }
            else
            {
                if (editorArray[n]->tabNumber() > -1)
                {
                    removeTab(editorArray[n]->tabNumber());
                }

                editorArray[n]->tabNumber(-1); // reset tab status
            }

            if (p->isMerger())
            {
                // std::cout << "It's a merger!" << std::endl;
                //createNewTab(editorArray[n]);
            }
        }
    }

    // Step 4: Refresh editors in editor array, based on active editor
    for (int n = 0; n < editorArray.size(); n++)
    {
        editorArray[n]->setVisible(false);
    }

    editorArray.clear();
    //std::cout << "Cleared editor array." << std::endl;

    GenericEditor* editorToAdd = activeEditor;

    while (editorToAdd != 0)
    {
        // std::cout << "Inserting " << editorToAdd->getName() << " at point 0." << std::endl;

        editorArray.insert(0,editorToAdd);
        GenericProcessor* currentProcessor = (GenericProcessor*) editorToAdd->getProcessor();
        GenericProcessor* source = currentProcessor->getSourceNode();

        if (source != nullptr)
        {
            //   std::cout << "Source: " << source->getName() << std::endl;

            // need to switch the splitter somehow
            if (action == ACTIVATE || action == UPDATE)
            {
                if (source->isSplitter())
                {
                    source->setPathToProcessor(currentProcessor);
                }
            }

            editorToAdd = (GenericEditor*) source->getEditor();

        }
        else
        {

            if (editorToAdd->tabNumber() >= 0 && editorToAdd->tabNumber() < signalChainArray.size())
                signalChainArray[editorToAdd->tabNumber()]->setToggleState(true, dontSendNotification);

            // std::cout << "No source found." << std::endl;
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
            //   std::cout << "Inserting " << editorToAdd->getName() << " at the end." << std::endl;

            if (dest->isMerger())
            {
                //    std::cout << "It's a merger!" << std::endl;

                editorToAdd->switchIO(0);

                if (dest->getSourceNode() != currentProcessor)
                    editorToAdd->switchIO(1);

            }

        }
        else
        {
            //   std::cout << "No dest found." << std::endl;
            editorToAdd = 0;
        }


    }

    // Step 5: check the validity of the signal chain
    if (true)
    {
        //action != ACTIVATE) {
        bool enable = true;

        if (editorArray.size() == 1)
        {

            GenericProcessor* source = (GenericProcessor*) editorArray[0]->getProcessor();
            if (source->isSource())
                editorArray[0]->setEnabledState(true);
            else
                editorArray[0]->setEnabledState(false);

        }
        else
        {

            for (int n = 0; n < editorArray.size()-1; n++)
            {
                GenericProcessor* source = (GenericProcessor*) editorArray[n]->getProcessor();
                GenericProcessor* dest = (GenericProcessor*) editorArray[n+1]->getProcessor();

                if (n == 0 && !source->isSource())
                    enable = false;

                editorArray[n]->setEnabledState(enable);

                if (source->canSendSignalTo(dest) && source->isEnabledState())
                    enable = true;
                else
                    enable = false;

                if (source->isSplitter())
                {
                    if (source->getDestNode() != dest)
                    {
                        //source->switchIO();
                        editorArray[n]->switchDest();
                    }
                }

                //   if (enable)
                //      std::cout << "Enabling node." << std::endl;
                //   else
                //     std::cout << "Not enabling node." << std::endl;

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
    if (action != ACTIVATE)
    {

		updateProcessorSettings();
    }


    // std::cout << "Finished adding new editor." << std::endl << std::endl << std::endl;

}

void SignalChainManager::updateProcessorSettings()
{
	// std::cout << "Updating settings." << std::endl;

	Array<GenericProcessor*> splitters;

	for (int n = 0; n < signalChainArray.size(); n++)
	{
		// iterate through signal chains

		GenericEditor* source = signalChainArray[n]->getEditor();
		GenericProcessor* p = source->getProcessor();

		//  p->update();

		//  GenericProcessor* dest = p->getDestNode();

		while (p != 0)
		{
			// iterate through processors
			p->update();

			if (p->isSplitter())
			{
				splitters.add(p);
			}

			p = p->getDestNode();

			if (p == 0 && splitters.size() > 0)
			{
				splitters.getFirst()->switchIO(); // switch the signal chain
				p = splitters[0]->getDestNode();
				splitters.getFirst()->switchIO(); // switch it back
				splitters.remove(0);
			}
		}
	}

    File recoveryFile = CoreServices::getSavedStateDirectory().getChildFile("recoveryConfig.xml");
    AccessClass::getEditorViewport()->saveState(recoveryFile);
}
