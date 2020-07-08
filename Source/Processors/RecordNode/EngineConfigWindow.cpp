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

#include "EngineConfigWindow.h"

#include "../../AccessClass.h"

EngineParameterComponent::EngineParameterComponent(EngineParameter& param)
    : Component(param.name), type(param.type), parameter(param)
{
    if (param.type == EngineParameter::BOOL)
    {
        ToggleButton* but = new ToggleButton();
        but->setToggleState(param.boolParam.value,dontSendNotification);
        but->setBounds(120,0,100,20);
        addAndMakeVisible(but);
        control = but;
		name = param.name;
    }
	else if (param.type == EngineParameter::MULTI)
	{
		ComboBox* box = new ComboBox();
		box->setBounds(120, 0, 100, 20);
		StringArray options = StringArray::fromTokens(param.name, "|", "\"");
		name = options[0];
		options.remove(0);
		box->addItemList(options, 1);
		box->setSelectedId(param.multiParam.value + 1, dontSendNotification);
		box->setEditableText(false);
		addAndMakeVisible(box);
		control = box;
	}
    else
    {
        Label* lab = new Label();
        lab->setFont(Font("Small Text",10,Font::plain));
		name = param.name;
        switch (param.type)
        {
            case EngineParameter::BOOL:
                lab->setText(String(param.boolParam.value),dontSendNotification);
                lab->setBounds(120,0,50,20);
                break;
            case EngineParameter::INT:
                lab->setText(String(param.intParam.value),dontSendNotification);
                lab->setBounds(120,0,50,20);
                break;
            case EngineParameter::FLOAT:
                lab->setText(String(param.floatParam.value),dontSendNotification);
                lab->setBounds(120,0,50,20);
                break;
            case EngineParameter::STR:
                lab->setText(String(param.strParam.value),dontSendNotification);
                lab->setBounds(120,0,150,20);
        }
        lab->setEditable(true);
        lab->setColour(Label::ColourIds::backgroundColourId,Colours::lightgrey);
        lab->setColour(Label::ColourIds::outlineColourId,Colours::black);
        lab->addListener(this);
        addAndMakeVisible(lab);
        control = lab;
    }
    this->setTooltip(name);
}

EngineParameterComponent::~EngineParameterComponent()
{
}

void EngineParameterComponent::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.setFont(13);
    g.drawText(name+":",0,0,100,30,Justification::left,true);
}

void EngineParameterComponent::labelTextChanged(Label* l)
{
    if (parameter.type == EngineParameter::INT)
    {
        int value = l->getText().getIntValue();
        if (value < parameter.intParam.min)
            value = parameter.intParam.min;
        if (value > parameter.intParam.max)
            value = parameter.intParam.max;
        l->setText(String(value),dontSendNotification);
    }
    else if (parameter.type == EngineParameter::FLOAT)
    {
        float value = l->getText().getFloatValue();
        if (value < parameter.floatParam.min)
            value = parameter.floatParam.min;
        if (value > parameter.floatParam.max)
            value = parameter.floatParam.max;
        l->setText(String(value),dontSendNotification);
    }
}

void EngineParameterComponent::saveValue()
{
    switch (parameter.type)
    {
        case EngineParameter::BOOL:
            parameter.boolParam.value = static_cast<ToggleButton*>(control.get())->getToggleState();
            break;
        case EngineParameter::INT:
            parameter.intParam.value = static_cast<Label*>(control.get())->getText().getIntValue();
            if (parameter.intParam.value < parameter.intParam.min)
                parameter.intParam.value = parameter.intParam.min;
            if (parameter.intParam.value > parameter.intParam.max)
                parameter.intParam.value = parameter.intParam.max;
            break;
        case EngineParameter::FLOAT:
            parameter.floatParam.value = static_cast<Label*>(control.get())->getText().getFloatValue();
            if (parameter.floatParam.value < parameter.floatParam.min)
                parameter.floatParam.value = parameter.floatParam.min;
            if (parameter.floatParam.value > parameter.floatParam.max)
                parameter.floatParam.value = parameter.floatParam.max;
            break;
        case EngineParameter::STR:
            parameter.strParam.value = static_cast<Label*>(control.get())->getText();
            break;
		case EngineParameter::MULTI:
			parameter.multiParam.value = static_cast<ComboBox*>(control.get())->getSelectedId() - 1;
			break;
    }
}

EngineConfigComponent::EngineConfigComponent(RecordEngineManager* man, int height)
    :  Component(man->getID())
{
    bool hasString = false;
    setName(man->getName()+" Recording Configuration");

	int i;

    for (i = 0; i < man->getNumParameters(); i++)
    {
        EngineParameterComponent* par = new EngineParameterComponent(man->getParameter(i));
        if (man->getParameter(i).type == EngineParameter::STR || man->getParameter(i).type == EngineParameter::MULTI)
            hasString=true;
        par->setBounds(10,10+40*i,300,30);
        addAndMakeVisible(par);
        parameters.add(par);
    }

	recordThreadToggleButton = new ToggleButton();

	recordThreadToggleButton->setToggleState(CoreServices::RecordNode::getRecordThreadStatus(), dontSendNotification);
	recordThreadToggleButton->setBounds(10, 10 + 40 * (i + 1), 100, 20);
	recordThreadToggleButton->addListener(this);
	addAndMakeVisible(recordThreadToggleButton);

	recordThreadToggleLabel = new Label();
	recordThreadToggleLabel->setText("Is record thread enabled?", NotificationType::dontSendNotification);
	recordThreadToggleLabel->setBounds(30, 10 + 40 * (i + 1), 240, 20);
	addAndMakeVisible(recordThreadToggleLabel);

	height = 10 + 40 * (i + 1) + 30;

    if (hasString)
        this->setSize(350,height);
    else
        this->setSize(350,height);


}

EngineConfigComponent::~EngineConfigComponent()
{
}

void EngineConfigComponent::buttonClicked(Button* b)
{

	if (!CoreServices::getRecordingStatus())
	{
		if (b->getToggleState() == false)
		{
		
			int response = AlertWindow::showOkCancelBox(AlertWindow::AlertIconType::WarningIcon,
				"Disable record thread?",
				"Are you sure you want to disable the record thread? You'll need to have a processor capable of recording data on its own.",
				"Yes", "No");

			if (response == 1)
            {
                for (auto* node : AccessClass::getProcessorGraph()->getRecordNodes())
                {   
                    node->setParameter(3,0.0);
                }
            }
			else
				b->setToggleState(true, juce::NotificationType::dontSendNotification);


		}
		else 
        {
			for (auto* node : AccessClass::getProcessorGraph()->getRecordNodes())
            {   
                node->setParameter(3,0.0);
            }
		}
		
	}
	else {
		CoreServices::sendStatusMessage("Cannot toggle record thread status while recording is active.");
	}
}

void EngineConfigComponent::saveParameters()
{
    for (int i=0; i < parameters.size(); i++)
        parameters[i]->saveValue();
}

void EngineConfigComponent::paint(Graphics& g)
{
    g.setColour(Colours::darkgrey);
    g.fillAll();
}

EngineConfigWindow::EngineConfigWindow(RecordEngineManager* man)
    : DocumentWindow("RecordEngine Configuration", Colours::red,
                     DocumentWindow::closeButton), manager(man)
{
    int height = man->getNumParameters() * 50;
    if (height == 0)
        height = 10;
    centreWithSize(200,height);
    setUsingNativeTitleBar(true);
    setResizable(false,false);
    setName(man->getName()+" recording configuration");

    ui = new EngineConfigComponent(man, height);
    setContentNonOwned(ui,true);
}

EngineConfigWindow::~EngineConfigWindow()
{
}

void EngineConfigWindow::closeButtonPressed()
{
    manager->toggleConfigWindow();
}

void EngineConfigWindow::saveParameters()
{
    ui->saveParameters();
}