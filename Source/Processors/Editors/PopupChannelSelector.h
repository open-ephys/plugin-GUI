/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2021 Open Ephys

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

#ifndef ___POPUPCHANNELSELECTOR_H_E47DE5C__
#define ___POPUPCHANNELSELECTOR_H_E47DE5C__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Utils/Utils.h"

#include "../PluginManager/OpenEphysPlugin.h"

enum Select { ALL, NONE, RANGE };

class PopupChannelSelector;

class PLUGIN_API ChannelButton : public Button
{
public:
	ChannelButton(int id, PopupChannelSelector* parent);
	~ChannelButton();
	int getId() { return id; };
private:
	void mouseDown(const MouseEvent& event);
	void mouseDrag(const MouseEvent& event);
	void mouseUp(const MouseEvent& event);

	int id;
	PopupChannelSelector* parent;
	int width;
	int height;
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class PLUGIN_API SelectButton : public Button
{
public:
	SelectButton(const String& name);
	~SelectButton();
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class PLUGIN_API RangeEditor : public TextEditor
{
public:
	RangeEditor(const String& name, const Font& font);
	~RangeEditor();
private:
	;
};


/**
Automatically creates an interactive pop-up editor for selecting channels.
@see GenericEditor

*/
class PLUGIN_API PopupChannelSelector :
	public Component,
	public Button::Listener,
	public TextEditor::Listener
{
public:

	class Listener
	{
	public:
		virtual ~Listener() { }
		virtual void channelStateChanged(Array<int> selectedChannels) = 0;
	};

	PopupChannelSelector(Listener* listener, std::vector<bool> channelStates);
	~PopupChannelSelector();

	void setMaximumSelectableChannels(int num);

	void setChannelButtonColour(Colour c);

	void mouseMove(const MouseEvent& event);
	void mouseDown(const MouseEvent& event);
	void mouseDrag(const MouseEvent& event);
	void mouseUp(const MouseEvent& event);
	void buttonClicked(Button*);
	void modifierKeysChanged(const ModifierKeys& modifiers);

	ChannelButton* getButtonForId(int btnId);

	bool firstButtonSelectedState;

	juce::Point<int> startDragCoords;

	Colour buttonColour;

	OwnedArray<ChannelButton> channelButtons;

private:
	Listener* listener;

	int convertStringToInteger(String s);
	Array<int> parseStringIntoRange(int rangeValue);

	void textEditorReturnKeyPressed(TextEditor&);
	void updateRangeString();
	void parseRangeString();

	OwnedArray<SelectButton> selectButtons;
	std::unique_ptr<RangeEditor> rangeEditor;

	bool editable;
	bool isDragging;
	bool mouseDragged;
	bool shiftKeyDown;

	juce::Rectangle<int> dragBox;

	int nChannels;
	int maxSelectable;

	String rangeString;

	Array<int> channelStates;
	Array<int> selectedButtons;
	Array<int> activeChannels;
};



#endif  // ___POPUPCHANNELSELECTOR_H_E47DE5C__