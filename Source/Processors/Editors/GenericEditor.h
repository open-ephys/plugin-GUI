/*
  ==============================================================================

    GenericEditor.h
    Created: 7 Jun 2011 11:37:12am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __GENERICEDITOR_H_DD406E71__
#define __GENERICEDITOR_H_DD406E71__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor.h"
#include "../../UI/FilterViewport.h"
#include "../../UI/Configuration.h"
#include <stdio.h>

class GenericProcessor;
class FilterViewport;

class GenericEditor : public AudioProcessorEditor//,
					 // public Button::Listener

{
public:
	GenericEditor (GenericProcessor* owner, FilterViewport* vp);
	virtual ~GenericEditor();

	void paint (Graphics& g);
	void setViewport(FilterViewport*);
	//void setTabbedComponent(TabbedComponent*);

	bool keyPressed (const KeyPress& key);

	void switchSelectedState();
	void select();
	void deselect();
	bool getSelectionState();

	void enable();
	void disable();
	bool getEnabledState();
	void setEnabledState(bool);

	String getName() {return name;}

	int desiredWidth;
	int nodeId;

	void tabNumber(int t) {tNum = t;}
	int tabNumber() {return tNum;}

	FilterViewport* viewport;
	Configuration* config;

	void setConfiguration(Configuration* cf) {config = cf;}
	Configuration* getConfiguration() {return config;}

	AudioProcessor* getProcessor() const {return getAudioProcessor();}
	
	void createRadioButtons(int x, int y, int w, StringArray values, const String& name);
		
	int radioGroupId;

	//virtual void buttonClicked(Button* b);
	
private:

	Colour backgroundColor;

	bool isSelected;
	bool isEnabled;

	int tNum;

	

	Font titleFont;

	String name;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);

};




class RadioButton : public Button
{
public:
    RadioButton(const String& name, int groupId);// : Button("Name") {configurationChanged = true;}
    ~RadioButton() {}

private:

    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown);

    Font buttonFont;
};




#endif  // __GENERICEDITOR_H_DD406E71__
