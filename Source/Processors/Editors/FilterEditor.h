/*
  ==============================================================================

    FilterEditor.h
    Created: 12 Jun 2011 5:04:08pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __FILTEREDITOR_H_969BDB5__
#define __FILTEREDITOR_H_969BDB5__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class FilterViewport;

class FilterEditor : public GenericEditor,
					 public Slider::Listener
{
public:
	FilterEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~FilterEditor();
	void sliderValueChanged (Slider* slider);

private:	
	Slider* lowSlider;
	Slider* highSlider;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterEditor);

};



#endif  // __FILTEREDITOR_H_969BDB5__
