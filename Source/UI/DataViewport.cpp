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

#include "DataViewport.h"
#include "EditorViewport.h"
#include "../Processors/Visualization/Visualizer.h"
#include "../Processors/Editors/VisualizerEditor.h"

void CloseTabButton::mouseEnter(const MouseEvent& event)
{
    setVisible(true);
}

void CloseTabButton::mouseExit(const MouseEvent& event)
{
    setVisible(false);
}

void CloseTabButton::paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown)
{

    int xoffset = (getWidth() - 14) /2 ;
    int yoffset = (getHeight() - 14) /2 ;
    
    g.setColour(Colours::white.withAlpha(0.25f));
    
    if (isMouseOverButton)
    {
        g.fillRoundedRectangle(xoffset, yoffset, 14, 14, 4.0f);
    }
    
    g.setColour(Colours::white);
    
    Path path;
    path.addLineSegment(Line<float>(2, 2, 8, 8), 0.0f);
    path.addLineSegment(Line<float>(2, 8, 8, 2), 0.0f);
    
    path.applyTransform(AffineTransform::translation(xoffset + 2, yoffset + 2));
    
    g.strokePath(path, PathStrokeType(1.0f));
}


CustomTabButton::CustomTabButton(const String& name, DraggableTabComponent* parent_, int nodeId_)
: juce::TabBarButton(name, parent_->getTabbedButtonBar()), nodeId(nodeId_), parent(parent_)
{
    CloseTabButton* closeButton = new CloseTabButton();
    closeButton->setBounds(0, 0, 20, 20);
    closeButton->addListener(this);
    
    setExtraComponent(closeButton, TabBarButton::ExtraComponentPlacement::afterText);
    
    closeButton->setVisible(false);
    
}


bool CustomTabButton::isInterestedInDragSource(const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String("Tab").equalsIgnoreCase(descr->getUnchecked(0));
    int incomingNodeId = descr->getUnchecked(1);
    
    return tabDrag && nodeId != incomingNodeId;
}

void CustomTabButton::mouseEnter(const MouseEvent& event)
{
    getExtraComponent()->setVisible(true);
}

void CustomTabButton::mouseExit(const MouseEvent& event)
{
    
    getExtraComponent()->setVisible(false);
}

void CustomTabButton::mouseDrag(const MouseEvent& event)
{
    
    DragAndDropContainer* const dragContainer
        = DragAndDropContainer::findParentDragContainerFor(this);
    
    Array<var> dragData;
    dragData.add("Tab");
    dragData.add(nodeId);
    dragData.add(getName());

    dragContainer->startDragging(dragData, this);
    
}

void CustomTabButton::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    isDraggingOver = true;
    
    repaint();
    
    LOGD("Entering ", getName());
}


void CustomTabButton::itemDragExit(const SourceDetails& dragSourceDetails)
{
    isDraggingOver = false;
    
    repaint();
    
    LOGD("Exiting ", getName());
}

void CustomTabButton::itemDropped(const SourceDetails& dragSourceDetails)
{
    LOGD("DROPPED ON ", getName());
    
    isDraggingOver = false;
    
    repaint();
    
    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String("Tab").equalsIgnoreCase(descr->getUnchecked(0));
    int incomingNodeId = descr->getUnchecked(1);
    String name = descr->getUnchecked(2);

    LOGD("ITEM DROPPED ON TAB");
    
    parent->moveTabAfter(incomingNodeId, name, nodeId);

}

void CustomTabButton::paintButton(Graphics& g,
                                  bool isMouseOver,
                                  bool isMouseDown)
{
    
    
    getLookAndFeel().drawTabButton(*this, g, isMouseOver, isMouseDown);
    
    if (isDraggingOver)
    {
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillAll();
    }
    
}


void CustomTabButton::buttonClicked(Button* button)
{
    LOGD("CLOSE BUTTON PRESSED");
    
    parent->dataViewport->removeTab(nodeId);
}

DraggableTabComponent::DraggableTabComponent(DataViewport* parent_) :
    TabbedComponent(TabbedButtonBar::TabsAtRight), dataViewport(parent_)
{
    setTabBarDepth(28);
    setOutline(0);
    setIndent(8); // gap to leave around the edge
                  // of the content component

}


bool DraggableTabComponent::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String("Tab").equalsIgnoreCase(descr->getUnchecked(0));
    int incomingNodeId = descr->getUnchecked(1);
    
    if (getNumTabs() == 0)
        return tabDrag;
    else
        return tabDrag && tabNodeIds.getLast() != incomingNodeId;
}


void DraggableTabComponent::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    
    isDraggingOver = true;
    repaint();
    
    LOGD("Entering PARENT");
}


void DraggableTabComponent::itemDragExit(const SourceDetails& dragSourceDetails)
{
    
    isDraggingOver = false;
    repaint();
    
    LOGD("Exiting PARENT");
}


void DraggableTabComponent::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{

    isDraggingOver = false;
    repaint();
    
    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String("Tab").equalsIgnoreCase(descr->getUnchecked(0));
    int incomingNodeId = descr->getUnchecked(1);
    String name = descr->getUnchecked(2);

    LOGD("ITEM DROPPED ON PARENT");
    
    Component* contentComponent = dataViewport->getContentComponentForNodeId(incomingNodeId);
    dataViewport->removeTab(incomingNodeId, false);
    addNewTab(name, contentComponent, incomingNodeId);
    
    dataViewport->resized();
    
}

void DraggableTabComponent::addNewTab(String name, Component* component, int nodeId)
{
    
    tabNodeIds.add(nodeId);
    
    addTab(name, Colours::darkgrey, component, false, tabNodeIds.size()-1);

    setCurrentTabIndex(tabNodeIds.size()-1);

}

bool DraggableTabComponent::removeTabForNodeId(int nodeId, bool sendNotification)
{
    LOGD("REMOVING TAB");
    
    int index = tabNodeIds.indexOf(nodeId);
    
    LOGD("INDEX: ", index);
    
    if (index > -1)
    {
        removeTab(index);
        tabNodeIds.remove(index);
        setCurrentTabIndex(tabNodeIds.size()-1);
        
        if (sendNotification)
        {
            if (nodeId > 99)
            {
                VisualizerEditor* editor = (VisualizerEditor*) AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId)->getEditor();
                
                editor->tabWasClosed();
            } else {
                
                if (nodeId == 0)
                    AccessClass::getUIComponent()->closeInfoTab();
                else if (nodeId == 1)
                    AccessClass::getUIComponent()->closeGraphViewer();
            }
        }
        
        return true;
    } else {
        return false;
    }
}

void DraggableTabComponent::moveTabAfter(int incomingNodeId, String name, int localNodeId)
{
    
    Component* contentComponent = dataViewport->getContentComponentForNodeId(incomingNodeId);
    dataViewport->removeTab(incomingNodeId, false);
    
    int index = tabNodeIds.indexOf(localNodeId) + 1;
    
    tabNodeIds.insert(index, incomingNodeId);
    
    addTab(name, Colours::darkcyan, contentComponent, false, index);

    setCurrentTabIndex(index);
    
}

Component* DraggableTabComponent::getContentComponentForNodeId(int nodeId)
{
    int index = tabNodeIds.indexOf(nodeId);
    
    if (index > -1)
        return getTabContentComponent(index);
    else
        return nullptr;
}


void DraggableTabComponent::selectTab(int nodeId)
{
    int index = tabNodeIds.indexOf(nodeId);
    
    if (index > -1)
    {
        setCurrentTabIndex(index);
    }
}

void DraggableTabComponent::paint(Graphics& g)
{


    const TabbedButtonBar::Orientation o = getOrientation();

    int x = 0;
    int y = 0;
    int r = getWidth();
    int b = getHeight();
    
    if (isDraggingOver)
    {
        g.setColour(Colour(100,100,100));
        
        g.fillAll();
    }

    if (o == TabbedButtonBar::TabsAtTop)
        y += 32;
    else if (o == TabbedButtonBar::TabsAtBottom)
        b -= 32;
    else if (o == TabbedButtonBar::TabsAtLeft)
        x += 32;
    else if (o == TabbedButtonBar::TabsAtRight)
        r -= 32;

    g.setColour(Colour(58,58,58));
    g.fillRoundedRectangle(x,y,r-x,b-y,5.0f);
    g.fillRect(x,y,r-20,b-y);
    g.fillRect(x,20,r-x,b-20);
    

}


void DraggableTabComponent::currentTabChanged(int newIndex, const String& newTabName)
{

    if (!shutdown)
    {
        if (getTabContentComponent(newIndex) != nullptr && tabNodeIds[newIndex] > 99)
        {
            LOGD("Refreshing state for ", newTabName);
            Visualizer* v = (Visualizer*) getTabContentComponent(newIndex);
            v->refreshState();
        }
    }
}


AddTabbedComponentButton::AddTabbedComponentButton()
: Button("Add Tabbed Component")
{
    
    path.addRoundedRectangle(3, 3, 14, 14, 3.0f);
    path.addLineSegment(Line<float>(7, 3, 7, 17), 0.0f);
    path.addLineSegment(Line<float>(12, 8, 12, 12), 0.0f);
    path.addLineSegment(Line<float>(10, 10, 14, 10), 0.0f);
    
}

void AddTabbedComponentButton::paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown)
{


    if (isMouseOverButton)
    {
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillRoundedRectangle(0, 0, 20, 20, 5.0f);
        
        
    }
    
    g.setColour(Colour(150,150,150));
    g.strokePath(path, PathStrokeType(1.0f));
    
}


DataViewport::DataViewport() :
    shutdown(false)
{

    DraggableTabComponent* c = new DraggableTabComponent(this);
    addAndMakeVisible(c);
    draggableTabComponents.add(c);
    
    addTabbedComponentButton = std::make_unique<AddTabbedComponentButton>();
    addAndMakeVisible(addTabbedComponentButton.get());
    addTabbedComponentButton->addListener(this);

}

void DataViewport::resized()
{
    
    int width = getWidth() / draggableTabComponents.size();
    
    for (int i = 0; i < draggableTabComponents.size(); i++)
    {
        draggableTabComponents[i]->setBounds(width * i, 0, width, getHeight());
    }
    
    addTabbedComponentButton->setBounds(getWidth() - 26, getHeight() - 26, 20, 20);
    addTabbedComponentButton->toFront(false);
    
    if (draggableTabComponents[activeTabbedComponent]->getNumTabs() > 1 && activeTabbedComponent < 2)
        addTabbedComponentButton->setVisible(true);
    else
        addTabbedComponentButton->setVisible(false);
}

void DataViewport::addTab(String name,
                          Component* component,
                          int nodeId)
{

    draggableTabComponents[activeTabbedComponent]->addNewTab(name, component, nodeId);
    
    resized();

}

void DataViewport::addTabAtIndex(int tabIndex_, String tabName, Component* tabComponent)
{

    /*if (!savedTabIndices.contains(tabIndex_))
	{
        savedTabIndices.add(tabIndex_);
        savedTabComponents.add(tabComponent);
        savedTabNames.add(tabName);
	}*/

}


void DataViewport::selectTab(int nodeId)
{

    for (auto draggableTabComponent : draggableTabComponents)
        draggableTabComponent->selectTab(nodeId);

}

void DataViewport::removeTab(int nodeId, bool sendNotification)
{
    
    if (!shutdown)
    {
        for (auto draggableTabComponent : draggableTabComponents)
        {
            bool foundTab = draggableTabComponent->removeTabForNodeId(nodeId, sendNotification);
            
            LOGD(foundTab, " ", draggableTabComponent->getNumTabs(), " ", draggableTabComponents.size());
            
            if (foundTab)
            {
                
                if (draggableTabComponent->getNumTabs() == 0 && draggableTabComponents.size() > 1)
                {
                    draggableTabComponents.removeObject(draggableTabComponent);
                    activeTabbedComponent--;
                    resized();
                    
                    if (draggableTabComponents[activeTabbedComponent]->getNumTabs() > 1)
                        addTabbedComponentButton->setVisible(true);
                    else
                        addTabbedComponentButton->setVisible(false);
                }
                
                return;
            }
        }
            
    }

}


void DataViewport::buttonClicked(Button* button)
{
    if (button == addTabbedComponentButton.get())
    {
        DraggableTabComponent* d = new DraggableTabComponent(this);
        addAndMakeVisible(d);
        draggableTabComponents.add(d);
        
        resized();
        
        activeTabbedComponent++;
        
        addTabbedComponentButton->setVisible(false);
    }
}

Component* DataViewport::getContentComponentForNodeId(int nodeId)
{
    for (auto draggableTabComponent : draggableTabComponents)
    {
        Component* c = draggableTabComponent->getContentComponentForNodeId(nodeId);
        
        if (c != nullptr)
            return c;
    }
    
    return nullptr;
        
}

Component* DataViewport::getActiveTabContentComponent()
{
    draggableTabComponents.getFirst()->getCurrentContentComponent();
}

void DataViewport::saveStateToXml(XmlElement* xml)
{
    /*XmlElement* dataViewportState = xml->createNewChildElement("DATAVIEWPORT");
    dataViewportState->setAttribute("selectedTab", tabArray[draggableTabComponents.getFirst()->getCurrentTabIndex()]);*/
}

void DataViewport::loadStateFromXml(XmlElement* xml)
{

    /*std::vector<int> tabOrder(savedTabIndices.size());
    std::iota(tabOrder.begin(), tabOrder.end(), 0); //Initializing
    sort(tabOrder.begin(), tabOrder.end(), [&](int i, int j)
        {return savedTabIndices[i] < savedTabIndices[j]; });

    for (int i = 0; i < tabOrder.size(); i++)
    {
        tabIndex = savedTabIndices[tabOrder[i]];
        addTabToDataViewport(savedTabNames[tabOrder[i]], savedTabComponents[tabOrder[i]]);
    }

    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName("DATAVIEWPORT"))
        {
			int index = xmlNode->getIntAttribute("selectedTab", -1);
            if (index != -1)
                selectTab(index);
        }

    }

    savedTabIndices.clear();
    savedTabComponents.clear();
    savedTabNames.clear();*/

}

void DataViewport::disableConnectionToEditorViewport()
{
    shutdown = true;
}


