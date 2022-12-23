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


#include "ProcessorList.h"
#include <stdio.h>

#include "UIComponent.h"
#include "../AccessClass.h"
#include "../Processors/ProcessorManager/ProcessorManager.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"

#include "../Utils/Utils.h"


enum colorIds
{
	PROCESSOR_COLOR = 801,
	FILTER_COLOR = 802,
	SINK_COLOR = 803,
	SOURCE_COLOR = 804,
	UTILITY_COLOR = 805,
	RECORD_COLOR = 806,
	AUDIO_COLOR = 807
};

ProcessorList::ProcessorList(Viewport* v) :
    viewport(v),
    isDragging(false),
    totalHeight(800),
    itemHeight(32),
    subItemHeight(22),
	xBuffer(1),
    yBuffer(1),
    hoverItem(nullptr),
    maximumNameOffset(0)
{

	listFontLight = Font("CP Mono", "Light", 25);
	listFontPlain = Font("CP Mono", "Plain", 20);

	setColour(PROCESSOR_COLOR, Colour(59, 59, 59));
	setColour(FILTER_COLOR, Colour(0, 174, 239));
	setColour(SINK_COLOR, Colour(0, 166, 81));
	setColour(SOURCE_COLOR, Colour(241, 90, 41));
	setColour(UTILITY_COLOR, Colour(147, 149, 152));
	setColour(RECORD_COLOR, Colour(255, 0, 0));
	setColour(AUDIO_COLOR, Colour(0,0,0));

	ProcessorListItem* sources = new ProcessorListItem("Sources");
	ProcessorListItem* filters = new ProcessorListItem("Filters");
	ProcessorListItem* sinks = new ProcessorListItem("Sinks");
	ProcessorListItem* utilities = new ProcessorListItem("Utilities");
	ProcessorListItem* record = new ProcessorListItem("Recording");

	baseItem = std::make_unique<ProcessorListItem>("Processors");
	baseItem->addSubItem(sources);
	baseItem->addSubItem(filters);
	baseItem->addSubItem(sinks);
	baseItem->addSubItem(utilities);
	baseItem->addSubItem(record);

	baseItem->setParentName("Processors");

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{
		const String category = baseItem->getSubItem(n)->getName();
		baseItem->getSubItem(n)->setParentName(category);
		for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
		{
			baseItem->getSubItem(n)->getSubItem(m)->setParentName(category);
		}
	}
}


void ProcessorList::resized()
{
	setBounds(0,0,195,getTotalHeight());
}

void ProcessorList::timerCallback()
{
    maximumNameOffset += 1;
    
    repaint();
}


bool ProcessorList::isOpen()
{
	return baseItem->isOpen();
}

void ProcessorList::paint(Graphics& g)
{
	drawItems(g);
}


void ProcessorList::drawItems(Graphics& g)
{
	totalHeight = yBuffer + itemHeight;

	category = baseItem->getName();

	drawItem(g, baseItem.get());

	if (baseItem->isOpen())
	{
		for (int n = 0; n < baseItem->getNumSubItems(); n++)
		{
			setViewport(g, baseItem->hasSubItems());
			category = baseItem->getSubItem(n)->getName();
			drawItem(g, baseItem->getSubItem(n));

			if (baseItem->getSubItem(n)->isOpen())
			{
				for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
				{

					setViewport(g, baseItem->
							getSubItem(n)->
							getSubItem(m)->
							hasSubItems());
					drawItem(g, baseItem->getSubItem(n)->getSubItem(m));
                    
				}
			}
		}
	}

	if (isOpen())
		setSize(getWidth(),totalHeight);

}

void ProcessorList::drawItem(Graphics& g, ProcessorListItem* item)
{

	Colour c = findColour(item->colorId);

	g.setColour(c);
    
	if (item->hasSubItems())
		g.fillRect(1.0, 0.0, getWidth()-2, itemHeight);
	else
		g.fillRect(1.0, 10.0, getWidth()-2, subItemHeight);

	drawItemName(g,item);
}

void ProcessorList::drawItemName(Graphics& g, ProcessorListItem* item)
{
    
	g.setColour(Colours::white);
	g.setFont(listFontPlain);

	float offsetX, offsetY;

	if (item->getNumSubItems() == 0)
	{
		
		String name = item->getName();
        
        float scrollbarOffset = 0.0f; 
        float maxWidth = getWidth();

		offsetX = 20.0f;
        
        if (item == hoverItem)
        {
            maxWidth = listFontPlain.getStringWidthFloat(name);
            
            if (maxWidth + 25 < getWidth() - scrollbarOffset)
            {
                maximumNameOffset = 0;
                stopTimer();
            } else if (maximumNameOffset + getWidth() > maxWidth + 25 + scrollbarOffset)
            {
                stopTimer();
            }
            
            offsetX -= maximumNameOffset;
        }
        
		offsetY = 0.72f;
        
        g.setFont(listFontPlain);
        
        if (item->isSelected())
        {
            g.drawText(">", offsetX - 15, 5, getWidth()-9, itemHeight, Justification::left, false);
        }
        g.drawText(name, offsetX, 5, maxWidth, itemHeight, Justification::left, false);

	}
	else
	{
		String name = item->getName().toUpperCase();
		offsetX = 5.0f;
		offsetY = 0.75f;
        
        g.setFont(listFontLight);
        g.drawText(name, offsetX, 0, getWidth(), itemHeight, Justification::left, false);
	}


}


void ProcessorList::clearSelectionState()
{
	baseItem->setSelected(false);

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{
		baseItem->getSubItem(n)->setSelected(false);

		for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
		{
			baseItem->getSubItem(n)->getSubItem(m)->setSelected(false);
		}
	}
}

ProcessorListItem* ProcessorList::getListItemForYPos(int y)
{
	int bottom = (yBuffer + itemHeight);

	if (y < bottom)
	{
		return baseItem.get();

	}
	else
	{

		if (baseItem->isOpen())
		{
			for (int n = 0; n < baseItem->getNumSubItems(); n++)
			{
				bottom += (yBuffer + itemHeight);

				if (y < bottom)
				{
					return baseItem->getSubItem(n);
				}

				if (baseItem->getSubItem(n)->isOpen())
				{
					for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
					{
						bottom += (yBuffer + subItemHeight);

						if (y < bottom)
						{
							return baseItem->getSubItem(n)->getSubItem(m);
						}

					}
				}
			}
		}

	}

	return 0;

}

void ProcessorList::setViewport(Graphics& g, bool hasSubItems)
{

	int height;

	if (hasSubItems)
	{
		height = itemHeight;
	}
	else
	{
		height = subItemHeight;
	}

	g.setOrigin(0, yBuffer + height);

	totalHeight += yBuffer + height;

}

int ProcessorList::getTotalHeight()
{
	return totalHeight;
}

void ProcessorList::toggleState()
{

	ProcessorListItem* fli = getListItemForYPos(0);
	fli->reverseOpenState();
	AccessClass::getUIComponent()->childComponentChanged();
	repaint();
}

void ProcessorList::mouseDown(const MouseEvent& e)
{

	isDragging = false;

	juce::Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	int ycoord = pos.getY();

	ProcessorListItem* listItem = getListItemForYPos(ycoord);

	if (listItem != 0)
	{
		LOGA("Processor List Selecting: ", listItem->getName());

		if (!listItem->hasSubItems())
		{
			clearSelectionState();
			listItem->setSelected(true);
		}

	}

	if (listItem != 0)
	{
		if (xcoord < getWidth())
		{
			if (e.mods.isRightButtonDown() || e.mods.isCtrlDown())
			{

				if (listItem->getName().equalsIgnoreCase("Sources"))
				{
					currentColor = SOURCE_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Filters"))
				{
					currentColor = FILTER_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Utilities"))
				{
					currentColor = UTILITY_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Sinks"))
				{
					currentColor = SINK_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Recording"))
				{
					currentColor = RECORD_COLOR;
				}
				else
				{
					return;
				}

				int options=0;
				options += (1 << 1); // showColorAtTop
				options += (1 << 2); // editableColour
				options += (1 << 4); // showColourSpace

				auto* colourSelector = new ColourSelector(options);
				colourSelector->setName("background");
				colourSelector->setCurrentColour(findColour(currentColor));
				colourSelector->addChangeListener(this);
				colourSelector->addChangeListener(AccessClass::getProcessorGraph());
				colourSelector->setColour(ColourSelector::backgroundColourId, Colours::lightgrey);
				colourSelector->setSize(250, 270);

				juce::Rectangle<int> rect = juce::Rectangle<int>(e.getScreenPosition().getX(),
                                                                 e.getScreenPosition().getY(),1,1);

				CallOutBox& myBox
                    = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(colourSelector),
                                                       rect,
                                                       nullptr);

			}
			else
			{
				listItem->reverseOpenState();
			}
		}

		if (listItem == baseItem.get())
		{
			if (listItem->isOpen())
			{
				AccessClass::getUIComponent()->childComponentChanged();
			}
			else
			{
				AccessClass::getUIComponent()->childComponentChanged();
			}

		}
	}

	repaint();
}

void ProcessorList::changeListenerCallback(ChangeBroadcaster* source)
{
	ColourSelector* cs = dynamic_cast <ColourSelector*>(source);

	setColour(currentColor, cs->getCurrentColour());

	repaint();

}

void ProcessorList::mouseMove(const MouseEvent& e)
{

    if (e.getMouseDownX() < getWidth() && !(isDragging))
    {
        ProcessorListItem* listItem = getListItemForYPos(e.getMouseDownY());
        
        if (hoverItem != listItem) // new hover item
        {
            hoverItem = listItem;
            maximumNameOffset = 0;
            startTimer(33);
        }
    }
}


void ProcessorList::mouseExit(const MouseEvent& e)
{

    hoverItem = nullptr;
    maximumNameOffset = 0;
    stopTimer();
    isDragging = false;
    
    repaint();
}

void ProcessorList::mouseDrag(const MouseEvent& e)
{

	if (e.getMouseDownX() < getWidth() && !(isDragging))
	{

		ProcessorListItem* listItem = getListItemForYPos(e.getMouseDownY());

		if (listItem != 0)
		{

			if (!listItem->hasSubItems())
			{
				isDragging = true;

				if (listItem->getName().isNotEmpty())
				{
					DragAndDropContainer* const dragContainer
						= DragAndDropContainer::findParentDragContainerFor(this);

					if (dragContainer != 0)
					{
						Image dragImage(Image::ARGB, 100, 15, true);

						LOGA("Processor List - ", listItem->getName(), " drag start.");

						Graphics g(dragImage);
						g.setColour(findColour(listItem->colorId));
						g.fillAll();
						g.setColour(Colours::white);
						g.setFont(14);
						g.drawSingleLineText(listItem->getName(),10,12);

						dragImage.multiplyAllAlphas(0.6f);

						juce::Point<int> imageOffset(20,10);

						Array<var> dragData;
						dragData.add(true); // fromProcessorList
						dragData.add(listItem->getName()); // pluginName
                        dragData.add(listItem->index);  // processorIndex
						dragData.add(listItem->pluginType); // pluginType
                        dragData.add(listItem->processorType); // processorType

						dragContainer->startDragging(dragData, this,
								dragImage, true, &imageOffset);
					}
				}
			}
		}
	}

}

void ProcessorList::saveStateToXml(XmlElement* xml)
{
	XmlElement* processorListState = xml->createNewChildElement("PROCESSORLIST");

	for (int i = 0; i < 7; i++)
	{
		XmlElement* colorState = processorListState->createNewChildElement("COLOR");

		int id;

		switch (i)
		{
			case 0:
				id = PROCESSOR_COLOR;
				break;
			case 1:
				id = SOURCE_COLOR;
				break;
			case 2:
				id = FILTER_COLOR;
				break;
			case 3:
				id = SINK_COLOR;
				break;
			case 4:
				id = UTILITY_COLOR;
				break;
			case 5: 
				id = RECORD_COLOR;
				break;
			case 6:
				id = AUDIO_COLOR;
				break;
			default:
				// do nothing
				;
		}

		Colour c = findColour(id);

		colorState->setAttribute("ID", (int) id);
		colorState->setAttribute("R", (int) c.getRed());
		colorState->setAttribute("G", (int) c.getGreen());
		colorState->setAttribute("B", (int) c.getBlue());

	}
}

void ProcessorList::loadStateFromXml(XmlElement* xml)
{
	for (auto* xmlNode : xml->getChildIterator())
	{
		if (xmlNode->hasTagName("PROCESSORLIST"))
		{
			for (auto* colorNode : xmlNode->getChildIterator())
			{
				setColour(colorNode->getIntAttribute("ID"),
						Colour(
							colorNode->getIntAttribute("R"),
							colorNode->getIntAttribute("G"),
							colorNode->getIntAttribute("B")));
			}
		}
	}

	repaint();

	AccessClass::getProcessorGraph()->refreshColors();
}

Array<Colour> ProcessorList::getColours()
{
	Array<Colour> c;

	c.add(findColour(PROCESSOR_COLOR));
	c.add(findColour(SOURCE_COLOR));
	c.add(findColour(FILTER_COLOR));
	c.add(findColour(SINK_COLOR));
	c.add(findColour(UTILITY_COLOR));
	c.add(findColour(RECORD_COLOR));
	c.add(findColour(AUDIO_COLOR));
	return c;
}

void ProcessorList::setColours(Array<Colour> c)
{
	for (int i = 0; i < c.size(); i++)
	{
		switch (i)
		{
			case 0:
				setColour(PROCESSOR_COLOR, c[i]);
				break;
			case 1:
				setColour(SOURCE_COLOR, c[i]);
				break;
			case 2:
				setColour(FILTER_COLOR, c[i]);
				break;
			case 3:
				setColour(SINK_COLOR, c[i]);
				break;
			case 4:
				setColour(UTILITY_COLOR, c[i]);
				break;
			case 5: 
				setColour(RECORD_COLOR, c[i]);
				break;
			case 6: 
				setColour(AUDIO_COLOR, c[i]);
			default:
				;// do nothing
		}
	}
}

void ProcessorList::fillItemList()
{

	LOGD("ProcessorList::fillItemList()");

	baseItem->getSubItem(0)->clearSubItems(); //Sources
	baseItem->getSubItem(1)->clearSubItems(); //Filters
	baseItem->getSubItem(2)->clearSubItems(); //Sinks
	baseItem->getSubItem(3)->clearSubItems(); //Utilities
	baseItem->getSubItem(4)->clearSubItems(); //Record
    
	for (auto pluginType : ProcessorManager::getAvailablePluginTypes())
	{
        
		for (int i = 0; i < ProcessorManager::getNumProcessorsForPluginType(pluginType); i++)
		{

            Plugin::Description description = ProcessorManager::getPluginDescription(pluginType, i);

			LOGD("Processor List - creating item for ", description.name);
            
            ProcessorListItem* item = new ProcessorListItem(description.name,
                                            i,
                                            description.type,
                                            description.processorType);
            
            if (description.processorType == Plugin::Processor::SOURCE)
                
                baseItem->getSubItem(0)->addSubItem(item);
            
            else if (description.processorType == Plugin::Processor::FILTER)
                
                baseItem->getSubItem(1)->addSubItem(item);
            
            else if (description.processorType == Plugin::Processor::SINK)
                
                baseItem->getSubItem(2)->addSubItem(item);
            
            else if (description.processorType == Plugin::Processor::UTILITY
                     || description.processorType == Plugin::Processor::MERGER
                     || description.processorType == Plugin::Processor::SPLITTER
                     || description.processorType == Plugin::Processor::AUDIO_MONITOR)
                
                baseItem->getSubItem(3)->addSubItem(item);
            
            else if (description.processorType == Plugin::Processor::RECORD_NODE)
                
                baseItem->getSubItem(4)->addSubItem(item);
		}
	}

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{
		const String category = baseItem->getSubItem(n)->getName();
        
        baseItem->getSubItem(n)->setParentName(category);
		
        for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
		{
			baseItem->getSubItem(n)->getSubItem(m)->setParentName(category);
		}
	}

}


Array<String> ProcessorList::getItemList()
{

	Array<String> listOfProcessors;
    
	for (int i = 0; i < 5; i++)
	{
        int numSubItems = baseItem->getSubItem(i)->getNumSubItems();

		ProcessorListItem* subItem = baseItem->getSubItem(i);

		for(int j = 0; j < numSubItems ; j++)
		{
			listOfProcessors.addIfNotAlreadyThere(subItem->getSubItem(j)->getName());
		}
	}

	return listOfProcessors;
}


Plugin::Description ProcessorList::getItemDescriptionfromList(const String& name)
{

	Plugin::Description description;
    
	for (int i = 0; i < 5; i++)
	{
        int numSubItems = baseItem->getSubItem(i)->getNumSubItems();

		ProcessorListItem* subItem = baseItem->getSubItem(i);

		for(int j = 0; j < numSubItems ; j++)
		{
			if(name.equalsIgnoreCase(subItem->getSubItem(j)->getName()))
			{
				description.fromProcessorList = true;
				description.index = subItem->getSubItem(j)->index;
				description.name = subItem->getSubItem(j)->getName();
				description.type = subItem->getSubItem(j)->pluginType;
				description.processorType = subItem->getSubItem(j)->processorType;

				break;
			}
		}
	}

	return description;
}

// ===================================================================

ProcessorListItem::ProcessorListItem(const String& name_,
                                     int index_,
                                     Plugin::Type pluginType_,
                                     Plugin::Processor::Type processorType_):
  index(index_),
  pluginType(pluginType_),
  processorType(processorType_),
  selected(false),
  open(true),
  name(name_)
{
}


bool ProcessorListItem::hasSubItems()
{
	if (subItems.size() > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int ProcessorListItem::getNumSubItems()
{
	return subItems.size();
}

ProcessorListItem* ProcessorListItem::getSubItem(int index)
{
	return subItems[index];
}

void ProcessorListItem::clearSubItems()
{
	subItems.clear();
}

void ProcessorListItem::addSubItem(ProcessorListItem* newItem)
{
	subItems.add(newItem);
}

void ProcessorListItem::removeSubItem(int index)
{
	subItems.remove(index);
}

bool ProcessorListItem::isOpen()
{
	return open;
}

void ProcessorListItem::setOpen(bool t)
{
	open = t;
}

const String& ProcessorListItem::getName()
{
	return name;
}


const String& ProcessorListItem::getParentName()
{
	return parentName;
}

void ProcessorListItem::setParentName(const String& name)
{
	parentName = name;

	if (parentName.equalsIgnoreCase("Processors"))
	{
		colorId = PROCESSOR_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Filters"))
	{
		colorId = FILTER_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Sinks"))
	{
		colorId = SINK_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Sources"))
	{
		colorId = SOURCE_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Recording"))
	{
		colorId = RECORD_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Audio"))
	{
		colorId = AUDIO_COLOR;
	}
	else
	{
		colorId = UTILITY_COLOR;
	}
}
