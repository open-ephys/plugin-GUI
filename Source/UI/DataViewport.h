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

#ifndef __DATAVIEWPORT_H_B38FE628__
#define __DATAVIEWPORT_H_B38FE628__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../AccessClass.h"
#include "../TestableExport.h"

class GenericEditor;

/**
 
 Custom button to remove a tab

*/
class CloseTabButton : public juce::Button
{
public:
    
    /** Constructor */
    CloseTabButton() : Button("Close Tab") { }
    
    /** Renders the button */
    void paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    
    /** Called when mouse enters  **/
    void mouseEnter(const MouseEvent& event) override;
    
    /** Called when mouse enters  **/
    void mouseExit(const MouseEvent& event) override;
    
    
};

/**
 
    Custom tab  button that's draggable, and has a close button
 
 */
class CustomTabButton : public TabBarButton,
    public DragAndDropTarget,
public Button::Listener
{
public:
    
    /** Constructor */
    CustomTabButton(const String& name, TabbedButtonBar& ownerBar);
    
    /** Determines whether a dragged component will affect this one  */
    bool isInterestedInDragSource(const DragAndDropTarget::SourceDetails& dragSourceDetails);
    
    /** Called when mouse enters  **/
    void mouseEnter(const MouseEvent& event) override;
    
    /** Called when mouse exits  **/
    void mouseExit(const MouseEvent& event) override;
    
    /** Called when mouse drag is active **/
    void mouseDrag(const MouseEvent& event) override;
    
    /** Called when item is dropped **/
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    
    /** Called when item is dropped **/
    void itemDragExit(const SourceDetails& dragSourceDetails) override;

    /** Called when item is dropped **/
    void itemDropped(const SourceDetails& dragSourceDetails) override;
    

    /** Override button painting method */
    void paintButton(juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
    
    /** Called when close button is pressed */
    void buttonClicked(Button* button);
    
    /** Called when tab is dragged */
    //void mouseDrag(const MouseEvent& event) override;
    
    /** Gets the tab length*/
    //int getBestTabLength(int depth) override;
    
private:
    
    bool isDraggingOver = false;
    
   // std::unique_ptr<CloseTabButton> closeButton;

};


/**

  Holds tabs containing the application's visualizers.

  This class is a subclass of juce_TabbedComponent.h

  @see GenericEditor, InfoLabel, LfpDisplayCanvas

*/

class DraggableTabComponent :
    public TabbedComponent,
    public DragAndDropTarget
{
public:
    
    /** Constructor */
    DraggableTabComponent();
    
    /** Paint**/
    void paint(Graphics& g);
    
    /** Determines whether a dragged component will affect this one  */
    bool isInterestedInDragSource(const DragAndDropTarget::SourceDetails& dragSourceDetails);
    
    /** Called when item is dropped **/
    void itemDropped(const DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    
    /** Called when item is dropped **/
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    
    /** Called when item is dropped **/
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    
    /** Create buttons*/
    TabBarButton* createTabButton(const juce::String& name, int index) override {
            return new CustomTabButton(name, getTabbedButtonBar());
    }
    
private:
    
    bool isDraggingOver = false;
    
    int draggedTabStartIndex = -1;

    void moveTab(int fromIndex, int toIndex);
    
};

/**
 
 Custom button to add a tabbed component

*/
class AddTabbedComponentButton : public juce::Button
{
public:
    
    /** Constructor */
    AddTabbedComponentButton();
    
    /** Destructor */
    ~AddTabbedComponentButton() { }
    
    /** Renders the button */
    void paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    
private:
    
    Path path;
    
};

/**

  The DataViewport sits in the center of the MainWindow
  and is always visible. Editors that create data
  visualizations can place them in the
  DataViewport for easy access on small monitors, or in
  a separate window for maximum flexibility.

  @see GenericEditor, InfoLabel, LfpDisplayCanvas

*/

class TESTABLE DataViewport : public Component,
    public DragAndDropContainer,
    public Button::Listener
{
public:
    
    /** Constructor*/
    DataViewport();
    
    /** Destructor*/
    ~DataViewport() { }

    /** Adds a new visualizer within a tab and returns the tab index.*/
    int addTabToDataViewport(String tabName, Component* componentToAdd);

    /** [ONLY USED WHEN LOADING A CONFIG] Adds a new visualizer within a tab at the speicifed tab index.*/
    void addTabAtIndex(int index, String tabName, Component* componentToAdd);

    /** Removes a tab with a specified index.*/
    void destroyTab(int);

    /** Selects a tab with a specified index.*/
    void selectTab(int);
    
    /** Sets layout of sub-compnents .*/
    void resized();
    
    /** Called to add another tabbed component */
    void buttonClicked(Button* button);

    /** Save settings.*/
    void saveStateToXml(XmlElement* xml);

    /** Load settings.*/
    void loadStateFromXml(XmlElement* xml);

    /** Informs the component within the current tab that it's now active.*/
    void currentTabChanged(int newIndex, const String& newTabName);

    /** Prevents the DataViewport from signaling EditorViewport when changing tabs.*/
    void disableConnectionToEditorViewport();
    
    /** Returns the current content component */
    Component* getActiveTabContentComponent();

private:

    /** Maps original tab indices to their location within the DataViewport. */
    Array<int> tabArray;

    /** Maps processors to their respective tabs within the DataViewport. */
    Array<int> savedTabIndices;
    Array<String> savedTabNames;
    Array<Component*> savedTabComponents;
    
    /** Tabbed sub-components **/
    OwnedArray<DraggableTabComponent> draggableTabComponents;
    
    /** Button to add a tab component */
    std::unique_ptr<AddTabbedComponentButton> addTabbedComponentButton;

    int tabIndex;

    bool shutdown;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataViewport);

};



#endif  // __DATAVIEWPORT_H_B38FE628__
