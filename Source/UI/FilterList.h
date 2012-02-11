/*
  ==============================================================================

    FilterList.h
    Created: 1 May 2011 4:01:50pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __FILTERLIST_H_1D6290B7__
#define __FILTERLIST_H_1D6290B7__



#include "../../JuceLibraryCode/JuceHeader.h"

//==============================================================================
// this is the listbox containing the draggable source components




class FilterList : public Component//,
                   //public Button::Listener
{
public:
    FilterList();
    ~FilterList();

    void paint (Graphics& g);
private:
   TreeView* treeView;
   TreeViewItem* rootItem;

   void resized();
   //Button* showHideButton;

};

class ListItem : public TreeViewItem
{
public:
    ListItem(const String, const String, bool);
    ~ListItem();

    void paintItem(Graphics&, int, int);
    //void paintOpenCloseButton (Graphics &g, int width, int height, bool isMouseOver);

    bool mightContainSubItems();
    const String getUniqueName();
    const String getDragSourceDescription();



private:
    bool containsSubItems;
    const String name;
    const String parentName;

};



#endif  // __FILTERLIST_H_1D6290B7__
