/*
  ==============================================================================

    EventNodeEditor.h
    Created: 16 Feb 2012 11:12:43am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __EVENTNODEEDITOR_H_A681BEBC__
#define __EVENTNODEEDITOR_H_A681BEBC__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class FilterViewport;

class EventNodeEditor : public GenericEditor,
					 public Button::Listener
{
public:
	EventNodeEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~EventNodeEditor();
	void buttonClicked(Button* button);

private:	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventNodeEditor);

};





#endif  // __EVENTNODEEDITOR_H_A681BEBC__
