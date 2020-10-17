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
#include <cmath>

#include "RHD2000Thread.h"
using namespace IntanRecordingController;

#ifdef WIN32
#if (_MSC_VER < 1800) //round doesn't exist on MSVC prior to 2013 version
inline double round(double x)
{
    return floor(x+0.5);
}
#endif
#endif

FPGAchannelList::FPGAchannelList(GenericProcessor* proc_, Viewport* p, FPGAcanvas* c) : chainUpdate(false), viewport(p), canvas(c)
{
    proc = (SourceNode*)proc_;
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

    RHD2000Editor* e = static_cast<RHD2000Editor*>(proc->getEditor());
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
   // const int columnWidth = 330;
	const int columnWidth = 250;
    // Query processor for number of channels, types, gains, etc... and update the UI
    channelComponents.clear();
    staticLabels.clear();

    RHD2000Thread* thread = (RHD2000Thread*)proc->getThread();
    DataChannel::DataChannelTypes type;

    // find out which streams are active.
    bool hsActive[MAX_NUM_HEADSTAGES+1];
    //bool adcActive = false;
    int numActiveHeadstages = 0;
    int hsColumn[MAX_NUM_HEADSTAGES + 1];
    int numChannelsPerHeadstage[MAX_NUM_HEADSTAGES + 1];
    chainUpdate = false;

    for (int k = 0; k<MAX_NUM_HEADSTAGES; k++)
    {
        if (thread->isHeadstageEnabled(k))
        {
            numChannelsPerHeadstage[k] = thread->getActiveChannelsInHeadstage(k);
            hsActive[k] = true;
            hsColumn[k] = numActiveHeadstages*columnWidth;
            numActiveHeadstages++;
        }
        else
        {
            numChannelsPerHeadstage[k] = 0;
            hsActive[k] = false;
            hsColumn[k] = 0;
        }
    }

    if (thread->getNumDataOutputs(DataChannel::ADC_CHANNEL,0) > 0)
    {
		numChannelsPerHeadstage[MAX_NUM_HEADSTAGES] = thread->getNumDataOutputs(DataChannel::ADC_CHANNEL, 0);
        hsActive[MAX_NUM_HEADSTAGES] = true;
        hsColumn[MAX_NUM_HEADSTAGES] = numActiveHeadstages*columnWidth;
        numActiveHeadstages++;
    }
    else
    {
        numChannelsPerHeadstage[MAX_NUM_HEADSTAGES] = 0;
        hsActive[MAX_NUM_HEADSTAGES] = false;
        hsColumn[MAX_NUM_HEADSTAGES] = 0;
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
	streamNames.add("Port E1");
	streamNames.add("Port E2");
	streamNames.add("Port F1");
	streamNames.add("Port F2");
	streamNames.add("Port G1");
	streamNames.add("Port G2");
	streamNames.add("Port H1");
	streamNames.add("Port H2");
    streamNames.add("ADC");

    for (int k = 0; k < MAX_NUM_HEADSTAGES + 1; k++)
    {
        if (hsActive[k])
        {
            Label* lbl = new Label(streamNames[k],streamNames[k]);
            lbl->setEditable(false);
            lbl->setBounds(10+hsColumn[k],40,columnWidth, 25);
            lbl->setJustificationType(juce::Justification::centred);
            lbl->setColour(Label::textColourId,juce::Colours::white);
            staticLabels.add(lbl);
            addAndMakeVisible(lbl);

        }

    }

    for (int k = 0; k < MAX_NUM_HEADSTAGES + 1; k++)
    {
        if (hsActive[k])
        {
            for (int ch = 0; ch < numChannelsPerHeadstage[k]+ (k < MAX_NUM_HEADSTAGES ? 3 : 0); ch++)
            {
                int channelGainIndex = 1;
                int realChan = thread->getChannelFromHeadstage(k, ch);
                float ch_gain = proc->getDataChannel(realChan)->getBitVolts() / proc->getBitVolts(proc->getDataChannel(realChan));
                for (int j = 0; j < gains.size(); j++)
                {
                    if (fabs(gains[j] - ch_gain) < 1e-3)
                    {
                        channelGainIndex = j;
                        break;
                    }
                }
                if (k < MAX_NUM_HEADSTAGES)
                    type = ch < numChannelsPerHeadstage[k] ? DataChannel::HEADSTAGE_CHANNEL : DataChannel::AUX_CHANNEL;
                else
                    type = DataChannel::ADC_CHANNEL;

                FPGAchannelComponent* comp = new FPGAchannelComponent(this, realChan, channelGainIndex + 1, thread->getChannelName(realChan), gains,type);
                comp->setBounds(10 + hsColumn[k], 70 + ch * 22, columnWidth, 22);
                comp->setUserDefinedData(k);
                addAndMakeVisible(comp);
                channelComponents.add(comp);
            }
        }
    }


    StringArray ttlNames;
    proc->getEventChannelNames(ttlNames);
    // add buttons for TTL channels
    for (int k=0; k<ttlNames.size(); k++)
    {
        FPGAchannelComponent* comp = new FPGAchannelComponent(this,k, -1, ttlNames[k],gains,DataChannel::INVALID); //let's treat invalid as an event channel
        comp->setBounds(10+numActiveHeadstages*columnWidth,70+k*22,columnWidth,22);
        comp->setUserDefinedData(k);
        addAndMakeVisible(comp);
        channelComponents.add(comp);
    }

    Label* lbl = new Label("TTL Events","TTL Events");
    lbl->setEditable(false);
    lbl->setBounds(numActiveHeadstages*columnWidth,40,columnWidth, 25);
    lbl->setJustificationType(juce::Justification::centred);
    lbl->setColour(Label::textColourId,juce::Colours::white);
    staticLabels.add(lbl);
    addAndMakeVisible(lbl);

    chainUpdate = true;
}

void FPGAchannelList::disableAll()
{
    for (int k=0; k<channelComponents.size(); k++)
    {
        channelComponents[k]->disableEdit();
    }
	impedanceButton->setEnabled(false);
	saveImpedanceButton->setEnabled(false);
	autoMeasureButton->setEnabled(false);
	numberingScheme->setEnabled(false);
}

void FPGAchannelList::enableAll()
{
    for (int k=0; k<channelComponents.size(); k++)
    {
        channelComponents[k]->enableEdit();
    }
	impedanceButton->setEnabled(true);
	saveImpedanceButton->setEnabled(true);
	autoMeasureButton->setEnabled(true);
	numberingScheme->setEnabled(true);
}

void FPGAchannelList::setNewGain(int channel, float gain)
{
    RHD2000Thread* thread = (RHD2000Thread*)proc->getThread();
    thread->modifyChannelGain(channel, gain);
    if (chainUpdate)
        proc->requestChainUpdate();
}

void FPGAchannelList::setNewName(int channel, String newName)
{
    RHD2000Thread* thread = (RHD2000Thread*)proc->getThread();
    thread->modifyChannelName(channel, newName);
    if (chainUpdate)
        proc->requestChainUpdate();
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
        RHD2000Thread* thread = (RHD2000Thread*)p->getThread();
        int scheme = numberingScheme->getSelectedId();
        thread->setDefaultNamingScheme(scheme);
        update();
        p->requestChainUpdate();
    }
}

void FPGAchannelList::updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase)
{
	int i = 0;
    for (int k = 0; k < streams.size(); k++)
    {
		if (i >= channelComponents.size())
			break; //little safety

		if (channelComponents[i]->type != DataChannel::HEADSTAGE_CHANNEL)
		{
			k--;
		}
		else
		{
			channelComponents[i]->setImpedanceValues(magnitude[k], phase[k]);
		}
		i++;
    }

}


/****************************************************/
FPGAchannelComponent::FPGAchannelComponent(FPGAchannelList* cl, int ch, int gainIndex_, String N, Array<float> gains_, DataChannel::DataChannelTypes type_) :
type(type_), gains(gains_), channelList(cl), channel(ch), name(N), gainIndex(gainIndex_)
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
/*    if (gainIndex > 0)
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
        gainComboBox->setSelectedId(gainIndex, sendNotificationSync);
        gainComboBox->addListener(this);
        addAndMakeVisible(gainComboBox);
    }
    else
    {*/
        gainComboBox = nullptr;
    //}

    if (type == DataChannel::HEADSTAGE_CHANNEL)
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
        float bitvolts = channelList->proc->getBitVolts(channelList->proc->getDataChannel(channel));
        channelList->setNewGain(channel, mult*bitvolts);
    }
}
void FPGAchannelComponent::labelTextChanged(Label* lbl)
{
    // channel name change
    String newName = lbl->getText();
    channelList->setNewName(channel, newName);
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
       // impedance->setBounds(180,0,130,20);
		impedance->setBounds(100, 0, 130, 20);
    }

}



/**********************************************/

FPGAcanvas::FPGAcanvas(GenericProcessor* n)
{
    processor = (SourceNode*)n;
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
	if (static_cast<RHD2000Thread*>(processor->getThread())->isAcquisitionActive())
	{
		channelList->disableAll();
	}
}

void FPGAcanvas::resized()
{
    //int screenWidth = getWidth();
    //int screenHeight = getHeight();

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

#define HS_WIDTH 70

RHD2000Editor::RHD2000Editor(GenericProcessor* parentNode,
                             RHD2000Thread* board_,
                             bool useDefaultParameterEditors
                            )
    : VisualizerEditor(parentNode, useDefaultParameterEditors), board(board_)
{
    canvas = nullptr;
	desiredWidth = 340 + HS_WIDTH;
    tabText = "FPGA";
    measureWhenRecording = false;
    saveImpedances = false;

	impedanceData = new ImpedanceData();
	impedanceData->valid = false;

    // add headstage-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 8; i++)
    {
        HeadstageOptionsInterface* hsOptions = new HeadstageOptionsInterface(board, this, i);
        headstageOptionsInterfaces.add(hsOptions);
        addAndMakeVisible(hsOptions);
		hsOptions->setBounds(3 + (i / 4)*HS_WIDTH, 28 + (i % 4) * 20, 70, 18);
    }

    // add sample rate selection
    sampleRateInterface = new SampleRateInterface(board, this);
    addAndMakeVisible(sampleRateInterface);
	sampleRateInterface->setBounds(80 + HS_WIDTH, 25, 110, 50);

    // add Bandwidth selection
    bandwidthInterface = new BandwidthInterface(board, this);
    addAndMakeVisible(bandwidthInterface);
	bandwidthInterface->setBounds(80 + HS_WIDTH, 58, 80, 50);

    // add DSP selection
  //  dspInterface = new DSPInterface(board, this);
  //  addAndMakeVisible(dspInterface);
  //  dspInterface->setBounds(80, 58, 80, 50);

    // add rescan button
    rescanButton = new UtilityButton("RESCAN", Font("Small Text", 13, Font::plain));
    rescanButton->setRadius(3.0f);
	rescanButton->setBounds(6 + HS_WIDTH, 108, 65, 18);
    rescanButton->addListener(this);
    rescanButton->setTooltip("Check for connected headstages");
    addAndMakeVisible(rescanButton);

    for (int i = 0; i < 2; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(-1);
        electrodeButtons.add(button);

		button->setBounds(200 + i * 25 + HS_WIDTH, 40, 25, 15);
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
	audioLabel->setBounds(190 + HS_WIDTH, 25, 75, 15);
    audioLabel->setFont(Font("Small Text", 10, Font::plain));
    audioLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(audioLabel);

    // add HW audio parameter selection
    audioInterface = new AudioInterface(board, this);
    addAndMakeVisible(audioInterface);
	audioInterface->setBounds(179 + HS_WIDTH, 58, 70, 50);

    adcButton = new UtilityButton("ADC 1-8", Font("Small Text", 13, Font::plain));
    adcButton->setRadius(3.0f);
	adcButton->setBounds(179 + HS_WIDTH, 108, 70, 18);
    adcButton->addListener(this);
    adcButton->setClickingTogglesState(true);
    adcButton->setTooltip("Enable/disable ADC channels");
    addAndMakeVisible(adcButton);

    // add DSP Offset Button
    dspoffsetButton = new UtilityButton("DSP", Font("Very Small Text", 13, Font::plain));
    dspoffsetButton->setRadius(3.0f); // sets the radius of the button's corners
	dspoffsetButton->setBounds(80 + HS_WIDTH, 108, 30, 18); // sets the x position, y position, width, and height of the button
    dspoffsetButton->addListener(this);
    dspoffsetButton->setClickingTogglesState(true); // makes the button toggle its state when clicked
    dspoffsetButton->setTooltip("Enable/disable DSP offset removal");
    addAndMakeVisible(dspoffsetButton); // makes the button a child component of the editor and makes it visible
    dspoffsetButton->setToggleState(true, dontSendNotification);

    // add DSP Frequency Selection field
    dspInterface = new DSPInterface(board, this);
    addAndMakeVisible(dspInterface);
	dspInterface->setBounds(110 + HS_WIDTH, 108, 60, 50);

    ttlSettleLabel = new Label("TTL Settle","TTL Settle");
    ttlSettleLabel->setFont(Font("Small Text", 11, Font::plain));
	ttlSettleLabel->setBounds(255 + HS_WIDTH, 80, 70, 20);
    ttlSettleLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(ttlSettleLabel);


    ttlSettleCombo = new ComboBox("FastSettleComboBox");
	ttlSettleCombo->setBounds(260 + HS_WIDTH, 100, 60, 18);
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
	dacTTLButton->setBounds(260 + HS_WIDTH, 25, 65, 18);
    dacTTLButton->addListener(this);
    dacTTLButton->setClickingTogglesState(true);
    dacTTLButton->setTooltip("Enable/disable DAC Threshold TTL Output");
    addAndMakeVisible(dacTTLButton);

    dacHPFlabel = new Label("DAC HPF","DAC HPF");
    dacHPFlabel->setFont(Font("Small Text", 11, Font::plain));
	dacHPFlabel->setBounds(260 + HS_WIDTH, 42, 65, 20);
    dacHPFlabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(dacHPFlabel);

    dacHPFcombo = new ComboBox("dacHPFCombo");
	dacHPFcombo->setBounds(260 + HS_WIDTH, 60, 60, 18);
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
	impedanceData->valid = false;
	board->runImpedanceTest(impedanceData);
}

void RHD2000Editor::handleAsyncUpdate()
{
	if (!impedanceData->valid)
		return;
    if (canvas == nullptr)
        VisualizerEditor::canvas = createNewCanvas();
    // update components...
	canvas->updateImpedance(impedanceData->streams, impedanceData->channels, impedanceData->magnitudes, impedanceData->phases);
    if (saveImpedances)
    {
        // this may not work with new Record Node architecture
		CoreServices::RecordNode::createNewrecordingDir();

		String path(CoreServices::RecordNode::getRecordingPath().getFullPathName()
                    + File::separatorString + "impedance_measurement.xml");
        std::cout << "Saving impedance measurements in " << path << "\n";
        File file(path);

        if (!file.getParentDirectory().exists())
            file.getParentDirectory().createDirectory();

        XmlDocument doc(file);
        ScopedPointer<XmlElement> xml = new XmlElement("CHANNEL_IMPEDANCES");
		for (int i = 0; i < impedanceData->channels.size(); i++)
        {
            XmlElement* chan = new XmlElement("CHANNEL");
            chan->setAttribute("name",board->getChannelName(i));
			chan->setAttribute("stream", impedanceData->streams[i]);
			chan->setAttribute("channel_number", impedanceData->channels[i]);
			chan->setAttribute("magnitude", impedanceData->magnitudes[i]);
			chan->setAttribute("phase", impedanceData->phases[i]);
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
    if (button == rescanButton && !acquisitionIsActive)
    {
        board->scanPorts();

        for (int i = 0; i < 4; i++)
        {
            headstageOptionsInterfaces[i]->checkEnabledState();
        }
        // board->updateChannelNames();
		CoreServices::updateSignalChain(this);
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
        std::cout << "ADC Button toggled" << "\n";
		CoreServices::updateSignalChain(this);
        std::cout << "Editor visible." << "\n";
    }
    else if (button == dacTTLButton)
    {
        board->setTTLoutputMode(dacTTLButton->getToggleState());
    }
    else if (button == dspoffsetButton && !acquisitionIsActive)
    {
        std::cout << "DSP offset " << button->getToggleState() << "\n";
        board->setDSPOffset(button->getToggleState());
    }

    /*
	else
	{
		VisualizerEditor::buttonEvent(button);
	}
    */

}

void RHD2000Editor::channelChanged (int channel, bool /*newState*/)
{
    // Audio output is tied to DAC channels 0 and 1
    for (int i = 0; i < 2; i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum (channel);
            electrodeButtons[i]->repaint();
            board->setDACchannel (i, channel - 1); // HW channels are zero-based
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
	if (canvas != nullptr)
		canvas->channelList->disableAll();
        //canvas->channelList->setEnabled(false);
}

void RHD2000Editor::stopAcquisition()
{

    channelSelector->stopAcquisition();

    rescanButton->setEnabledState(true);
    adcButton->setEnabledState(true);
    dspoffsetButton-> setEnabledState(true);

    acquisitionIsActive = false;
	if (canvas != nullptr)
		canvas->channelList->enableAll();
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
    //electrodeButtons[0]->setChannelNum(xml->getIntAttribute("AudioOutputL"));
    //board->assignAudioOut(0, xml->getIntAttribute("AudioOutputL"));
    //electrodeButtons[1]->setChannelNum(xml->getIntAttribute("AudioOutputR"));
    //board->assignAudioOut(1, xml->getIntAttribute("AudioOutputR"));
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
                CoreServices::sendStatusMessage("Value out of range.");

                label->setText(lastHighCutString, dontSendNotification);

                return;
            }

            actualUpperBandwidth = board->setUpperBandwidth(requestedValue);

            std::cout << "Setting Upper Bandwidth to " << requestedValue << "\n";
            std::cout << "Actual Upper Bandwidth:  " <<  actualUpperBandwidth  << "\n";
            label->setText(String(round(actualUpperBandwidth*10.f)/10.f), dontSendNotification);

        }
        else
        {

            Value val = label->getTextValue();
            double requestedValue = double(val.getValue());

            if (requestedValue < 0.1 || requestedValue > 500.0 || requestedValue > lastHighCutString.getFloatValue())
            {
				CoreServices::sendStatusMessage("Value out of range.");

                label->setText(lastLowCutString, dontSendNotification);

                return;
            }

            actualLowerBandwidth = board->setLowerBandwidth(requestedValue);

            std::cout << "Setting Lower Bandwidth to " << requestedValue << "\n";
            std::cout << "Actual Lower Bandwidth:  " <<  actualLowerBandwidth  << "\n";

            label->setText(String(round(actualLowerBandwidth*10.f)/10.f), dontSendNotification);
        }
    }
    else if (editor->acquisitionIsActive)
    {
		CoreServices::sendStatusMessage("Can't change bandwidth while acquisition is active!");
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

            std::cout << "Setting sample rate to index " << cb->getSelectedId()-1 << "\n";

			CoreServices::updateSignalChain(editor);
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
		case 4:
			name = "E";
			break;
		case 5:
			name = "F";
			break;
		case 6:
			name = "G";
			break;
		case 7:
			name = "H";
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
        channelsOnHs1 = board->getActiveChannelsInHeadstage(hsNumber1);
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
        channelsOnHs2 = board->getActiveChannelsInHeadstage(hsNumber2);
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

        //std::cout << "Acquisition is not active" << "\n";
        if ((button == hsButton1) && (board->getChannelsInHeadstage(hsNumber1) == 32))
        {
            if (channelsOnHs1 == 32)
                channelsOnHs1 = 16;
            else
                channelsOnHs1 = 32;

            //std::cout << "HS1 has " << channelsOnHs1 << " channels." << "\n";

            hsButton1->setLabel(String(channelsOnHs1));
            board->setNumChannels(hsNumber1, channelsOnHs1);

            //board->updateChannels();
            editor->updateSettings();

        }
        else if ((button == hsButton2) && (board->getChannelsInHeadstage(hsNumber2) == 32))
        {
            if (channelsOnHs2 == 32)
                channelsOnHs2 = 16;
            else
                channelsOnHs2 = 32;

            hsButton2->setLabel(String(channelsOnHs2));
            board->setNumChannels(hsNumber2, channelsOnHs2);
            //board->updateChannels();
            editor->updateSettings();
        }

		CoreServices::updateSignalChain(editor);
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
				CoreServices::sendStatusMessage("Value out of range.");

                label->setText(lastNoiseSlicerString, dontSendNotification);

                return;
            }

            actualNoiseSlicerLevel = board->setNoiseSlicerLevel(requestedValue);

            std::cout << "Setting Noise Slicer Level to " << requestedValue << "\n";
            label->setText(String((roundFloatToInt)(actualNoiseSlicerLevel)), dontSendNotification);

        }
    }
    else
    {
        Value val = label->getTextValue();
        int requestedValue = int(val.getValue()); // Note that it might be nice to translate to actual uV levels (16*value)
        if (requestedValue < 0 || requestedValue > 127)
        {
			CoreServices::sendStatusMessage("Value out of range.");
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

            std::cout << "Setting DSP Cutoff Freq to " << requestedValue << "\n";
            std::cout << "Actual DSP Cutoff Freq:  " <<  actualDspCutoffFreq  << "\n";
            label->setText(String(round(actualDspCutoffFreq*10.f)/10.f), dontSendNotification);

        }
    }
    else if (editor->acquisitionIsActive)
    {
		CoreServices::sendStatusMessage("Can't change DSP cutoff while acquisition is active!");
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
