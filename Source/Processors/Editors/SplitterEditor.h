/*
  ==============================================================================

    SplitterEditor.h
    Created: 5 Sep 2011 2:18:10pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __SPLITTEREDITOR_H_33F644A8__
#define __SPLITTEREDITOR_H_33F644A8__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class FilterViewport;

// class PipelineSelectorButton : public DrawableButton
// {
// 	public:
// 		PipelineSelectorButton();
// 		~PipelineSelectorButton();	
// };

class SplitterEditor : public GenericEditor,
					   public Button::Listener
{
public:
	SplitterEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~SplitterEditor();

	void buttonClicked (Button* button);

private:	
	
	ImageButton* pipelineSelectorA;
	ImageButton* pipelineSelectorB;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplitterEditor);

};


#endif  // __SPLITTEREDITOR_H_33F644A8__
