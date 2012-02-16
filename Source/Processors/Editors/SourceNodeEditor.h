/*
  ==============================================================================

    SourceNodeEditor.h
    Created: 7 Sep 2011 5:08:31pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __SOURCENODEEDITOR_H_A1B19E1E__
#define __SOURCENODEEDITOR_H_A1B19E1E__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "ImageIcon.h"

class FilterViewport;
class ImageIcon;

class SourceNodeEditor : public GenericEditor

{
public:
	SourceNodeEditor (GenericProcessor* parentNode, FilterViewport* vp);
	virtual ~SourceNodeEditor();

private:	

	ImageIcon* icon;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNodeEditor);

};



#endif  // __SOURCENODEEDITOR_H_A1B19E1E__
