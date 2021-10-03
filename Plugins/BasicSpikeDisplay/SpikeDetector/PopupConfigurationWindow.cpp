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


void EditableTextCustomComponent::mouseDown(const MouseEvent& event)
{
    owner->electrodeTable->selectRowsBasedOnModifierKeys(row, event.mods, false);

    Label::mouseDown(event);
}

void EditableTextCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    row = newRow;
    columnId = newColumn;
    setText(owner->getChannelName(row), dontSendNotification);
}

SpikeDetectorTableModel::SpikeDetectorTableModel(SpikeDetectorEditor* editor_,
    PopupConfigurationWindow* owner_)
    : editor(editor_), owner(owner_)
{

}

void SpikeDetectorTableModel::cellClicked(int rowNumber, int columnId, const MouseEvent& event)
{
    std::cout << rowNumber << " " << columnId << " : selected " << std::endl;

    if (columnId == SpikeDetectorTableModel::Columns::CHANNELS)
    {
        std::vector<bool> channelStates;

        for (int i = 0; i < spikeChannels[rowNumber]->maxLocalChannel; i++)
        {
            channelStates.push_back(spikeChannels[rowNumber]->localChannelIndexes.contains(i));

        }

        std::cout << "Num channels: " << spikeChannels[rowNumber]->maxLocalChannel << std::endl;

        auto* channelSelector = new PopupChannelSelector(editor, channelStates);
        channelSelector->setMaximumSelectableChannels(spikeChannels[rowNumber]->expectedChannelCount);
        channelSelector->setChannelButtonColour(Colour(0, 174, 239));

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector), 
                event.eventComponent->getScreenBounds(), 
                nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
    }

    //SparseSet<int> selectedRows = table->getSelectedRows();

    //for (int i = 0; i < selectedRows.size(); i++)
    //{
    //    std::cout << selectedRows[i] << " ";
    //}

    //std::cout << std::endl;

    if (event.mods.isRightButtonDown())
        std::cout << "Right click!" << std::endl;
}

Component* SpikeDetectorTableModel::refreshComponentForCell(int rowNumber, 
    int columnId, 
    bool isRowSelected,
    Component* existingComponentToUpdate)
{

    //if (columnId == columnId == SpikeDetectorTableModel::Columns::NAME)  // [8]
    //{
    //    auto* selectionBox = static_cast<SelectionColumnCustomComponent*> (existingComponentToUpdate);

    //    if (selectionBox == nullptr)
    //        selectionBox = new SelectionColumnCustomComponent(*this);

     //   selectionBox->setRowAndColumn(rowNumber, columnId);
     //   return selectionBox;
    //}

    if (columnId == SpikeDetectorTableModel::Columns::NAME)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        if (textLabel == nullptr)
        {
            textLabel = new EditableTextCustomComponent(owner);
            
        }
            
        textLabel->addListener(owner);
        textLabel->setRowAndColumn(rowNumber, columnId);
        
        return textLabel;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
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
    if (rowIsSelected)
    {
        if (rowNumber % 2 == 0)
            g.fillAll(Colour(100, 100, 100));
        else
            g.fillAll(Colour(80, 80, 80));

        return;
    }

    if (rowNumber % 2 == 0)
        g.fillAll(Colour(50, 50, 50));
    else
        g.fillAll(Colour(30, 30, 30));


}

void SpikeDetectorTableModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (columnId == SpikeDetectorTableModel::Columns::INDEX)
    {
        g.setColour(Colours::white);
        g.drawText(String(rowNumber + 1), 4, 0, width, height, Justification::left);
    } 
    else if (columnId == SpikeDetectorTableModel::Columns::NAME)
    {
        g.setColour(Colours::white);
        //std::cout << "Drawing name column for row " << rowNumber << std::endl;
        g.drawText(spikeChannels[rowNumber]->name, 4, 0, width, height, Justification::left);
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
    else if (columnId == SpikeDetectorTableModel::Columns::CHANNELS)
    {
        g.setColour(Colours::white);

        String channelString = "[";

        for (int i = 0; i < spikeChannels[rowNumber]->expectedChannelCount; i++)
        {
            channelString += String(spikeChannels[rowNumber]->localChannelIndexes[i]);
            
            if (i != spikeChannels[rowNumber]->expectedChannelCount - 1)
                channelString += ", ";
        }
        
        channelString += "]";
           
        g.drawText(channelString, 4, 4, width - 8, height - 8, Justification::centred);
    } 
    else if (columnId == SpikeDetectorTableModel::Columns::THRESHOLD)
    {
        switch (spikeChannels[rowNumber]->thresholdType)
        {
        case SpikeChannelSettings::ThresholdType::FIXED:
            g.setColour(Colours::blue);
            g.fillRoundedRectangle(4, 4, width - 8, height - 8, 3);
            g.setColour(Colours::white);
            g.drawText("FIXED", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        case SpikeChannelSettings::ThresholdType::STD:
            g.setColour(Colours::purple);
            g.fillRoundedRectangle(4, 4, width - 8, height - 8, 3);
            g.setColour(Colours::white);
            g.drawText("STD", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        case SpikeChannelSettings::ThresholdType::DYNAMIC:
            g.setColour(Colours::green);
            g.fillRoundedRectangle(4, 4, width - 8, height - 8, 3);
            g.setColour(Colours::white);
            g.drawText("DYNAMIC", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        }
    }
    else if (columnId == SpikeDetectorTableModel::Columns::WAVEFORM)
    {
        g.setColour(Colours::white);
        if (spikeChannels[rowNumber]->sendFullWaveform)
        {
            g.drawText("FULL", 4, 4, width - 8, height - 8, Justification::centred);
        }
        else {
            g.drawText("PEAK ONLY", 4, 4, width - 8, height - 8, Justification::centred);
        }
    }

    else if (columnId == SpikeDetectorTableModel::Columns::DELETE)
    {
        g.setColour(Colours::red);
        g.fillEllipse(7, 7, width - 14, height - 14);
        g.setColour(Colours::white);
        g.drawLine(9, height / 2, width - 9, height / 2, 3.0);
    }
}


PopupConfigurationWindow::PopupConfigurationWindow(SpikeDetectorEditor* editor_, Array<SpikeChannelSettings*> spikeChannels) : editor(editor_)
{
    //tableHeader.reset(new TableHeaderComponent());

    setSize(40, 40);

    tableModel.reset(new SpikeDetectorTableModel(editor, this));

    electrodeTable = std::make_unique<TableListBox>("Electrode Table", tableModel.get());
    electrodeTable->setHeader(std::make_unique<TableHeaderComponent>());

    electrodeTable->getHeader().addColumn("#", SpikeDetectorTableModel::Columns::INDEX, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Name", SpikeDetectorTableModel::Columns::NAME, 140, 140, 140, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Type", SpikeDetectorTableModel::Columns::TYPE, 70, 70, 70, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Channels", SpikeDetectorTableModel::Columns::CHANNELS, 100, 100, 100, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Thresholds", SpikeDetectorTableModel::Columns::THRESHOLD, 100, 100, 100, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Waveform", SpikeDetectorTableModel::Columns::WAVEFORM, 50, 50, 50, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn(" ", SpikeDetectorTableModel::Columns::DELETE, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);

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

    spikeChannelsForCurrentStream = spikeChannels;

    if (spikeChannels.size() > 0)
    {
        electrodeTable->setVisible(true);

        int maxRows = 16;

        int numRows = spikeChannels.size() <= maxRows ? spikeChannels.size() : maxRows;

        tableModel->update(spikeChannels);

        int scrollBarWidth = 0;

        if (spikeChannels.size() > maxRows)
            scrollBarWidth += 20;

        setSize(530 + scrollBarWidth, (numRows + 1) * 30 + 45);
        electrodeTable->setBounds(5, 5, 520 + scrollBarWidth, (numRows + 1) * 30);
        plusButton->setBounds(5, getHeight() - 35, 30, 30);

        electrodeTable->resized();

    }
    else {
        electrodeTable->setVisible(false);
    }
    
}

String PopupConfigurationWindow::getChannelName(int row)
{
    return spikeChannelsForCurrentStream[row]->name;
}

void PopupConfigurationWindow::labelTextChanged(Label* label)
{
    EditableTextCustomComponent* l = (EditableTextCustomComponent*)label;
    std::cout << "New name for electrode " << l->row << ": " << l->getText() << std::endl;
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