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

#include "Visualizer.h"

Visualizer::Visualizer(GenericProcessor* parentProcessor)
    : ParameterOwner(ParameterOwner::Type::OTHER)
    , ParameterEditorOwner(this)
    , processor(parentProcessor)
{

}

Visualizer::~Visualizer()
{
    
}

void Visualizer::update()
{

    if (processor != nullptr)
    {
        LOGDD(processor->getEditor()->getNameAndId(), " visualizer updating parameter editors");
        
        auto streamId = processor->getEditor()->getCurrentStream();
        bool streamAvailable = streamId > 0 ? true : false;
        
        Array<ParameterEditor*> allParamEditors;

        allParamEditors.addArray(parameterEditors);

        for(auto editorOwner : parameterEditorOwners)
            allParamEditors.addArray(editorOwner->getParameterEditors());

        for (auto ed : allParamEditors)
        {
            const String parameterName = ed->getParameterName();

            if (streamAvailable)
            {
            //LOGD("Stream scope");
                auto stream = processor->getDataStream(streamId);
                
                if (stream->hasParameter(parameterName))
                {
                    Parameter* streamParam = stream->getParameter(parameterName);
                    ed->setParameter(streamParam);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                //LOGD("Stream not available");
                if (!processor->hasParameter(parameterName) || !hasParameter(parameterName))
                    ed->setParameter(nullptr);
            }
            
            ed->updateView();
        }
    }

    updateSettings();
}

void Visualizer::startCallbacks()
{
    startTimer(1/float(refreshRate)*1000.0f);

    for (int n = 0; n < parameterEditors.size(); n++)
    {
        if (parameterEditors[n]->shouldDeactivateDuringAcquisition())
            parameterEditors[n]->setEnabled(false);
    }
}

void Visualizer::stopCallbacks()
{
	stopTimer();

    for (int n = 0; n < parameterEditors.size(); n++)
    {
        if (parameterEditors[n]->shouldDeactivateDuringAcquisition())
            parameterEditors[n]->setEnabled(true);
    }
}

void Visualizer::timerCallback()
{
	refresh();
}

void Visualizer::addBooleanParameter(const String& name,
    const String& displayName,
	const String& description,
	bool defaultValue,
	bool deactivateDuringAcquisition)
{

	BooleanParameter* p = new BooleanParameter(
		nullptr, 
		Parameter::VISUALIZER_SCOPE,
		name, 
        displayName,
		description, 
		defaultValue, 
		deactivateDuringAcquisition);

	addParameter(p);

}

void Visualizer::addCategoricalParameter(const String& name,
    const String& displayName,
	const String& description,
	Array<String> categories,
	int defaultIndex,
	bool deactivateDuringAcquisition)
{

	CategoricalParameter* p = new CategoricalParameter(
		nullptr, 
		Parameter::VISUALIZER_SCOPE,
		name, 
        displayName,
		description, 
		categories, 
		defaultIndex, 
		deactivateDuringAcquisition);

	addParameter(p);

}

void Visualizer::addIntParameter(const String& name,
    const String& displayName,
    const String& description,
	int defaultValue,
	int minValue,
	int maxValue,
	bool deactivateDuringAcquisition)
{

	IntParameter* p = 
		new IntParameter(nullptr, 
			Parameter::VISUALIZER_SCOPE,
			name, 
            displayName,
			description, 
			defaultValue, 
			minValue, 
			maxValue, 
			deactivateDuringAcquisition);

	addParameter(p);

}

void Visualizer::addStringParameter(const String& name,
    const String& displayName,
    const String& description,
    String defaultValue,
    bool deactivateDuringAcquisition)
{
    StringParameter* p =
        new StringParameter(nullptr,
            Parameter::VISUALIZER_SCOPE,
            name,
            displayName,
            description,
            defaultValue,
            deactivateDuringAcquisition);

    addParameter(p);

}

void Visualizer::addFloatParameter(
    const String& name,
    const String& displayName,
    const String& description,
    const String& unit,
	float defaultValue,
	float minValue,
	float maxValue,
	float stepSize,
	bool deactivateDuringAcquisition)
{

	FloatParameter* p =
		new FloatParameter(nullptr,
			Parameter::VISUALIZER_SCOPE,
			name,
            displayName,
			description,
            unit,
			defaultValue,
			minValue,
			maxValue,
			stepSize,
			deactivateDuringAcquisition);

	addParameter(p);

}

void Visualizer::addMaskChannelsParameter(
    const String& name,
    const String& displayName,
    const String& description,
	bool deactivateDuringAcquisition)
{

	Array<var> defaultValue;

	MaskChannelsParameter* p =
		new MaskChannelsParameter(nullptr,
			Parameter::VISUALIZER_SCOPE,
			name,
            displayName,
			description,
			deactivateDuringAcquisition);

	addParameter(p);

}



void Visualizer::addSelectedChannelsParameter(
    const String& name,
    const String& displayName,
    const String& description,
    int maxSelectedChannels,
    bool deactivateDuringAcquisition)
{

    Array<var> defaultValue;

    SelectedChannelsParameter* p =
        new SelectedChannelsParameter(nullptr,
            Parameter::VISUALIZER_SCOPE,
            name,
            displayName,
            description,
            defaultValue,
            maxSelectedChannels,
            deactivateDuringAcquisition);

    addParameter(p);
}

void Visualizer::parameterChangeRequest(Parameter* param)
{
	param->updateValue();
    parameterValueChanged(param);
}

void Visualizer::addParameterEditorOwner(ParameterEditorOwner* editorOwner)
{
    parameterEditorOwners.add(editorOwner);
    addAndMakeVisible(editorOwner->getComponent());
}

void Visualizer::saveToXml(XmlElement* xml)
{    
    XmlElement* paramsXml = xml->createNewChildElement("VISUALIZER_PARAMETERS");
    
    for (auto param : getParameters())
    {
        param->toXml(paramsXml);
    }

	saveCustomParametersToXml(xml->createNewChildElement("CUSTOM_PARAMETERS"));
}

void Visualizer::loadFromXml(XmlElement* xml)
{    
    XmlElement* visParams = xml->getChildByName("VISUALIZER_PARAMETERS");

    if (visParams != nullptr)
    {
        for (int i = 0; i < visParams->getNumAttributes(); i++)
        {
            auto param = getParameter(visParams->getAttributeName(i));

            if(param != nullptr)
            {
                param->fromXml(visParams);
                param->valueChanged();
                parameterValueChanged(param);
            }
        }
    }
    
    loadCustomParametersFromXml(xml);
}
