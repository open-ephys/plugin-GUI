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
#include "SpikeDetectorEditor.h"
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
    return spikeChannels.size();
}

void SpikeDetectorTableModel::update(Array<SpikeChannelSettings*> spikeChannels_)
{
    spikeChannels = spikeChannels_;
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

void SpikeDetectorTableModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (columnId == SpikeDetectorTableModel::Columns::INDEX)
    {
        g.setColour(Colours::white);
        g.drawText(String(rowNumber + 1), 0, 0, width, height, Justification::left);
    } 
    else if (columnId == SpikeDetectorTableModel::Columns::NAME)
    {
        g.setColour(Colours::white);
        //std::cout << "Drawing name column for row " << rowNumber << std::endl;
        g.drawText(spikeChannels[rowNumber]->name, 0, 0, width, height, Justification::left);
    } 
    else if (columnId == SpikeDetectorTableModel::Columns::TYPE)
    {
        switch (spikeChannels[rowNumber]->type)
        {
        case SpikeChannel::Type::SINGLE:
            g.setColour(Colours::blue);
            g.fillRoundedRectangle(4, 4, width - 8, height - 8, 3);
            g.setColour(Colours::white);
            g.drawText("SINGLE", 4, 4, width-8, height-8, Justification::centred);
            break;
        case SpikeChannel::Type::STEREOTRODE:
            g.setColour(Colours::purple);
            g.fillRoundedRectangle(4, 4, width - 8, height - 8, 3);
            g.setColour(Colours::white);
            g.drawText("STEREOTRODE", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        case SpikeChannel::Type::TETRODE:
            g.setColour(Colours::green);
            g.fillRoundedRectangle(4, 4, width - 8, height - 8, 3);
            g.setColour(Colours::white);
            g.drawText("TETRODE", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        }
        
    }
    else if (columnId == SpikeDetectorTableModel::Columns::DELETE)
    {
        g.setColour(Colours::red);
        g.fillEllipse(6, 6, width - 12, height - 12);
        g.setColour(Colours::white);
        g.drawLine(8, height / 2, width - 10, height / 2, 3.0);
    }
}


PopupConfigurationWindow::PopupConfigurationWindow(SpikeDetectorEditor* editor_, Array<SpikeChannelSettings*> spikeChannels) : editor(editor_)
{
    //tableHeader.reset(new TableHeaderComponent());

    setSize(40, 40);

    tableModel.reset(new SpikeDetectorTableModel(editor));

    electrodeTable = std::make_unique<TableListBox>("Electrode Table", tableModel.get());
    electrodeTable->setHeader(std::make_unique<TableHeaderComponent>());

    electrodeTable->getHeader().addColumn("#", SpikeDetectorTableModel::Columns::INDEX, 30, 30, 30);
    electrodeTable->getHeader().addColumn("Name", SpikeDetectorTableModel::Columns::NAME, 140, 140, 140);
    electrodeTable->getHeader().addColumn("Type", SpikeDetectorTableModel::Columns::TYPE, 70, 70, 70);
    electrodeTable->getHeader().addColumn("Channels", SpikeDetectorTableModel::Columns::CHANNELS, 100, 100, 100);
    electrodeTable->getHeader().addColumn("Thresholds", SpikeDetectorTableModel::Columns::THRESHOLD, 100, 100, 100);
    electrodeTable->getHeader().addColumn("Waveform", SpikeDetectorTableModel::Columns::WAVEFORM, 50, 50, 50);
    electrodeTable->getHeader().addColumn(" ", SpikeDetectorTableModel::Columns::DELETE, 30, 30, 30);

    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::backgroundColourId, Colour(240, 240, 240));
    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::highlightColourId, Colour(50, 240, 240));
    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::textColourId, Colour(40, 40, 40));

    electrodeTable->setHeaderHeight(24);
    electrodeTable->setRowHeight(30);
    electrodeTable->setMultipleSelectionEnabled(true);

    addChildComponent(electrodeTable.get());

    plusButton = std::make_unique<UtilityButton>("+", Font("Default", 16, Font::plain));
    plusButton->addListener(this);
    plusButton->setBounds(5, 5, 30, 30);
    addAndMakeVisible(plusButton.get());

    update(spikeChannels);
}

PopupConfigurationWindow::~PopupConfigurationWindow()
{

}


void PopupConfigurationWindow::update(Array<SpikeChannelSettings*> spikeChannels)
{
    if (spikeChannels.size() > 0)
    {
        electrodeTable->setVisible(true);

        int maxRows = 16;

        int numRows = spikeChannels.size() <= maxRows ? spikeChannels.size() : maxRows;

        tableModel->update(spikeChannels);

        setSize(530, (numRows + 1) * 30 + 45);
        electrodeTable->setBounds(5, 5, 520, (numRows + 1) * 30);
        plusButton->setBounds(5, getHeight() - 35, 30, 30);

    }
    else {
        electrodeTable->setVisible(false);
    }
        
    
}

void PopupConfigurationWindow::labelTextChanged(Label* label)
{

}

void PopupConfigurationWindow::sliderValueChanged(Slider* slider)
{

}

void PopupConfigurationWindow::buttonClicked(Button* button)
{
    if (button == plusButton.get())
    {
        PopupMenu countMenu;
        PopupMenu individualSpikeChannelMenu;

        for (int i = 15; i > 0; i--)
        {
            PopupMenu multipleSpikeChannelMenu;

            multipleSpikeChannelMenu.addItem(SpikeChannel::SINGLE + i * 3, "Single Electrodes");
            multipleSpikeChannelMenu.addItem(SpikeChannel::STEREOTRODE + i * 3, "Stereotrodes");
            multipleSpikeChannelMenu.addItem(SpikeChannel::TETRODE + i * 3, "Tetrodes");

            countMenu.addSubMenu(String(i+1), multipleSpikeChannelMenu);
        }

        individualSpikeChannelMenu.addItem(SpikeChannel::SINGLE, "Single Electrode");
        individualSpikeChannelMenu.addItem(SpikeChannel::STEREOTRODE, "Stereotrode");
        individualSpikeChannelMenu.addItem(SpikeChannel::TETRODE, "Tetrode");

        countMenu.addSubMenu("1", individualSpikeChannelMenu);

        const int result = countMenu.show();

        if (result == 0)
            return;

        int numSpikeChannelsToAdd = (result - 1) / 3 + 1;
        int channelType = (result -1) % 3 + 1;

        editor->addSpikeChannels((SpikeChannel::Type) channelType, numSpikeChannelsToAdd);

    }
}