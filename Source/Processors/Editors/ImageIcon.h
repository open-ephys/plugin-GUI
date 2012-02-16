/*
  ==============================================================================

    ImageIcon.h
    Created: 16 Feb 2012 11:39:17am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __IMAGEICON_H_ED764AE7__
#define __IMAGEICON_H_ED764AE7__

#include "../../../JuceLibraryCode/JuceHeader.h"

class ImageIcon : public Component
{
public:
	ImageIcon (Image& image_) { image = image_; opacity = 1.0;}
	~ImageIcon () {}

	void setOpacity(float);

private:	

	void paint (Graphics& g)
	{
		g.setOpacity(opacity);
		g.drawImageWithin(image, // image&
				    0, // destX
				    0, // destY
				    getWidth(), // destWidth
				    getHeight(), // destHeight
				    RectanglePlacement::xLeft);
	}

	Image image;
	float opacity;

};

#endif  // __IMAGEICON_H_ED764AE7__
