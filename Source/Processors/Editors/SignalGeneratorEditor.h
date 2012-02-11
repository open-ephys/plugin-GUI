/*
  ==============================================================================

    SignalGeneratorEditor.h
    Created: 10 Feb 2012 1:28:45pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __SIGNALGENERATOREDITOR_H_841A7078__
#define __SIGNALGENERATOREDITOR_H_841A7078__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class FilterViewport;

class SignalGeneratorEditor : public GenericEditor,
					 		  public Slider::Listener
{
public:
	SignalGeneratorEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~SignalGeneratorEditor();
	void sliderValueChanged (Slider* slider);

private:	
	Slider* amplitudeSlider;
	Slider* frequencySlider;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGeneratorEditor);

};


#endif  // __SIGNALGENERATOREDITOR_H_841A7078__
