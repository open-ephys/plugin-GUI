/*
  ==============================================================================

    DataViewport.h
    Created: 26 Jan 2012 12:38:09pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __DATAVIEWPORT_H_B38FE628__
#define __DATAVIEWPORT_H_B38FE628__

#include "../../JuceLibraryCode/JuceHeader.h"


class DataViewport : public TabbedComponent

{
public: 
	DataViewport();
	~DataViewport();

    int addTabToDataViewport(String tabName, Component* componentToAdd);
    void removeTab(int);

private:

	Array<int>* tabArray;

	void paint(Graphics& g);

	int tabDepth;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataViewport);
	
};



#endif  // __DATAVIEWPORT_H_B38FE628__
