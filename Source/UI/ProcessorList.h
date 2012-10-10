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

#ifndef __PROCESSORLIST_H_C3A661E9__
#define __PROCESSORLIST_H_C3A661E9__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Visualization/OpenGLCanvas.h"
#include "../AccessClass.h"

class ProcessorListItem;
class UIComponent;

/**
  
  Holds a list of processors that can be used to build the signal
  chain.

  Must be manually updated every time a new processor is created,
  and the names must match those recognized by the ProcessorGraph.

  @see EditorViewport, ProcessorGraph

*/

class ProcessorList : public OpenGLCanvas,
				   public DragAndDropContainer,
				   public AccessClass,
				   public ChangeListener

{
public:

	ProcessorList();
	~ProcessorList();
	void newOpenGLContextCreated();
	void renderOpenGL();

	//void setUIComponent(UIComponent* ui) {UI = ui;}
	void toggleState();

	void changeListenerCallback(ChangeBroadcaster* source);

	bool isOpen();

private:

	void drawItems();
	void drawItem(ProcessorListItem*);
	void drawItemName(ProcessorListItem*);
	void drawButton(bool isOpen);

	ProcessorListItem* getListItemForYPos(int y);

	void setViewport(bool);


	enum {
		PROCESSOR_COLOR = 801,
		FILTER_COLOR = 802,
		SINK_COLOR = 803,
		SOURCE_COLOR = 804,
		UTILITY_COLOR = 805,
	};

	int currentColor;

	int getTotalHeight();
	void clearSelectionState();

	bool isDragging;
	int totalHeight, itemHeight, subItemHeight;
	int xBuffer, yBuffer;

	String category;
	
	void mouseDownInCanvas(const MouseEvent& e);
	void mouseDragInCanvas(const MouseEvent& e);

	ProcessorListItem* baseItem;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorList);	

};

class ProcessorListItem : public Component
{
public:
	ProcessorListItem(const String& name);
	~ProcessorListItem();

	int getNumSubItems();
	ProcessorListItem* getSubItem (int index);

	void clearSubItems();
	void addSubItem (ProcessorListItem* newItem);
	void removeSubItem (int index);
	bool hasSubItems();

	bool isOpen();
	void setOpen(bool);
	bool isSelected() {return selected;}
	void setSelected(bool b) {selected = b;}

	void reverseOpenState() {open = !open;}

	const String& getName();
	const String& getParentName();
	void setParentName(const String& name);

	//Colour color;
	int colorId;

private:

	bool selected;
	bool open;
	const String name;
	String parentName;
	OwnedArray<ProcessorListItem> subItems;
	
};


#endif  // __PROCESSORLIST_H_C3A661E9__
