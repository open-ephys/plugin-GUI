/*
  ==============================================================================

    SpikeDetectorEditor.h
    Created: 14 Aug 2011 6:27:19pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __SPIKEDETECTOREDITOR_H_F0BD2DD9__
#define __SPIKEDETECTOREDITOR_H_F0BD2DD9__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class FilterViewport;

class SpikeDetectorEditor : public GenericEditor,
					 	    public Slider::Listener
{
public:
	SpikeDetectorEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~SpikeDetectorEditor();
	void sliderValueChanged (Slider* slider);

private:	
	Slider* threshSlider;
	DocumentWindow* docWindow;

	//int tabIndex;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetectorEditor);

};




#endif  // __SPIKEDETECTOREDITOR_H_F0BD2DD9__
