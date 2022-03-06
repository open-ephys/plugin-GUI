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

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

PopupThresholdComponent::PopupThresholdComponent(SpikeDetectorTableModel* table_,
                                                 ThresholdSelectorCustomComponent* owner_,
                                                 int row_,
                                                 int numChannels,
                                                 ThresholderType type,
                                                 Array<FloatParameter*> abs_thresholds_,
                                                 Array<FloatParameter*> std_thresholds_,
                                                 Array<FloatParameter*> dyn_thresholds_,
                                                 bool lockThresholds) :
    table(table_),
    owner(owner_),
    row(row_),
    thresholdType(type),
    abs_thresholds(abs_thresholds_),
    dyn_thresholds(dyn_thresholds_),
    std_thresholds(std_thresholds_)

{
    
    const int sliderWidth = 18;

    label = std::make_unique<Label>("Label", "Type:");
    label->setBounds(5,5,55,15);
    label->setEditable(false);
    label->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(label.get());
    
    absButton = std::make_unique<UtilityButton>("uV", Font("Fira Code", "Regular", 10));
    absButton->setBounds(7,25,40,25);
    absButton->setTooltip("Detection threshold = microvolt value");
    absButton->setToggleState(type == ThresholderType::ABS, dontSendNotification);
    absButton->addListener(this);
    addAndMakeVisible(absButton.get());
    
    stdButton = std::make_unique<UtilityButton>("STD", Font("Fira Code", "Regular", 10));
    stdButton->setBounds(7,55,40,25);
    stdButton->setTooltip("Detection threshold = multiple of the channel's standard deviation");
    stdButton->setToggleState(type == ThresholderType::STD, dontSendNotification);
    stdButton->addListener(this);
    addAndMakeVisible(stdButton.get());
    
    dynButton = std::make_unique<UtilityButton>("MED", Font("Fira Code", "Regular", 10));
    dynButton->setBounds(7,85,40,25);
    dynButton->setTooltip("Detection threshold = multiple of the median of the channel's absolute value");
    dynButton->setToggleState(type == ThresholderType::DYN, dontSendNotification);
    dynButton->addListener(this);
    addAndMakeVisible(dynButton.get());
    
    createSliders();
    
    lockButton = std::make_unique<UtilityButton>("LOCK", Font("Fira Code", "Regular", 10));
    lockButton->setBounds(68+sliderWidth*numChannels,50,42,20);
    lockButton->setClickingTogglesState(true);
    
    if (numChannels > 1)
    {
        lockButton->setToggleState(lockThresholds, dontSendNotification);
        addAndMakeVisible(lockButton.get());
        
        setSize(lockButton->getRight() + 5,117);
    } else {
        
        lockButton->setToggleState(false, dontSendNotification);
        setSize(95,117);
    }
    
}

void PopupThresholdComponent::createSliders()
{
    
    const int sliderWidth = 18;
    
    sliders.clear();
    
    for (int i = 0; i < abs_thresholds.size(); i++)
    {
        
        Slider* slider = new Slider("SLIDER" + String(i+1));
        slider->setSliderStyle(Slider::LinearBarVertical);
        slider->setTextBoxStyle(Slider::NoTextBox, false, sliderWidth, 10);
        slider->setColour(Slider::textBoxTextColourId, Colours::white);
        slider->setColour(Slider::backgroundColourId, Colours::darkgrey);
        slider->setColour(Slider::trackColourId, Colours::blue);
        
        switch (thresholdType)
        {
            case ABS:
                slider->setRange(25, 200, 1);
                slider->setValue(-abs_thresholds[i]->getFloatValue(), dontSendNotification);
                break;
                
            case STD:
                slider->setRange(2, 10, 0.1);
                slider->setValue(std_thresholds[i]->getFloatValue(), dontSendNotification);
                break;
                
            case DYN:
                slider->setRange(2, 10, 0.1);
                slider->setValue(dyn_thresholds[i]->getFloatValue(), dontSendNotification);
                break;
        }
        slider->addListener(this);
        slider->setSize(sliderWidth, 100);
        
        if (thresholdType == ABS)
        {
            slider->setCentrePosition(0, 0);
            slider->setTransform(AffineTransform::rotation(M_PI)
                                    .translated(60 + (sliderWidth)*i + sliderWidth/2 - i, 60));
        } else {
            slider->setTopLeftPosition(60 + sliderWidth*i - i, 10);
        }
        
        sliders.add(slider);
        addAndMakeVisible(slider);
    }
}

PopupThresholdComponent::~PopupThresholdComponent()
{
    
}

void PopupThresholdComponent::sliderValueChanged(Slider* slider)
{
    if (lockButton->getToggleState())
    {
        for (auto sl : sliders)
        {
            sl->setValue(slider->getValue(), dontSendNotification);
        }
    }
    
    table->broadcastThresholdToSelectedRows(row,                 // original row
                                            thresholdType,       // threshold type
                                            sliders.indexOf(slider), // channel index
                                            lockButton->getToggleState(), // isLocked
                                            slider->getValue());         // value
    
    owner->repaint();
}

void PopupThresholdComponent::buttonClicked(Button* button)
{
    absButton->setToggleState(button == absButton.get(), dontSendNotification);
    stdButton->setToggleState(button == stdButton.get(), dontSendNotification);
    dynButton->setToggleState(button == dynButton.get(), dontSendNotification);
    
    if (button == absButton.get())
    {
        thresholdType = ABS;
    } else if (button == stdButton.get())
    {
        thresholdType = STD;
    } else if (button == dynButton.get())
    {
        thresholdType = DYN;
    }
    
    table->broadcastThresholdTypeToSelectedRows(row,                 // original row
                                                thresholdType);       // threshold type

    createSliders();
    
    owner->repaint();
}

ThresholdSelectorCustomComponent::ThresholdSelectorCustomComponent(SpikeChannel* channel_, bool acquisitionIsActive_)
    : channel(channel_),
      numChannels(channel_->getNumChannels()),
      acquisitionIsActive(acquisitionIsActive_)
{
    thresholder_type = (CategoricalParameter*) channel->getParameter("thrshlder_type");
    
    for (int ch = 0; ch < channel->getNumChannels(); ch++)
    {
        abs_thresholds.add((FloatParameter*) channel->getParameter("abs_threshold" + String(ch+1)));
        std_thresholds.add((FloatParameter*) channel->getParameter("std_threshold" + String(ch+1)));
        dyn_thresholds.add((FloatParameter*) channel->getParameter("dyn_threshold" + String(ch+1)));
    }
}

ThresholdSelectorCustomComponent::~ThresholdSelectorCustomComponent()
{
    
}

void ThresholdSelectorCustomComponent::mouseDown(const MouseEvent& event)
{

    auto* popupComponent = new PopupThresholdComponent(table,
                                                       this,
                                                       row,
                                                       numChannels,
                                                       ThresholderType(thresholder_type->getSelectedIndex()),
                                                       abs_thresholds,
                                                       std_thresholds,
                                                       dyn_thresholds,
                                                       true );

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(popupComponent),
            getScreenBounds(),
            nullptr);
    
    myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
    
}

void ThresholdSelectorCustomComponent::setSpikeChannel(SpikeChannel* ch)
{
    channel = ch;
}

void ThresholdSelectorCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    row = newRow;
    columnId = newColumn;
}

void ThresholdSelectorCustomComponent::paint(Graphics& g)
{
    
    if (channel == nullptr)
        return;
    
    thresholdString = "";
    
    switch (thresholder_type->getSelectedIndex())
    {
        case 0:
            thresholdString += "µV: ";
            break;
        case 1:
            thresholdString += "STD: ";
            break;
        case 2:
            thresholdString += "MED: ";
            break;
    }
    
    for (int i = 0; i < numChannels; i++)
    {
        switch (thresholder_type->getSelectedIndex())
        {
            case 0:
                thresholdString += String(abs_thresholds[i]->getFloatValue(),0);
                break;
            case 1:
                thresholdString += String(std_thresholds[i]->getFloatValue(),1);
                break;
            case 2:
                thresholdString += String(dyn_thresholds[i]->getFloatValue(),1);
                break;
        }
        
        thresholdString += ",";
    }
    
    thresholdString = thresholdString.substring(0, thresholdString.length()-1);
    
    g.setColour(Colours::white);
    g.setFont(12);
    g.drawText(thresholdString, 0, 0, getWidth(), getHeight(), Justification::centredLeft);
}


void ThresholdSelectorCustomComponent::setThreshold(ThresholderType type, int channelNum, float value)
{
    switch (type)
    {
        case ABS:
            abs_thresholds[channelNum]->setNextValue(value);
            break;
        case STD:
            std_thresholds[channelNum]->setNextValue(value);
            break;
        case DYN:
            dyn_thresholds[channelNum]->setNextValue(value);
            break;
    }

    repaint();
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

    /*if (waveformtype->getValueAsString().equalsIgnoreCase("FULL"))
    {
        table->broadcastWaveformTypeToSelectedRows(row, 1);
    }
    else
    {
        table->broadcastWaveformTypeToSelectedRows(row, 0);
    }
    
    repaint();*/
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
    //std::cout << rowNumber << " " << columnId << " : selected " << std::endl;
    
    SparseSet<int> selectedRows = table->getSelectedRows();

    if (columnId == SpikeDetectorTableModel::Columns::DELETE && !acquisitionIsActive)
    {
        //std::cout << "Delete " << selectedRows.size() << " electrodes?" << std::endl;
        
        Array<SpikeChannel*> channelsToDelete;
        Array<SpikeChannel*> channelsToKeep;
        
        for (int i = 0; i < spikeChannels.size(); i++)
        {
            if (selectedRows.contains(i))
                channelsToDelete.add(spikeChannels[i]);
            else
                channelsToKeep.add(spikeChannels[i]);
        }
        
        update(channelsToKeep);
        
        owner->update(channelsToKeep);
        
        editor->removeSpikeChannels(channelsToDelete);
        
    }

    //if (event.mods.isRightButtonDown())
    //    std::cout << "Right click!" << std::endl;
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


void SpikeDetectorTableModel::broadcastThresholdTypeToSelectedRows(int rowThatWasClicked, ThresholderType type)
{
    SparseSet<int> selectedRows = table->getSelectedRows();

    for (int i = 0; i < spikeChannels.size(); i++)
    {
       if (selectedRows.contains(i) || i == rowThatWasClicked)
       {
           switch (type)
           {
               case ABS:
                   spikeChannels[i]->getParameter("thrshlder_type")->setNextValue(0);
                   break;
               case STD:
                   spikeChannels[i]->getParameter("thrshlder_type")->setNextValue(1);
                   break;
               case DYN:
                   spikeChannels[i]->getParameter("thrshlder_type")->setNextValue(2);
                   break;
           }
           
           Component* c = table->getCellComponent(SpikeDetectorTableModel::Columns::THRESHOLD, i);

           if (c != nullptr)
               c->repaint();
           
       }
    }

    table->updateContent();
}

void SpikeDetectorTableModel::broadcastThresholdToSelectedRows(int rowThatWasClicked,
    ThresholderType type,
    int channelIndex,
    bool isLocked,
    float value)
{
    SparseSet<int> selectedRows = table->getSelectedRows();
    
    //std::cout << "Broadcasting value." << std::endl;
    
    float actualValue;

    for (int i = 0; i < spikeChannels.size(); i++)
    {
        if (selectedRows.contains(i) || i == rowThatWasClicked)
        {
            
            //std::cout << "Row = " << i << std::endl;
            
            String parameterString;

            switch (type)
            {
                case ABS:
                    parameterString = "abs_threshold";
                    actualValue = -value;
                    break;
                case STD:
                    parameterString = "std_threshold";
                    actualValue = value;
                    break;
                case DYN:
                    parameterString = "dyn_threshold";
                    actualValue = value;
                    break;
            }
            
            //std::cout << "Type = " << parameterString << std::endl;
            
            if (isLocked)
            {
                //std::cout << "Not locked." << std::endl;
                
                for (int ch = 0; ch < spikeChannels[i]->getNumChannels(); ch++)
                {
                    //std::cout << "Setting value for channel " << ch << ": " << actualValue << std::endl;
                    spikeChannels[i]->getParameter(parameterString + String(ch+1))->setNextValue(actualValue);
                }
            } else {
                
                //std::cout << "Setting value for channel " << channelIndex << ": " << actualValue << std::endl;
                
                if (spikeChannels[i]->getNumChannels() > channelIndex)
                    spikeChannels[i]->getParameter(parameterString + String(channelIndex+1))->setNextValue(actualValue);
            }
            
            Component* c = table->getCellComponent(SpikeDetectorTableModel::Columns::THRESHOLD, i);

            if (c != nullptr)
                c->repaint();

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
                spikeChannels[rowNumber],
                acquisitionIsActive);
        }

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
    
    for (int i = 0; i < getNumRows(); i++)
    {
           
       Component* c = table->getCellComponent(SpikeDetectorTableModel::Columns::THRESHOLD, i);

       if (c == nullptr)
           continue;

       ThresholdSelectorCustomComponent* th = (ThresholdSelectorCustomComponent*) c;

        //std::cout << "Checking thresholder component for row " << i << std::endl;
       
       if (!spikeChannels.contains(th->channel))
       {
           //std::cout << "No longer needed, deleting spikeChannel" << std::endl;
           th->setSpikeChannel(nullptr);
       } else {
           //std::cout << "Still needed" << std::endl;
       }
        
        th->repaint();
           
    }

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

    if (rowNumber >= spikeChannels.size())
        return;

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
        if (rowNumber >= spikeChannels.size()) return;
        switch (spikeChannels[rowNumber]->getChannelType())
        {
        case SpikeChannel::Type::SINGLE:
            g.setColour(Colours::blue);
            g.fillRoundedRectangle(6, 6, width - 12, height - 12, 4);
            g.setColour(Colours::white);
            g.drawText("SE", 4, 4, width - 8, height - 8, Justification::centred);
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
    electrodeTable->getHeader().addColumn("Thresholds", SpikeDetectorTableModel::Columns::THRESHOLD, 120, 120, 120, TableHeaderComponent::notResizableOrSortable);
    electrodeTable->getHeader().addColumn("Waveform", SpikeDetectorTableModel::Columns::WAVEFORM, 60, 60, 60, TableHeaderComponent::notResizableOrSortable);
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


void PopupConfigurationWindow::update(Array<SpikeChannel*> spikeChannels)
{

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
