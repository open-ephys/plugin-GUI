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

#include "PopupConfigurationWindow.h"
#include "SpikeDetector.h"
#include <stdio.h>

SpikeDetectorTableModel::SpikeDetectorTableModel(SpikeDetectorEditor* editor_)
    : editor(editor_)
{

}

void SpikeDetectorTableModel::cellClicked(int rowNumber, int columnId, const MouseEvent& event)
{
    std::cout << rowNumber << " " << columnId << std::endl;
}

int SpikeDetectorTableModel::getNumRows()
{
    return 12;
}

void SpikeDetectorTableModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowNumber % 2 == 0)
        g.fillAll(Colour(50, 50, 50));
    else
        g.fillAll(Colour(30, 30, 30));

    if (rowIsSelected)
        g.fillAll(Colour(100, 100, 100));
}

void SpikeDetectorTableModel::paintCell(Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{

}


PopupConfigurationWindow::PopupConfigurationWindow(SpikeDetectorEditor* editor_, Array<SpikeChannel*> spikeChannels) : editor(editor_)
{
    //tableHeader.reset(new TableHeaderComponent());

    setSize(500, 500);

    tableModel.reset(new SpikeDetectorTableModel(editor));

    electrodeTable = std::make_unique<TableListBox>("Electrode Table", tableModel.get());
    electrodeTable->setHeader(std::make_unique<TableHeaderComponent>());

    electrodeTable->getHeader().addColumn("#", SpikeDetectorTableModel::Columns::INDEX, 20, 20, 20);
    electrodeTable->getHeader().addColumn("Name", SpikeDetectorTableModel::Columns::NAME, 100, 50, 140);
    electrodeTable->getHeader().addColumn("Type", SpikeDetectorTableModel::Columns::TYPE, 30, 30, 30);
    electrodeTable->getHeader().addColumn("CH", SpikeDetectorTableModel::Columns::CHANNELS, 50, 50, 50);
    electrodeTable->getHeader().addColumn("Thresh", SpikeDetectorTableModel::Columns::THRESHOLD, 40, 40, 40);
    electrodeTable->getHeader().addColumn("Window", SpikeDetectorTableModel::Columns::WAVEFORM_TYPE, 50, 50, 50);
    electrodeTable->getHeader().addColumn("Delete", SpikeDetectorTableModel::Columns::DELETE, 20, 20, 20);

    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::backgroundColourId, Colour(240, 240, 240));
    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::highlightColourId, Colour(50, 240, 240));
    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::textColourId, Colour(40, 40, 40));

    electrodeTable->setHeaderHeight(20);
    electrodeTable->setRowHeight(20);
    electrodeTable->setMultipleSelectionEnabled(true);

    electrodeTable->setBounds(15, 25, 500-30, 500-50);
    addAndMakeVisible(electrodeTable.get());
}

PopupConfigurationWindow::~PopupConfigurationWindow()
{

}


void PopupConfigurationWindow::update(Array<SpikeChannel*> spikeChannels)
{

}

void PopupConfigurationWindow::labelTextChanged(Label* label)
{

}

void PopupConfigurationWindow::sliderValueChanged(Slider* slider)
{

}