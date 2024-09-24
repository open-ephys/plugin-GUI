/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "../../TestableExport.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include <JuceHeader.h>

#include "../../Utils/Utils.h"

/**
    Class for holding user-definable processor parameters.

    Parameters can either hold boolean, categorical, continuous (float) values,
    or a list of selected channels.

    Using the Parameter class makes it easier to create a graphical interface for editing
    parameters, because each Parameter has a ParameterEditor that can be generated automatically.

    @see GenericProcessor, GenericEditor
*/

class ParameterOwner;
class SpikeChannel;
class ContinuousChannel;
class EventChannel;
class DataStream;

/** 

    Base class for all Parameter objects.

    The Parameter class facilitates the following functions:
     - Keeping track of parameter settings for different streams
     - Loading and saving parameter values
     - Ensuring that parameters are safely updated while acquisition
       is active
     - Auto-generating user interfaces for individual parameters

    It's recommended that all plugins use the Parameter class
    and default or custom ParameterEditors to handle getting
    and setting parameters.

    Parameters can be associated with:
     - Plugins (GLOBAL_SCOPE)
     - Data streams (STREAM_SCOPE)
     - Event channels (EVENT_CHANNEL_SCOPE)
     - Continuous channels (CONTINUOUS_CHANNEL_SCOPE)
     - Spike channels (SPIKE_CHANNEL_SCOPE)

    Only Parameters associated with plugins and data streams
    will be saved as loaded automatically.

*/
class PLUGIN_API Parameter
{
public:
    enum ParameterType
    {
        BOOLEAN_PARAM = 1,
        CATEGORICAL_PARAM,
        STRING_PARAM,
        FLOAT_PARAM,
        INT_PARAM,
        SELECTED_CHANNELS_PARAM,
        SELECTED_SPIKE_CHANNEL_PARAM,
        SELECTED_EVENT_CHANNEL_PARAM,
        SELECTED_PROCESSOR_PARAM,
        SELECTED_STREAM_PARAM,
        MASK_CHANNELS_PARAM,
        TTL_LINE_PARAM,
        PATH_PARAM,
        TIME_PARAM,
        NAME_PARAM,
        COLOUR_PARAM,
        NOTIFICATION_PARAM
    };

    enum ParameterEditorType
    {
        UNKNOWN = 0,
        TEXTBOX_EDITOR,
        TOGGLE_EDITOR,
        COMBOBOX_EDITOR,
        BOUNDED_VALUE_EDITOR,
        SELECTED_CHANNELS_EDITOR,
        SELECTED_STREAM_EDITOR,
        MASK_CHANNELS_EDITOR,
        TTL_LINE_EDITOR,
        PATH_EDITOR,
        TIME_EDITOR
    };

    enum ParameterScope
    {
        GLOBAL_SCOPE = 1, // doesn't require a processor
        DEVICE_SCOPE, // doesn't require a processor
        PROCESSOR_SCOPE,
        VISUALIZER_SCOPE,
        STREAM_SCOPE,
        EVENT_CHANNEL_SCOPE,
        CONTINUOUS_CHANNEL_SCOPE,
        SPIKE_CHANNEL_SCOPE,
        INFO_OBJECT_SCOPE
    };

    /** Parameter constructor.*/
    Parameter (ParameterOwner* owner,
               ParameterType type_,
               ParameterScope scope_,
               const String& name_,
               const String& displatyName_,
               const String& description_,
               var defaultValue_,
               bool deactivateDuringAcquisition_ = false)
        : parameterOwner (owner),
          m_parameterType (type_),
          m_parameterScope (scope_),
          m_name (name_),
          m_displayName (displatyName_),
          m_description (description_),
          currentValue (defaultValue_),
          defaultValue (defaultValue_),
          newValue (defaultValue_),
          m_deactivateDuringAcquisition (deactivateDuringAcquisition_),
          m_identifier ("UNKNOWN"),
          isEnabledFlag (true),
          m_editorType (ParameterEditorType::UNKNOWN)
    {
    }

    /** Custom copy constructor -- set attributes and don't copy parameter listeners*/
    Parameter (const Parameter& other) : m_parameterType (other.m_parameterType),
                                         m_parameterScope (other.m_parameterScope),
                                         m_name (other.m_name),
                                         m_displayName (other.m_displayName),
                                         m_description (other.m_description),
                                         currentValue (other.currentValue),
                                         defaultValue (other.defaultValue),
                                         previousValue (other.previousValue),
                                         m_deactivateDuringAcquisition (other.m_deactivateDuringAcquisition),
                                         isEnabledFlag (other.isEnabledFlag),
                                         m_editorType (other.m_editorType)
    {
        parameterListeners.clear();
    }

    /** Destructor */
    virtual ~Parameter()
    {
        if (parameterListeners.size() > 0)
        {
            for (auto listener : parameterListeners)
                listener->removeParameter (this);

            parameterListeners.clear();
        }
    }

    /** Returns true if the parameter is valid */
    virtual bool isValid() { return true; }

    /** Static identifier to keep track of unique parameters */
    static int64 parameterCounter;

    /** Static map from Parameter identifier to name */
    static std::map<int64, String> p_name_map;

    /** Returns the name of the Parameter given its identifier  */
    static String getNameForKey (int64 key)
    {
        return p_name_map[key];
    }

    /** Static map that keeps track of all Parameters */
    static std::map<std::string, Parameter*> parameterMap;

    /** Returns a pointer to the Parameter for a specific key */
    static void registerParameter (Parameter* p) { parameterMap[p->getKey()] = p; };

    /** Set the globally unique key for this parameter */
    void setKey (std::string key) { m_identifier = key; };

    /** Returns a globally unique key of the parameter */
    std::string getKey() { return m_identifier; }

    /** Returns the common name of the parameter. */
    String getName() const noexcept { return m_name; }

    /** Returns the displayed name of the parameter. */
    String getDisplayName() const noexcept { return m_displayName; }

    /** Returns a description of the parameter.*/
    String getDescription() const noexcept { return m_description; }

    /** Returns the type of the parameter. */
    ParameterType getType() const noexcept { return m_parameterType; }

    /** Returns the scope of the parameter. */
    ParameterScope getScope() const noexcept { return m_parameterScope; }

    /** Returns the streamId for this parameter (if available)*/
    uint16 getStreamId();

    /** Determines whether the parameter's editor is accessible after acquisition starts*/
    bool shouldDeactivateDuringAcquisition()
    {
        return m_deactivateDuringAcquisition;
    }

    /** Sets the parameter value*/
    virtual void setNextValue (var newValue, bool undoable = true) = 0;

    /** Returns the parameter value*/
    var getValue()
    {
        return currentValue;
    }

    /** Returns the parameter default value*/
    var getDefaultValue()
    {
        return defaultValue;
    }

    /** Updates parameter value (called by GenericProcessor::setParameter)*/
    void updateValue()
    {
        previousValue = currentValue;
        currentValue = newValue;
    }

    /** Returns a string describing this parameter's type*/
    String getParameterTypeString() const;

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) = 0;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) = 0;

    /** Returns the value as a string**/
    virtual String getValueAsString() = 0;

    /** Can be used to directly set the parameter value (but be careful with this)*/
    var currentValue;

    /** Can be used to restore the previous value if the new value is out of range*/
    void restorePreviousValue()
    {
        currentValue = previousValue;
    }

    /** Returns a description of how the value changed from its previous state */
    virtual String getChangeDescription() = 0;

    /** Returns a pointer to the ParameterOwner this parameter is associated with**/
    ParameterOwner* getOwner() { return parameterOwner; }

    /** Sets a pointer to the ParameterOwner this parameter is associated with**/
    void setOwner (ParameterOwner* newOwner);

    /** Enables/disables the parameter */
    void setEnabled (bool shouldBeEnabled);

    /** Returns true if the parameter is enabled */
    bool isEnabled() { return isEnabledFlag; }

    /** Returns the colour any visualization of this parameter should use */
    Colour getColour();

    /** Set the type of the parameter editor used for this parameter 
     * Used by the GenericEditor to set which parameter editor to use when creating default editors 
    */
    void setParameterEditorType (ParameterEditorType editorType)
    {
        m_editorType = editorType;
    }

    /** Returns the type of the parameter editor.
     * Returns UNKNOWN if not set.
    */
    ParameterEditorType getParameterEditorType()
    {
        return m_editorType;
    }

    /** Parameter listener class -- used for Parameter Editors */
    class Listener
    {
    public:
        /** Constructor */
        Listener() {}

        /** Destructor */
        virtual ~Listener() {}

        /** Called when the parameter value changes */
        virtual void parameterChanged (Parameter* parameterThatHasChanged) = 0;

        /** Called when the parameter is destroyed */
        virtual void removeParameter (Parameter* parameterToRemove) = 0;

        /** Called when the parameter is enabled/disbaled */
        virtual void parameterEnabled (bool isEnabled) = 0;
    };

    /** Add Listener for this parameter */
    void addListener (Listener* listenerToAdd);

    /** Remove Listener for this parameter */
    void removeListener (Listener* listenerToRemove);

    /** Broadcasts a value change to all listeners */
    void valueChanged();

    /** Makes it possible to undo value changes */
    class ChangeValue : public UndoableAction
    {
    public:
        /** Constructor */
        ChangeValue (std::string key, var newValue);

        /** Destructor */
        ~ChangeValue() {}

        /** Perform the action */
        bool perform();

        /** Undo the action*/
        bool undo();

        /** Log the action */
        void log();

    private:
        std::string key;

        var originalValue;
        var newValue;
        bool logChange = false;
    };

    void logValueChange();

protected:
    ParameterOwner* parameterOwner;

    var newValue;
    var previousValue;

    var defaultValue;

private:
    std::string m_identifier;

    ParameterType m_parameterType;
    ParameterScope m_parameterScope;
    ParameterEditorType m_editorType;
    String m_name;
    String m_displayName;
    String m_description;

    bool m_deactivateDuringAcquisition;
    bool isEnabledFlag;

    Array<Listener*> parameterListeners;
};

/** 
* 
    Represents a Parameter that can take two values,
    true or false.

*/
class PLUGIN_API BooleanParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    BooleanParameter (ParameterOwner* owner,
                      ParameterScope scope,
                      const String& name,
                      const String& displayName,
                      const String& description,
                      bool defaultValue,
                      bool deactivateDuringAcquisition = false);

    /** Stages a value, to be changed by the processor*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as a boolean*/
    bool getBoolValue();

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;
};

/**
*
    Represents a Parameter that can take a finite 
    number of custom values (strings).

*/
class PLUGIN_API CategoricalParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    CategoricalParameter (ParameterOwner* owner,
                          ParameterScope scope,
                          const String& name,
                          const String& displayName,
                          const String& description,
                          Array<String> categories,
                          int defaultIndex,
                          bool deactivateDuringAcquisition = false);

    /** Stages a value, to be changed by the processor*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the index as an integer*/
    int getSelectedIndex();

    /** Gets the category at the current index*/
    String getSelectedString();

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Updates the categories*/
    void setCategories (Array<String> categories);

    /** Updates the categories*/
    const Array<String>& getCategories();

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

private:
    Array<String> categories;
};

/**
*
    Represents a Parameter that can take any integer values
    in a given range.

*/
class PLUGIN_API IntParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    IntParameter (ParameterOwner* owner,
                  ParameterScope scope,
                  const String& name,
                  const String& displayName,
                  const String& description,
                  int defaultValue,
                  int minValue = 0,
                  int maxValue = 100,
                  bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    int getIntValue();

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    int getMinValue() { return minValue; }

    int getMaxValue() { return maxValue; }

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

private:
    int maxValue;
    int minValue;
};

/**
*
    Represents a Parameter with a string value.

*/
class PLUGIN_API StringParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    StringParameter (ParameterOwner* owner,
                     ParameterScope scope,
                     const String& name,
                     const String& displayName,
                     const String& description,
                     String defaultValue,
                     bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    String getStringValue();

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;
};

/**
*
    Represents a Parameter that can take any float value
    within a given range.

*/
class PLUGIN_API FloatParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    FloatParameter (ParameterOwner* owner,
                    ParameterScope scope,
                    const String& name,
                    const String& displayName,
                    const String& description,
                    const String& unit,
                    float defaultValue,
                    float minValue = 0.f,
                    float maxValue = 100.f,
                    float stepSize = 1.f,
                    bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    float getFloatValue();

    /** Gets the unit as a String*/
    String getUnit() { return unit; }

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Gets the minimum value for this parameter*/
    float getMinValue() { return minValue; }

    /** Gets the maximum value for this parameter*/
    float getMaxValue() { return maxValue; }

    /** Gets the step size for this parameter*/
    float getStepSize() { return stepSize; }

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

private:
    float maxValue;
    float minValue;
    float stepSize;

    String unit;
};

/**
*
    Represents a Parameter that holds the name of the active stream
    within a processor that may have more than one stream.

    Defaults to the first available stream by index.

*/
class PLUGIN_API SelectedStreamParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    SelectedStreamParameter (ParameterOwner* owner,
                             ParameterScope scope,
                             const String& name,
                             const String& displayName,
                             const String& description,
                             Array<String> streamNames,
                             const int defaultIndex,
                             bool deactivateDuringAcquisition = true);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Sets the list of available stream names */
    void setStreamNames (Array<String> names);

    /** Gets the list of available stream names */
    Array<String>& getStreamNames() { return streamNames; };

    /** Gets the value as an integer*/
    int getSelectedIndex();

    /** Gets the value as a string**/
    String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

private:
    Array<String> streamNames;
    int streamIndex;
};

/**
*
    Represents a Parameter that holds the selection
    state of all continuous channels in a given data stream.

    Defaults to all channels de-selected (false).

    (Optional) The maximum number of selectable channels
               can be specified.

    See: @MaskChannelsParameter

*/
class PLUGIN_API SelectedChannelsParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    SelectedChannelsParameter (ParameterOwner* owner,
                               ParameterScope scope,
                               const String& name,
                               const String& displayName,
                               const String& description,
                               Array<var> defaultValue,
                               int maxSelectableChannels = std::numeric_limits<int>::max(),
                               bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    Array<int> getArrayValue();

    /** Returns the max selectable channels*/
    int getMaxSelectableChannels()
    {
        return maxSelectableChannels;
    }

    void setMaxSelectableChannels (int m_)
    {
        maxSelectableChannels = m_;
    }

    /** Returns a vector of channel selection states (true or false)*/
    std::vector<bool> getChannelStates();

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Sets the total number of available channels in this stream*/
    void setChannelCount (int count);

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

private:
    String selectedChannelsToString();

    Array<var> parseSelectedString (const String& input);

    int maxSelectableChannels;
    int channelCount;
};

/**
*
    Represents a Parameter that holds the selection
    state of all continuous channels in a given data stream.

    Defaults to all channels selected (true).

    See: @SelectedChannelsParameter

*/
class PLUGIN_API MaskChannelsParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    MaskChannelsParameter (ParameterOwner* owner,
                           ParameterScope scope,
                           const String& name,
                           const String& displayName,
                           const String& description,
                           bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    Array<int> getArrayValue();

    /** Returns a vector of channel selection states (true or false)*/
    std::vector<bool> getChannelStates();

    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Sets the total number of available channels in this stream*/
    void setChannelCount (int count);

    /** Returns a list of masked channels as a string */
    String maskChannelsToString();

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

private:
    Array<var> parseMaskString (const String& input);

    int channelCount;
};

/**
*
    Represents a Parameter that holds value of currently selected line in a given Event Channel.

    (Optional) The maximum number of avaialble lines
               can be specified.

*/
class PLUGIN_API TtlLineParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    TtlLineParameter (ParameterOwner* owner,
                      ParameterScope scope,
                      const String& name,
                      const String& displayName,
                      const String& description,
                      int maxAvailableLines = 8,
                      bool syncMode = false,
                      bool canSelectNone = false,
                      bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    int getSelectedLine();

    /** Returns the max selectable channels*/
    int getMaxAvailableLines() const
    {
        return lineCount;
    }

    void setMaxAvailableLines (int count)
    {
        lineCount = count;
    }

    /** Gets the value as a string**/
    String getValueAsString() override;

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Saves the parameter to an XML Element*/
    void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    void fromXml (XmlElement*) override;

    bool syncModeEnabled() { return syncMode; }

    bool canSelectNone() { return selectNone; }

private:
    int lineCount;
    bool syncMode;
    bool selectNone;
};

/**
*
    Represents a Parameter that holds a path to a file or directory.
*/

class PLUGIN_API PathParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    PathParameter (ParameterOwner* owner,
                   ParameterScope scope,
                   const String& name,
                   const String& displayName,
                   const String& description,
                   const String& defaultValue,
                   const StringArray& filePatternsAllowed,
                   const bool isDirectory,
                   bool deactivateDuringAcquisition = true);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as an integer*/
    virtual String getValueAsString() override { return currentValue.toString(); };

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

    /** Returns true if the path should be a directory */
    bool getIsDirectory() { return isDirectory; }

    /** Returns a list of valid file extensions if applicable */
    StringArray getValidFilePatterns() { return filePatternsAllowed; }

    /** Returns true if the current path is valid for this parameter */
    bool isValid() override;

private:
    StringArray filePatternsAllowed;
    bool isDirectory;
};

/**
*
    Represents a Parameter that holds a time value.
*/

class PLUGIN_API TimeParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    TimeParameter (ParameterOwner* owner,
                   ParameterScope scope,
                   const String& name,
                   const String& displayName,
                   const String& description,
                   const String& defaultValue,
                   bool deactivateDuringAcquisition = true);

    /** Sets the current value*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as a string**/
    virtual String getValueAsString() override { return currentValue.toString(); }

    /** Gets the value change description */
    virtual String getChangeDescription() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml (XmlElement*) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml (XmlElement*) override;

    class TimeValue
    {
    public:
        /** Constructor */
        TimeValue (int hour_, int minute_, double second_)
            : hour (hour_),
              minute (minute_),
              second (second_) {}

        TimeValue (int ms) { setTimeFromMilliseconds (ms); }

        TimeValue (String time) { setTimeFromString (time); }

        /** Destructor */
        ~TimeValue() {}

        String toString()
        {
            return String::formatted ("%02d:%02d:%06.3f", this->hour, this->minute, this->second);
        }

        void setTimeFromString (String time)
        {
            StringArray tokens;
            tokens.addTokens (time, ":", "");
            hour = tokens[0].getIntValue();
            minute = tokens[1].getIntValue();
            second = tokens[2].getDoubleValue();
        }

        void setHours (int h) { hour = h; }
        void setMinutes (int m) { minute = m; }
        void setSeconds (double s) { second = s; }

        int getHours() { return hour; }
        int getMinutes() { return minute; }
        double getSeconds() { return second; }

        int getTimeInMilliseconds() { return 1000 * (second + 60.0 * double (minute + 60 * hour)); }
        void setTimeFromMilliseconds (int ms)
        {
            hour = ms / 3600000;
            minute = (ms - hour * 3600000) / 60000;
            second = (ms - hour * 3600000.0f - minute * 60000.0f) / 1000.0f;
        }

        void setMinTimeInMilliseconds (int ms) { minTimeInMilliseconds = ms; }
        int getMinTimeInMilliseconds() { return minTimeInMilliseconds; }

        void setMaxTimeInMilliseconds (int ms) { maxTimeInMilliseconds = ms; }
        int getMaxTimeInMilliseconds() { return maxTimeInMilliseconds; }

    private:
        int hour;
        int minute;
        double second;

        int minTimeInMilliseconds;
        int maxTimeInMilliseconds;
    };

    TimeValue* getTimeValue() { return timeValue.get(); }

    class ChangeValue : public UndoableAction
    {
    public:
        /** Constructor */
        ChangeValue (std::string key, var newValue);

        /** Destructor */
        ~ChangeValue() {}

        /** Perform the action */
        bool perform();

        /** Undo the action*/
        bool undo();

        /** Log the action*/
        void log();

    private:
        std::string key;

        var originalValue;
        var newValue;
    };

private:
    std::shared_ptr<TimeValue> timeValue;
};

/** 
* 
    Represents a Parameter that is only used for change notification.
    E.g. to notify the processor that a certain event has occured.
*/
class PLUGIN_API NotificationParameter : public Parameter
{
public:
    /** Parameter constructor.*/
    NotificationParameter (ParameterOwner* owner,
                           ParameterScope scope,
                           const String& name,
                           const String& displayName,
                           const String& description,
                           bool deactivateDuringAcquisition = false);

    /** Notifies the processor that a certain event has occured by calling parameterValueChanged() */
    void triggerNotification();

    /** Stages a value, to be changed by the processor*/
    virtual void setNextValue (var newValue, bool undoable = true) override;

    /** Gets the value as a string**/
    String getValueAsString() override { return String(); }

    /** Gets the value change description */
    String getChangeDescription() override { return String(); }

    /** Saves the parameter to an XML Element*/
    void toXml (XmlElement*) override {}

    /** Loads the parameter from an XML Element*/
    void fromXml (XmlElement*) override {}
};

#endif // __PARAMETER_H_62922AE5__
