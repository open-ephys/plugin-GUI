/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#ifndef EXAMPLEEDITOR_H_INCLUDED
#define EXAMPLEEDITOR_H_INCLUDED

#include <EditorHeaders.h>

/**

This class serves as a template for creating new editors.

If this were a real editor, this comment section would be used to
describe the editor's structure. In this example, the editor will
have a single button which will set a parameter in the processor.

@see GenericEditor

*/

class ExampleEditor : public GenericEditor	//Generic Editor adds listeners for buttons and sliders. 
											//Other possible JUCE controls can be added and listened by inheriting from the appropiate XXX:Listener class
											//See JUCE documentation to find other available controls.
{
public:
	
	/** The class constructor, used to initialize any members. */
	ExampleEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);

	/** The class destructor, used to deallocate memory */
	~ExampleEditor();

	/** 
	Unlike processors, which have a minimum set of required methods,
	editor are completely customized. There are still a couple of
	sometimes useful overloaded methods, which will appear here
	*/

	/** This method executes whenever a custom button is pressed */
	void buttonEvent(Button* button);

	/** Called to inform the editor that acquisition is about to start*/
	//void startAcquisition();

	/** Called to inform the editor that acquisition has just stopped*/
	//void stopAcquisition();

	/** Called whenever there is a change in the signal chain or it refreshes.
		It's called after the processors' same named method.
	*/
	//void updateSettings();


private:

	/**
	Here would be all the required internal variables.
	In this case, we have a single button.
	*/

	//Always use JUCE RAII classes instead of pure pointers.
	ScopedPointer<Button> exampleButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExampleEditor);
};

#endif