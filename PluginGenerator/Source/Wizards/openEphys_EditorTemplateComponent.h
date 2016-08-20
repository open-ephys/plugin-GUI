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

#ifndef __OPEN_EPHYS_EDITOR_TEMPLATE_COMPONENT_H__
#define __OPEN_EPHYS_EDITOR_TEMPLATE_COMPONENT_H__

#include <JuceHeader.h>


class EditorTemplateComponent   : public TextButton
{
public:
    EditorTemplateComponent (const String& buttonName, const String& templateName, const String& toolTip = String::empty);

    void resized() override;
    void paintOverChildren (Graphics& g) override;

    void setContentComponent (Component* templateComponentToDisplay);
    void setContentLookAndFeel (LookAndFeel* contentLookAndFeel);

    void setTemplateName (const String& newTemplateName);

    String getTemplateName() const noexcept;


private:
    String m_templateName;

    ScopedPointer<Component> m_templateComponentToDisplay;

    ScopedPointer<LookAndFeel> m_buttonLookAndFeel;
    SharedResourcePointer<LookAndFeel_V2> m_defaultContentLookAndFeel;
    LookAndFeel* m_contentLookAndFeel;


    // ========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorTemplateComponent);
};



#endif // __OPEN_EPHYS_EDITOR_TEMPLATE_COMPONENT_H__

