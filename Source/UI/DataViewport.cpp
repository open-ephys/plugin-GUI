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

DataViewport::DataViewport() :
    TabbedComponent(TabbedButtonBar::TabsAtRight),
    tabDepth(32), tabIndex(0), shutdown(false)
{

    tabArray.clear();
    tabNameMap.clear();
    tabComponentMap.clear();

    setTabBarDepth(tabDepth);
    setIndent(8); // gap to leave around the edge
                  // of the content component
    setColour(TabbedComponent::outlineColourId,
              Colours::darkgrey);
    setColour(TabbedComponent::backgroundColourId,
              Colours::darkgrey);

}

int DataViewport::addTabToDataViewport(String name,
                                       Component* component)
{

    if (tabArray.size() == 0)
        setVisible(true);

    tabIndex++;

    addTab(name, Colours::lightgrey, component, false, tabIndex);

    getTabbedButtonBar().setTabBackgroundColour(tabIndex, Colours::darkgrey);

    setOutline(0);

    tabArray.add(tabIndex);

    LOGDD("Data Viewport adding tab with index ", tabIndex);

    setCurrentTabIndex(tabArray.size()-1);

    return tabIndex;

}

void DataViewport::addTabAtIndex(int tabIndex, String tabName, Component* tabComponent)
{
    tabNameMap.emplace(tabIndex, tabName);
    tabComponentMap.emplace(tabIndex, tabComponent);
}


void DataViewport::selectTab(int index)
{

    int newIndex = tabArray.indexOf(index);

    getTabbedButtonBar().setCurrentTabIndex(newIndex);

}

void DataViewport::destroyTab(int index)
{

    int newIndex = tabArray.indexOf(index);

    tabArray.remove(newIndex);
    tabIndex--;

    removeTab(newIndex);

    if (tabArray.size() == 0)
        setVisible(false);

    setCurrentTabIndex(tabArray.size()-1);

}

void DataViewport::saveStateToXml(XmlElement* xml)
{
    XmlElement* dataViewportState = xml->createNewChildElement("DATAVIEWPORT");
    dataViewportState->setAttribute("selectedTab", tabArray[getCurrentTabIndex()]);
}

void DataViewport::loadStateFromXml(XmlElement* xml)
{
    for (const auto& tab : tabNameMap) 
    {
        // LOGC("********* ADDING ", tab.second, " to index ", tab.first, " ", tabComponentMap[tab.first]->getName());
        addTabToDataViewport(tab.second, tabComponentMap[tab.first]);
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

    tabNameMap.clear();
    tabComponentMap.clear();

}

void DataViewport::disableConnectionToEditorViewport()
{
    shutdown = true;
}

void DataViewport::currentTabChanged(int newIndex, const String& newTabName)
{
    LOGD("Data Viewport current tab changed; newIndex = ", newIndex);

    if (!shutdown)
    {

        if (getTabContentComponent(newIndex) != nullptr)
        {
            LOGD("Refreshing state for ", newTabName);
            Visualizer* v = (Visualizer*)getTabContentComponent(newIndex);
            v->refreshState();
        }
    }
}

void DataViewport::paint(Graphics& g)
{

    const TabbedButtonBar::Orientation o = getOrientation();

    int x = 0;
    int y = 0;
    int r = getWidth();
    int b = getHeight();

    if (o == TabbedButtonBar::TabsAtTop)
        y += tabDepth;
    else if (o == TabbedButtonBar::TabsAtBottom)
        b -= tabDepth;
    else if (o == TabbedButtonBar::TabsAtLeft)
        x += tabDepth;
    else if (o == TabbedButtonBar::TabsAtRight)
        r -= tabDepth;

    g.setColour(Colour(58,58,58));
    g.fillRoundedRectangle(x,y,r-x,b-y,5.0f);
    g.fillRect(x,y,r-20,b-y);
    g.fillRect(x,20,r-x,b-20);

}
