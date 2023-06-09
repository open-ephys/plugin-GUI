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

#include "MergerEditor.h"
#include "Merger.h"
#include "../ProcessorGraph/ProcessorGraph.h"
#include "../../UI/EditorViewport.h"
#include "../../AccessClass.h"
#include "../../UI/GraphViewer.h"
#include "../MessageCenter/MessageCenterEditor.h"

#include "../Editors/StreamSelector.h"
#include "../Settings/DataStream.h"

MergerEditor::MergerEditor(GenericProcessor* parentNode)
    : GenericEditor(parentNode)

{
    desiredWidth = 90;

    pipelineSelectorA = std::make_unique<ImageButton>("Pipeline A");

    Image normalImageA = ImageCache::getFromMemory(BinaryData::MergerB01_png, BinaryData::MergerB01_pngSize);
    Image downImageA = ImageCache::getFromMemory(BinaryData::MergerA01_png, BinaryData::MergerA01_pngSize);
    Image normalImageB = ImageCache::getFromMemory(BinaryData::MergerA02_png, BinaryData::MergerA02_pngSize);
    Image downImageB = ImageCache::getFromMemory(BinaryData::MergerB02_png, BinaryData::MergerB02_pngSize);

    pipelineSelectorA->setImages(true, true, true,
                                 normalImageA, 1.0f, Colours::white.withAlpha(0.0f),
                                 normalImageA, 1.0f, Colours::black.withAlpha(0.0f),
                                 downImageA, 1.0f, Colours::white.withAlpha(0.0f));


    pipelineSelectorA->addListener(this);
    pipelineSelectorA->setBounds(-10,25,95,50);
    pipelineSelectorA->setToggleState(true, dontSendNotification);
    addAndMakeVisible(pipelineSelectorA.get());

    pipelineSelectorB = std::make_unique<ImageButton>("Pipeline B");

    pipelineSelectorB->setImages(true, true, true,
                                 normalImageB, 1.0f, Colours::white.withAlpha(0.0f),
                                 normalImageB, 1.0f, Colours::black.withAlpha(0.0f),
                                 downImageB, 1.0f, Colours::white.withAlpha(0.0f));

    pipelineSelectorB->addListener(this);
    pipelineSelectorB->setBounds(-10,75,95,50);
    pipelineSelectorB->setToggleState(false, dontSendNotification);
    addAndMakeVisible(pipelineSelectorB.get());

}


void MergerEditor::startAcquisition()
{

}

void MergerEditor::stopAcquisition()
{

}

void MergerEditor::buttonClicked(Button* button)
{
    
    if (button == pipelineSelectorA.get())
    {
        AccessClass::getEditorViewport()->switchIO(getProcessor(), 0);
    }
    else if (button == pipelineSelectorB.get())
    {
        AccessClass::getEditorViewport()->switchIO(getProcessor(), 1);
    }
}

Array<GenericProcessor*> MergerEditor::getSelectableProcessors()
{
    Array<GenericProcessor*> selectableProcessors;
    
    Array<GenericProcessor*> availableProcessors =
        AccessClass::getProcessorGraph()->getListOfProcessors();
    
    if (availableProcessors.size() > 0)
    {
        for (auto& processorToCheck : availableProcessors)
        {
            if (!processorToCheck->isSplitter() &&
                processorToCheck != getProcessor() &&
                processorToCheck->getDestNode() == 0)
            {
                
                bool isDownstream = false;
                GenericProcessor* sourceNode = processorToCheck->getSourceNode();
                    
                while (sourceNode != 0)
                {
                    if (sourceNode == getProcessor())
                    {
                        isDownstream = true;
                        break;
                    }
                    
                    sourceNode = sourceNode->getSourceNode();
                }
                       
                if (!isDownstream)
                {
                    selectableProcessors.add(processorToCheck);
                }
                
            }
        }
    }
    
    return selectableProcessors;
    
}

String MergerEditor::getNameString(GenericProcessor* p)
{
    return p->getName() + " (" + String(p->getNodeId()) + ")";
}

void MergerEditor::mouseDown(const MouseEvent& e)
{

    Merger* merger = (Merger*) getProcessor();

    if (e.mods.isRightButtonDown())
    {

        PopupMenu menu;
        int menuItemIndex = 0;
        int sourceNodeAIndex = -1;
        int sourceNodeBIndex = -1;
        int inputSelectionIndexA = -1;
        int inputSelectionIndexB = -1;
        
        Array<GenericProcessor*> selectableProcessors = getSelectableProcessors();
        
        if (merger->sourceNodeA != 0)
        {
            menu.addItem(++menuItemIndex, // index
            "Input A: ", // message
            false); // isSelectable

            menu.addItem(++menuItemIndex, // index
                getNameString(merger->sourceNodeA), // message
                false); // isSelectable, isTicked

            sourceNodeAIndex = menuItemIndex;

        } else {
            menu.addItem(++menuItemIndex, // index
            "Choose input A:", // message
            false); // isSelectable
            
            if (selectableProcessors.size() > 0)
            {
                inputSelectionIndexA = menuItemIndex + 1;
                
                for (auto& selectableProcessor : selectableProcessors)
                {
                    menu.addItem(++menuItemIndex, // index
                                 getNameString(selectableProcessor), // message
                                 true); // isSelectable
                }
            } else {
                menu.addItem(++menuItemIndex, // index
                             " NONE AVAILABLE", // message
                             false); // isSelectable
            }
            
        }
        
        menu.addItem(++menuItemIndex,
                     " ",
                     false);
        
        if (merger->sourceNodeB != 0)
        {
            menu.addItem(++menuItemIndex, // index
                        "Input B: ", // message
                        false); // isSelectable

            menu.addItem(++menuItemIndex, // index
                getNameString(merger->sourceNodeB), // message
                false); // isSelectable, isTicked

            sourceNodeBIndex = menuItemIndex;

        } else {
            menu.addItem(++menuItemIndex, // index
            "Choose input B:", // message
            false); // isSelectable
            
            if (selectableProcessors.size() > 0)
            {
                inputSelectionIndexB = menuItemIndex + 1;
                
                for (auto& selectableProcessor : selectableProcessors)
                {
                    menu.addItem(++menuItemIndex, // index
                                 getNameString(selectableProcessor), // message
                                 true); // isSelectable
                }
            } else {
                menu.addItem(++menuItemIndex, // index
                " NONE AVAILABLE", // message
                false); // isSelectable
            }
        }

        const int result = menu.show(); // returns 0 if nothing is selected
        
        LOGD("Selection: ", result);

        
        /*if (result == sourceNodeAIndex)
        {
           
            switchSource(0);
            merger->getSourceNode(0)->setDestNode(nullptr);
            merger->setMergerSourceNode(nullptr);
            
            AccessClass::getProcessorGraph()->updateSettings(getProcessor());
            return;
        } else if (result == sourceNodeBIndex)
        {
            
            switchSource(1);
            merger->getSourceNode(1)->setDestNode(nullptr);
            merger->setMergerSourceNode(nullptr);

            AccessClass::getProcessorGraph()->updateSettings(getProcessor());
            return;
        }*/
        
        if (inputSelectionIndexA > 0)
        {
            if (result >= inputSelectionIndexA
                    && result < inputSelectionIndexA + selectableProcessors.size())
            {
                switchSource(0);
                merger->setMergerSourceNode(selectableProcessors[result - inputSelectionIndexA]);
                selectableProcessors[result-inputSelectionIndexA]->setDestNode(merger);

                AccessClass::getProcessorGraph()->updateSettings(getProcessor());
                return;
            }
        }
        
        if (inputSelectionIndexB > 0)
        {
            if (result >= inputSelectionIndexB
                    && result < inputSelectionIndexB + selectableProcessors.size())
            {
                switchSource(1);
                merger->setMergerSourceNode(selectableProcessors[result - inputSelectionIndexB]);
                selectableProcessors[result-inputSelectionIndexB]->setDestNode(merger);

                AccessClass::getProcessorGraph()->updateSettings(getProcessor());
                return;
            }
        }
    }
}


Array<GenericEditor*> MergerEditor::getConnectedEditors()
{

    Array<GenericEditor*> editors;

    Merger* processor = (Merger*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        if (processor->getSourceNode(pathNum) != nullptr)
            editors.add(processor->getSourceNode(pathNum)->getEditor());
        else
            editors.add(nullptr);
    }

    return editors;

}

int MergerEditor::getPathForEditor(GenericEditor* editor)
{
    Merger* processor = (Merger*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        if (processor->getSourceNode(pathNum) != nullptr)
        {
            if (processor->getEditor() == editor)
                return pathNum;
        }
    }

    return -1;

}

void MergerEditor::switchIO(int source)
{
    switchSource(source);

    select();
}


void MergerEditor::switchSource()
{

    bool isBOn = pipelineSelectorB->getToggleState();
    bool isAOn = pipelineSelectorA->getToggleState();

    pipelineSelectorB->setToggleState(!isBOn, dontSendNotification);
    pipelineSelectorA->setToggleState(!isAOn, dontSendNotification);

    Merger* processor = (Merger*) getProcessor();
    processor->switchIO();

}

void MergerEditor::switchSource(int source, bool notify)
{
    if (source == 0)
    {
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);
    }
    else if (source == 1)
    {
        pipelineSelectorB->setToggleState(true, dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);

    }
    else
    {
        return;
    }

    if (notify)
    {
        Merger* processor = (Merger*) getProcessor();
        processor->switchIO(source);
    }


}




bool MergerEditor::checkStream(const DataStream* stream)
{

    // buttons already exist:
    return streamSelector->checkStream(stream);

}


void MergerEditor::updateSettings()
{
    /*for (auto button : streamButtons)
    {
        if (!incomingStreams.contains(button->getStreamId()))
        {
            streamButtonHolder->remove(button);
            streamButtons.removeObject(button);
        }
    }

    incomingStreams.clear();*/
}
