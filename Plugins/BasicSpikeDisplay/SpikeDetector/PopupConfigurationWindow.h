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

class SpikeDetectorEditor;

class SpikeDetectorTableModel : public TableListBoxModel
{

public:

    SpikeDetectorTableModel(SpikeDetectorEditor* editor);

    enum Columns {
        INDEX = 1,
        NAME,
        TYPE,
        CHANNELS,
        WAVEFORM_TYPE,
        THRESHOLD,
        DELETE
    };

    void cellClicked(int rowNumber, int columnId, const MouseEvent& event) override;

    int getNumRows() override;

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);

    void paintCell(Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected);

private:

    SpikeDetectorEditor* editor;
};


class PopupConfigurationWindow : public Component,
    public Label::Listener,
    public Slider::Listener
{

public:
    
    PopupConfigurationWindow(SpikeDetectorEditor* editor, Array<SpikeChannel*> spikeChannels);

    ~PopupConfigurationWindow();

    void update(Array<SpikeChannel*> spikeChannels);

    void labelTextChanged(Label* label);

    void sliderValueChanged(Slider* slider);

    std::unique_ptr<UtilityButton> plusButton;

    std::unique_ptr<Viewport> tableViewport;

    //std::unique_ptr<TableHeaderComponent> tableHeader;
    std::unique_ptr<SpikeDetectorTableModel> tableModel;
    std::unique_ptr<TableListBox> electrodeTable;

private:
    SpikeDetectorEditor* editor;
};


#endif  // __SPIKEDETECTORCONFIGWINDOW_H_F0BD2DD9__
