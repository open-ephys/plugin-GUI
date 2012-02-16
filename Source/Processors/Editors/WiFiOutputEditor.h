/*
  ==============================================================================

    WiFiOutputEditor.h
    Created: 16 Feb 2012 11:13:03am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __WIFIOUTPUTEDITOR_H_7161DB44__
#define __WIFIOUTPUTEDITOR_H_7161DB44__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "ImageIcon.h"

class FilterViewport;
class ImageIcon;

class WiFiOutputEditor : public GenericEditor, 
	                     public Timer

{
public:
	WiFiOutputEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~WiFiOutputEditor();

	void receivedEvent();


	ImageIcon* icon;

private:	

	int accumulator;

	void timerCallback();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WiFiOutputEditor);

};




#endif  // __WIFIOUTPUTEDITOR_H_7161DB44__
