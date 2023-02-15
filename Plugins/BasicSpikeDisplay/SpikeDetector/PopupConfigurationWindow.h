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

#ifndef __SPIKEDETECTORCONFIGWINDOW_H_F0BD2DD9__
#define __SPIKEDETECTORCONFIGWINDOW_H_F0BD2DD9__


#include <EditorHeaders.h>

#include "SpikeDetector.h"

class SpikeDetectorEditor;
class PopupConfigurationWindow;
class SpikeDetectorTableModel;

/** 
*   Table component used to edit Spike Channel names
*/
class EditableTextCustomComponent : 
    public juce::Label,
    public Label::Listener
{
public:

    /** Constructor */
    EditableTextCustomComponent(SpikeDetector* spikeDetector_, StringParameter* name_, bool acquisitionIsActive_)
        : name(name_),
          spikeDetector(spikeDetector_),
          acquisitionIsActive(acquisitionIsActive_)
    {
        setEditable(false, !acquisitionIsActive, false);
        addListener(this);
        setColour(Label::textColourId, Colours::white);
        setColour(Label::textWhenEditingColourId, Colours::yellow);
        setColour(TextEditor::highlightedTextColourId, Colours::yellow);
    }

    /** Responds to button clicks */
    void mouseDown(const juce::MouseEvent& event) override;
    
    /** Called when the label is updated */
    void labelTextChanged(Label* label) override;

    /** Sets row and column */
    void setRowAndColumn(const int newRow, const int newColumn);
    
    /** Sets the "name* parameter referenced by this component */
    void setParameter(StringParameter* name_) { name = name_; }

    int row;

private:
    StringParameter* name;
    SpikeDetector* spikeDetector;
    bool acquisitionIsActive;
    int columnId;
};

/**
*   Table component used to edit the continuous channels
*   used by a Spike Channel
*/
class ChannelSelectorCustomComponent : 
    public juce::Label,
    public PopupChannelSelector::Listener
{
public:

    /** Constructor */
    ChannelSelectorCustomComponent(SelectedChannelsParameter* channels_, bool acquisitionIsActive_)
        : channels(channels_),
          acquisitionIsActive(acquisitionIsActive_)
    {
        setEditable(false, false, false);
    }

    /** Responds to mouse clicks */
    void mouseDown(const juce::MouseEvent& event) override;
    
    /** Callback for changes in PopupChannelSelector */
    void channelStateChanged(Array<int> newChannels) override
    {
        Array<var> newArray;
    
        for (int i = 0; i < newChannels.size(); i++)
        {
            newArray.add(newChannels[i]);
            LOGD("Channel ", newChannels[i], " selected");
        }
        
        String s = "[";
        
        for (auto chan : newArray)
        {
            s += String(int(chan)+1) + ",";
        }
        
        s += "]";
        
        setText(s, dontSendNotification);
            
        channels->setNextValue(newArray);
    
    }
    
    /** Sets row and column */
    void setRowAndColumn(const int newRow, const int newColumn);
    
    /** Sets the underlying parametr for this component */
    void setParameter(SelectedChannelsParameter* channels_) { channels = channels_; }

    int row;

private:
    SelectedChannelsParameter* channels;
    int columnId;
    juce::Colour textColour;
    bool acquisitionIsActive;
};

class ThresholdSelectorCustomComponent;

/** 
    Popup component that is used to set threshold
    type and threshold level

*/
class PopupThresholdComponent : public Component,
    public Slider::Listener,
    public Button::Listener
{
public:

    /** Constructor */
    PopupThresholdComponent(SpikeDetectorTableModel* table,
                            ThresholdSelectorCustomComponent* owner,
                            int row,
                            int numChannels,
                            ThresholderType type,
                            Array<FloatParameter*> abs_thresholds,
                            Array<FloatParameter*> dyn_thresholds,
                            Array<FloatParameter*> std_thresholds,
                            bool isLocked);

    /** Destructor */
    ~PopupThresholdComponent();
    
    /** Creates the threshold sliders */
    void createSliders();
    
    /** Responds to slider value changes */
    void sliderValueChanged(Slider* slider);

    /** Responds to button clicks */
    void buttonClicked(Button* button);
    
private:
    std::unique_ptr<UtilityButton> lockButton;
    std::unique_ptr<UtilityButton> absButton;
    std::unique_ptr<UtilityButton> stdButton;
    std::unique_ptr<UtilityButton> dynButton;
    std::unique_ptr<Label> label;
    OwnedArray<Slider> sliders;
    
    Array<FloatParameter*> abs_thresholds;
    Array<FloatParameter*> dyn_thresholds;
    Array<FloatParameter*> std_thresholds;
    
    ThresholderType thresholdType;
    SpikeDetectorTableModel* table;
    ThresholdSelectorCustomComponent* owner;
    
    const int sliderWidth = 18;
    int row;
};

/**
*   Table component used to edit the thresholds
*   used by a Spike Channel
*/
class ThresholdSelectorCustomComponent : public Component
{
public:

    /** Constructor */
    ThresholdSelectorCustomComponent(SpikeChannel* channel_, bool acquisitionIsActive_);

    /** Destructor */
    ~ThresholdSelectorCustomComponent();
    
    /** Sets the SpikeChannel for this component*/
    void setSpikeChannel(SpikeChannel* channel);

    /** Handles mouse clicks */
    void mouseDown(const juce::MouseEvent& event) override;
    
    /** Renders the threshold values */
    void paint(Graphics& g) override;

    /** Sets the row and column */
    void setRowAndColumn(const int newRow, const int newColumn);

    /** Sets the threshold parameters */
    void setThreshold(ThresholderType type, int channelNum, float value);
    
    /** Sets a pointer to the SpikeDetectorTableModel object */
    void setTableModel(SpikeDetectorTableModel* table_) { table = table_; };

    int row;
    
    SpikeChannel* channel;

private:

    SpikeDetectorTableModel* table;
    

    int columnId;
    juce::Colour textColour;
    bool acquisitionIsActive;
    
    Array<FloatParameter*> dyn_thresholds;
    Array<FloatParameter*> abs_thresholds;
    Array<FloatParameter*> std_thresholds;
    
    CategoricalParameter* thresholder_type;
    
    String thresholdString;
    

};

/**
*   Table component used to select the waveform type
*   (full vs. peak) for a Spike Channel.
*/
class WaveformSelectorCustomComponent : public Component
{
public:

    /** Constructor */
    WaveformSelectorCustomComponent(CategoricalParameter* waveformtype_, bool acquisitionIsActive_)
        : waveformtype(waveformtype_),
          acquisitionIsActive(acquisitionIsActive_)
    {
    }

    /** Handles mouse clicks */
    void mouseDown(const juce::MouseEvent& event) override;
    
    /** Renders the waveform type icon */
    void paint(Graphics& g) override;
    
    /** Sets row and column */
    void setRowAndColumn(const int newRow, const int newColumn);
    
    /** Sets the underlying parameter object controlled by this component */
    void setParameter(CategoricalParameter* waveformtype_) { waveformtype = waveformtype_; }

    /** Sets the value of the parameter */
    void setWaveformValue(int value);

    /** Sets a pointer to the SpikeDetectorTableModel object */
    void setTableModel(SpikeDetectorTableModel* table_) { table = table_; };

    int row;

private:
    CategoricalParameter* waveformtype;
    SpikeDetectorTableModel* table;
    int columnId;
    juce::Colour textColour;
    bool acquisitionIsActive;
};


/**
*   Table component used to delete electrodes
*/
class DeleteButtonCustomComponent : public Component
{
public:

    /** Constructor */
    DeleteButtonCustomComponent(bool acquisitionIsActive_)
        : acquisitionIsActive(acquisitionIsActive_)
    {
    }

    /** Handles mouse click events */
    void mouseDown(const juce::MouseEvent& event) override;

    /** Renders the delete icon */
    void paint(Graphics& g) override;

    /** Sets row and column */
    void setRowAndColumn(const int newRow, const int newColumn);

    /** Sets a pointer to the SpikeDetectorTableModel object */
    void setTableModel(SpikeDetectorTableModel* table_) { table = table_; };

    int row;

private:
    SpikeDetectorTableModel* table;
    int columnId;
    bool acquisitionIsActive;
};

/**
*   TableListBoxModel used for editing Spike Channel parameters
*/
class SpikeDetectorTableModel : public TableListBoxModel
{

public:

    /** Constructor */
    SpikeDetectorTableModel(SpikeDetectorEditor* editor, 
                            PopupConfigurationWindow* owner,
                            bool acquisitionIsActive);

    /** Column types*/
    enum Columns {
        INDEX = 1,
        NAME,
        TYPE,
        CHANNELS,
        THRESHOLD,
        WAVEFORM,
        DELETE
    };

    /** Callback when a cell is clicked (not a sub-component) */
    void cellClicked(int rowNumber, int columnId, const MouseEvent& event) override;

    /** Called whenever a cell needs to be updated; creates custom components inside each cell*/
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
        Component* existingComponentToUpdate) override;

    /** Returns the number of rows in the table */
    int getNumRows() override;
    
    /** Updates the underlying SpikeChannel objects */
    void update(Array<SpikeChannel*> spikeChannels);

    /** Determines row colors */
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    /** Changes waveform type when multiple rows are selected */
    void broadcastWaveformTypeToSelectedRows(int rowThatWasClicked, int value);

    /** Changes threshold value when multiple rows are selected */
    void broadcastThresholdToSelectedRows(int rowThatWasClicked, ThresholderType type, int channelIndex, bool isLocked, float value);

    /** Changes threshold type when multiple rows are selected */
    void broadcastThresholdTypeToSelectedRows(int rowThatWasClicked, ThresholderType type);

    /** Deletes the SpikeChannel objects associated with each row */
    void deleteSelectedRows(int rowThatWasClicked);

    /** Paints the INDEX and TYPE columns*/
    void paintCell(Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    Array<SpikeChannel*> spikeChannels;
    TableListBox* table;
private:

    OwnedArray<WaveformSelectorCustomComponent> waveformComponents;
    OwnedArray<ThresholdSelectorCustomComponent> thresholdComponents;

    SpikeDetectorEditor* editor;
    
    PopupConfigurationWindow* owner;

    bool acquisitionIsActive;

};

/** 
    Interface to generate new Spike Channels
*/
class SpikeChannelGenerator : 
    public Component,
    public PopupChannelSelector::Listener,
    public Button::Listener,
    public Label::Listener
{
public:

    /** Constructor */
    SpikeChannelGenerator(SpikeDetectorEditor* editor, 
                          PopupConfigurationWindow* window,
                          int channelCount,
                          bool acquisitionIsActive);

    /** Destructor*/
    ~SpikeChannelGenerator() { }

    /** Responds to changes in the PopupChannelSelector*/
    void channelStateChanged(Array<int> selectedChannels);

    /** Responds to button clicks*/
    void buttonClicked(Button* button);

    /** Responds to Label */
    void labelTextChanged(Label* label);

    /** Draws border and text */
    void paint(Graphics& g);

private:

    SpikeDetectorEditor* editor;
    PopupConfigurationWindow* window;

    int channelCount;
    String lastLabelValue;
    Array<int> startChannels;

    std::unique_ptr<Label> spikeChannelCountLabel;
    std::unique_ptr<ComboBox> spikeChannelTypeSelector;
    std::unique_ptr<Button> channelSelectorButton;
    std::unique_ptr<UtilityButton> plusButton;
};

/**
*   Popup window used to edit Spike Channel settings
*/
class PopupConfigurationWindow : public Component,
    public ScrollBar::Listener
{

public:
    
    /** Constructor */
    PopupConfigurationWindow(SpikeDetectorEditor* editor, 
                             Array<SpikeChannel*> spikeChannels,
                             bool acquisitionIsActive);

    /** Destructor */
    ~PopupConfigurationWindow() { }

    /** Updates the window with a new set of Spike Channels*/
    void update(Array<SpikeChannel*> spikeChannels);

    /** Custom table header component (not currently used)*/
    //std::unique_ptr<TableHeaderComponent> tableHeader;

    /** Custom table model*/
    std::unique_ptr<SpikeDetectorTableModel> tableModel;

    /** Custom list box for Spike Channel settings*/
    std::unique_ptr<TableListBox> electrodeTable;
    
    /** Listens for viewport scrolling */
    void scrollBarMoved(ScrollBar* scrollBar, double newRangeStart);

private:
    SpikeDetectorEditor* editor;

    std::unique_ptr<SpikeChannelGenerator> spikeChannelGenerator;
    
    std::unique_ptr<Viewport> viewport;

    int scrollDistance = 0;
    
    bool updating = false;
};


#endif  // __SPIKEDETECTORCONFIGWINDOW_H_F0BD2DD9__
