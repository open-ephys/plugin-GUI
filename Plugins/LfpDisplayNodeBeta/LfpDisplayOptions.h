// LfpDisplayOptions.h

#ifndef LFPDISPLAYOPTIONS_H

#define LFPDISPLAYOPTIONS_H

#include "LfpDisplayCanvas.h"
#include "LfpDisplayCanvasElements.h"

namespace LfpDisplayNodeBeta {

class LfpDisplayOptions : public Component,
    public Slider::Listener,
    public ComboBox::Listener,
    public Button::Listener
{
public:
    LfpDisplayOptions(LfpDisplayCanvas*, LfpTimescale*, LfpDisplay*, LfpDisplayNode*);
    ~LfpDisplayOptions();

    void paint(Graphics& g);
    void resized();

    void setRangeSelection(float range, bool canvasMustUpdate = false); // set range selection combo box to correct value if it has been changed by scolling etc.
    void setSpreadSelection(int spread, bool canvasMustUpdate = false); // set spread selection combo box to correct value if it has been changed by scolling etc.

    void comboBoxChanged(ComboBox* cb);
    void buttonClicked(Button* button);
    
    /** Handles slider events for all editors. */
    void sliderValueChanged(Slider* sl);
    
    /** Called by sliderValueChanged(). Deals with clicks on custom sliders. Subclasses
     of GenericEditor should modify this method only.*/
    void sliderEvent(Slider* sl);

    int getChannelHeight();
    bool getDrawMethodState();
    bool getInputInvertedState();

    //void setRangeSelection(float range, bool canvasMustUpdate);
    void setSpreadSelection();

    void togglePauseButton();

    void saveParameters(XmlElement* xml);
    void loadParameters(XmlElement* xml);

	DataChannel::DataChannelTypes getChannelType(int n);
	DataChannel::DataChannelTypes getSelectedType();
    String getTypeName(DataChannel::DataChannelTypes type);
	int getRangeStep(DataChannel::DataChannelTypes type);

	void setSelectedType(DataChannel::DataChannelTypes type, bool toggleButton = true);

    int selectedSpread;
    String selectedSpreadValue;

    int selectedTimebase;
    String selectedTimebaseValue;

    int selectedOverlap;
    String selectedOverlapValue;
    
    int selectedSaturation; // for saturation warning
    String selectedSaturationValue;
    float selectedSaturationValueFloat; // TODO: this is way ugly - we should refactor all these parameters soon and get them into a nicer format- probably when we do the general plugin parameter overhaul.

private:

    LfpDisplayCanvas* canvas;
    LfpDisplay* lfpDisplay;
    LfpTimescale* timescale;
    LfpDisplayNode* processor;

    ScopedPointer<ComboBox> timebaseSelection;
    ScopedPointer<ComboBox> rangeSelection;
    ScopedPointer<ComboBox> spreadSelection;
    
    ScopedPointer<ComboBox> overlapSelection;
    ScopedPointer<UtilityButton> drawClipWarningButton; // optinally draw (subtle) warning if data is clipped in display
    
    ScopedPointer<ComboBox> saturationWarningSelection;
    ScopedPointer<UtilityButton> drawSaturateWarningButton; // optionally raise hell if the actual data is saturating
    
    ScopedPointer<ComboBox> colorGroupingSelection;
    ScopedPointer<UtilityButton> invertInputButton;
    ScopedPointer<UtilityButton> drawMethodButton;
    ScopedPointer<UtilityButton> pauseButton;
    OwnedArray<UtilityButton> typeButtons;
    
    
    ScopedPointer<Slider> brightnessSliderA;
    ScopedPointer<Slider> brightnessSliderB;
    
    ScopedPointer<Label> sliderALabel;
    ScopedPointer<Label> sliderBLabel;

    ScopedPointer<ComboBox> triggerSourceSelection;

    ScopedPointer<ShowHideOptionsButton> showHideOptionsButton;

    StringArray voltageRanges[CHANNEL_TYPES];
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels
    StringArray colorGroupings; // option for coloring every N channels the same
    StringArray triggerSources; // option for trigger source event channel
    StringArray overlaps; //
    StringArray saturationThresholds; //default values for when different amplifiers saturate
    
	DataChannel::DataChannelTypes selectedChannelType;
    int selectedVoltageRange[CHANNEL_TYPES];
    String selectedVoltageRangeValues[CHANNEL_TYPES];
    float rangeGain[CHANNEL_TYPES];
    StringArray rangeUnits;
    StringArray typeNames;
    int rangeSteps[CHANNEL_TYPES];

    OwnedArray<EventDisplayInterface> eventDisplayInterfaces;

};

};

#endif
