/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "../Processors/Editors/VisualizerEditor.h"
#include "../Processors/Visualization/Visualizer.h"
#include "EditorViewport.h"

void CloseTabButton::mouseEnter (const MouseEvent& event)
{
    setVisible (true);
}

void CloseTabButton::mouseExit (const MouseEvent& event)
{
    setVisible (false);
}

void CloseTabButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    int xoffset = (getWidth() - 14) / 2;
    int yoffset = (getHeight() - 14) / 2;

    g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.25f));

    if (isMouseOverButton)
    {
        g.fillRoundedRectangle (xoffset, yoffset, 14, 14, 4.0f);
    }

    g.setColour (findColour (ThemeColours::defaultText));

    Path path;
    path.addLineSegment (Line<float> (2, 2, 8, 8), 0.0f);
    path.addLineSegment (Line<float> (2, 8, 8, 2), 0.0f);

    path.applyTransform (AffineTransform::translation (xoffset + 2, yoffset + 2));

    g.strokePath (path, PathStrokeType (1.0f));
}

CustomTabButton::CustomTabButton (const String& name, DraggableTabComponent* parent_, int nodeId_)
    : juce::TabBarButton (name, parent_->getTabbedButtonBar()), nodeId (nodeId_), parent (parent_)
{
    CloseTabButton* closeButton = new CloseTabButton();
    closeButton->setBounds (0, 0, 20, 20);
    closeButton->addListener (this);

    setExtraComponent (closeButton, TabBarButton::ExtraComponentPlacement::afterText);

    closeButton->setVisible (false);
}

bool CustomTabButton::isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String ("Tab").equalsIgnoreCase (descr->getUnchecked (0));
    int incomingNodeId = descr->getUnchecked (1);

    return tabDrag && nodeId != incomingNodeId;
}

void CustomTabButton::mouseEnter (const MouseEvent& event)
{
    getExtraComponent()->setVisible (true);
}

void CustomTabButton::mouseExit (const MouseEvent& event)
{
    getExtraComponent()->setVisible (false);
}

void CustomTabButton::mouseDrag (const MouseEvent& event)
{
    DragAndDropContainer* const dragContainer = DragAndDropContainer::findParentDragContainerFor (this);

    Array<var> dragData;
    dragData.add ("Tab");
    dragData.add (nodeId);
    dragData.add (getName());

    dragContainer->startDragging (dragData, this);
}

void CustomTabButton::itemDragEnter (const SourceDetails& dragSourceDetails)
{
    isDraggingOver = true;

    repaint();

    LOGD ("Entering ", getName());
}

void CustomTabButton::itemDragExit (const SourceDetails& dragSourceDetails)
{
    isDraggingOver = false;

    repaint();

    LOGD ("Exiting ", getName());
}

void CustomTabButton::itemDropped (const SourceDetails& dragSourceDetails)
{
    LOGD ("DROPPED ON ", getName());

    isDraggingOver = false;

    repaint();

    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String ("Tab").equalsIgnoreCase (descr->getUnchecked (0));
    int incomingNodeId = descr->getUnchecked (1);
    String name = descr->getUnchecked (2);

    LOGD ("ITEM DROPPED ON TAB");

    parent->moveTabAfter (incomingNodeId, name, nodeId);
}

void CustomTabButton::paintButton (Graphics& g,
                                   bool isMouseOver,
                                   bool isMouseDown)
{
    getTabbedButtonBar().setTabBackgroundColour (getIndex(), findColour (ThemeColours::componentBackground));

    getLookAndFeel().drawTabButton (*this, g, isMouseOver, isMouseDown);

    if (isDraggingOver)
    {
        g.setColour (Colours::white.withAlpha (0.25f));
        g.fillAll();
    }
}

void CustomTabButton::buttonClicked (Button* button)
{
    LOGD ("CLOSE BUTTON PRESSED");

    parent->dataViewport->removeTab (nodeId);
}

DraggableTabComponent::DraggableTabComponent (DataViewport* parent_) : TabbedComponent (TabbedButtonBar::TabsAtRight), dataViewport (parent_)
{
    setTabBarDepth (28);
    setOutline (0);
    setIndent (5); // gap to leave around the edge of the content component

    Path closeButtonPath;
    closeButtonPath.addLineSegment (Line<float> (0, 0, 12, 12), 1.0f);
    closeButtonPath.addLineSegment (Line<float> (0, 12, 12, 0), 1.0f);

    closeButton = std::make_unique<ShapeButton> ("X", Colours::black, Colours::black, Colours::black);
    closeButton->setShape (closeButtonPath, false, false, false);
    closeButton->addListener (this);
    addAndMakeVisible (closeButton.get());

    if (dataViewport->getNumTabbedComponents() == 0)
        closeButton->setVisible (false);
}

bool DraggableTabComponent::isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String ("Tab").equalsIgnoreCase (descr->getUnchecked (0));
    int incomingNodeId = descr->getUnchecked (1);

    if (getNumTabs() == 0)
        return tabDrag;
    else
        return tabDrag && tabNodeIds.getLast() != incomingNodeId;
}

void DraggableTabComponent::itemDragEnter (const SourceDetails& dragSourceDetails)
{
    isDraggingOver = true;
    repaint();

    LOGD ("Entering PARENT");
}

void DraggableTabComponent::itemDragExit (const SourceDetails& dragSourceDetails)
{
    isDraggingOver = false;
    repaint();

    LOGD ("Exiting PARENT");
}

void DraggableTabComponent::itemDropped (const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    isDraggingOver = false;
    repaint();

    Array<var>* descr = dragSourceDetails.description.getArray();
    bool tabDrag = String ("Tab").equalsIgnoreCase (descr->getUnchecked (0));
    int incomingNodeId = descr->getUnchecked (1);
    String name = descr->getUnchecked (2);

    LOGD ("ITEM DROPPED ON PARENT");

    Component* contentComponent = dataViewport->getContentComponentForNodeId (incomingNodeId);
    dataViewport->removeTab (incomingNodeId, false);
    addNewTab (name, contentComponent, incomingNodeId);

    dataViewport->resized();
}

void DraggableTabComponent::addNewTab (String name, Component* component, int nodeId)
{
    tabNodeIds.add (nodeId);

    addTab (name, Colours::darkgrey, component, false, tabNodeIds.size() - 1);

    setCurrentTabIndex (tabNodeIds.size() - 1);

    closeButton->setVisible (false);
}

bool DraggableTabComponent::removeTabForNodeId (int nodeId, bool sendNotification)
{
    LOGD ("REMOVING TAB");

    int index = tabNodeIds.indexOf (nodeId);

    LOGD ("INDEX: ", index);

    if (index > -1)
    {
        removeTab (index);
        tabNodeIds.remove (index);
        setCurrentTabIndex (tabNodeIds.size() - 1);

        if (dataViewport->getNumTabbedComponents() == 1 && getNumTabs() == 0)
            closeButton->setVisible (false);

        if (sendNotification)
        {
            if (nodeId > 99)
            {
                VisualizerEditor* editor = (VisualizerEditor*) AccessClass::getProcessorGraph()->getProcessorWithNodeId (nodeId)->getEditor();

                editor->tabWasClosed();
            }
            else
            {
                if (nodeId == 0)
                    AccessClass::getUIComponent()->closeInfoTab();
                else if (nodeId == 1)
                    AccessClass::getUIComponent()->closeGraphViewer();
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

void DraggableTabComponent::moveTabAfter (int incomingNodeId, String name, int localNodeId)
{
    Component* contentComponent = dataViewport->getContentComponentForNodeId (incomingNodeId);
    dataViewport->removeTab (incomingNodeId, false);

    int index = tabNodeIds.indexOf (localNodeId) + 1;

    tabNodeIds.insert (index, incomingNodeId);

    addTab (name, Colours::darkgrey, contentComponent, false, index);

    setCurrentTabIndex (index);
}

Component* DraggableTabComponent::getContentComponentForNodeId (int nodeId)
{
    int index = tabNodeIds.indexOf (nodeId);

    if (index > -1)
        return getTabContentComponent (index);
    else
        return nullptr;
}

void DraggableTabComponent::selectTab (int nodeId)
{
    int index = tabNodeIds.indexOf (nodeId);

    if (index > -1)
    {
        setCurrentTabIndex (index);
    }
}

void DraggableTabComponent::paint (Graphics& g)
{
    const TabbedButtonBar::Orientation o = getOrientation();

    int x = 0;
    int y = 0;
    int r = getWidth();
    int b = getHeight();

    if (o == TabbedButtonBar::TabsAtTop)
        y += 28;
    else if (o == TabbedButtonBar::TabsAtBottom)
        b -= 28;
    else if (o == TabbedButtonBar::TabsAtLeft)
        x += 28;
    else if (o == TabbedButtonBar::TabsAtRight)
        r -= 28;

    Rectangle<float> bounds (x, y, r - x, b - y);

    g.setColour (Colours::black.withAlpha (getTabbedButtonBar().isEnabled() ? 0.15f : 0.08f));
    g.fillRect (bounds.withTrimmedLeft (10.0f));

    if (isDraggingOver)
    {
        g.setColour (findColour (ThemeColours::highlightedFill).withAlpha (0.5f));

        g.fillRect (getTabbedButtonBar().getBounds());
    }

    g.setColour (findColour (ThemeColours::componentBackground));
    g.fillRoundedRectangle (bounds.reduced (1.0f), 5.0f);

    g.setColour (findColour (
                     isDraggingOver ? ThemeColours::highlightedFill : ThemeColours::outline)
                     .withAlpha (0.5f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), 5.0f, 2.0f);

    if (getNumTabs() == 0)
    {
        closeButton->setColours (findColour (ThemeColours::defaultText),
                                 findColour (ThemeColours::defaultText).withAlpha (0.5f),
                                 findColour (ThemeColours::highlightedText));

        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
        g.setFont (FontOptions ("Inter", "Medium", 20.0f));

        String text = dataViewport->getNumTabbedComponents() == 1 ? "Open a new tab to show here" : "Drag a tab here or open a new tab";
        g.drawFittedText (text, bounds.toNearestInt(), Justification::centred, 2);
    }
}

void DraggableTabComponent::resized()
{
    TabbedComponent::resized();

    if (getWidth() > 50 && getHeight() > 25)
    {
        closeButton->setBounds (getWidth() - 48, 8, 12, 12);
    }
}

void DraggableTabComponent::currentTabChanged (int newIndex, const String& newTabName)
{
    if (! shutdown)
    {
        if (getTabContentComponent (newIndex) != nullptr && tabNodeIds[newIndex] > 99)
        {
            LOGD ("Refreshing state for ", newTabName);
            Visualizer* v = (Visualizer*) getTabContentComponent (newIndex);
            v->refreshState();
        }
    }
}

void DraggableTabComponent::buttonClicked (Button* button)
{
    if (button == closeButton.get())
    {
        dataViewport->removeTabbedComponent (this);
    }
}

AddTabbedComponentButton::AddTabbedComponentButton()
    : Button ("Add Tabbed Component")
{
    path.addRoundedRectangle (1, 1, 18, 18, 3.0f);
    path.addLineSegment (Line<float> (9, 1, 9, 18), 0.0f);
    path.addTriangle (12, 7, 12, 13, 17, 10);
}

void AddTabbedComponentButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    Colour btnColour = findColour (ThemeColours::windowBackground).contrasting (0.5f);

    if (isMouseOverButton)
    {
        g.setColour (btnColour.withAlpha (0.5f));
    }
    else
    {
        g.setColour (btnColour);
    }

    g.strokePath (path, PathStrokeType (1.0f));
}

DataViewport::DataViewport() : shutdown (false)
{
    DraggableTabComponent* c = new DraggableTabComponent (this);
    addAndMakeVisible (c);
    draggableTabComponents.add (c);

    addTabbedComponentButton = std::make_unique<AddTabbedComponentButton>();
    addAndMakeVisible (addTabbedComponentButton.get());
    addTabbedComponentButton->addListener (this);
}

void DataViewport::resized()
{
    int width = getWidth() / draggableTabComponents.size();

    for (int i = 0; i < draggableTabComponents.size(); i++)
    {
        draggableTabComponents[i]->setBounds (width * i, 0, width, getHeight());
    }

    addTabbedComponentButton->setBounds (getWidth() - 24, getHeight() - 26, 20, 20);
    addTabbedComponentButton->toFront (false);

    if (draggableTabComponents[activeTabbedComponent]->getNumTabs() > 1 && activeTabbedComponent < 2)
        addTabbedComponentButton->setVisible (true);
    else
        addTabbedComponentButton->setVisible (false);
}

void DataViewport::addTab (String name,
                           Component* component,
                           int nodeId)
{
    draggableTabComponents[activeTabbedComponent]->addNewTab (name, component, nodeId);

    resized();
}

void DataViewport::selectTab (int nodeId)
{
    for (auto draggableTabComponent : draggableTabComponents)
        draggableTabComponent->selectTab (nodeId);
}

void DataViewport::removeTab (int nodeId, bool sendNotification)
{
    if (! shutdown)
    {
        for (auto draggableTabComponent : draggableTabComponents)
        {
            bool foundTab = draggableTabComponent->removeTabForNodeId (nodeId, sendNotification);

            LOGD (foundTab, " ", draggableTabComponent->getNumTabs(), " ", draggableTabComponents.size());

            if (foundTab)
            {
                if (draggableTabComponent->getNumTabs() == 0 && draggableTabComponents.size() > 1)
                {
                    draggableTabComponents.removeObject (draggableTabComponent);
                    activeTabbedComponent--;
                    resized();

                    if (draggableTabComponents[activeTabbedComponent]->getNumTabs() > 1)
                        addTabbedComponentButton->setVisible (true);
                    else
                        addTabbedComponentButton->setVisible (false);
                }

                return;
            }
        }
    }
}

void DataViewport::buttonClicked (Button* button)
{
    if (button == addTabbedComponentButton.get())
    {
        DraggableTabComponent* d = new DraggableTabComponent (this);
        addAndMakeVisible (d);
        draggableTabComponents.add (d);

        resized();

        activeTabbedComponent++;

        addTabbedComponentButton->setVisible (false);
    }
}

Component* DataViewport::getContentComponentForNodeId (int nodeId)
{
    for (auto draggableTabComponent : draggableTabComponents)
    {
        Component* c = draggableTabComponent->getContentComponentForNodeId (nodeId);

        if (c != nullptr)
            return c;
    }

    return nullptr;
}

Component* DataViewport::getActiveTabContentComponent()
{
    return draggableTabComponents.getFirst()->getCurrentContentComponent();
}

void DataViewport::removeTabbedComponent (DraggableTabComponent* draggableTabComponent)
{
    if (draggableTabComponents.size() > 1 && draggableTabComponent->getNumTabs() == 0)
    {
        draggableTabComponents.removeObject (draggableTabComponent);

        if (activeTabbedComponent > draggableTabComponents.size() - 1)
            activeTabbedComponent--;

        resized();
    }
}

void DataViewport::saveStateToXml (XmlElement* xml)
{
    XmlElement* dataViewportState = xml->createNewChildElement ("DATAVIEWPORT");

    // save tab order in each draggableTabComponent
    for (int i = 0; i < draggableTabComponents.size(); i++)
    {
        XmlElement* tabOrder = dataViewportState->createNewChildElement ("TABBEDCOMPONENT");
        tabOrder->setAttribute ("index", i);

        Array<int> tabNodeIds = draggableTabComponents[i]->getTabNodeIds();
        tabOrder->setAttribute ("selectedTabNodeId",
                                tabNodeIds[draggableTabComponents[i]->getCurrentTabIndex()]);

        for (int j = 0; j < draggableTabComponents[i]->getNumTabs(); j++)
        {
            XmlElement* tab = tabOrder->createNewChildElement ("TAB");
            tab->setAttribute ("nodeId", tabNodeIds[j]);
        }
    }
}

void DataViewport::loadStateFromXml (XmlElement* xml)
{
    auto* dvXml = xml->getChildByName ("DATAVIEWPORT");

    if (dvXml != nullptr)
    {
        LOGD ("Loading DataViewport state from XML...");
        for (auto* xmlNode : dvXml->getChildIterator())
        {
            if (xmlNode->hasTagName ("TABBEDCOMPONENT"))
            {
                int index = xmlNode->getIntAttribute ("index", -1);
                int selectedTab = xmlNode->getIntAttribute ("selectedTabNodeId", -1);

                if (index > -1)
                {
                    // add new tabbed component if necessary
                    if ((draggableTabComponents.size() - 1) < index)
                    {
                        LOGD ("Adding new tabbed component")
                        DraggableTabComponent* d = new DraggableTabComponent (this);
                        addAndMakeVisible (d);
                        draggableTabComponents.add (d);
                    }

                    activeTabbedComponent = index;

                    int tabIndex = -1;

                    // add tabs to the tabbed component
                    for (auto* tab : xmlNode->getChildIterator())
                    {
                        if (tab->hasTagName ("TAB"))
                        {
                            tabIndex++;
                            int nodeId = tab->getIntAttribute ("nodeId", -1);

                            LOGD ("Adding tab ", nodeId, " to tabbed component ", index);

                            if (nodeId == 0) // info tab
                                AccessClass::getUIComponent()->addInfoTab();
                            else if (nodeId == 1) // graph tab
                                AccessClass::getUIComponent()->addGraphTab();
                            else if (nodeId > 99) // visualizer tab
                            {
                                GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId (nodeId);
                                if (processor != nullptr)
                                {
                                    if (processor->getEditor()->isVisualizerEditor())
                                    {
                                        VisualizerEditor* editor = (VisualizerEditor*) processor->getEditor();
                                        editor->addTab();
                                    }
                                }
                            }
                        }
                    }

                    resized();

                    // select the tab in the tabbed component
                    draggableTabComponents[index]->selectTab (selectedTab);
                }
            }
        }

        LOGD ("DataViewport state loaded.");
    }
}

void DataViewport::disableConnectionToEditorViewport()
{
    shutdown = true;
}
