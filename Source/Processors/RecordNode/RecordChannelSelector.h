#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../Utils/Utils.h"

class RecordChannelSelector;

enum Select { ALL, NONE, RANGE };

class ChannelButton : public Button	
{
public:
	ChannelButton(int id, RecordChannelSelector* parent);
	~ChannelButton();
    int getId() { return id; };
private:
	void mouseDown(const MouseEvent &event);
	void mouseDrag(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);

	int id; 
	RecordChannelSelector* parent;
    int width;
    int height;
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class SelectButton : public Button	
{
public:
	SelectButton(const String& name);
	~SelectButton();
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class RangeEditor : public TextEditor
{
public:
	RangeEditor(const String& name, const Font& font);
	~RangeEditor();
private:
	//TODO:
};

class RecordChannelSelector : public Component, public Button::Listener, public TextEditor::Listener
{
public:
	RecordChannelSelector(std::vector<bool> channelStates, bool editable);
	~RecordChannelSelector();

	void mouseMove(const MouseEvent &event);
	void mouseDown(const MouseEvent &event);
	void mouseDrag(const MouseEvent &event);
	void mouseUp(const MouseEvent &event);
	void buttonClicked(Button *);
	void modifierKeysChanged(const ModifierKeys& modifiers);

	bool editable;
	bool isDragging;
	juce::Point<int> startDragCoords;
	bool firstButtonSelectedState;

	int nChannels;

	OwnedArray<ChannelButton> channelButtons;

private:
	int convertStringToInteger(String s);
	Array<int> parseStringIntoRange(int rangeValue);

	void textEditorReturnKeyPressed(TextEditor &);
	void updateRangeString();
	void parseRangeString();
	OwnedArray<SelectButton> selectButtons;
	ScopedPointer<RangeEditor> rangeEditor;

	String rangeString;
	Array<int> channelStates;
	bool mouseDragged;
	juce::Rectangle<int> dragBox;
	Array<int> selectedButtons;
	bool shiftKeyDown;
    
};
