/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "ContentComponentHandler.h"
#include "../GenericProcessor/GenericProcessor.h"


void ContentComponentHandler::updateParameterComponent (Parameter* parameterToUpdate)
{
    Component& contentComponent = getContentComponent();

    // TODO<Kirill A>: change to search for component ID's, not for names
    const int numChildComponents = contentComponent.getNumChildComponents();
    const auto componentIDForParameter = parameterToUpdate->getComponentID();
    for (int i = 0; i < numChildComponents; ++i)
    {
        if (contentComponent.getChildComponent (i)->getName() == componentIDForParameter)
        {
            auto neededChildComponent = contentComponent.getChildComponent (i);

            if (auto buttonComponent = dynamic_cast<Button*> (neededChildComponent))
            {
                buttonComponent->setToggleState ((bool)parameterToUpdate->getCurrentValue(), dontSendNotification);
            }
            else if (auto sliderComponent = dynamic_cast<Slider*> (neededChildComponent))
            {
                //DBG (String ("Slider value: ") + String ((double)parameterToUpdate->getCurrentValue()));
                sliderComponent->setValue ((double)parameterToUpdate->getCurrentValue(), dontSendNotification);
            }
            else if (auto textEditorComponent = dynamic_cast<TextEditor*> (neededChildComponent))
            {
                DBG (String ("Text editor value: ") + String ((double)parameterToUpdate->getCurrentValue()));
                textEditorComponent->setText (parameterToUpdate->getCurrentValue().toString(), dontSendNotification);
            }

            //TODO<Kirill A>: We might have some components referring to the same parameter
            break;
        }
    }
}


void ContentComponentHandler::configureParameterEditors (GenericProcessor* processor)
{
    DBG ("Configuring parameter editors...");
    auto& contentComponent = getContentComponent();
    const int numChildComponents = contentComponent.getNumChildComponents();

    // Check all child components and setup parameters for all buttons, sliders
    // (TODO<Kirill A>: add support for more components)
    for (int i = 0; i < numChildComponents; ++i)
    {
        auto childComponent = contentComponent.getChildComponent (i);
        DBG (childComponent->getName());
        // TODO<Kirill A>: change to search for component ID's, not for names
        const bool isParameterExists = processor->isParameterExists (childComponent->getName());

        // If parameter with given component ID doesn't exist, just skip it
        if (! isParameterExists)
            continue;

        // TODO<Kirill A>: change to search for component ID's, not for names
        auto parameter = processor->findParameterWithComponentID (childComponent->getName());
        if (auto buttonComponent = dynamic_cast<Button*> (childComponent))
        {
            buttonComponent->setClickingTogglesState (true);
            buttonComponent->setToggleState ((bool)parameter->getDefaultValue(), dontSendNotification);

            if (m_buttonListener)
                buttonComponent->addListener (m_buttonListener);
        }
        else if (auto sliderComponent = dynamic_cast<Slider*> (childComponent))
        {
            // For boolean parameters the only possible values for slider should be 0.0 and 1.0 (off/on)
            if (parameter->isBoolean())
            {
                sliderComponent->setRange (0.0, 1.0, 1.0);
            }
            else if (parameter->isContinuous())
            {
                Array<var> possibleValues = parameter->getPossibleValues();
                jassert (possibleValues.size() == 2);

                const float minPossibleValue = float (possibleValues[0]);
                const float maxPossibleValue = float (possibleValues[1]);

                sliderComponent->setRange (minPossibleValue, maxPossibleValue);
            }
            // It's not supported to use sliders for discrete or numerical parameters
            else
                continue;

            sliderComponent->setValue ((double)parameter->getDefaultValue(), dontSendNotification);
            sliderComponent->getValueObject().referTo (parameter->getCurrentValueObject());

            if (m_sliderListener)
                sliderComponent->addListener (m_sliderListener);
        }
        else if (auto textEditorComponent = dynamic_cast<TextEditor*> (childComponent))
        {
            textEditorComponent->setInputRestrictions (6, "01234567890-.,");

            textEditorComponent->setText (parameter->getDefaultValue().toString(), dontSendNotification);
            textEditorComponent->getTextValue().referTo (parameter->getCurrentValueObject());

            if (m_textEditorListener)
                textEditorComponent->addListener (m_textEditorListener);
        }
    }
}



void ContentComponentHandler::setContentComponentHandlerOwner (Component* parent)
{
    m_buttonListener        = dynamic_cast<Button::Listener*>     (parent);
    m_sliderListener        = dynamic_cast<Slider::Listener*>     (parent);
    m_textEditorListener    = dynamic_cast<TextEditor::Listener*> (parent);

    jassert (m_buttonListener != nullptr);
    jassert (m_sliderListener != nullptr);
    jassert (m_textEditorListener != nullptr);
}

