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

//class SpikeDetectorTableHeader : public TableHeaderComponent
//{
 //   SpikeDetector
//}

class EditableTextCustomComponent : public juce::Label
{
public:
    EditableTextCustomComponent(PopupConfigurationWindow* configWindow)
        : owner(configWindow)
    {
        setEditable(false, true, false);
    }

    void mouseDown(const juce::MouseEvent& event) override;

    void setRowAndColumn(const int newRow, const int newColumn);

    int row;

private:
    PopupConfigurationWindow* owner;
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

private:

    SpikeDetectorEditor* editor;
    PopupConfigurationWindow* owner;
    Array<SpikeChannel*> spikeChannels;
};


class PopupConfigurationWindow : public Component,
    public Label::Listener,
    public Slider::Listener,
    public Button::Listener
{

public:
    
    PopupConfigurationWindow(SpikeDetectorEditor* editor, Array<SpikeChannel*> spikeChannels);

    ~PopupConfigurationWindow();

    void update(Array<SpikeChannel*> spikeChannels);

    String getChannelName(int row);

    void labelTextChanged(Label* label);

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
