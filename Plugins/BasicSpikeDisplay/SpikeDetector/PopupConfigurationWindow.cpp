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
    //owner->selectRowsBasedOnModifierKeys(row, event.mods, false);
    Label::mouseDown(event);
}

void EditableTextCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    row = newRow;
    columnId = newColumn;
    setText(name->getStringValue(), dontSendNotification);
    
}

void EditableTextCustomComponent::labelTextChanged(Label* label)
{
    name->setNextValue(label->getText());
}

void ThresholdSelectorCustomComponent::mouseDown(const MouseEvent& event)
{
    //owner->selectRowsBasedOnModifierKeys(row, event.mods, false);
    Label::mouseDown(event);
}

void ThresholdSelectorCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    row = newRow;
    columnId = newColumn;
    setText(threshold->getValueAsString(), dontSendNotification);
}

void ThresholdSelectorCustomComponent::labelTextChanged(Label* label)
{
    table->broadcastThresholdToSelectedRows(row, label->getText().getFloatValue());
}

void ThresholdSelectorCustomComponent::setThreshold(float value)
{
    threshold->setNextValue(value);
    //setText(String(value), dontSendNotification);

    //repaint();
}


void ChannelSelectorCustomComponent::mouseDown(const juce::MouseEvent& event)
{
    if (acquisitionIsActive)
        return;

    auto* channelSelector = new PopupChannelSelector(this, channels->getChannelStates());
    
    channelSelector->setChannelButtonColour(Colour(0, 174, 239));
    channelSelector->setMaximumSelectableChannels(channels->getMaxSelectableChannels());

     CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            getScreenBounds(),
            nullptr);
}
    
void ChannelSelectorCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    
    Array<int> chans = channels->getArrayValue();
    
    String s = "[";
    
    for (auto chan : chans)
    {
        s += String(chan+1) + ",";
    }
    
    s += "]";
    
    setText(s, dontSendNotification);
}


void WaveformSelectorCustomComponent::mouseDown(const juce::MouseEvent& event)
{
    if (acquisitionIsActive)
        return;

    if (waveformtype->getValueAsString().equalsIgnoreCase("FULL"))
    {
        table->broadcastWaveformTypeToSelectedRows(row, 1);
    }
    else
    {
        table->broadcastWaveformTypeToSelectedRows(row, 0);
    }
    
    repaint();
}

void WaveformSelectorCustomComponent::setWaveformValue(int value)
{

    waveformtype->setNextValue(value);

    //repaint();
}
    
void WaveformSelectorCustomComponent::paint(Graphics& g)
{
 
    if (waveformtype->getValueAsString().equalsIgnoreCase("FULL"))
        g.setColour(Colours::green);
    else
        g.setColour(Colours::red);
    
    g.fillRoundedRectangle(6, 6, getWidth() - 12, getHeight() - 12, 4);
    g.setColour(Colours::white);
    g.drawText(waveformtype->getValueAsString(), 4, 4, getWidth()-8, getHeight()-8, Justification::centred);
}
    
void WaveformSelectorCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    row = newRow;
    repaint();
}



SpikeDetectorTableModel::SpikeDetectorTableModel(SpikeDetectorEditor* editor_,
    PopupConfigurationWindow* owner_,
    bool acquisitionIsActive_)
    : editor(editor_), owner(owner_), acquisitionIsActive(acquisitionIsActive_)
{

}

void SpikeDetectorTableModel::cellClicked(int rowNumber, int columnId, const MouseEvent& event)
{
    std::cout << rowNumber << " " << columnId << " : selected " << std::endl;
    
    SparseSet<int> selectedRows = table->getSelectedRows();

    //for (int i = 0; i < selectedRows.size(); i++)
    //{
    //    std::cout << selectedRows[i] << " ";
    //}
    if (columnId == SpikeDetectorTableModel::Columns::DELETE && !acquisitionIsActive)
    {
        std::cout << "Delete " << selectedRows.size() << " electrodes?" << std::endl;
        
        Array<SpikeChannel*> channelsToDelete;
        Array<SpikeChannel*> channelsToKeep;
        
        for (int i = 0; i < spikeChannels.size(); i++)
        {
            if (selectedRows.contains(i))
                channelsToDelete.add(spikeChannels[i]);
            else
                channelsToKeep.add(spikeChannels[i]);
        }
        
        owner->update(channelsToKeep);
        
        editor->removeSpikeChannels(channelsToDelete);
    }

    if (event.mods.isRightButtonDown())
        std::cout << "Right click!" << std::endl;
}

void SpikeDetectorTableModel::broadcastWaveformTypeToSelectedRows(int rowThatWasClicked, int value)
{
    SparseSet<int> selectedRows = table->getSelectedRows();

    for (int i = 0; i < spikeChannels.size(); i++)
    {
        if (selectedRows.contains(i) || i == rowThatWasClicked)
        {
            Component* c = refreshComponentForCell(i, SpikeDetectorTableModel::WAVEFORM, selectedRows.contains(i), nullptr);

            jassert(c != nullptr);

            WaveformSelectorCustomComponent* waveformButton = (WaveformSelectorCustomComponent*)c;

            jassert(waveformButton != nullptr);

            waveformComponents.add(waveformButton);

            waveformButton->setWaveformValue(value);
        }
    }

    table->updateContent();
}


void SpikeDetectorTableModel::broadcastThresholdToSelectedRows(int rowThatWasClicked, float value)
{
    SparseSet<int> selectedRows = table->getSelectedRows();

    for (int i = 0; i < spikeChannels.size(); i++)
    {
        if (selectedRows.contains(i) || i == rowThatWasClicked)
        {

            std::cout << "Row " << i << ": " << " threshold = " << value << std::endl;
            Component* c = refreshComponentForCell(i, SpikeDetectorTableModel::THRESHOLD, selectedRows.contains(i), nullptr);

            jassert(c != nullptr);

            ThresholdSelectorCustomComponent* thresholdSelector = (ThresholdSelectorCustomComponent*)c;

            jassert(thresholdSelector != nullptr);

            thresholdComponents.add(thresholdSelector);

            thresholdSelector->setThreshold(value);
        }
    }

    table->updateContent();
}

Component* SpikeDetectorTableModel::refreshComponentForCell(int rowNumber, 
    int columnId, 
    bool isRowSelected,
    Component* existingComponentToUpdate)
{
    if (columnId == SpikeDetectorTableModel::Columns::NAME)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        if (textLabel == nullptr)
        {
            textLabel = new EditableTextCustomComponent((StringParameter*) spikeChannels[rowNumber]->getParameter("name"),
                                                        acquisitionIsActive);
        }
            
        textLabel->setColour(Label::textColourId, Colours::white);
        textLabel->setParameter((StringParameter*)spikeChannels[rowNumber]->getParameter("name"));
        textLabel->setRowAndColumn(rowNumber, columnId);
        
        return textLabel;
    }
    else if (columnId == SpikeDetectorTableModel::Columns::CHANNELS)
    {
        auto* channelsLabel = static_cast<ChannelSelectorCustomComponent*> (existingComponentToUpdate);

        if (channelsLabel == nullptr)
        {
            channelsLabel = new ChannelSelectorCustomComponent(
                (SelectedChannelsParameter*) spikeChannels[rowNumber]->getParameter("local_channels"),
                acquisitionIsActive);
        }

        channelsLabel->setColour(Label::textColourId, Colours::white);
        channelsLabel->setParameter((SelectedChannelsParameter*)spikeChannels[rowNumber]->getParameter("local_channels"));
        channelsLabel->setRowAndColumn(rowNumber, columnId);
        
        return channelsLabel;
    }
    else if (columnId == SpikeDetectorTableModel::Columns::WAVEFORM)
    {
        auto* waveformButton = static_cast<WaveformSelectorCustomComponent*> (existingComponentToUpdate);

        if (waveformButton == nullptr)
        {
            waveformButton = new WaveformSelectorCustomComponent(
                (CategoricalParameter*) spikeChannels[rowNumber]->getParameter("waveform_type"),
                acquisitionIsActive);
        }

        waveformButton->setParameter((CategoricalParameter*)spikeChannels[rowNumber]->getParameter("waveform_type"));
        waveformButton->setRowAndColumn(rowNumber, columnId);
        waveformButton->setTableModel(this);
        
        return waveformButton;

    } else if (columnId == SpikeDetectorTableModel::Columns::THRESHOLD)
    {
        auto* thresholdSelector = static_cast<ThresholdSelectorCustomComponent*> (existingComponentToUpdate);

        if (thresholdSelector == nullptr)
        {
            thresholdSelector = new ThresholdSelectorCustomComponent(
                (FloatParameter*) spikeChannels[rowNumber]->getParameter("threshold"),
                acquisitionIsActive);
        }

        thresholdSelector->setParameter((FloatParameter*) spikeChannels[rowNumber]->getParameter("threshold"));
        thresholdSelector->setRowAndColumn(rowNumber, columnId);
        thresholdSelector->setTableModel(this);
        
        return thresholdSelector;
    }

    jassert(existingComponentToUpdate == nullptr);

    return nullptr;
}

int SpikeDetectorTableModel::getNumRows()
{
    return spikeChannels.size();
}

void SpikeDetectorTableModel::update(Array<SpikeChannel*> spikeChannels_)
{
    spikeChannels = spikeChannels_;

    waveformComponents.clear();
    thresholdComponents.clear();
    
    table->updateContent();
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

    if (spikeChannels[rowNumber]->isValid())
    {
        if (rowNumber % 2 == 0)
            g.fillAll(Colour(50, 50, 50));
        else
            g.fillAll(Colour(30, 30, 30));
        
        return;
    }

    if (rowNumber % 2 == 0)
        g.fillAll(Colour(90, 50, 50));
    else
        g.fillAll(Colour(60, 30, 30));
        
}

void SpikeDetectorTableModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (columnId == SpikeDetectorTableModel::Columns::INDEX)
    {
        g.setColour(Colours::white);
        g.drawText(String(rowNumber + 1), 4, 0, width, height, Justification::left);
    }
    else if (columnId == SpikeDetectorTableModel::Columns::TYPE)
    {
        switch (spikeChannels[rowNumber]->getChannelType())
        {
        case SpikeChannel::Type::SINGLE:
            g.setColour(Colours::blue);
            g.fillRoundedRectangle(6, 6, width - 12, height - 12, 4);
            g.setColour(Colours::white);
            g.drawText("SE", 4, 4, width-8, height-8, Justification::centred);
            break;
        case SpikeChannel::Type::STEREOTRODE:
            g.setColour(Colours::purple);
            g.fillRoundedRectangle(6, 6, width - 12, height - 12, 4);
            g.setColour(Colours::white);
            g.drawText("ST", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        case SpikeChannel::Type::TETRODE:
            g.setColour(Colours::green);
            g.fillRoundedRectangle(6, 6, width - 12, height - 12, 4);
            g.setColour(Colours::white);
            g.drawText("TT", 4, 4, width - 8, height - 8, Justification::centred);
            break;
        }
        
    }
    else if (columnId == SpikeDetectorTableModel::Columns::CHANNELS)
    {
        g.setColour(Colours::white);

        String channelString = "[";

        for (int i = 0; i < spikeChannels[rowNumber]->getNumChannels(); i++)
        {
            //channelString += String(spikeChannels[rowNumber]->localChannelIndexes[i]);
            
            //if (i != spikeChannels[rowNumber]->expectedChannelCount - 1)
             //   channelString += ", ";
        }
        
        channelString += "]";
           
        g.drawText(channelString, 4, 4, width - 8, height - 8, Justification::centred);
    } 
    else if (columnId == SpikeDetectorTableModel::Columns::DELETE)
    {
        g.setColour(Colours::red);
        g.fillEllipse(7, 7, width - 14, height - 14);
        g.setColour(Colours::white);
        g.drawLine(9, height / 2, width - 9, height / 2, 3.0);
    }
}


PopupConfigurationWindow::PopupConfigurationWindow(SpikeDetectorEditor* editor_, 
                                                   Array<SpikeChannel*> spikeChannels, 
                                                   bool acquisitionIsActive) 
    : editor(editor_)
{
    //tableHeader.reset(new TableHeaderComponent());

    setSize(40, 40);

    tableModel.reset(new SpikeDetectorTableModel(editor, this, acquisitionIsActive));

    electrodeTable = std::make_unique<TableListBox>("Electrode Table", tableModel.get());
    tableModel->table = electrodeTable.get();
    electrodeTable->setHeader(std::make_unique<TableHeaderComponent>());

    electrodeTable->getHeader().addColumn("#", SpikeDetectorTableModel::Columns::INDEX, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Name", SpikeDetectorTableModel::Columns::NAME, 140, 140, 140, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Type", SpikeDetectorTableModel::Columns::TYPE, 40, 40, 40, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Channels", SpikeDetectorTableModel::Columns::CHANNELS, 100, 100, 100, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Thresholds", SpikeDetectorTableModel::Columns::THRESHOLD, 100, 100, 100, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Waveform", SpikeDetectorTableModel::Columns::WAVEFORM, 50, 50, 50, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn(" ", SpikeDetectorTableModel::Columns::DELETE, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);

    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::backgroundColourId, Colour(240, 240, 240));
    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::highlightColourId, Colour(50, 240, 240));
    electrodeTable->getHeader().setColour(TableHeaderComponent::ColourIds::textColourId, Colour(40, 40, 40));

    electrodeTable->setHeaderHeight(30);
    electrodeTable->setRowHeight(30);
    electrodeTable->setMultipleSelectionEnabled(true);

    addChildComponent(electrodeTable.get());



    update(spikeChannels);
}

PopupConfigurationWindow::~PopupConfigurationWindow()
{
    //countMenu.release();
}


void PopupConfigurationWindow::update(Array<SpikeChannel*> spikeChannels)
{

    std::cout << "Updating configuration window" << std::endl;
    
    std::cout << spikeChannels.size() << " spike channels found." << std::endl;
    
    if (spikeChannels.size() > 0)
    {
        electrodeTable->setVisible(true);

        int maxRows = 16;

        int numRows = spikeChannels.size() <= maxRows ? spikeChannels.size() : maxRows;

        tableModel->update(spikeChannels);

        int scrollBarWidth = 0;

        if (spikeChannels.size() > maxRows)
            scrollBarWidth += 20;

        setSize(530 + scrollBarWidth, (numRows + 1) * 30 + 10);
        electrodeTable->setBounds(5, 5, 520 + scrollBarWidth, (numRows + 1) * 30);
        electrodeTable->resized();

    }
    else {
        electrodeTable->setVisible(false);
    }
    
}
