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

#ifndef __RHD2000EDITOR_H_2AD3C591__
#define __RHD2000EDITOR_H_2AD3C591__

#include <VisualizerEditorHeaders.h>

class HeadstageOptionsInterface;
class SampleRateInterface;
class BandwidthInterface;
class DSPInterface;
class AudioInterface;
class ClockDivideInterface;
class RHD2000Thread;

class UtilityButton;
struct ImpedanceData;

/**

  User interface for the RHD2000 source module.

  @see SourceNode

*/
class SourceNode;

class FPGAchannelComponent;
class RHD2000Editor;
class FPGAcanvas;

class FPGAchannelList : public Component,
    Button::Listener, ComboBox::Listener
{
public:

    FPGAchannelList(GenericProcessor* proc, Viewport* p, FPGAcanvas* c);
    ~FPGAchannelList();
    void setNewName(int channelIndex, String newName);
    void setNewGain(int channel, float gain);
    void disableAll();
    void enableAll();
    void paint(Graphics& g);
    void buttonClicked(Button* btn);
    void update();
    void updateButtons();
    int getNumChannels();
    void comboBoxChanged(ComboBox* b);
    void updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase);
    SourceNode* proc;

private:
    Array<float> gains;
    Array<DataChannel::DataChannelTypes> types;

    bool chainUpdate;

    Viewport* viewport;
    FPGAcanvas* canvas;
    ScopedPointer<UtilityButton> impedanceButton;
    ScopedPointer<ToggleButton> saveImpedanceButton;
    ScopedPointer<ToggleButton> autoMeasureButton;
    ScopedPointer<ComboBox> numberingScheme;
    ScopedPointer<Label> numberingSchemeLabel;
    OwnedArray<Label> staticLabels;
    OwnedArray<FPGAchannelComponent> channelComponents;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FPGAchannelList);
};


class FPGAchannelComponent : public Component, Button::Listener, public ComboBox::Listener, public Label::Listener
{
public:
    FPGAchannelComponent(FPGAchannelList* cl, int ch, int gainIndex_, String name_, Array<float> gains_, DataChannel::DataChannelTypes type_);
    ~FPGAchannelComponent();
    Colour getDefaultColor(int ID);
    void setImpedanceValues(float mag, float phase);
    void disableEdit();
    void enableEdit();


    void setEnabledState(bool);
    bool getEnabledState()
    {
        return isEnabled;
    }
    void buttonClicked(Button* btn);
    void setUserDefinedData(int d);
    int getUserDefinedData();
    void comboBoxChanged(ComboBox* comboBox);
    void labelTextChanged(Label* lbl);

    void resized();

	const DataChannel::DataChannelTypes type;
private:
    Array<float> gains;
    FPGAchannelList* channelList;
    ScopedPointer<Label> staticLabel, editName, impedance;
    ScopedPointer<ComboBox> rangeComboBox;
    int channel;
    String name;
    int gainIndex;
    int userDefinedData;
    Font font;
    bool isEnabled;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FPGAchannelComponent);
};


class FPGAcanvas : public Visualizer, public Button::Listener
{
public:
    FPGAcanvas(GenericProcessor* n);
    ~FPGAcanvas();

    void paint(Graphics& g);

    void refresh();

    void beginAnimation();
    void endAnimation();

    void refreshState();
    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float);

    void updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase);

    void resized();
    void buttonClicked(Button* button);
    ScopedPointer<Viewport> channelsViewport;
    SourceNode* processor;
    ScopedPointer<FPGAchannelList> channelList;
};

class RHD2000Editor : public VisualizerEditor, public ComboBox::Listener, public AsyncUpdater

{
public:
    RHD2000Editor(GenericProcessor* parentNode, RHD2000Thread*, bool useDefaultParameterEditors);
    ~RHD2000Editor();

    void buttonEvent(Button* button);

    void scanPorts();
    void comboBoxChanged(ComboBox* comboBox);

    void startAcquisition();
    void stopAcquisition();

    void channelChanged (int channel, bool newState) override;

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);
    Visualizer* createNewCanvas(void);
    void measureImpedance();

    void setSaveImpedance(bool en);
    void setAutoMeasureImpedance(bool en);
    bool getSaveImpedance();
    bool getAutoMeasureImpedance();

	void handleAsyncUpdate();

private:

    OwnedArray<HeadstageOptionsInterface> headstageOptionsInterfaces;
    OwnedArray<ElectrodeButton> electrodeButtons;

    ScopedPointer<SampleRateInterface> sampleRateInterface;
    ScopedPointer<BandwidthInterface> bandwidthInterface;
    ScopedPointer<DSPInterface> dspInterface;

    ScopedPointer<AudioInterface> audioInterface;
    ScopedPointer<ClockDivideInterface> clockInterface;

    ScopedPointer<UtilityButton> rescanButton,dacTTLButton;
    ScopedPointer<UtilityButton> adcButton;
	ScopedPointer<UtilityButton> ledButton;

    ScopedPointer<UtilityButton> dspoffsetButton;
    ScopedPointer<ComboBox> ttlSettleCombo,dacHPFcombo;


    ScopedPointer<Label> audioLabel,ttlSettleLabel,dacHPFlabel ;

    bool saveImpedances, measureWhenRecording;

    RHD2000Thread* board;
    FPGAcanvas* canvas;

	ScopedPointer<ImpedanceData> impedanceData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHD2000Editor);

};


class HeadstageOptionsInterface : public Component,
    public Button::Listener
{
public:
    HeadstageOptionsInterface(RHD2000Thread*, RHD2000Editor*, int hsNum);
    ~HeadstageOptionsInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void checkEnabledState();

private:

    int hsNumber1, hsNumber2;
    int channelsOnHs1, channelsOnHs2;
    String name;

    bool isEnabled;

    RHD2000Thread* board;
    RHD2000Editor* editor;

    ScopedPointer<UtilityButton> hsButton1;
    ScopedPointer<UtilityButton> hsButton2;

};


class BandwidthInterface : public Component,
    public Label::Listener
{
public:
    BandwidthInterface(RHD2000Thread*, RHD2000Editor*);
    ~BandwidthInterface();

    void paint(Graphics& g);
    void labelTextChanged(Label* te);

    void setLowerBandwidth(double value);
    void setUpperBandwidth(double value);
    double getLowerBandwidth();
    double getUpperBandwidth();

private:

    String name;

    String lastLowCutString, lastHighCutString;

    RHD2000Thread* board;
    RHD2000Editor* editor;

    ScopedPointer<Label> upperBandwidthSelection;
    ScopedPointer<Label> lowerBandwidthSelection;

    double actualUpperBandwidth;
    double actualLowerBandwidth;

};

class DSPInterface : public Component,
    public Label::Listener
{
public:
    DSPInterface(RHD2000Thread*, RHD2000Editor*);
    ~DSPInterface();

    void paint(Graphics& g);
    void labelTextChanged(Label* te);

    void setDspCutoffFreq(double value);
    double getDspCutoffFreq();

private:

    String name;

    RHD2000Thread* board;
    RHD2000Editor* editor;

    ScopedPointer<Label> dspOffsetSelection;

    double actualDspCutoffFreq;

};



class SampleRateInterface : public Component,
    public ComboBox::Listener
{
public:
    SampleRateInterface(RHD2000Thread*, RHD2000Editor*);
    ~SampleRateInterface();

    int getSelectedId();
    void setSelectedId(int);

    void paint(Graphics& g);
    void comboBoxChanged(ComboBox* cb);

private:

    int sampleRate;
    String name;

    RHD2000Thread* board;
    RHD2000Editor* editor;

    ScopedPointer<ComboBox> rateSelection;
    StringArray sampleRateOptions;

};

class AudioInterface : public Component,
    public Label::Listener
{
public:
    AudioInterface(RHD2000Thread*, RHD2000Editor*);
    ~AudioInterface();

    void paint(Graphics& g);
    void labelTextChanged(Label* te);

    void setNoiseSlicerLevel(int value);
    int getNoiseSlicerLevel();
    //void setGain(double value);
    //double getGain();

private:

    String name;

    String lastNoiseSlicerString;
    String lastGainString;

    RHD2000Thread* board;
    RHD2000Editor* editor;

    ScopedPointer<Label> noiseSlicerLevelSelection;
    //ScopedPointer<Label> gainSelection;

    int actualNoiseSlicerLevel;
    //double actualGain;

};

class ClockDivideInterface : public Component,
    public Label::Listener
{
public:
    ClockDivideInterface(RHD2000Thread*, RHD2000Editor*);

    void paint(Graphics& g);
    void labelTextChanged(Label* te);

    void setClockDivideRatio(int value);
    int getClockDivideRatio() const { return actualDivideRatio; };

private:

    String name;
    String lastDivideRatioString;

    RHD2000Thread * board;
    RHD2000Editor * editor;

    ScopedPointer<Label> divideRatioSelection;
    int actualDivideRatio;

};
#endif  // __RHD2000EDITOR_H_2AD3C591__
