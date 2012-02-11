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

class FilterViewport;

class ImageIcon : public Component
{
public:
	ImageIcon (Image& image_) { image = image_;}
	~ImageIcon () {}


private:	

	void paint (Graphics& g)
	{
		g.drawImageWithin(image, // image&
				    0, // destX
				    0, // destY
				    getWidth(), // destWidth
				    getHeight(), // destHeight
				    RectanglePlacement::xLeft);
	}

	Image image;

};

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
