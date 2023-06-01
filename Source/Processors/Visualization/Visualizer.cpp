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

Visualizer::Visualizer(GenericProcessor* parentProcessor) : processor(parentProcessor)
{

}

void Visualizer::update()
{

    if (processor != nullptr)
    {
        LOGDD(processor->getEditor()->getNameAndId(), " visualizer updating parameter editors");
        
        auto streamId = processor->getEditor()->getCurrentStream();
        bool streamAvailable = streamId > 0 ? true : false;

        for (auto ed : parameterEditors)
        {
            const String parameterName = ed->getParameterName();
            
            Parameter* param = processor->getParameter(parameterName);
            
            if (param == nullptr)
                continue;

            //LOGD("Parameter: ", param->getName());
            
            if (param->getScope() == Parameter::GLOBAL_SCOPE)
            {
                //LOGD("Global scope");
                ed->setParameter(processor->getParameter(ed->getParameterName()));
            }
            else if (param->getScope() == Parameter::STREAM_SCOPE)
            {
                if (streamAvailable)
                {
                //LOGD("Stream scope");
                    Parameter* p2 = processor->getDataStream(streamId)->getParameter(param->getName());
                    ed->setParameter(p2);
                }
                else
                {
                    //LOGD("Stream not available");
                    ed->setParameter(nullptr);
                }
                    
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

void Visualizer::addTextBoxParameterEditor(const String& parameterName,
                                           int xPos_, int yPos_,
                                           Component *parentComponent)
{

    Parameter* param = processor->getParameter(parameterName);

    addCustomParameterEditor(new TextBoxParameterEditor(param),
                             xPos_, yPos_,
                             parentComponent);
}

void Visualizer::addCheckBoxParameterEditor(const String& parameterName,
                                            int xPos_, int yPos_,
                                            Component *parentComponent)
{

    Parameter* param = processor->getParameter(parameterName);

    addCustomParameterEditor(new CheckBoxParameterEditor(param),
                             xPos_, yPos_,
                             parentComponent);
}


void Visualizer::addSliderParameterEditor(const String& parameterName,
                                          int xPos_, int yPos_,
                                          Component *parentComponent)
{
    
    Parameter* param = processor->getParameter(parameterName);

    addCustomParameterEditor(new SliderParameterEditor(param),
                             xPos_, yPos_,
                             parentComponent);
}


void Visualizer::addComboBoxParameterEditor(const String& parameterName,
                                            int xPos_, int yPos_,
                                            Component *parentComponent)
{

    Parameter* param = processor->getParameter(parameterName);

    addCustomParameterEditor(new ComboBoxParameterEditor(param), 
                             xPos_, yPos_,
                             parentComponent);
}


void Visualizer::addSelectedChannelsParameterEditor(const String& parameterName, 
                                                    int xPos_, int yPos_,
                                                    Component *parentComponent)
{

    //std::cout << "CREATING EDITOR: " << parameterName << std::endl;
    
    Parameter* param = processor->getParameter(parameterName);

    addCustomParameterEditor(new SelectedChannelsParameterEditor(param), 
                             xPos_, yPos_,
                             parentComponent);
}

void Visualizer::addMaskChannelsParameterEditor(const String& parameterName,
                                                int xPos_, int yPos_,
                                                Component *parentComponent)
{

    //std::cout << "CREATING EDITOR: " << parameterName << std::endl;
    
    Parameter* param = processor->getParameter(parameterName);

    addCustomParameterEditor(new MaskChannelsParameterEditor(param), 
                             xPos_, yPos_,
                             parentComponent);
}


void Visualizer::addCustomParameterEditor(ParameterEditor* ed,
                                          int xPos_, int yPos_,
                                          Component *parentComponent)
{
    parameterEditors.add(ed);
    
    if (parentComponent != nullptr)
        parentComponent->addAndMakeVisible(ed);
    else
        addAndMakeVisible(ed);

    ed->setBounds(xPos_, yPos_, ed->getWidth(), ed->getHeight());
}


void Visualizer::updateView()
{

    const MessageManagerLock mml;
    
    for (auto ed : parameterEditors)
    {
        ed->updateView();
    }
}

ParameterEditor* Visualizer::getParameterEditor(const String& parameterName)
{
    for (auto e : parameterEditors)
    {
        if (e->getParameterName().equalsIgnoreCase(parameterName))
            return e;
    }
    
    jassertfalse;
    return nullptr;
}