/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Allen Institute for Brain Science and Open Ephys

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

#ifndef __RHD2000EDITOR_H_2AD3C591__
#define __RHD2000EDITOR_H_2AD3C591__

#include <VisualizerEditorHeaders.h>

class UtilityButton;
/**

User interface for the RHD2000 source module.

@see SourceNode

*/
class SourceNode;

namespace Neuropix {

	class NeuropixCanvas;
	class NeuropixInterface;
	class Annotation;
	class ColorSelector;

	class NeuropixEditor : public VisualizerEditor, public ComboBox::Listener
	{
	public:
	    NeuropixEditor(GenericProcessor* parentNode, NeuropixThread* thread, bool useDefaultParameterEditors);
	    virtual ~NeuropixEditor();

	    void comboBoxChanged(ComboBox* comboBox);
	    void buttonEvent(Button* button);

		void saveEditorParameters(XmlElement*);
		void loadEditorParameters(XmlElement*);

	    Visualizer* createNewCanvas(void);

	private:

		ScopedPointer<UtilityButton> apButton;
		ScopedPointer<UtilityButton> lfpButton;

	    ScopedPointer<ComboBox> optionComboBox;
	    ScopedPointer<UtilityButton> triggerTypeButton;
	    ScopedPointer<Label> triggerTypeLabel;
		ScopedPointer<UtilityButton> recordButton;
		ScopedPointer<Label> recordLabel;
	    Viewport* viewport;
	    NeuropixCanvas* canvas;
	    NeuropixThread* thread;

	    bool internalTrigger;
		bool recordToNpx;
		bool sendAp;
		bool sendLfp;
		int option;

	    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuropixEditor);

	};

	class NeuropixCanvas : public Visualizer, public Button::Listener
	{
	public:
		NeuropixCanvas(GenericProcessor* p, NeuropixEditor*, NeuropixThread*);
		~NeuropixCanvas();

		void paint(Graphics& g);

		void refresh();

		void beginAnimation();
		void endAnimation();

		void refreshState();
		void update();

		void setOption(int);

		void setParameter(int, float);
	    void setParameter(int, int, int, float);
	    void buttonClicked(Button* button);

	    void saveVisualizerParameters(XmlElement* xml);
		void loadVisualizerParameters(XmlElement* xml);

		void resized();


		SourceNode* processor;
		NeuropixThread* thread;
		NeuropixEditor* editor;
		ScopedPointer<Viewport> neuropixViewport;
		ScopedPointer<NeuropixInterface> neuropixInterface;

		int option;

	};

	class NeuropixInterface : public Component, public Button::Listener, public ComboBox::Listener, public Label::Listener, public Timer
	{
	public:
		NeuropixInterface(NeuropixThread*, NeuropixEditor*);
		~NeuropixInterface();

		void setOption(int);

		void paint(Graphics& g);

		void mouseMove(const MouseEvent& event);
	    void mouseDown(const MouseEvent& event);
	    void mouseDrag(const MouseEvent& event);
	    void mouseUp(const MouseEvent& event);
	    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;
	    MouseCursor getMouseCursor();

	    void buttonClicked(Button*);
	    void comboBoxChanged(ComboBox*);
	    void labelTextChanged(Label* l);

		void saveParameters(XmlElement* xml);
		void loadParameters(XmlElement* xml);

	    void setAnnotationLabel(String, Colour);
		void updateInfoString();

		void timerCallback();

	private:
		int option;

		NeuropixThread* thread;
		NeuropixEditor* editor;
		DataBuffer* inputBuffer;
		AudioSampleBuffer displayBuffer;

		ScopedPointer<ComboBox> lfpGainComboBox;
		ScopedPointer<ComboBox> apGainComboBox;
		ScopedPointer<ComboBox> referenceComboBox;
		ScopedPointer<ComboBox> filterComboBox;
		ScopedPointer<ComboBox> activityViewComboBox;

		ScopedPointer<UtilityButton> enableButton;
		ScopedPointer<UtilityButton> selectAllButton;

		ScopedPointer<Label> infoLabel;
		ScopedPointer<Label> lfpGainLabel;
		ScopedPointer<Label> apGainLabel;
		ScopedPointer<Label> referenceLabel;
		ScopedPointer<Label> filterLabel;
		ScopedPointer<Label> outputLabel;
		ScopedPointer<Label> activityViewLabel;
		ScopedPointer<Label> annotationLabelLabel;
		ScopedPointer<Label> annotationLabel;

		ScopedPointer<UtilityButton> enableViewButton;
		ScopedPointer<UtilityButton> lfpGainViewButton;
		ScopedPointer<UtilityButton> apGainViewButton;
		ScopedPointer<UtilityButton> referenceViewButton;
		ScopedPointer<UtilityButton> activityViewButton;
		ScopedPointer<UtilityButton> outputOnButton;
		ScopedPointer<UtilityButton> outputOffButton;
		ScopedPointer<UtilityButton> annotationButton;
		ScopedPointer<UtilityButton> calibrationButton;
		ScopedPointer<UtilityButton> calibrationButton2;
		ScopedPointer<UtilityButton> calibrationButton3;

		ScopedPointer<ColorSelector> colorSelector;
		
		Array<int> channelStatus;
		Array<int> channelReference;
		Array<int> channelApGain;
		Array<int> channelLfpGain;
		Array<int> channelOutput;
		Array<int> channelSelectionState;

		Array<int> option1and2refs;
		Array<int> option3refs;
		Array<int> option4refs;

		Array<Colour> channelColours;

		Array<int> refs;

		bool isOverZoomRegion;
		bool isOverUpperBorder;
		bool isOverLowerBorder;
		bool isOverChannel;

		int zoomHeight;
		int zoomOffset;
		int initialOffset;
		int initialHeight;
		int lowerBound;
		int dragZoneWidth;

		int lowestChan;
		int highestChan;

		float channelHeight;

		int visualizationMode;

		Rectangle<int> selectionBox;
		bool isSelectionActive;

		MouseCursor::StandardCursorType cursorType;

		Path shankPath;

		String channelInfoString;

		Colour getChannelColour(int chan);
		int getNearestChannel(int x, int y);
		String getChannelInfoString(int chan);

		void drawLegend(Graphics& g);
		void drawAnnotations(Graphics& g);

		void updateAvailableRefs();

		Array<Annotation> annotations;

		Array<int> getSelectedChannels();

		int getChannelForElectrode(int);
		int getConnectionForChannel(int);

		void resetParameters();
		bool wasReset;
	};

	class Annotation
	{
	public:
		Annotation(String text, Array<int> channels, Colour c);
		~Annotation();

		Array<int> channels;
		String text;

		float currentYLoc;

		bool isMouseOver;
		bool isSelected;

		Colour colour;

	};

	class ColorSelector : public Component, public ButtonListener
	{
	public:
		ColorSelector(NeuropixInterface* np);
		~ColorSelector();

		Array<Colour> standardColors;
		Array<Colour> hoverColors;
		StringArray strings;

		OwnedArray<ShapeButton> buttons;

		void buttonClicked(Button* button);

		void updateCurrentString(String s);

		Colour getCurrentColour();

		NeuropixInterface* npi;

		int activeButton;

	};
};
#endif  // __RHD2000EDITOR_H_2AD3C591__
