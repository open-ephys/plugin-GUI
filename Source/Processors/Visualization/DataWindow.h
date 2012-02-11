/*
  ==============================================================================

    DataWindow.h
    Created: 8 Feb 2012 1:10:13pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __DATAWINDOW_H_FDDAB8D0__
#define __DATAWINDOW_H_FDDAB8D0__

#include "../../../JuceLibraryCode/JuceHeader.h"

class DataWindow : public DocumentWindow
{
public:
	DataWindow(Button* button);
	~DataWindow();

	void closeButtonPressed();

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataWindow);

	Button* controlButton;

};


#endif  // __DATAWINDOW_H_FDDAB8D0__
