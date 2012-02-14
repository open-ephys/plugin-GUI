/*
  ==============================================================================

    FilterList.h
    Created: 4 Feb 2012 6:55:05pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __FILTERLIST_H_C3A661E9__
#define __FILTERLIST_H_C3A661E9__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Visualization/OpenGLCanvas.h"


class FilterListItem;
class UIComponent;

class FilterList : public OpenGLCanvas,
				   public DragAndDropContainer

{

public:

	FilterList();
	~FilterList();
	void newOpenGLContextCreated();
	void renderOpenGL();

	void setUIComponent(UIComponent* ui) {UI = ui;}


	bool isOpen();

private:

	void drawItems();
	void drawItem(FilterListItem*);
	void drawItemName(FilterListItem*);
	void drawButton(bool isOpen);

	FilterListItem* getListItemForYPos(int y);

	void setViewport(bool);

	int getTotalHeight();
	void clearSelectionState();

	bool isDragging;
	int totalHeight, itemHeight, subItemHeight;
	int xBuffer, yBuffer;


	UIComponent* UI;

	String category;
	
	// bool checkBounds(int chan);

	// void setViewport(int chan);
	// void drawBorder(bool isSelected);
	// void drawChannelInfo(int chan, bool isSelected);

	// void drawTicks();

	void resized();
	void mouseDown(const MouseEvent& e);
	void mouseDrag(const MouseEvent& e);
	void mouseMove(const MouseEvent& e);
	void mouseUp(const MouseEvent& e);
	void mouseWheelMove(const MouseEvent&, float, float);

	//bool isDragAndDropActive();
	// void startDragging(const var &sourceDescription, 
	// 				   Component* sourceComponent,
	// 				   const Image &dragImage,
	// 				   bool allowDraggingToOtherJuceWindows,
	// 				   const Point<int>* imageOffsetFromMouse);

	//String getCurrentDragDescription();

	//const String getDragSourceDescription ()

	FilterListItem* baseItem;
	

//	int nChans, plotHeight, totalHeight;
//	int selectedChan;
//	int xBuffer, yBuffer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterList);	

};

class FilterListItem
{
public:
	FilterListItem(const String& name);
	~FilterListItem();

	int getNumSubItems();
	FilterListItem* getSubItem (int index);

	void clearSubItems();
	void addSubItem (FilterListItem* newItem);
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

	Colour color;

private:

	bool selected;
	bool open;
	const String name;
	String parentName;
	OwnedArray<FilterListItem> subItems;
	
};

#endif  // __FILTERLIST_H_C3A661E9__
