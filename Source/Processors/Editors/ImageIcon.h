/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __IMAGEICON_H_ED764AE7__
#define __IMAGEICON_H_ED764AE7__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"

/**

  Convenient class for displaying an image within an editor.

  @see GenericEditor

*/

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
