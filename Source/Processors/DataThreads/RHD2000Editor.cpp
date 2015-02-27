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

#include "RHD2000Editor.h"
#include "../../UI/EditorViewport.h"
#include <cmath>

#include "../Editors/ChannelSelector.h"
#include "../SourceNode/SourceNode.h"
#include "../RecordNode/RecordNode.h"
#include "RHD2000Thread.h"

#ifdef WIN32
#if (_MSC_VER < 1800) //round doesn't exist on MSVC prior to 2013 version
inline double round(double x)
{
    return floor(x+0.5);
}
#endif
#endif

FPGAchannelList::FPGAchannelList(GenericProcessor* proc_, Viewport* p, FPGAcanvas* c) : proc(proc_), viewport(p), canvas(c)
{
    channelComponents.clear();

    numberingSchemeLabel = new Label("Numbering scheme:","Numbering scheme:");
    numberingSchemeLabel->setEditable(false);
    numberingSchemeLabel->setBounds(10,10,150, 25);
    numberingSchemeLabel->setColour(Label::textColourId,juce::Colours::white);
    addAndMakeVisible(numberingSchemeLabel);

    numberingScheme = new ComboBox("numberingScheme");
    numberingScheme->addItem("Continuous",1);
    numberingScheme->addItem("Per Stream",2);
    numberingScheme->setBounds(160,10,100,25);
    numberingScheme->addListener(this);
    numberingScheme->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(numberingScheme);

    impedanceButton = new UtilityButton("Measure Impedance", Font("Default", 13, Font::plain));
    impedanceButton->setRadius(3);
    impedanceButton->setBounds(280,10,140,25);
    impedanceButton->addListener(this);
    addAndMakeVisible(impedanceButton);

	RHD2000Editor *e = static_cast<RHD2000Editor*>(proc->getEditor());
	saveImpedanceButton = new ToggleButton("Save impedance measurements");
	saveImpedanceButton->setBounds(430,10,110,25);
	saveImpedanceButton->setToggleState(e->getSaveImpedance(),dontSendNotification);
	saveImpedanceButton->addListener(this);
	addAndMakeVisible(saveImpedanceButton);
	
	autoMeasureButton = new ToggleButton("Measure impedance at acquisition start");
	autoMeasureButton->setBounds(550,10,150,25);
	autoMeasureButton->setToggleState(e->getAutoMeasureImpedance(),dontSendNotification);
	autoMeasureButton->addListener(this);
	addAndMakeVisible(autoMeasureButton);

    gains.clear();
    gains.add(0.01);
    gains.add(0.1);
    gains.add(1);
    gains.add(2);
    gains.add(5);
    gains.add(10);
    gains.add(20);
    gains.add(50);
    gains.add(100);
    gains.add(500);
    gains.add(1000);


    update();
}

FPGAchannelList::~FPGAchannelList()
{

}

void FPGAchannelList::paint(Graphics& g)
{
}

void FPGAchannelList::buttonClicked(Button* btn)
{
	RHD2000Editor* p = (RHD2000Editor*)proc->getEditor();
    if (btn == impedanceButton)
    {
        p->measureImpedance();
    }
	else if (btn == saveImpedanceButton)
	{
		p->setSaveImpedance(btn->getToggleState());
	}
	else if (btn == autoMeasureButton)
	{
		p->setAutoMeasureImpedance(btn->getToggleState());
	}
}

void FPGAchannelList::update()
{
    // Query processor for number of channels, types, gains, etc... and update the UI
    channelComponents.clear();
    staticLabels.clear();
    StringArray names;
    Array<float> oldgains;
    proc->getChannelsInfo(names,types,stream,orig_number,oldgains);
    int numChannels = names.size();

    // find out which streams are active.
    bool streamActive[MAX_NUM_DATA_STREAMS+1];
    bool adcActive = false;
    int numActiveStreams = 0;
    int streamColumn[MAX_NUM_DATA_STREAMS+1];
    int numChannelsPerStream[MAX_NUM_DATA_STREAMS+1];

    for (int k=0; k<MAX_NUM_DATA_STREAMS+1; k++)
    {
        numChannelsPerStream[k] = 0;
        streamActive[k] = false;
        streamColumn[k] = 0;
    }
    int columnWidth = 330;

    for (int k=0; k<numChannels; k++)
    {
        if (streamActive[stream[k]] == false)
        {
            streamColumn[stream[k]] = numActiveStreams*columnWidth;
            numActiveStreams++;
            streamActive[stream[k]] = true;
        }
    }

    StringArray streamNames;
    streamNames.add("Port A1");
    streamNames.add("Port A2");
    streamNames.add("Port B1");
    streamNames.add("Port B2");
    streamNames.add("Port C1");
    streamNames.add("Port C2");
    streamNames.add("Port D1");
    streamNames.add("Port D2");
    streamNames.add("ADC");

    for (int k = 0; k < MAX_NUM_DATA_STREAMS + 1; k++)
    {
        if (streamActive[k])
        {
            Label* lbl = new Label(streamNames[k],streamNames[k]);
            lbl->setEditable(false);
            lbl->setBounds(10+streamColumn[k],40,columnWidth, 25);
            lbl->setJustificationType(juce::Justification::centred);
            lbl->setColour(Label::textColourId,juce::Colours::white);
            staticLabels.add(lbl);
            addAndMakeVisible(lbl);

        }

    }

    // add buttons for all DATA, AUX, channels
    for (int k = 0; k < numChannels; k++)
    {
        int channelGainIndex = 1;
		float ch_gain = 1.0f; ///%oldgains[k]/static_cast<SourceNode*>(proc)->getBitVolts(k);
        for (int j = 0; j < gains.size(); j++)
        {
            if (fabs(gains[j]-ch_gain) < 1e-3)
            {
                channelGainIndex = j;
                break;
            }
        }

        FPGAchannelComponent* comp = new FPGAchannelComponent(this, stream[k],orig_number[k],types[k],channelGainIndex+1, names[k],gains);
        comp->setBounds(10+streamColumn[stream[k]],70+numChannelsPerStream[stream[k]]*22,columnWidth,22);
        numChannelsPerStream[stream[k]]++;

        comp->setUserDefinedData(k);
        addAndMakeVisible(comp);
        channelComponents.add(comp);
    }

    StringArray ttlNames;
    proc->getEventChannelNames(ttlNames);
    // add buttons for TTL channels
    for (int k=0; k<ttlNames.size(); k++)
    {
        FPGAchannelComponent* comp = new FPGAchannelComponent(this,-1,k, EVENT_CHANNEL,-1, ttlNames[k],gains);
        comp->setBounds(10+numActiveStreams*columnWidth,70+k*22,columnWidth,22);
        comp->setUserDefinedData(k);
        addAndMakeVisible(comp);
        channelComponents.add(comp);
    }

    Label* lbl = new Label("TTL Events","TTL Events");
    lbl->setEditable(false);
    lbl->setBounds(numActiveStreams*columnWidth,40,columnWidth, 25);
    lbl->setJustificationType(juce::Justification::centred);
    lbl->setColour(Label::textColourId,juce::Colours::white);
    staticLabels.add(lbl);
    addAndMakeVisible(lbl);


}

void FPGAchannelList::disableAll()
{
    for (int k=0; k<channelComponents.size(); k++)
    {
        channelComponents[k]->disableEdit();
    }
}

void FPGAchannelList::enableAll()
{
    for (int k=0; k<channelComponents.size(); k++)
    {
        channelComponents[k]->enableEdit();
    }

}

void FPGAchannelList::setNewGain(int stream, int channel, ChannelType type, float gain)
{
    float newGain;
	int realChan;
	SourceNode* p = (SourceNode*) proc;

	if (type==ADC_CHANNEL)
	{
		realChan = p->getNumOutputs()-p->getThread()->getNumADCchannels()+channel;
	}
	else
	{
		realChan = channel;
	}
	//newGain = p->getBitVolts(realChan)*gain;
    p->modifyChannelGain(stream, channel, type, newGain, true);
}

void FPGAchannelList::setNewName(int stream, int channelIndex, ChannelType t, String newName)
{
    proc->modifyChannelName(t, stream, channelIndex, newName, true);
}

void FPGAchannelList::updateButtons()
{
}

int FPGAchannelList::getNumChannels()
{
    return 0;
}

void FPGAchannelList::comboBoxChanged(ComboBox* b)
{
    if (b == numberingScheme)
    {
        SourceNode* p = (SourceNode*)proc;
        int scheme = numberingScheme->getSelectedId();
        if (scheme == 1)
        {
            p->setDefaultNamingScheme(scheme);
        }
        else if (scheme == 2)
        {
            p->setDefaultNamingScheme(scheme);
        }
        update();
		canvas->processor->getEditorViewport()->makeEditorVisible(canvas->processor->getEditor(),false,true);
    }
}

void FPGAchannelList::updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase)
{
    for (int k=0; k<streams.size(); k++)
    {
        for (int j=k; j<stream.size(); j++)
        {
            if (stream[j] == streams[k] && types[j] == HEADSTAGE_CHANNEL && orig_number[j] == channels[k])
            {
                channelComponents[j]->setImpedanceValues(magnitude[k],phase[k]);
                break;
            }

        }
    }

}


/****************************************************/
FPGAchannelComponent::FPGAchannelComponent(FPGAchannelList* cl, int stream_, int ch, ChannelType t, int gainIndex_, String N, Array<float> gains_) :   gains(gains_), channelList(cl), channel(ch), name(N), stream(stream_), type(t), gainIndex(gainIndex_)
{
    Font f = Font("Small Text", 13, Font::plain);

    staticLabel = new Label("Channel","Channel");
    staticLabel->setFont(f);
    staticLabel->setEditable(false);
    addAndMakeVisible(staticLabel);

    editName = new Label(name,name);
    editName->setFont(f);
    editName->setEditable(true);
    editName->setColour(Label::backgroundColourId,juce::Colours::lightgrey);
    editName->addListener(this);
    addAndMakeVisible(editName);
    if (gainIndex > 0)
    {

        gainComboBox = new ComboBox("Gains");
        for (int k=0; k<gains.size(); k++)
        {
            if (gains[k] < 1)
            {
                gainComboBox->addItem("x"+String(gains[k],2),k+1);
            }
            else
            {
                gainComboBox->addItem("x"+String((int)gains[k]),k+1);
            }
        }
        gainComboBox->setSelectedId(gainIndex, sendNotification);
        gainComboBox->addListener(this);
        addAndMakeVisible(gainComboBox);
    }
    else
    {
        gainComboBox = nullptr;
    }

    if (type == HEADSTAGE_CHANNEL)
    {
        impedance = new Label("Impedance","? Ohm");
        impedance->setFont(Font("Default", 13, Font::plain));
        impedance->setEditable(false);
        addAndMakeVisible(impedance);
    }
    else
    {
        impedance = nullptr;
    }
}
FPGAchannelComponent::~FPGAchannelComponent()
{

}

void FPGAchannelComponent::setImpedanceValues(float mag, float phase)
{
    if (impedance != nullptr)
    {
        if (mag > 10000)
            impedance->setText(String(mag/1e6,2)+" mOhm, "+String((int)phase) + " deg",juce::NotificationType::dontSendNotification);
        else if (mag > 1000)
            impedance->setText(String(mag/1e3,0)+" kOhm, "+String((int)phase) + " deg" ,juce::NotificationType::dontSendNotification);
        else
            impedance->setText(String(mag,0)+" Ohm, "+String((int)phase) + " deg" ,juce::NotificationType::dontSendNotification);
    }
    else
    {

    }
}

void FPGAchannelComponent::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == gainComboBox)
    {
        int newGainIndex = gainComboBox->getSelectedId();
        float mult = gains[newGainIndex-1];
        float bitvolts = channelList->proc->getDefaultBitVolts();
        channelList->setNewGain(stream,channel,type, mult*bitvolts);
    }
}
void FPGAchannelComponent::labelTextChanged(Label* lbl)
{
    // channel name change
    String newName = lbl->getText();
    channelList->setNewName(stream,channel, type, newName);
}

void FPGAchannelComponent::disableEdit()
{
    editName->setEnabled(false);
}

void FPGAchannelComponent::enableEdit()
{
    editName->setEnabled(true);
}

void FPGAchannelComponent::buttonClicked(Button* btn)
{
}

void FPGAchannelComponent::setUserDefinedData(int d)
{
}

int FPGAchannelComponent::getUserDefinedData()
{
    return 0;
}

void FPGAchannelComponent::resized()
{
    editName->setBounds(0,0,90,20);
    if (gainComboBox != nullptr)
    {
        gainComboBox->setBounds(100,0,70,20);
    }
    if (impedance != nullptr)
    {
        impedance->setBounds(180,0,130,20);
    }

}



/**********************************************/

FPGAcanvas::FPGAcanvas(GenericProcessor* n) : processor(n)
{

    channelsViewport = new Viewport();
    channelList = new FPGAchannelList(processor, channelsViewport, this);
    channelsViewport->setViewedComponent(channelList, false);
    channelsViewport->setScrollBarsShown(true, true);
    addAndMakeVisible(channelsViewport);

    resized();
    update();
}

FPGAcanvas::~FPGAcanvas()
{
}

void FPGAcanvas::setParameter(int x, float f)
{

}

void FPGAcanvas::setParameter(int a, int b, int c, float d)
{
}

void FPGAcanvas::paint(Graphics& g)
{
    g.fillAll(Colours::grey);

}

void FPGAcanvas::refresh()
{
    repaint();
}

void FPGAcanvas::refreshState()
{
    resized();
}


void FPGAcanvas::beginAnimation()
{
}

void FPGAcanvas::endAnimation()
{
}

void FPGAcanvas::update()
{
    // create channel buttons (name, gain, recording, impedance, ... ?)
    channelList->update();
}

void FPGAcanvas::resized()
{
    int screenWidth = getWidth();
    int screenHeight = getHeight();

    int scrollBarThickness = channelsViewport->getScrollBarThickness();
    int numChannels = 35; // max channels per stream? (32+3)*2

    channelsViewport->setBounds(0,0,getWidth(),getHeight());
    channelList->setBounds(0,0,getWidth()-scrollBarThickness, 200+22*numChannels);
}

void FPGAcanvas::buttonClicked(Button* button)
{
}

void FPGAcanvas::updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase)
{
    channelList->updateImpedance(streams, channels,  magnitude, phase);
}

/***********************************************************************/

RHD2000Editor::RHD2000Editor(GenericProcessor* parentNode,
                             RHD2000Thread* board_,
                             bool useDefaultParameterEditors
                            )
    : VisualizerEditor(parentNode, useDefaultParameterEditors), board(board_)
{
    canvas = nullptr;
    desiredWidth = 330;
    tabText = "FPGA";
	measureWhenRecording = false;
	saveImpedances = false;

    // add headstage-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 4; i++)
    {
        HeadstageOptionsInterface* hsOptions = new HeadstageOptionsInterface(board, this, i);
        headstageOptionsInterfaces.add(hsOptions);
        addAndMakeVisible(hsOptions);
        hsOptions->setBounds(3, 28+i*20, 70, 18);
    }

    // add sample rate selection
    sampleRateInterface = new SampleRateInterface(board, this);
    addAndMakeVisible(sampleRateInterface);
    sampleRateInterface->setBounds(80, 25, 100, 50);

    // add Bandwidth selection
    bandwidthInterface = new BandwidthInterface(board, this);
    addAndMakeVisible(bandwidthInterface);
    bandwidthInterface->setBounds(80, 58, 80, 50);

    // add DSP selection
    dspInterface = new DSPInterface(board, this);
    addAndMakeVisible(dspInterface);
    dspInterface->setBounds(80, 58, 80, 50);

    // add rescan button
    rescanButton = new UtilityButton("RESCAN", Font("Small Text", 13, Font::plain));
    rescanButton->setRadius(3.0f);
    rescanButton->setBounds(6, 108,65,18);
    rescanButton->addListener(this);
    rescanButton->setTooltip("Check for connected headstages");
    addAndMakeVisible(rescanButton);

    for (int i = 0; i < 2; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(-1);
        electrodeButtons.add(button);

        button->setBounds(190+i*25, 40, 25, 15);
        button->setChannelNum(-1);
        button->setToggleState(false, dontSendNotification);
        button->setRadioGroupId(999);

        addAndMakeVisible(button);
        button->addListener(this);

        if (i == 0)
        {
            button->setTooltip("Audio monitor left channel");
        }
        else
        {
            button->setTooltip("Audio monitor right channel");
        }
    }

    audioLabel = new Label("audio label", "Audio out");
    audioLabel->setBounds(180,25,75,15);
    audioLabel->setFont(Font("Small Text", 10, Font::plain));
    audioLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(audioLabel);

    // add HW audio parameter selection
    audioInterface = new AudioInterface(board, this);
    addAndMakeVisible(audioInterface);
    audioInterface->setBounds(165, 65, 65, 50);


    adcButton = new UtilityButton("ADC 1-8", Font("Small Text", 13, Font::plain));
    adcButton->setRadius(3.0f);
    adcButton->setBounds(165,100,65,18);
    adcButton->addListener(this);
    adcButton->setClickingTogglesState(true);
    adcButton->setTooltip("Enable/disable ADC channels");
    addAndMakeVisible(adcButton);

    // add DSP Offset Button
    dspoffsetButton = new UtilityButton("DSP", Font("Very Small Text", 13, Font::plain));
    dspoffsetButton->setRadius(3.0f); // sets the radius of the button's corners
    dspoffsetButton->setBounds(80, 108,30,18); // sets the x position, y position, width, and height of the button
    dspoffsetButton->addListener(this);
    dspoffsetButton->setClickingTogglesState(true); // makes the button toggle its state when clicked
    dspoffsetButton->setTooltip("Enable/disable DSP offset removal");
    addAndMakeVisible(dspoffsetButton); // makes the button a child component of the editor and makes it visible
    dspoffsetButton->setToggleState(true, dontSendNotification);

    // add DSP Frequency Selection field
    dspInterface = new DSPInterface(board, this);
    addAndMakeVisible(dspInterface);
    dspInterface->setBounds(110, 108, 80, 50);

    ttlSettleLabel = new Label("TTL Settle","TTL Settle");
    ttlSettleLabel->setFont(Font("Small Text", 11, Font::plain));
    ttlSettleLabel->setBounds(245,80,100,20);
    ttlSettleLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(ttlSettleLabel);


    ttlSettleCombo = new ComboBox("FastSettleComboBox");
    ttlSettleCombo->setBounds(250,100,60,18);
    ttlSettleCombo->addListener(this);
    ttlSettleCombo->addItem("-",1);
    for (int k=0; k<8; k++)
    {
        ttlSettleCombo->addItem("TTL"+String(1+k),2+k);
    }
    ttlSettleCombo->setSelectedId(1, sendNotification);
    addAndMakeVisible(ttlSettleCombo);

    dacTTLButton = new UtilityButton("DAC TTL", Font("Small Text", 13, Font::plain));
    dacTTLButton->setRadius(3.0f);
    dacTTLButton->setBounds(250,25,65,18);
    dacTTLButton->addListener(this);
    dacTTLButton->setClickingTogglesState(true);
    dacTTLButton->setTooltip("Enable/disable DAC Threshold TTL Output");
    addAndMakeVisible(dacTTLButton);

    dacHPFlabel = new Label("DAC HPF","DAC HPF");
    dacHPFlabel->setFont(Font("Small Text", 11, Font::plain));
    dacHPFlabel->setBounds(250,42,100,20);
    dacHPFlabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(dacHPFlabel);

    dacHPFcombo = new ComboBox("dacHPFCombo");
    dacHPFcombo->setBounds(250,60,60,18);
    dacHPFcombo->addListener(this);
    dacHPFcombo->addItem("OFF",1);
    int HPFvalues[10] = {50,100,200,300,400,500,600,700,800,900};
    for (int k=0; k<10; k++)
    {
        dacHPFcombo->addItem(String(HPFvalues[k])+" Hz",2+k);
    }
    dacHPFcombo->setSelectedId(1, sendNotification);
    addAndMakeVisible(dacHPFcombo);

}

RHD2000Editor::~RHD2000Editor()
{

}

void RHD2000Editor::scanPorts()
{
    rescanButton->triggerClick();
}

void RHD2000Editor::measureImpedance()
{
    Array<int> stream, channel;
    Array<float> magnitude, phase;
    board->runImpedanceTest(stream,channel,magnitude,phase);

	if (canvas == nullptr)
		VisualizerEditor::canvas = createNewCanvas();
    // update components...
    canvas->updateImpedance(stream,channel,magnitude,phase);
	if (saveImpedances)
	{
		getProcessorGraph()->getRecordNode()->createNewDirectory();

		String path(getProcessorGraph()->getRecordNode()->getDataDirectory().getFullPathName() 
			+ File::separatorString + "impedance_measurement.xml");
		std::cout << "Saving impedance measurements in " << path << std::endl;
		File file(path);

		if (!file.getParentDirectory().exists())
			file.getParentDirectory().createDirectory();

		XmlDocument doc(file);
		ScopedPointer<XmlElement> xml = new XmlElement("CHANNEL_IMPEDANCES");
		for (int i = 0; i < channel.size(); i++)
		{
			XmlElement* chan = new XmlElement("CHANNEL");
			chan->setAttribute("name",board->getChannelName(HEADSTAGE_CHANNEL,stream[i],channel[i]));
			chan->setAttribute("stream",stream[i]);
			chan->setAttribute("channel_number",channel[i]);
			chan->setAttribute("magnitude",magnitude[i]);
			chan->setAttribute("phase",phase[i]);
			xml->addChildElement(chan);
		}
		xml->writeToFile(file,String::empty);
	}

}

void RHD2000Editor::setSaveImpedance(bool en)
{
	saveImpedances = en;
}

void RHD2000Editor::setAutoMeasureImpedance(bool en)
{
	measureWhenRecording = en;
}

bool RHD2000Editor::getSaveImpedance()
{
	return saveImpedances;
}

bool RHD2000Editor::getAutoMeasureImpedance()
{
	return measureWhenRecording;
}

void RHD2000Editor::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == ttlSettleCombo)
    {
        int selectedChannel = ttlSettleCombo->getSelectedId();
        if (selectedChannel == 1)
        {
            board->setFastTTLSettle(false,0);
        }
        else
        {
            board->setFastTTLSettle(true,selectedChannel-2);
        }
    }
    else if (comboBox == dacHPFcombo)
    {
        int selection = dacHPFcombo->getSelectedId();
        if (selection == 1)
        {
            board->setDAChpf(100,false);
        }
        else
        {
            int HPFvalues[10] = {50,100,200,300,400,500,600,700,800,900};
            board->setDAChpf(HPFvalues[selection-2],true);
        }
    }
}


void RHD2000Editor::buttonEvent(Button* button)
{
    VisualizerEditor::buttonEvent(button);
    if (button == rescanButton && !acquisitionIsActive)
    {
        board->scanPorts();

        for (int i = 0; i < 4; i++)
        {
            headstageOptionsInterfaces[i]->checkEnabledState();
        }
       // board->updateChannelNames();
		getEditorViewport()->makeEditorVisible(this, false, true);
    }
    else if (button == electrodeButtons[0])
    {
        channelSelector->setRadioStatus(true);
    }
    else if (button == electrodeButtons[1])
    {
        channelSelector->setRadioStatus(true);
    }
    else if (button == adcButton && !acquisitionIsActive)
    {
        board->enableAdcs(button->getToggleState());
//        board->updateChannelNames();
        std::cout << "ADC Button toggled" << std::endl;
        getEditorViewport()->makeEditorVisible(this, false, true);
        std::cout << "Editor visible." << std::endl;
    }
    else if (button == dacTTLButton)
    {
        board->setTTLoutputMode(dacTTLButton->getToggleState());
    }
    else if (button == dspoffsetButton && !acquisitionIsActive)
    {
        std::cout << "DSP offset " << button->getToggleState() << std::endl;
        board->setDSPOffset(button->getToggleState());
    }

}

void RHD2000Editor::channelChanged(int chan)
{
    for (int i = 0; i < 2; i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum(chan);
            electrodeButtons[i]->repaint();
            board->assignAudioOut(i, chan);

        }
    }
}

void RHD2000Editor::startAcquisition()
{
	if (measureWhenRecording)
		measureImpedance();

    channelSelector->startAcquisition();

    rescanButton->setEnabledState(false);
    adcButton->setEnabledState(false);
    dspoffsetButton-> setEnabledState(false);
    acquisitionIsActive = true;
    if (canvas !=nullptr)
        canvas->channelList->setEnabled(false);
}

void RHD2000Editor::stopAcquisition()
{

    channelSelector->stopAcquisition();

    rescanButton->setEnabledState(true);
    adcButton->setEnabledState(true);
    dspoffsetButton-> setEnabledState(true);

    acquisitionIsActive = false;
    if (canvas != nullptr)
        canvas->channelList->setEnabled(true);
    //  canvas->channelList->setEnabled(true);
}

void RHD2000Editor::saveCustomParameters(XmlElement* xml)
{
    xml->setAttribute("SampleRate", sampleRateInterface->getSelectedId());
    xml->setAttribute("LowCut", bandwidthInterface->getLowerBandwidth());
    xml->setAttribute("HighCut", bandwidthInterface->getUpperBandwidth());
    xml->setAttribute("ADCsOn", adcButton->getToggleState());
    xml->setAttribute("SampleRate", sampleRateInterface->getSelectedId());
    xml->setAttribute("LowCut", bandwidthInterface->getLowerBandwidth());
    xml->setAttribute("HighCut", bandwidthInterface->getUpperBandwidth());
    xml->setAttribute("ADCsOn", adcButton->getToggleState());
    xml->setAttribute("AudioOutputL", electrodeButtons[0]->getChannelNum());
    xml->setAttribute("AudioOutputR", electrodeButtons[1]->getChannelNum());
    xml->setAttribute("NoiseSlicer", audioInterface->getNoiseSlicerLevel());
    xml->setAttribute("TTLFastSettle", ttlSettleCombo->getSelectedId());
    xml->setAttribute("DAC_TTL", dacTTLButton->getToggleState());
    xml->setAttribute("DAC_HPF", dacHPFcombo->getSelectedId());
    xml->setAttribute("DSPOffset", dspoffsetButton->getToggleState());
    xml->setAttribute("DSPCutoffFreq", dspInterface->getDspCutoffFreq());
	xml->setAttribute("save_impedance_measurements",saveImpedances);
	xml->setAttribute("auto_measure_impedances",measureWhenRecording);
}

void RHD2000Editor::loadCustomParameters(XmlElement* xml)
{

    sampleRateInterface->setSelectedId(xml->getIntAttribute("SampleRate"));
    bandwidthInterface->setLowerBandwidth(xml->getDoubleAttribute("LowCut"));
    bandwidthInterface->setUpperBandwidth(xml->getDoubleAttribute("HighCut"));
    adcButton->setToggleState(xml->getBoolAttribute("ADCsOn"), sendNotification);
    electrodeButtons[0]->setChannelNum(xml->getIntAttribute("AudioOutputL"));
    board->assignAudioOut(0, xml->getIntAttribute("AudioOutputL"));
    electrodeButtons[1]->setChannelNum(xml->getIntAttribute("AudioOutputR"));
    board->assignAudioOut(1, xml->getIntAttribute("AudioOutputR"));
    audioInterface->setNoiseSlicerLevel(xml->getIntAttribute("NoiseSlicer"));
    ttlSettleCombo->setSelectedId(xml->getIntAttribute("TTLFastSettle"));
    dacTTLButton->setToggleState(xml->getBoolAttribute("DAC_TTL"), sendNotification);
    dacHPFcombo->setSelectedId(xml->getIntAttribute("DAC_HPF"));
    dspoffsetButton->setToggleState(xml->getBoolAttribute("DSPOffset"), sendNotification);
    dspInterface->setDspCutoffFreq(xml->getDoubleAttribute("DSPCutoffFreq"));
	saveImpedances = xml->getBoolAttribute("save_impedance_measurements");
	measureWhenRecording = xml->getBoolAttribute("auto_measure_impedances");
}


Visualizer* RHD2000Editor::createNewCanvas()
{
    GenericProcessor* processor = (GenericProcessor*) getProcessor();
    canvas= new FPGAcanvas(processor);
    //ActionListener* listener = (ActionListener*) canvas;
    //getUIComponent()->registerAnimatedComponent(listener);
    return canvas;
}

// Bandwidth Options --------------------------------------------------------------------

BandwidthInterface::BandwidthInterface(RHD2000Thread* board_,
                                       RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name = "Bandwidth";

    lastHighCutString = "7500";
    lastLowCutString = "1";

    actualUpperBandwidth = 7500.0f;
    actualLowerBandwidth = 1.0f;

    upperBandwidthSelection = new Label("UpperBandwidth",lastHighCutString); // this is currently set in RHD2000Thread, the cleaner would be to set it here again
    upperBandwidthSelection->setEditable(true,false,false);
    upperBandwidthSelection->addListener(this);
    upperBandwidthSelection->setBounds(30,30,60,20);
    upperBandwidthSelection->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(upperBandwidthSelection);


    lowerBandwidthSelection = new Label("LowerBandwidth",lastLowCutString);
    lowerBandwidthSelection->setEditable(true,false,false);
    lowerBandwidthSelection->addListener(this);
    lowerBandwidthSelection->setBounds(25,10,60,20);
    lowerBandwidthSelection->setColour(Label::textColourId, Colours::darkgrey);

    addAndMakeVisible(lowerBandwidthSelection);



}

BandwidthInterface::~BandwidthInterface()
{

}


void BandwidthInterface::labelTextChanged(Label* label)
{

    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (label == upperBandwidthSelection)
        {

            Value val = label->getTextValue();
            double requestedValue = double(val.getValue());

            if (requestedValue < 100.0 || requestedValue > 20000.0 || requestedValue < lastLowCutString.getFloatValue())
            {
                editor->sendActionMessage("Value out of range.");

                label->setText(lastHighCutString, dontSendNotification);

                return;
            }

            actualUpperBandwidth = board->setUpperBandwidth(requestedValue);

            std::cout << "Setting Upper Bandwidth to " << requestedValue << std::endl;
            std::cout << "Actual Upper Bandwidth:  " <<  actualUpperBandwidth  << std::endl;
            label->setText(String(round(actualUpperBandwidth*10.f)/10.f), dontSendNotification);

        }
        else
        {

            Value val = label->getTextValue();
            double requestedValue = double(val.getValue());

            if (requestedValue < 0.1 || requestedValue > 500.0 || requestedValue > lastHighCutString.getFloatValue())
            {
                editor->sendActionMessage("Value out of range.");

                label->setText(lastLowCutString, dontSendNotification);

                return;
            }

            actualLowerBandwidth = board->setLowerBandwidth(requestedValue);

            std::cout << "Setting Lower Bandwidth to " << requestedValue << std::endl;
            std::cout << "Actual Lower Bandwidth:  " <<  actualLowerBandwidth  << std::endl;

            label->setText(String(round(actualLowerBandwidth*10.f)/10.f), dontSendNotification);
        }
    }
    else if (editor->acquisitionIsActive)
    {
        editor->sendActionMessage("Can't change bandwidth while acquisition is active!");
        if (label == upperBandwidthSelection)
            label->setText(lastHighCutString, dontSendNotification);
        else
            label->setText(lastLowCutString, dontSendNotification);
        return;
    }

}

void BandwidthInterface::setLowerBandwidth(double value)
{
    actualLowerBandwidth = board->setLowerBandwidth(value);
    lowerBandwidthSelection->setText(String(round(actualLowerBandwidth*10.f)/10.f), dontSendNotification);
}

void BandwidthInterface::setUpperBandwidth(double value)
{
    actualUpperBandwidth = board->setUpperBandwidth(value);
    upperBandwidthSelection->setText(String(round(actualUpperBandwidth*10.f)/10.f), dontSendNotification);
}

double BandwidthInterface::getLowerBandwidth()
{
    return actualLowerBandwidth;
}

double BandwidthInterface::getUpperBandwidth()
{
    return actualUpperBandwidth;
}


void BandwidthInterface::paint(Graphics& g)
{

    g.setColour(Colours::darkgrey);

    g.setFont(Font("Small Text",10,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

    g.drawText("Low: ", 0, 10, 200, 20, Justification::left, false);

    g.drawText("High: ", 0, 30, 200, 20, Justification::left, false);

}

// Sample rate Options --------------------------------------------------------------------

SampleRateInterface::SampleRateInterface(RHD2000Thread* board_,
                                         RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name = "Sample Rate";

    sampleRateOptions.add("1.00 kS/s");
    sampleRateOptions.add("1.25 kS/s");
    sampleRateOptions.add("1.50 kS/s");
    sampleRateOptions.add("2.00 kS/s");
    sampleRateOptions.add("2.50 kS/s");
    sampleRateOptions.add("3.00 kS/s");
    sampleRateOptions.add("3.33 kS/s");
    sampleRateOptions.add("4.00 kS/s");
    sampleRateOptions.add("5.00 kS/s");
    sampleRateOptions.add("6.25 kS/s");
    sampleRateOptions.add("8.00 kS/s");
    sampleRateOptions.add("10.0 kS/s");
    sampleRateOptions.add("12.5 kS/s");
    sampleRateOptions.add("15.0 kS/s");
    sampleRateOptions.add("20.0 kS/s");
    sampleRateOptions.add("25.0 kS/s");
    sampleRateOptions.add("30.0 kS/s");


    rateSelection = new ComboBox("Sample Rate");
    rateSelection->addItemList(sampleRateOptions, 1);
    rateSelection->setSelectedId(17, dontSendNotification);
    rateSelection->addListener(this);

    rateSelection->setBounds(0,15,300,20);
    addAndMakeVisible(rateSelection);


}

SampleRateInterface::~SampleRateInterface()
{

}

void SampleRateInterface::comboBoxChanged(ComboBox* cb)
{
    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (cb == rateSelection)
        {
            board->setSampleRate(cb->getSelectedId()-1);

            std::cout << "Setting sample rate to index " << cb->getSelectedId()-1 << std::endl;

            editor->getEditorViewport()->makeEditorVisible(editor, false, true);
        }
    }
}

int SampleRateInterface::getSelectedId()
{
    return rateSelection->getSelectedId();
}

void SampleRateInterface::setSelectedId(int id)
{
    rateSelection->setSelectedId(id);
}


void SampleRateInterface::paint(Graphics& g)
{

    g.setColour(Colours::darkgrey);

    g.setFont(Font("Small Text",10,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

}


// Headstage Options --------------------------------------------------------------------

HeadstageOptionsInterface::HeadstageOptionsInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_,
                                                     int hsNum) :
    isEnabled(false), board(board_), editor(editor_)
{

    switch (hsNum)
    {
        case 0 :
            name = "A";
            break;
        case 1:
            name = "B";
            break;
        case 2:
            name = "C";
            break;
        case 3:
            name = "D";
            break;
        default:
            name = "X";
    }

    hsNumber1 = hsNum*2; // data stream 1
    hsNumber2 = hsNumber1+1; // data stream 2

    channelsOnHs1 = 0;
    channelsOnHs2 = 0;



    hsButton1 = new UtilityButton(" ", Font("Small Text", 13, Font::plain));
    hsButton1->setRadius(3.0f);
    hsButton1->setBounds(23,1,20,17);
    hsButton1->setEnabledState(false);
    hsButton1->setCorners(true, false, true, false);
    hsButton1->addListener(this);
    addAndMakeVisible(hsButton1);

    hsButton2 = new UtilityButton(" ", Font("Small Text", 13, Font::plain));
    hsButton2->setRadius(3.0f);
    hsButton2->setBounds(43,1,20,17);
    hsButton2->setEnabledState(false);
    hsButton2->setCorners(false, true, false, true);
    hsButton2->addListener(this);
    addAndMakeVisible(hsButton2);

    checkEnabledState();
}

HeadstageOptionsInterface::~HeadstageOptionsInterface()
{

}

void HeadstageOptionsInterface::checkEnabledState()
{
    isEnabled = (board->isHeadstageEnabled(hsNumber1) ||
                 board->isHeadstageEnabled(hsNumber2));

    if (board->isHeadstageEnabled(hsNumber1))
    {
		channelsOnHs1 = board->getChannelsInHeadstage(hsNumber1);
        hsButton1->setLabel(String(channelsOnHs1));
        hsButton1->setEnabledState(true);
    }
    else
    {
        channelsOnHs1 = 0;
        hsButton1->setLabel(" ");
        hsButton1->setEnabledState(false);
    }

    if (board->isHeadstageEnabled(hsNumber2))
    {
        channelsOnHs2 = board->getChannelsInHeadstage(hsNumber2);
        hsButton2->setLabel(String(channelsOnHs2));
        hsButton2->setEnabledState(true);
    }
    else
    {
        channelsOnHs2 = 0;
        hsButton2->setLabel(" ");
        hsButton2->setEnabledState(false);
    }

    repaint();

}

void HeadstageOptionsInterface::buttonClicked(Button* button)
{

   if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {

        //std::cout << "Acquisition is not active" << std::endl;
		if ((button == hsButton1) && (board->getChannelsInHeadstage(hsNumber1)))
        {
            if (channelsOnHs1 == 32)
                channelsOnHs1 = 16;
            else
                channelsOnHs1 = 32;

            //std::cout << "HS1 has " << channelsOnHs1 << " channels." << std::endl;

            hsButton1->setLabel(String(channelsOnHs1));
            board->setNumChannels(hsNumber1, channelsOnHs1);

            board->updateChannelNames();
            editor->updateSettings();

        }
		else if ((button == hsButton2) && (board->getChannelsInHeadstage(hsNumber2)))
        {
            if (channelsOnHs2 == 32)
                channelsOnHs2 = 16;
            else
                channelsOnHs2 = 32;

            hsButton2->setLabel(String(channelsOnHs2));
            board->setNumChannels(hsNumber2, channelsOnHs2);
            board->updateChannelNames();
            editor->updateSettings();
        }


        editor->getEditorViewport()->makeEditorVisible(editor, false, true);
    }

}


void HeadstageOptionsInterface::paint(Graphics& g)
{
    g.setColour(Colours::lightgrey);

    g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(Font("Small Text",15,Font::plain));

    g.drawText(name, 8, 2, 200, 15, Justification::left, false);

}


// (Direct OpalKelly) Audio Options --------------------------------------------------------------------

AudioInterface::AudioInterface(RHD2000Thread* board_,
                               RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name = "Noise Slicer";

    lastNoiseSlicerString = "0";

    actualNoiseSlicerLevel = 0.0f;

    noiseSlicerLevelSelection = new Label("Noise Slicer",lastNoiseSlicerString); // this is currently set in RHD2000Thread, the cleaner would be to set it here again
    noiseSlicerLevelSelection->setEditable(true,false,false);
    noiseSlicerLevelSelection->addListener(this);
    noiseSlicerLevelSelection->setBounds(30,10,30,20);
    noiseSlicerLevelSelection->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(noiseSlicerLevelSelection);


}

AudioInterface::~AudioInterface()
{

}


void AudioInterface::labelTextChanged(Label* label)
{
    if (board->foundInputSource())
    {
        if (label == noiseSlicerLevelSelection)
        {

            Value val = label->getTextValue();
            int requestedValue = int(val.getValue()); // Note that it might be nice to translate to actual uV levels (16*value)

            if (requestedValue < 0 || requestedValue > 127)
            {
                editor->sendActionMessage("Value out of range.");

                label->setText(lastNoiseSlicerString, dontSendNotification);

                return;
            }

            actualNoiseSlicerLevel = board->setNoiseSlicerLevel(requestedValue);

            std::cout << "Setting Noise Slicer Level to " << requestedValue << std::endl;
            label->setText(String((roundFloatToInt)(actualNoiseSlicerLevel)), dontSendNotification);

        }
    }
    else
    {
        Value val = label->getTextValue();
        int requestedValue = int(val.getValue()); // Note that it might be nice to translate to actual uV levels (16*value)
        if (requestedValue < 0 || requestedValue > 127)
        {
            editor->sendActionMessage("Value out of range.");
            label->setText(lastNoiseSlicerString, dontSendNotification);
            return;
        }
    }
}

void AudioInterface::setNoiseSlicerLevel(int value)
{
    actualNoiseSlicerLevel = board->setNoiseSlicerLevel(value);
    noiseSlicerLevelSelection->setText(String(roundFloatToInt(actualNoiseSlicerLevel)), dontSendNotification);
}

int AudioInterface::getNoiseSlicerLevel()
{
    return actualNoiseSlicerLevel;
}


void AudioInterface::paint(Graphics& g)
{

    g.setColour(Colours::darkgrey);

    g.setFont(Font("Small Text",9,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

    g.drawText("Level: ", 0, 10, 200, 20, Justification::left, false);

}



// DSP Options --------------------------------------------------------------------

DSPInterface::DSPInterface(RHD2000Thread* board_,
                           RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{
    name = "DSP";

    dspOffsetSelection = new Label("DspOffsetSelection",String(round(board->getDspCutoffFreq()*10.f)/10.f));
    dspOffsetSelection->setEditable(true,false,false);
    dspOffsetSelection->addListener(this);
    dspOffsetSelection->setBounds(0,0,30,20);
    dspOffsetSelection->setColour(Label::textColourId, Colours::darkgrey);

    addAndMakeVisible(dspOffsetSelection);

}

DSPInterface::~DSPInterface()
{

}


void DSPInterface::labelTextChanged(Label* label)
{

    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (label == dspOffsetSelection)
        {

            Value val = label->getTextValue();
            double requestedValue = double(val.getValue());

            actualDspCutoffFreq = board->setDspCutoffFreq(requestedValue);

            std::cout << "Setting DSP Cutoff Freq to " << requestedValue << std::endl;
            std::cout << "Actual DSP Cutoff Freq:  " <<  actualDspCutoffFreq  << std::endl;
            label->setText(String(round(actualDspCutoffFreq*10.f)/10.f), dontSendNotification);

        }
    }
    else if (editor->acquisitionIsActive)
    {
        editor->sendActionMessage("Can't change DSP cutoff while acquisition is active!");
    }

}

void DSPInterface::setDspCutoffFreq(double value)
{
    actualDspCutoffFreq = board->setDspCutoffFreq(value);
    dspOffsetSelection->setText(String(round(actualDspCutoffFreq*10.f)/10.f), dontSendNotification);
}


double DSPInterface::getDspCutoffFreq()
{
    return actualDspCutoffFreq;
}

void DSPInterface::paint(Graphics& g)
{

    g.setColour(Colours::darkgrey);

    g.setFont(Font("Small Text",10,Font::plain));

}
