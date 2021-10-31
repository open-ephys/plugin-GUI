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

//class SpikeDetectorTableHeader : public TableHeaderComponent
//{
 //   SpikeDetector
//}

class EditableTextCustomComponent : public juce::Label,
    public Label::Listener
{
public:
    EditableTextCustomComponent(StringParameter* name_)
        : name(name_)
    {
        setEditable(false, true, false);
        addListener(this);
    }

    void mouseDown(const juce::MouseEvent& event) override;
    
    void labelTextChanged(Label* label);

    void setRowAndColumn(const int newRow, const int newColumn);

    int row;

private:
    StringParameter* name;
    int columnId;
    juce::Colour textColour;
};

class ChannelSelectorCustomComponent : public juce::Label,
public PopupChannelSelector::Listener
{
public:
    ChannelSelectorCustomComponent(SelectedChannelsParameter* channels_)
        : channels(channels_)
    {
        setEditable(false, false, false);
    }

    void mouseDown(const juce::MouseEvent& event) override;
    
    void channelStateChanged(Array<int> newChannels) override
    {
        Array<var> newArray;
    
        for (int i = 0; i < newChannels.size(); i++)
        {
            newArray.add(newChannels[i]);
            std::cout << "Channel " << newChannels[i] << " selected" << std::endl;
        }
            
        
        channels->setNextValue(newArray);
    
    }
    
    void setRowAndColumn(const int newRow, const int newColumn);

    int row;

private:
    SelectedChannelsParameter* channels;
    int columnId;
    juce::Colour textColour;
};


class WaveformSelectorCustomComponent : public Component
{
public:
    WaveformSelectorCustomComponent(CategoricalParameter* waveformtype_)
        : waveformtype(waveformtype_)
    {
    }

    void mouseDown(const juce::MouseEvent& event) override;
    
    void paint(Graphics& g);
    
    void setRowAndColumn(const int newRow, const int newColumn);

    int row;

private:
    CategoricalParameter* waveformtype;
    int columnId;
    juce::Colour textColour;
};

class SpikeDetectorTableModel : public TableListBoxModel
{

public:

    SpikeDetectorTableModel(SpikeDetectorEditor* editor, PopupConfigurationWindow* owner);

    enum Columns {
        INDEX = 1,
        NAME,
        TYPE,
        CHANNELS,
        THRESHOLD,
        WAVEFORM,
        DELETE
    };

    void cellClicked(int rowNumber, int columnId, const MouseEvent& event) override;

    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
        Component* existingComponentToUpdate) override;

    int getNumRows() override;
    
    void update(Array<SpikeChannel*> spikeChannels);

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    void paintCell(Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    Array<SpikeChannel*> spikeChannels;
    TableListBox* table;
private:

    SpikeDetectorEditor* editor;
    
    PopupConfigurationWindow* owner;

};


class PopupConfigurationWindow : public Component,
    public Slider::Listener,
    public Button::Listener
{

public:
    
    PopupConfigurationWindow(SpikeDetectorEditor* editor, Array<SpikeChannel*> spikeChannels);

    ~PopupConfigurationWindow();

    void update(Array<SpikeChannel*> spikeChannels);


    void sliderValueChanged(Slider* slider);

    void buttonClicked(Button* button);

    std::unique_ptr<UtilityButton> plusButton;

    std::unique_ptr<Viewport> tableViewport;

    //std::unique_ptr<TableHeaderComponent> tableHeader;
    std::unique_ptr<SpikeDetectorTableModel> tableModel;
    std::unique_ptr<TableListBox> electrodeTable;

private:
    SpikeDetectorEditor* editor;

};


#endif  // __SPIKEDETECTORCONFIGWINDOW_H_F0BD2DD9__
