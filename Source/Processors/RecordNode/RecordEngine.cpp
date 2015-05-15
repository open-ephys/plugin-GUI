/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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

#include "RecordEngine.h"
#include "RecordNode.h"
#include "../ProcessorGraph/ProcessorGraph.h"
#include "../../AccessClass.h"

#include "EngineConfigWindow.h"
#include "HDF5Recording.h"
#include "OriginalRecording.h"

RecordEngine::RecordEngine()
    : manager(nullptr)
{
}

RecordEngine::~RecordEngine() {}

void RecordEngine::setParameter(EngineParameter& parameter) {}

void RecordEngine::resetChannels() {}

void RecordEngine::registerProcessor(GenericProcessor* processor) {}

Channel* RecordEngine::getChannel(int index)
{
    return AccessClass::getProcessorGraph()->getRecordNode()->getDataChannel(index);
}

String RecordEngine::generateDateString()
{
    return AccessClass::getProcessorGraph()->getRecordNode()->generateDateString();
}

SpikeRecordInfo* RecordEngine::getSpikeElectrode(int index)
{
    return AccessClass::getProcessorGraph()->getRecordNode()->getSpikeElectrode(index);
}

void RecordEngine::updateTimestamps(std::map<uint8, int64>* ts)
{
    timestamps = ts;
}

void RecordEngine::updateNumSamples(std::map<uint8, int>* ns)
{
    numSamples = ns;
}


void RecordEngine::registerSpikeSource(GenericProcessor* processor) {}

void RecordEngine::startAcquisition() {}

void RecordEngine::directoryChanged() {}

void RecordEngine::registerManager(RecordEngineManager* recordManager)
{
    manager = recordManager;
}

void RecordEngine::configureEngine()
{
    if (!manager)
        return;

    for (int i=0; i < manager->getNumParameters(); i++)
        setParameter(manager->getParameter(i));
}

//Manager

EngineParameter::EngineParameter(EngineParameter::EngineParameterType paramType, int paramId, String paramName, var defaultValue, var min, var max)
    : type(paramType), name(paramName), id(paramId), def(defaultValue)
{
    if (paramType == BOOL && defaultValue.isBool())
        boolParam.value = defaultValue;
    else if (paramType == INT)
    {
        intParam.value = defaultValue;
        intParam.min= min;
        intParam.max= max;
    }
    else if (paramType == FLOAT)
    {
        floatParam.value = defaultValue;
        floatParam.min= min;
        floatParam.max= max;
    }
    else if (paramType == STR)
        strParam.value = defaultValue;
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
    }
}

RecordEngineManager::RecordEngineManager(String engineID, String engineName, EngineCreator creatorFunc) :
    creator(creatorFunc), id(engineID), name(engineName), window(nullptr)
{
}

RecordEngineManager::~RecordEngineManager()
{
}

void RecordEngineManager::addParameter(EngineParameter* param)
{
    parameters.add(param);
}

RecordEngine* RecordEngineManager::instantiateEngine()
{
    if (creator)
        return creator();

    //Built-in engines
    if (id == "KWIK")
        return new HDF5Recording();

    if (id == "OPENEPHYS")
        return new OriginalRecording();

    return nullptr;
}

int RecordEngineManager::getNumParameters()
{
    return parameters.size();
}

EngineParameter& RecordEngineManager::getParameter(int index)
{
    return *(parameters[index]);
}

String RecordEngineManager::getName()
{
    return name;
}

String RecordEngineManager::getID()
{
    return id;
}

bool RecordEngineManager::isWindowOpen()
{
    return window ? true : false;
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
    for (int i=0; i < parameters.size(); i++)
    {
        XmlElement* param = xml->createNewChildElement("PARAMETER");
        param->setAttribute("id",parameters[i]->id);
        switch (parameters[i]->type)
        {
            case EngineParameter::BOOL:
                param->setAttribute("type","bool");
                param->setAttribute("value",parameters[i]->boolParam.value);
                break;
            case EngineParameter::INT:
                param->setAttribute("type","int");
                param->setAttribute("value",parameters[i]->intParam.value);
                break;
            case EngineParameter::FLOAT:
                param->setAttribute("type","float");
                param->setAttribute("value",parameters[i]->floatParam.value);
                break;
            case EngineParameter::STR:
                param->setAttribute("type","string");
                param->setAttribute("value",parameters[i]->strParam.value);
                break;
        }
    }
}

void RecordEngineManager::loadParametersFromXml(XmlElement* xml)
{
    for (int i=0; i < parameters.size(); i++)
    {
        forEachXmlChildElementWithTagName(*xml,xmlNode,"PARAMETER")
        {
            if (xmlNode->getIntAttribute("id") == parameters[i]->id)
            {
                if ((xmlNode->getStringAttribute("type") == "bool") && (parameters[i]->type == EngineParameter::BOOL))
                    parameters[i]->boolParam.value = xmlNode->getBoolAttribute("value");
                else if ((xmlNode->getStringAttribute("type") == "int") && (parameters[i]->type == EngineParameter::INT))
                    parameters[i]->intParam.value = xmlNode->getIntAttribute("value");
                else if ((xmlNode->getStringAttribute("type") == "float") && (parameters[i]->type == EngineParameter::FLOAT))
                    parameters[i]->floatParam.value = xmlNode->getDoubleAttribute("value");
                else if ((xmlNode->getStringAttribute("type") == "string") && (parameters[i]->type == EngineParameter::STR))
                    parameters[i]->strParam.value = xmlNode->getStringAttribute("value");
            }
        }
    }
}

int RecordEngineManager::getNumOfBuiltInEngines()
{
    return 2;
}

RecordEngineManager* RecordEngineManager::createBuiltInEngineManager(int index)
{
    switch (index)
    {
        case 0:
            return OriginalRecording::getEngineManager();
            break;
        case 1:
            return HDF5Recording::getEngineManager();
            break;
        default:
            return nullptr;
    }
}