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
    editorArray.clear();

    setTabBarDepth(tabDepth);
    setIndent(8); // gap to leave around the edge
    // of the content component
    setColour(TabbedComponent::outlineColourId,
              Colours::darkgrey);
    setColour(TabbedComponent::backgroundColourId,
              Colours::darkgrey);

}

DataViewport::~DataViewport()
{

}

int DataViewport::addTabToDataViewport(String name, Component* component, GenericEditor* editor)
{

    if (tabArray.size() == 0)
        setVisible(true);

    //int tabIndex = getTabbedButtonBar().getNumTabs();
    tabIndex++;

    // Viewport* viewport = new Viewport();
    // viewport->setViewedComponent(component, false);
    //  viewport->setBounds(0,0,getWidth(), getHeight());
    //  viewport->setVisible(true);

    addTab(name, Colours::lightgrey, component, false, tabIndex);

    getTabbedButtonBar().setCurrentTabIndex(tabIndex);

    getTabbedButtonBar().setTabBackgroundColour(tabIndex, Colours::darkgrey);

    setOutline(0);

    tabArray.add(tabIndex);

    editorArray.add(editor);

    LOGDD("Adding tab with index ", tabIndex);

    setCurrentTabIndex(tabArray.size()-1);

    return tabIndex;

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
    editorArray.remove(newIndex); // do this after the editor has been refreshed
    
    removeTab(newIndex);

    if (tabArray.size() == 0)
        setVisible(false);

    setCurrentTabIndex(tabArray.size()-1);

}

void DataViewport::disableConnectionToEditorViewport()
{
    LOGD("DISABLING DATAVIEWPORT CONNECTION");
    shutdown = true;
}

void DataViewport::currentTabChanged(int newIndex, const String& newTabName)
{
    LOGDD("CURRENT TAB CHANGED");
    LOGDD("number of editors remaining: ", editorArray.size());

    if (!shutdown)
    {
        //getEditorViewport()->makeEditorVisible(editorArray[newIndex]);
        getTopLevelComponent()->repaint();
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