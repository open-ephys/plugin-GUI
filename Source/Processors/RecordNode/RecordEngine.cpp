/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Open Ephys

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

#include "RecordNode.h"
#include "RecordEngine.h"

#include "EngineConfigWindow.h"
#include "BinaryFormat/BinaryRecording.h"

RecordEngine::RecordEngine()
	: manager(nullptr), recordNode(nullptr)
{
}

void RecordEngine::registerRecordNode(RecordNode* node)
{
	recordNode = node;
}

void RecordEngine::registerManager(RecordEngineManager* recordManager)
{
	manager = recordManager;
}

const DataStream* RecordEngine::getDataStream(int index) const
{
    return recordNode->dataStreams[index];
}

const ContinuousChannel* RecordEngine::getContinuousChannel(int index) const
{
	return recordNode->continuousChannels[index];
}

const EventChannel* RecordEngine::getEventChannel(int index) const
{
	if (index >= 0)
		return recordNode->eventChannels[index];

	return recordNode->getMessageChannel();

}


const SpikeChannel* RecordEngine::getSpikeChannel(int index) const
{
	return recordNode->spikeChannels[index];
}

String RecordEngine::generateDateString() const
{
	return recordNode->generateDateString();
}

void RecordEngine::updateLatestSampleNumbers(const Array<int64>& num, int channel)
{
	if (channel < 0)
		sampleNumbers = num;
	else
		sampleNumbers.set(channel, num[channel]);
}

void RecordEngine::setChannelMap(const Array<int>& globalChans,
                                 const Array<int>& localChans)
{
    globalChannelMap.clear();
    localChannelMap.clear();
    
    for (auto chan : globalChans)
        globalChannelMap.add(chan);
    
    for (auto chan : localChans)
        localChannelMap.add(chan);
}

int64 RecordEngine::getLatestSampleNumber(int channel) const
{
	return sampleNumbers[channel];
}

int RecordEngine::getGlobalIndex(int channel) const
{
	return globalChannelMap[channel];
}

int RecordEngine::getLocalIndex(int channel) const
{
    return localChannelMap[channel];
}

int RecordEngine::getNumRecordedDataStreams() const
{
    return recordNode->getTotalRecordedStreams();
}


int RecordEngine::getNumRecordedContinuousChannels() const
{
	return globalChannelMap.size();
}

int RecordEngine::getNumRecordedEventChannels() const
{
	return recordNode->getTotalEventChannels();
}

int RecordEngine::getNumRecordedSpikeChannels() const
{
	return recordNode->getTotalSpikeChannels();
}

const String& RecordEngine::getLatestSettingsXml() const
{
	return recordNode->getLastSettingsXml();
}


void RecordEngine::configureEngine()
{
	if (!manager)
		return;

	for (int i = 0; i < manager->getNumParameters(); ++i)
		setParameter(manager->getParameter(i));
}

//Manager

EngineParameter::EngineParameter(EngineParameter::EngineParameterType paramType,
	int paramId,
	String paramName,
	var defaultValue,
	var min,
	var max)
	: type(paramType)
	, name(paramName)
	, id(paramId)
	, def(defaultValue)
{
	if (paramType == BOOL && defaultValue.isBool())
	{
		boolParam.value = defaultValue;
	}
	else if (paramType == INT)
	{
		intParam.value = defaultValue;
		intParam.min = min;
		intParam.max = max;
	}
	else if (paramType == FLOAT)
	{
		floatParam.value = defaultValue;
		floatParam.min = min;
		floatParam.max = max;
	}
	else if (paramType == STR)
	{
		strParam.value = defaultValue;
	}
	else if (paramType == MULTI)
	{
		multiParam.value = defaultValue;
	}
}


void EngineParameter::restoreDefault()
{
	switch (type)
	{
	case INT:
		intParam.value = def;
		break;

	case FLOAT:
		floatParam.value = def;
		break;

	case BOOL:
		boolParam.value = def;
		break;

	case STR:
		strParam.value = def;
		break;

	case MULTI:
		multiParam.value = def;

	default:
		break;
	}
}

RecordEngineManager::RecordEngineManager(String engineID, String engineName, EngineCreator creatorFunc)
	: creator(creatorFunc)
	, id(engineID)
	, name(engineName)
	, window(nullptr)
{
}

RecordEngineManager::~RecordEngineManager()
{
}

void RecordEngineManager::addParameter(EngineParameter* param)
{
	parameters.add(param);
}

int RecordEngineManager::getNumOfBuiltInEngines()
{
	return 1;
}

RecordEngineManager* RecordEngineManager::createBuiltInEngineManager(int index)
{
	switch (index)
	{
	case 0:
		return BinaryRecording::getEngineManager();

	default:
		return nullptr;
	}
}

RecordEngine* RecordEngineManager::instantiateEngine()
{
	if (creator)
	{
		LOGDD("Got creator");
		return creator();
	}
	
	//Built-in engines
	LOGD("Got:", id);

	if (id == "BINARY")
	{
		return new BinaryRecording();
	}

	return nullptr;
}

int RecordEngineManager::getNumParameters() const
{
	return parameters.size();
}

EngineParameter& RecordEngineManager::getParameter(int index)
{
	return *(parameters[index]);
}

String RecordEngineManager::getName() const
{
	return name;
}

String RecordEngineManager::getID() const
{
	return id;
}

bool RecordEngineManager::isWindowOpen() const
{
	return false;
	//return window ? true : false;
}

void RecordEngineManager::toggleConfigWindow()
{
	if (window)
	{
		window->saveParameters();
		window->setVisible(false);
		window = nullptr;
	}

	else
	{
		window = new EngineConfigWindow(this);
		window->setVisible(true);
	}
}

void RecordEngineManager::saveParametersToXml(XmlElement* xml)
{
	for (int i = 0; i < parameters.size(); ++i)
	{
		XmlElement* param = xml->createNewChildElement("PARAMETER");
		param->setAttribute("id", parameters[i]->id);
		switch (parameters[i]->type)
		{
		case EngineParameter::BOOL:
			param->setAttribute("type", "bool");
			param->setAttribute("value", parameters[i]->boolParam.value);
			break;

		case EngineParameter::INT:
			param->setAttribute("type", "int");
			param->setAttribute("value", parameters[i]->intParam.value);
			break;

		case EngineParameter::FLOAT:
			param->setAttribute("type", "float");
			param->setAttribute("value", parameters[i]->floatParam.value);
			break;

		case EngineParameter::STR:
			param->setAttribute("type", "string");
			param->setAttribute("value", parameters[i]->strParam.value);
			break;

		case EngineParameter::MULTI:
			param->setAttribute("type", "multi");
			param->setAttribute("value", parameters[i]->multiParam.value);

		default:
			break;
		}
	}
}

void RecordEngineManager::loadParametersFromXml(XmlElement* xml)
{
	for (int i = 0; i < parameters.size(); ++i)
	{
		for (auto* xmlNode : xml->getChildWithTagNameIterator("PARAMETER"))
		{
			if (xmlNode->getIntAttribute("id") == parameters[i]->id)
			{
				if ((xmlNode->getStringAttribute("type") == "bool")
					&& (parameters[i]->type == EngineParameter::BOOL))
				{
					parameters[i]->boolParam.value = xmlNode->getBoolAttribute("value");
				}
				else if ((xmlNode->getStringAttribute("type") == "int")
					&& (parameters[i]->type == EngineParameter::INT))
				{
					parameters[i]->intParam.value = xmlNode->getIntAttribute("value");
				}
				else if ((xmlNode->getStringAttribute("type") == "float")
					&& (parameters[i]->type == EngineParameter::FLOAT))
				{
					parameters[i]->floatParam.value = xmlNode->getDoubleAttribute("value");
				}
				else if ((xmlNode->getStringAttribute("type") == "string")
					&& (parameters[i]->type == EngineParameter::STR))
				{
					parameters[i]->strParam.value = xmlNode->getStringAttribute("value");
				}
				else if ((xmlNode->getStringAttribute("type") == "multi")
					&& (parameters[i]->type == EngineParameter::MULTI))
				{
					parameters[i]->multiParam.value = xmlNode->getIntAttribute("value");
				}
			}
		}
	}
}



