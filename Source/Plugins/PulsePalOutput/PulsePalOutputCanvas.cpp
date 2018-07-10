/*
    ------------------------------------------------------------------

    This file is part of the Tracking plugin for the Open Ephys GUI
    Written by:

    Alessio Buccino     alessiob@ifi.uio.no
    Mikkel Lepperod
    Svenn-Arne Dragly

    Center for Integrated Neuroplasticity CINPLA
    Department of Biosciences
    University of Oslo
    Norway

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


#include "PulsePalOutputCanvas.h"
#include "PulsePalOutput.h"


PulsePalOutputCanvas::PulsePalOutputCanvas(PulsePalOutput* PulsePalOutput)
    : processor(PulsePalOutput)
    , labelColour(Colour(200, 255, 0))
    , labelTextColour(Colour(255, 200, 0))
    , labelBackgroundColour(Colour(100,100,100))

{
    // Setup buttons
    initButtons();
    // Setup Labels
    initLabels();

    addKeyListener(this);

    startCallbacks();
    update();
}

PulsePalOutputCanvas::~PulsePalOutputCanvas()
{
    TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

float PulsePalOutputCanvas::my_round(float x)
{
    return x < 0.0 ? ceil(x - 0.5) : floor(x + 0.5);
}


void PulsePalOutputCanvas::paint (Graphics& g)
{

    g.setColour(labelBackgroundColour); // backbackround color
    g.fillRect(0, 0, getWidth(), getHeight());


    // Check pulse Pal connection
    if (processor->getPulsePalVersion() > 0)
        pulsePalLabel->setText(String("Pulse Pal: ") +=  String("CONNECTED"), dontSendNotification);
    else
        pulsePalLabel->setText(String("Pulse Pal: ") +=  String("NOT CONNECTED"), dontSendNotification);

    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        g.setColour(Colours::grey);
        g.fillRoundedRectangle(0.005*getWidth() + 0.25*i*getWidth(), 0.1*getHeight(), 0.23*getWidth(), 0.89*getHeight(), 4.0f);
    }

}

void PulsePalOutputCanvas::resized()
{
    pulsePalLabel->setBounds(0.01*getWidth(), 0.02*getHeight(), 0.5*getWidth(),0.04*getHeight());

    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        channelLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.1*getHeight(), 0.1*getWidth(),0.04*getHeight());

        phase1Label[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.2*getHeight(), 0.1*getWidth(),0.04*getHeight());
        phase1EditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.2*getHeight(), 0.1*getWidth(),0.04*getHeight());
        voltage1Label[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.25*getHeight(), 0.1*getWidth(),0.04*getHeight());
        voltage1EditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.25*getHeight(), 0.1*getWidth(),0.04*getHeight());
        phase2Label[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.3*getHeight(), 0.1*getWidth(),0.04*getHeight());
        phase2EditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.3*getHeight(), 0.1*getWidth(),0.04*getHeight());
        voltage2Label[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.35*getHeight(), 0.1*getWidth(),0.04*getHeight());
        voltage2EditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.35*getHeight(), 0.1*getWidth(),0.04*getHeight());
        interphaseLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.4*getHeight(), 0.1*getWidth(),0.04*getHeight());
        interphaseEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.4*getHeight(), 0.1*getWidth(),0.04*getHeight());

        restingVoltageLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.45*getHeight(), 0.1*getWidth(),0.04*getHeight());
        restingVoltageEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.45*getHeight(), 0.1*getWidth(),0.04*getHeight());
        interpulseLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.5*getHeight(), 0.1*getWidth(),0.04*getHeight());
        interpulseEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.5*getHeight(), 0.1*getWidth(),0.04*getHeight());
        burstDurationLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.55*getHeight(), 0.1*getWidth(),0.04*getHeight());
        burstDurationEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.55*getHeight(), 0.1*getWidth(),0.04*getHeight());
        interburstLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.6*getHeight(), 0.1*getWidth(),0.04*getHeight());
        interburstEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.6*getHeight(), 0.1*getWidth(),0.04*getHeight());
        trainDurationLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.65*getHeight(), 0.1*getWidth(),0.04*getHeight());
        trainDurationEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.65*getHeight(), 0.1*getWidth(),0.04*getHeight());
        trainDelayLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.7*getHeight(), 0.1*getWidth(),0.04*getHeight());
        trainDelayEditLabel[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.7*getHeight(), 0.1*getWidth(),0.04*getHeight());
        triggerModeLabel[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.75*getHeight(), 0.1*getWidth(),0.04*getHeight());
        triggerMode[i]->setBounds(0.13*getWidth() + 0.25*i*getWidth(), 0.75*getHeight(), 0.1*getWidth(),0.04*getHeight());

        link2tr1Button[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.83*getHeight(), 0.11*getWidth(), 0.03*getHeight());
        link2tr2Button[i]->setBounds(0.12*getWidth() + 0.25*i*getWidth(), 0.83*getHeight(), 0.11*getWidth(), 0.03*getHeight());
        biphasicButton[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.87*getHeight(), 0.11*getWidth(), 0.03*getHeight());
        burstButton[i]->setBounds(0.12*getWidth() + 0.25*i*getWidth(), 0.87*getHeight(), 0.11*getWidth(), 0.03*getHeight());
        ttlButton[i]->setBounds(0.01*getWidth() + 0.25*i*getWidth(), 0.91*getHeight(), 0.11*getWidth(), 0.06*getHeight());
		continuousButton[i]->setBounds(0.12*getWidth() + 0.25*i*getWidth(), 0.91*getHeight(), 0.11*getWidth(), 0.06*getHeight());
    }

    refresh();
}

void PulsePalOutputCanvas::buttonClicked(Button* button)
{
    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        if (button == biphasicButton[i])
        {
            if (button->getToggleState()==true)
            {
                processor->setIsBiphasic(i, true);
                phase2Label[i]->setVisible(true);
                phase2EditLabel[i]->setVisible(true);
                interphaseLabel[i]->setVisible(true);
                interphaseEditLabel[i]->setVisible(true);
                voltage2Label[i]->setVisible(true);
                voltage2EditLabel[i]->setVisible(true);
            }
            else
            {
                processor->setIsBiphasic(i, false);
                phase2Label[i]->setVisible(false);
                phase2EditLabel[i]->setVisible(false);
                interphaseLabel[i]->setVisible(false);
                interphaseEditLabel[i]->setVisible(false);
                voltage2Label[i]->setVisible(false);
                voltage2EditLabel[i]->setVisible(false);
            }
        }
        else if (button == burstButton[i])
        {
            if (button->getToggleState()==true)
            {
                if (processor->getBurstDuration(i) == 0)
                {
                    processor->setBurstDuration(i, DEF_BURSTDURATION);
                    processor->setInterBurstInt(i, DEF_INTER_BURST);
                }
                burstDurationLabel[i]->setVisible(true);
                burstDurationEditLabel[i]->setVisible(true);
                interburstLabel[i]->setVisible(true);
                interburstEditLabel[i]->setVisible(true);
            }
            else
            {
                if (processor->getBurstDuration(i) != 0)
                {
                    processor->setBurstDuration(i, 0);
                    processor->setInterBurstInt(i, 0);
                }
                burstDurationLabel[i]->setVisible(false);
                burstDurationEditLabel[i]->setVisible(false);
                interburstLabel[i]->setVisible(false);
                interburstEditLabel[i]->setVisible(false);
            }
        }
        else if (button == link2tr1Button[i])
        {
            if (button->getToggleState()==true)
                processor->setLinkTriggerChannel1(i, 1);
            else if (button->getToggleState()==false)
                processor->setLinkTriggerChannel1(i, 0);
        }
        else if (button == link2tr2Button[i])
        {
            if (button->getToggleState()==true)
                processor->setLinkTriggerChannel2(i, 1);
            else if (button->getToggleState()==false)
                processor->setLinkTriggerChannel2(i, 0);
        }
        else if (button == ttlButton[i])
        {
            // set ttl channel i
            processor->setTTLsettings(i);
            processor->setIsBiphasic(i, false);
            phase2Label[i]->setVisible(false);
            phase2EditLabel[i]->setVisible(false);
            interphaseLabel[i]->setVisible(false);
            interphaseEditLabel[i]->setVisible(false);
            voltage2Label[i]->setVisible(false);
            voltage2EditLabel[i]->setVisible(false);
            burstDurationLabel[i]->setVisible(false);
            burstDurationEditLabel[i]->setVisible(false);
            interburstLabel[i]->setVisible(false);
            interburstEditLabel[i]->setVisible(false);
            if (biphasicButton[i]->getToggleState()==true)
            {
                biphasicButton[i]->setToggleState(false, dontSendNotification);
            }
            if (burstButton[i]->getToggleState()==true)
            {
                burstButton[i]->setToggleState(false, dontSendNotification);
            }
        }
		else if (button == continuousButton[i])
		{
			if (button->getToggleState() == true)
				processor->setContinuous(i, 1);
			else if (button->getToggleState() == false)
				processor->setContinuous(i, 0);
		}
        if (!processor->checkParameterConsistency(i))
        {
            CoreServices::sendStatusMessage("Inconsistent parameters: set train duration first");
            processor->adjustParameters(i);
        }
        updateLabels(i);
        processor->updatePulsePal(i);
    }
    repaint();
}

bool PulsePalOutputCanvas::keyPressed(const KeyPress &key, Component *originatingComponent)
{
	return false;
}

void PulsePalOutputCanvas::labelTextChanged(Label *label)
{
    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        if (label == phase1EditLabel[i])
        {
            // 100 - 3600*10e3 (3600 s)
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue()) >= 0 && int(val.getValue())<=MAX_INTERVAL))
            {
                processor->setPhase1Duration(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600000 with 0.1 step!");
                label->setText("", dontSendNotification);

            }
        }
        else if (label == phase2EditLabel[i])
        {
            // 100 - 3600000 (3600 s)
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue())>=0 && int(val.getValue())<=MAX_INTERVAL))
            {
                processor->setPhase2Duration(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);

            }
        }
        else if (label == interphaseEditLabel[i])
        {
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue())>=0 && int(val.getValue())<=MAX_INTERVAL))
            {
                processor->setInterPhaseInt(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == voltage1EditLabel[i])
        {
            Value val = label->getTextValue();
            if (float(val.getValue()) >= 0 && float(val.getValue()) <= MAX_VOLTAGE)
            {
                processor->setVoltage1(i, float(val.getValue()));
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 10 with 0.05 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == voltage2EditLabel[i])
        {
            Value val = label->getTextValue();
            if (float(val.getValue()) >= 0 && float(val.getValue()) <= MAX_VOLTAGE)
            {
                processor->setVoltage2(i, float(val.getValue()));
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 10 with 0.05 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == burstDurationEditLabel[i])
        {
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue()) >= 0 && int(val.getValue()) <= MAX_INTERVAL))
            {
                processor->setBurstDuration(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == interburstEditLabel[i])
        {
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue()) >= 0 && int(val.getValue()) <= MAX_INTERVAL))
            {
                processor->setInterBurstInt(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == interpulseEditLabel[i])
        {
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue()) >= 0 && int(val.getValue()) <= MAX_INTERVAL))
            {
                processor->setInterPulseInt(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == trainDurationEditLabel[i])
        {
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue()) >= 0 && int(val.getValue()) <= MAX_INTERVAL))
            {
                processor->setTrainDuration(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);
            }
        }
        else if (label == trainDelayEditLabel[i])
        {
            Value val = label->getTextValue();
            float value = float(my_round(float(val.getValue())*10) / 10); //only multiple of 100us
            if ((float(val.getValue()) >= 0 && int(val.getValue()) <= MAX_INTERVAL))
            {
                processor->setTrainDelay(i, value);
                label->setText(String(value), dontSendNotification);
            }
            else
            {
                CoreServices::sendStatusMessage("Selected values must be within 0 and 3600*10e3 with 0.1 step!");
                label->setText("", dontSendNotification);
            }
        }
        if (!processor->checkParameterConsistency(i))
        {
            CoreServices::sendStatusMessage("Inconsistent parameters: set train duration first");
            processor->adjustParameters(i);
        }
        updateLabels(i);
        processor->updatePulsePal(i);
    }
}

void PulsePalOutputCanvas::comboBoxChanged(ComboBox *combobox)
{
    for (int i = 0; i < PULSEPALCHANNELS; i++)
        if (combobox == triggerMode[i])
            processor->setTriggerMode(i, combobox->getSelectedId() - 1);
}

void PulsePalOutputCanvas::initButtons()
{

    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        ScopedPointer<UtilityButton> biph = new UtilityButton("biphasic", Font("Small Text", 20, Font::plain));
        biph->setRadius(3.0f);
        biph->addListener(this);
        biph->setClickingTogglesState(true);
        biphasicButton[i] = biph;
        addAndMakeVisible(biphasicButton[i]);

        ScopedPointer<UtilityButton> burst = new UtilityButton("burst", Font("Small Text", 20, Font::plain));
        burst->setRadius(3.0f);
        burst->addListener(this);
        burst->setClickingTogglesState(true);
        burstButton[i] = burst;
        addAndMakeVisible(burstButton[i]);

        ScopedPointer<UtilityButton> link21 = new UtilityButton("link trig 1", Font("Small Text", 20, Font::plain));
        link21->setRadius(3.0f);
        link21->addListener(this);
        link21->setClickingTogglesState(true);
        link2tr1Button[i] = link21;
        addAndMakeVisible(link2tr1Button[i]);

        ScopedPointer<UtilityButton> link22 = new UtilityButton("link trig 2", Font("Small Text", 20, Font::plain));
        link22->setRadius(3.0f);
        link22->addListener(this);
        link22->setClickingTogglesState(true);
        link2tr2Button[i] = link22;
        addAndMakeVisible(link2tr2Button[i]);

        ScopedPointer<UtilityButton> ttl = new UtilityButton("ttl", Font("Small Text", 20, Font::plain));
        ttl->setRadius(3.0f);
        ttl->addListener(this);
        ttlButton[i] = ttl;
        addAndMakeVisible(ttlButton[i]);

		ScopedPointer<UtilityButton> continuous = new UtilityButton("continuous", Font("Small Text", 20, Font::plain));;
		continuous->setRadius(3.0f);
		continuous->addListener(this);
		continuous->setClickingTogglesState(true);
		continuousButton[i] = continuous;
		addAndMakeVisible(continuousButton[i]);


        ScopedPointer<ComboBox> mode = new ComboBox();
        mode->addListener(this);
        for (int i = 0; i < 3; i++)
            mode->addItem(String(i), i+1);

        // user-visible channels
        mode->setSelectedId(processor->getTriggerMode(i)+1, dontSendNotification);
        triggerMode[i] = mode;
        addAndMakeVisible(triggerMode[i]);


        if (processor->getIsBiphasic(i))
            biphasicButton[i]->triggerClick();
        if (processor->getLinkTriggerChannel1(i))
            link2tr1Button[i]->triggerClick();
        if (processor->getLinkTriggerChannel2(i))
            link2tr2Button[i]->triggerClick();
    }
}

void PulsePalOutputCanvas::updateLabels(int i)
{
    phase1EditLabel[i]->setText(String(processor->getPhase1Duration(i)), dontSendNotification);
    phase2EditLabel[i]->setText(String(processor->getPhase2Duration(i)), dontSendNotification);
    voltage1EditLabel[i]->setText(String(processor->getVoltage1(i)), dontSendNotification);
    voltage2EditLabel[i]->setText(String(processor->getVoltage2(i)), dontSendNotification);
    interphaseEditLabel[i]->setText(String(processor->getInterPhaseInt(i)), dontSendNotification);
    restingVoltageEditLabel[i]->setText(String(processor->getRestingVoltage(i)), dontSendNotification);
    interpulseEditLabel[i]->setText(String(processor->getInterPulseInt(i)), dontSendNotification);
    burstDurationEditLabel[i]->setText(String(processor->getBurstDuration(i)), dontSendNotification);
    interburstEditLabel[i]->setText(String(processor->getInterBurstInt(i)), dontSendNotification);
    trainDurationEditLabel[i]->setText(String(processor->getTrainDuration(i)), dontSendNotification);
    trainDelayEditLabel[i]->setText(String(processor->getTrainDelay(i)), dontSendNotification);
}

void PulsePalOutputCanvas::initLabels()
{
    pulsePalLabel = new Label("s_pulsePal", "Pulse Pal Status: ");
    pulsePalLabel->setFont(Font(40));
    pulsePalLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(pulsePalLabel);

    pulsePalLabel = new Label("s_pp", "Pulse Pal:");
    pulsePalLabel->setFont(Font(40));
    pulsePalLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(pulsePalLabel);

    for (int i = 0; i < PULSEPALCHANNELS; i++)
    {
        ScopedPointer<Label> chan = new Label("s_phase1", "Channel "+String(i+1));
        chan->setFont(Font(30));
        chan->setColour(Label::textColourId, labelColour);
        channelLabel[i] = chan;
        addAndMakeVisible(channelLabel[i]);

        ScopedPointer<Label> ph1 = new Label("s_phase1", "phase1 [ms]:");
        ph1->setFont(Font(20));
        ph1->setColour(Label::textColourId, labelColour);
        phase1Label[i] = ph1;
        addAndMakeVisible(phase1Label[i]);

        ScopedPointer<Label> ph2 = new Label("s_phase2", "phase2 [ms]:");
        ph2->setFont(Font(20));
        ph2->setColour(Label::textColourId, labelColour);
        phase2Label[i] = ph2;
        addAndMakeVisible(phase2Label[i]);
        phase2Label[i]->setVisible(false);

        ScopedPointer<Label> intph = new Label("s_interphase", "interphase [ms]:");
        intph->setFont(Font(20));
        intph->setColour(Label::textColourId, labelColour);
        interphaseLabel[i] = intph;
        addAndMakeVisible(interphaseLabel[i]);
        interphaseLabel[i]->setVisible(false);

        ScopedPointer<Label> v1 = new Label("s_v1", "voltage1 [V]:");
        v1->setFont(Font(20));
        v1->setColour(Label::textColourId, labelColour);
        voltage1Label[i] = v1;
        addAndMakeVisible(voltage1Label[i]);

        ScopedPointer<Label> v2 = new Label("s_v2", "voltage2 [V]:");
        v2->setFont(Font(20));
        v2->setColour(Label::textColourId, labelColour);
        voltage2Label[i] = v2;
        addAndMakeVisible(voltage2Label[i]);
        voltage2Label[i]->setVisible(false);

        ScopedPointer<Label> rv = new Label("s_v2", "rest voltage [V]:");
        rv->setFont(Font(20));
        rv->setColour(Label::textColourId, labelColour);
        restingVoltageLabel[i] = rv;
        addAndMakeVisible(restingVoltageLabel[i]);

        ScopedPointer<Label> intpul = new Label("s_intpul", "interpulse [ms]:");
        intpul->setFont(Font(20));
        intpul->setColour(Label::textColourId, labelColour);
        interpulseLabel[i] = intpul;
        addAndMakeVisible(interpulseLabel[i]);

        ScopedPointer<Label> burst = new Label("s_train", "burst duration [ms]:");
        burst->setFont(Font(20));
        burst->setColour(Label::textColourId, labelColour);
        burstDurationLabel[i] = burst;
        addAndMakeVisible(burstDurationLabel[i]);
        burstDurationLabel[i]->setVisible(false);

        ScopedPointer<Label> burstint = new Label("s_train", "inter burst [ms]:");
        burstint->setFont(Font(20));
        burstint->setColour(Label::textColourId, labelColour);
        interburstLabel[i] = burstint;
        addAndMakeVisible(interburstLabel[i]);
        interburstLabel[i]->setVisible(false);

        ScopedPointer<Label> train = new Label("s_train", "train duration [ms]:");
        train->setFont(Font(20));
        train->setColour(Label::textColourId, labelColour);
        trainDurationLabel[i] = train;
        addAndMakeVisible(trainDurationLabel[i]);

        ScopedPointer<Label> traindel = new Label("s_traindel", "train delay [ms]:");
        traindel->setFont(Font(20));
        traindel->setColour(Label::textColourId, labelColour);
        trainDelayLabel[i] = traindel;
        addAndMakeVisible(trainDelayLabel[i]);

        ScopedPointer<Label> trigmode = new Label("s_traindel", "trigger mode:");
        trigmode->setFont(Font(20));
        trigmode->setColour(Label::textColourId, labelColour);
        triggerModeLabel[i] = trigmode;
        addAndMakeVisible(triggerModeLabel[i]);

        ScopedPointer<Label> phEd1 = new Label("phase1", String(DEF_PHASE_DURATION));
        phEd1->setFont(Font(20));
        phEd1->setColour(Label::textColourId, labelTextColour);
        phEd1->setColour(Label::backgroundColourId, labelBackgroundColour);
        phEd1->setEditable(true);
        phase1EditLabel[i] = phEd1;
        phase1EditLabel[i]->addListener(this);
        addAndMakeVisible(phase1EditLabel[i]);

        ScopedPointer<Label> phEd2 = new Label("phase2", String(DEF_PHASE_DURATION));
        phEd2->setFont(Font(20));
        phEd2->setColour(Label::textColourId, labelTextColour);
        phEd2->setColour(Label::backgroundColourId, labelBackgroundColour);
        phEd2->setEditable(true);
        phase2EditLabel[i] = phEd2;
        phase2EditLabel[i]->addListener(this);
        addAndMakeVisible(phase2EditLabel[i]);
        phase2EditLabel[i]->setVisible(false);

        ScopedPointer<Label> intphEd = new Label("interphase", String(DEF_INTER_PHASE));
        intphEd->setFont(Font(20));
        intphEd->setColour(Label::textColourId, labelTextColour);
        intphEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        intphEd->setEditable(true);
        interphaseEditLabel[i] = intphEd;
        interphaseEditLabel[i]->addListener(this);
        addAndMakeVisible(interphaseEditLabel[i]);
        interphaseEditLabel[i]->setVisible(false);

        ScopedPointer<Label> vEd1 = new Label("v1", String(DEF_VOLTAGE));
        vEd1->setFont(Font(20));
        vEd1->setColour(Label::textColourId, labelTextColour);
        vEd1->setColour(Label::backgroundColourId, labelBackgroundColour);
        vEd1->setEditable(true);
        voltage1EditLabel[i] = vEd1;
        voltage1EditLabel[i]->addListener(this);
        addAndMakeVisible(voltage1EditLabel[i]);

        ScopedPointer<Label> vEd2 = new Label("v2", String(DEF_VOLTAGE));
        vEd2->setFont(Font(20));
        vEd2->setColour(Label::textColourId, labelTextColour);
        vEd2->setColour(Label::backgroundColourId, labelBackgroundColour);
        vEd2->setEditable(true);
        voltage2EditLabel[i] = vEd2;
        voltage2EditLabel[i]->addListener(this);
        addAndMakeVisible(voltage2EditLabel[i]);
        voltage2EditLabel[i]->setVisible(false);

        ScopedPointer<Label> rvEd = new Label("v2", String(0));
        rvEd->setFont(Font(20));
        rvEd->setColour(Label::textColourId, labelTextColour);
        rvEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        rvEd->setEditable(true);
        restingVoltageEditLabel[i] = rvEd;
        restingVoltageEditLabel[i]->addListener(this);
        addAndMakeVisible(restingVoltageEditLabel[i]);

        ScopedPointer<Label> intpulEd = new Label("pul", String(DEF_INTER_PULSE));
        intpulEd->setFont(Font(20));
        intpulEd->setColour(Label::textColourId, labelTextColour);
        intpulEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        intpulEd->setEditable(true);
        interpulseEditLabel[i] = intpulEd;
        interpulseEditLabel[i]->addListener(this);
        addAndMakeVisible(interpulseEditLabel[i]);


        ScopedPointer<Label> burstEd = new Label("burst", String(0));
        burstEd->setFont(Font(20));
        burstEd->setColour(Label::textColourId, labelTextColour);
        burstEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        burstEd->setEditable(true);
        burstDurationEditLabel[i] = burstEd;
        burstDurationEditLabel[i]->addListener(this);
        addAndMakeVisible(burstDurationEditLabel[i]);
        burstDurationEditLabel[i]->setVisible(false);

        ScopedPointer<Label> burstintEd = new Label("train", String(0));
        burstintEd->setFont(Font(20));
        burstintEd->setColour(Label::textColourId, labelTextColour);
        burstintEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        burstintEd->setEditable(true);
        interburstEditLabel[i] = burstintEd;
        interburstEditLabel[i]->addListener(this);
        addAndMakeVisible(interburstEditLabel[i]);
        interburstEditLabel[i]->setVisible(false);

        ScopedPointer<Label> traindelEd = new Label("train", String(0));
        traindelEd->setFont(Font(20));
        traindelEd->setColour(Label::textColourId, labelTextColour);
        traindelEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        traindelEd->setEditable(true);
        trainDelayEditLabel[i] = traindelEd;
        trainDelayEditLabel[i]->addListener(this);
        addAndMakeVisible(trainDelayEditLabel[i]);

        ScopedPointer<Label> trainEd = new Label("train", String(DEF_TRAINDURATION))    ;
        trainEd->setFont(Font(20));
        trainEd->setColour(Label::textColourId, labelTextColour);
        trainEd->setColour(Label::backgroundColourId, labelBackgroundColour);
        trainEd->setEditable(true);
        trainDurationEditLabel[i] = trainEd;
        trainDurationEditLabel[i]->addListener(this);
        addAndMakeVisible(trainDurationEditLabel[i]);

    }

}


void PulsePalOutputCanvas::refreshState()
{
}

void PulsePalOutputCanvas::refresh()
{
}

void PulsePalOutputCanvas::beginAnimation()
{
}

void PulsePalOutputCanvas::endAnimation()
{
}

void PulsePalOutputCanvas::update()
{

}

void PulsePalOutputCanvas::setParameter(int, float)
{
}

void PulsePalOutputCanvas::setParameter(int, int, int, float)
{
}
