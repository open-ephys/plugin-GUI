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

#ifndef __PARAMETER_H_62922AE5__
#define __PARAMETER_H_62922AE5__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include <stdio.h>


/**
    Class for holding user-definable processor parameters.

    Parameters can either hold boolean, categorical, or continuous (float) values.

    Using the Parameter class makes it easier to create a graphical interface for editing
    parameters, because each Parameter has a ParameterEditor that is created automatically.

    @see GenericProcessor, GenericEditor
*/
class PLUGIN_API Parameter
{
public:
    /** Constructor for boolean parameters.*/
    Parameter (const String& name,
               bool defaultValue,
               int ID,
               bool deactivateDuringAcquisition = false);

    /** Constructor for continuous (float) parameters.*/
    Parameter (const String& name,
               float low, float high, float defaultValue,
               int ID,
               bool deactivateDuringAcquisition = false);

    /** Constructor for categorical parameters.*/
    Parameter (const String& name,
               Array<var> a,
               int defaultValue, int ID,
               bool deactivateDuringAcquisition = false);


    /** Returns the name of the parameter.*/
    const String& getName() const noexcept;

    /** Returns a description of the parameter.*/
    const String& getDescription() const noexcept;

    /** Returns the unique integer ID of a parameter.*/
    int getID() const noexcept;

    /** Returns the default value of a parameter (can be boolean, int, or float).*/
    var getDefaultValue() const noexcept;

    /** Returns the value of a parameter for a given channel.*/
    var getValue (int chan) const;

    /** Returns the value of a parameter for a given channel.*/
    var operator[](int chan) const;

    /** Returns all the possible values that a parameter can take for Boolean and Discrete parameters;
        Returns the minimum and maximum value that a parameter can take for Continuous parameters.*/
    Array<var> getPossibleValues() const noexcept;

    /** Returns true if a parameter is boolean, false otherwise.*/
    bool isBoolean() const noexcept;

    /** Returns true if a parameter is continuous, false otherwise.*/
    bool isContinuous() const noexcept;

    /** Returns true if a parameter is discrete, false otherwise.*/
    bool isDiscrete() const noexcept;

    /** Returns true if a user set custom bounds for the possible parameter editor, false otherwise. */
    bool hasCustomEditorBounds() const noexcept;

    /** Returns the recommended width value for the parameter editor if parameter has it. */
    int getEditorRecommendedWidth() const noexcept;

    /** Returns the recommended height value for the parameter editor if parameter has it. */
    int getEditorRecommendedHeight() const noexcept;

    /** Returns the desired bounds for editor if parameter has it. */
    const Rectangle<int>& getEditorDesiredBounds() const noexcept;

    /** Sets the description of the parameter.*/
    void setDescription (const String& desc);

    /** Sets the value of a parameter for a given channel.*/
    void setValue (float val, int chan);

    /** Sets desired size for the parameter editor. */
    void setEditorDesiredSize (int desiredWidth, int desiredHeight);

    /** Sets desired bounds for the parameter editor. */
    void setEditorDesiredBounds (int x, int y, int width, int height);

    /** Certain parameters should not be changed while data acquisition is active.
         This variable indicates whether or not these parameters can be edited.*/
    bool shouldDeactivateDuringAcquisition;


private:
    enum ParameterType
    {
        PARAMETER_TYPE_BOOLEAN = 0
        , PARAMETER_TYPE_CONTINUOUS
        , PARAMETER_TYPE_DISCRETE
    };

    const String m_name;
    String m_description;

    int m_parameterId;

    bool m_hasCustomEditorBounds { false };

    Rectangle<int> m_editorBounds;

    ParameterType m_parameterType;

    var m_defaultValue;
    Array<var> m_values;
    Array<var> m_possibleValues;
};


#endif  // __PARAMETER_H_62922AE5__
