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

#include <JuceHeader.h>
#include "../PluginManager/OpenEphysPlugin.h"

/**
    Class for holding user-definable processor parameters.

    Parameters can either hold boolean, categorical, continuous (float) values,
    or a list of selected channels.

    Using the Parameter class makes it easier to create a graphical interface for editing
    parameters, because each Parameter has a ParameterEditor that can be generated automatically.

    @see GenericProcessor, GenericEditor
*/

class InfoObject;
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
        NAME_PARAM,
        COLOUR_PARAM,
        NOTIFICATION_PARAM
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
    Parameter(InfoObject* infoObject_,
        ParameterType type_,
        ParameterScope scope_,
        const String& name_,
        const String& description_,
        var defaultValue_,
        bool deactivateDuringAcquisition_ = false)
        : infoObject(infoObject_),
        // dataStream(nullptr),
        // spikeChannel(nullptr),
        // eventChannel(nullptr),
        // continuousChannel(nullptr),
        m_parameterType(type_),
        m_parameterScope(scope_),
        m_name(name_),
        m_description(description_),
        currentValue(defaultValue_),
        defaultValue(defaultValue_),
        newValue(defaultValue_),
        m_deactivateDuringAcquisition(deactivateDuringAcquisition_),
        m_identifier("UNKNOWN")
    {
    }

    /** Destructor */
    virtual ~Parameter() { }

    /** Static identifier to keep track of unique parameters */
    static int64 parameterCounter;

    /** Static map from Parameter identifier to name */
    static std::map<int64, String> p_name_map;

    /** Returns the name of the Parameter given its identifier  */
    static String getNameForKey(int64 key) {
        return p_name_map[key];
    }

    /** Static map that keeps track of all Parameters */
    static std::map<std::string, Parameter*> parameterMap;

    /** Returns a pointer to the Parameter for a specific key */
    static void registerParameter(Parameter* p) { parameterMap[p->getKey()] = p; };

    /** Set the globally unique key for this parameter */
    void setKey(std::string key) { m_identifier = key; };

    /** Returns a globally unique key of the parameter */
    std::string getKey() { return m_identifier; }

    /** Returns the common name of the parameter. */
    String getName() const noexcept { return m_name; }

    /** Returns a description of the parameter.*/
    String getDescription() const noexcept { return m_description; }

    /** Returns the type of the parameter. */
    ParameterType getType() const noexcept { return m_parameterType; }
    
    /** Returns the scope of the parameter. */
    ParameterScope getScope() const noexcept { return m_parameterScope; }

    /** Returns the streamId for this parameter (if available)*/
    uint16 getStreamId();

    // /** Sets the streamId for this parameter*/
    // void setDataStream(DataStream* dataStream_) { dataStream = dataStream_;  }
    
    // /** Returns the SpikeChannel for this parameter (if available)*/
    // SpikeChannel* getSpikeChannel() { return spikeChannel; }

    // /** Sets the SpikeChannel for this parameter*/
    // void setSpikeChannel(SpikeChannel* spikeChannel_) { spikeChannel = spikeChannel_;  }
    
    // /** Returns the EventChannel for this parameter (if available)*/
    // EventChannel* getEventChannel() { return eventChannel; }

    // /** Sets the EventChannel for this parameter*/
    // void setEventChannel(EventChannel* eventChannel_) { eventChannel = eventChannel_;  }
    
    // /** Returns the ContinuousChannel for this parameter (if available)*/
    // ContinuousChannel* getContinuousChannel() { return continuousChannel; }

    // /** Sets the ContinuousChannel for this parameter*/
    // void setContinuousChannel(ContinuousChannel* continuousChannel_) { continuousChannel = continuousChannel_;  }

    /** Determines whether the parameter's editor is accessible after acquisition starts*/
    bool shouldDeactivateDuringAcquisition() {
        return m_deactivateDuringAcquisition;
    }

    /** Sets the parameter value*/
    virtual void setNextValue(var newValue) = 0;

    /** Returns the parameter value*/
    var getValue() {
        return currentValue;
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
    virtual void toXml(XmlElement*, bool useKey = false) = 0;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) = 0;
    
    /** Returns the value as a string**/
    virtual String getValueAsString() = 0;
    
    /** Can be used to directly set the parameter value (but be careful with this)*/
    var currentValue;

    /** Can be used to restore the previous value if the new value is out of range*/
    void restorePreviousValue() 
    {
        currentValue = previousValue;
    }
    
    /** Returns a pointer to the InfoObject this parameter is associated with**/
    InfoObject* getOwner() { return infoObject; }

    /** Sets a pointer to the InfoObject this parameter is associated with**/
    void setOwner(InfoObject* newOwner);
    
    /** Makes it possible to undo value changes */
    class ChangeValue : public UndoableAction
    {
    public:
        /** Constructor */
        ChangeValue(std::string key, var newValue);
        
        /** Destructor */
        ~ChangeValue() { }
        
        /** Perform the action */
        bool perform();
        
        /** Undo the action*/
        bool undo();
        
    private:
        std::string key;

        var originalValue;
        var newValue;
    };
    
protected:

    InfoObject* infoObject;
    
    // DataStream* dataStream;
    // SpikeChannel* spikeChannel;
    // EventChannel* eventChannel;
    // ContinuousChannel* continuousChannel;

    var newValue;
    var previousValue;
   
    var defaultValue;

private:

    std::string m_identifier;

    ParameterType m_parameterType;
    ParameterScope m_parameterScope;
    String m_name;
    String m_description;

    bool m_deactivateDuringAcquisition;

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
    BooleanParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        bool defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Stages a value, to be changed by the processor*/
    virtual void setNextValue(var newValue) override;

    /** Gets the value as a boolean*/
    bool getBoolValue();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;

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
    CategoricalParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        Array<String> categories,
        int defaultIndex,
        bool deactivateDuringAcquisition = false);

    /** Stages a value, to be changed by the processor*/
    virtual void setNextValue(var newValue) override;

    /** Gets the index as an integer*/
    int getSelectedIndex();

    /** Gets the index as an integer*/
    String getSelectedString();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Updates the categories*/
    void setCategories(Array<String> categories);

    /** Updates the categories*/
    const Array<String>& getCategories();

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;

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
    IntParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        int defaultValue,
        int minValue = 0,
        int maxValue = 100,
        bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue(var newValue) override;

    /** Gets the value as an integer*/
    int getIntValue();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    int getMinValue() { return minValue; }

    int getMaxValue() { return maxValue; }

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;

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
    StringParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        String defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue(var newValue) override;

    /** Gets the value as an integer*/
    String getStringValue();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;
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
    FloatParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        float defaultValue,
        float minValue = 0.f,
        float maxValue = 100.f,
        float stepSize = 1.f,
        bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue(var newValue) override;

    /** Gets the value as an integer*/
    float getFloatValue();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Gets the minimum value for this parameter*/
    float getMinValue() { return minValue; }

    /** Gets the maximum value for this parameter*/
    float getMaxValue() { return maxValue; }

    /** Gets the step size for this parameter*/
    float getStepSize() { return stepSize; }

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;

private:
    float maxValue;
    float minValue;
    float stepSize;
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
    SelectedChannelsParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        Array<var> defaultValue,
        int maxSelectableChannels = std::numeric_limits<int>::max(),
        bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue(var newValue) override;

    /** Gets the value as an integer*/
    Array<int> getArrayValue();

    /** Returns the max selectable channels*/
    int getMaxSelectableChannels() {
        return maxSelectableChannels;
    }
    
    void setMaxSelectableChannels(int m_) {
        maxSelectableChannels = m_;
     }
    
    /** Returns a vector of channel selection states (true or false)*/
    std::vector<bool> getChannelStates();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Sets the total number of available channels in this stream*/
    void setChannelCount(int count);

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;

private:

    String selectedChannelsToString();

    Array<var> parseSelectedString(const String& input);

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
    MaskChannelsParameter(InfoObject* infoObject,
        ParameterScope scope,
        const String& name,
        const String& description,
        bool deactivateDuringAcquisition = false);

    /** Sets the current value*/
    virtual void setNextValue(var newValue) override;

    /** Gets the value as an integer*/
    Array<int> getArrayValue();
    
    /** Returns a vector of channel selection states (true or false)*/
    std::vector<bool> getChannelStates();
    
    /** Gets the value as a string**/
    virtual String getValueAsString() override;

    /** Sets the total number of available channels in this stream*/
    void setChannelCount(int count);

    /** Saves the parameter to an XML Element*/
    virtual void toXml(XmlElement*, bool useKey = false) override;

    /** Loads the parameter from an XML Element*/
    virtual void fromXml(XmlElement*, bool useKey = false) override;

private:

    String maskChannelsToString();

    Array<var> parseMaskString(const String& input);

    int channelCount;
};


#endif  // __PARAMETER_H_62922AE5__
