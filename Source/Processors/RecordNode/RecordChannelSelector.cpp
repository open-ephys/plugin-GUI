#include "RecordChannelSelector.h"
#include <string>
#include <vector>

ChannelButton::ChannelButton(int _id, RecordChannelSelector* _parent) : Button(String(_id)), id(_id), parent(_parent) {
    setClickingTogglesState(true);
}

ChannelButton::~ChannelButton() {}

void ChannelButton::mouseDown(const MouseEvent &event)
{
    parent->startDragCoords.setX(event.x + this->getX());
    parent->startDragCoords.setY(event.y + this->getY());
    parent->firstButtonSelectedState = !this->getToggleState();
    parent->mouseDown(event);
}

void ChannelButton::mouseDrag(const MouseEvent &event)
{
    parent->mouseDrag(event);
}

void ChannelButton::mouseUp(const MouseEvent &event)
{
    parent->mouseUp(event);
}

void ChannelButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{

	g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 0.001*getWidth());

    if (isMouseOver)
	{
		if (getToggleState())
			g.setColour(Colour(255, 65, 65));
		else
			g.setColour(Colour(210, 210, 210));
	}
	else 
	{
		if (getToggleState())
			g.setColour(Colour(255, 0, 0));
		else
			g.setColour(Colour(110, 110, 110));
	}
	g.fillRoundedRectangle(1,1,getWidth()-2,getHeight()-2,0.001*getWidth());

    //Draw text string in middle of button
	g.setColour(Colour(255,255,255));
	g.setFont(10);
	g.drawText (String(id), 0,0, getWidth(), getHeight(), Justification::centred); 

}

SelectButton::SelectButton(const String& name) : Button(name) {
	setClickingTogglesState(true);
}

SelectButton::~SelectButton() {}

void SelectButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle (0.0f, 0.0f, getWidth(), getHeight(), 0.001*getWidth());

    if (isMouseOver)
    {
        g.setColour(Colour(220, 220, 220));
    }
    else
    {
        if (getToggleState())
            g.setColour(Colour(200, 200, 200));
        else
            g.setColour(Colour(110, 110, 110));
    }
    g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 0.01*getWidth());
    
	g.setColour(Colour(255,255,255));
	g.setFont(12);
	g.drawText (String(getName()), 0, 0, getWidth(), getHeight(), Justification::centred);
}

RangeEditor::RangeEditor(const String& name, const Font& font) : TextEditor(name)
{
    setFont(font);
}

RangeEditor::~RangeEditor() {}

/**************************
 * RECORD CHANNEL SELECTOR
***************************/

RecordChannelSelector::RecordChannelSelector(std::vector<bool> channelStates, bool editable) 
    : Component(), 
    nChannels(channelStates.size()),
    mouseDragged(false), 
    startDragCoords(0,0),
    shiftKeyDown(false),
    firstButtonSelectedState(false),
    isDragging(false),
    editable(editable)
{

    int width = 368; //can use any multiples of 16 here for dynamic resizing

    int nColumns = 16;
    int nRows = nChannels / nColumns + (int)(!(nChannels % nColumns == 0));
    int buttonSize = width / 16;
    int height = buttonSize * nRows;

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nColumns; j++)
		{
            if (nColumns*i+j < nChannels)
            {
                channelButtons.add(new ChannelButton(nColumns*i+j+1, this));
                channelButtons.getLast()->setBounds(width/nColumns*j, height/nRows*i, buttonSize, buttonSize);
                channelButtons.getLast()->setToggleState(channelStates[nColumns * i + j], NotificationType::dontSendNotification);
                channelButtons.getLast()->addListener(this);
                addChildAndSetID(channelButtons.getLast(), String(nColumns*i+j+1));
            }
			
		}
	}

    if (editable)
    {

        //Add "SELECT ALL" button
        selectButtons.add(new SelectButton("ALL"));
        selectButtons.getLast()->setBounds(0, height, 0.25*width, width / nColumns);
        selectButtons.getLast()->addListener(this);
        addChildAndSetID(selectButtons.getLast(),"ALL");

        //Add "SELECT NONE" button
        selectButtons.add(new SelectButton("NONE"));
        selectButtons.getLast()->setBounds(0.25*width, height, 0.25*width, width / nColumns);
        selectButtons.getLast()->addListener(this);
        addChildAndSetID(selectButtons.getLast(),"ALL");
        
        if (nChannels > 8)
        {

            //Add "SELECT RANGE" button
            selectButtons.add(new SelectButton("RANGE"));
            selectButtons.getLast()->setBounds(0.5*width, height, 0.25*width, width / nColumns);
            selectButtons.getLast()->addListener(this);
            addChildAndSetID(selectButtons.getLast(),"ALL");

            //Add Range Editor
            rangeEditor = new RangeEditor("Range", Font("Small Text", 12, Font::plain));
            rangeEditor->setBounds(0.75*width, height, 0.25*width, width / nColumns);
            rangeEditor->addListener(this);
            addChildAndSetID(rangeEditor,"RANGE_EDITOR");
            
        }

    }
    
    if (nChannels <= 8)
        width /= 2;

    if (editable)
	    setSize (width, height + buttonSize);
    else
        setSize(width, height);
    
	setColour(ColourSelector::backgroundColourId, Colours::transparentBlack);

}

RecordChannelSelector::~RecordChannelSelector() {}

void RecordChannelSelector::mouseMove(const MouseEvent &event)
{

};
void RecordChannelSelector::mouseDown(const MouseEvent &event)
{
    if (editable)
        selectedButtons.clear();
};

void RecordChannelSelector::mouseDrag(const MouseEvent &event)
{

    if (editable)
    {

        mouseDragged = true;

        int w = event.getDistanceFromDragStartX();
        int h = event.getDistanceFromDragStartY();
        int x = startDragCoords.getX();
        int y = startDragCoords.getY();

        if (w < 0)
        {
            x = x + w;
            w = -w;
        }

        if (h < 0)
        {
            y = y + h;
            h = -h;
        }

        dragBox.setBounds(x, y, w > 0 ? w : 1, h > 0 ? h : 1);

        for (auto button : channelButtons)
        {
            if (button->getBounds().intersects(dragBox) && !selectedButtons.contains(button->getId()))
            {
                selectedButtons.add(button->getId());

                if (shiftKeyDown) //toggle
                    button->triggerClick();
                else //Use state of the first selected button
                {
                    button->setToggleState(firstButtonSelectedState, NotificationType::dontSendNotification);
                }
                
            }
        }

    }
        
};

void RecordChannelSelector::modifierKeysChanged(const ModifierKeys &modifiers)
{
    shiftKeyDown = modifiers.isShiftDown();
}

void RecordChannelSelector::mouseUp(const MouseEvent &event)
{

    if (!mouseDragged && editable)
    {
        for (auto button : channelButtons)
        {
            if (button->getBounds().contains(startDragCoords))
            {
                button->triggerClick();
                break;
            }
        }
    }
    mouseDragged = false;
}

void RecordChannelSelector::textEditorReturnKeyPressed(TextEditor& editor)
{

    if (editable)
    {
        channelStates = parseStringIntoRange(384);

        if (channelStates.size() < 3)
            return;

        for (auto* btn : channelButtons)
            btn->setToggleState(false, NotificationType::dontSendNotification);

        int i = 0;
        while (i <= channelStates.size() - 3)
        {
            const int lim = channelStates[i+1];
            const int comd = channelStates[i+2];
            for (int fa = channelStates[i]; fa < lim; fa += comd)
            {
                channelButtons[i++]->setToggleState(true, NotificationType::dontSendNotification);
            }
            i+=3;
        }

        for (auto val : channelStates)
        {
            std::cout << val << ",";
        }
        std::cout << std::endl;

        for (auto* btn : channelButtons)
            btn->setToggleState(false, NotificationType::dontSendNotification);

        for (i = 0; i < channelStates.size(); i += 3)
        {
            int startIdx = channelStates[i];
            int endIdx = channelStates[i+1];
            int step = channelStates[i+2];
            
            int ch = startIdx;
            while (ch < endIdx)
            {
                channelButtons[ch]->setToggleState(true, NotificationType::dontSendNotification);
                ch+=step;
            }

        }
    }

}

void RecordChannelSelector::buttonClicked(Button* button)
{

    if (editable)
    {
    
        for (auto* btn : selectButtons)
            btn->setToggleState(false, NotificationType::dontSendNotification);
        
        if (button->getButtonText() == String("ALL"))
        {
            for (auto* btn : channelButtons)
                btn->setToggleState(true, NotificationType::dontSendNotification);
            button->setToggleState(true, NotificationType::dontSendNotification);
            
        }
        else if (button->getButtonText() == String("NONE"))
        {
            for (auto* btn : channelButtons)
                btn->setToggleState(false, NotificationType::dontSendNotification);
            button->setToggleState(true, NotificationType::dontSendNotification);
        }
        else if (button->getButtonText() == String("RANGE:"))
        {
            button->setToggleState(true, NotificationType::dontSendNotification);
        }
        else //channel button was manually selected
        {
            //TODO: Update text box with range string
        }
        
        //rangeEditor->setText(rangeString);

    }
    
}

void RecordChannelSelector::updateRangeString()
{
    
    rangeString = "";
    
    int startIdx = 0;
    int endIdx = 0;

    bool inRange = false;
    
    for (int i = 0; i < nChannels; i++)
    {
        if (channelButtons[i]->getToggleState())
        {
            if (inRange)
            {
                if (i == nChannels-1)
                {
                    rangeString += (rangeString == "" ? "" : ",");
                    rangeString += String(startIdx) + ":" + String(nChannels);
                }
            }
            else
            {
                if (i == nChannels-1)
                {
                    rangeString += (rangeString == "" ? "" : ",");
                    rangeString += String(nChannels);
                }
                startIdx = i + 1;
            }
            inRange = true;
            endIdx = i + 1;
        }
        else
        {
            if (inRange)
            {
                rangeString += (rangeString == "" ? "" : ",");
                if (startIdx < endIdx)
                    rangeString += String(startIdx) + ":" + String(endIdx);
                else
                    rangeString += String(startIdx);
            }
            else
            {
                
            }
            inRange = false;
        }
        
    }
    
}

int RecordChannelSelector::convertStringToInteger(String s)
{
    char ar[20];
    int i, k = 0;
    for (i = 0; i < s.length(); i++)
    {
        if (s[i] >= 48 && s[i] <= 57)
        {
            ar[k] = s[i];
            k++;
        }
    }
    if (k > 7)
    {
        return 1000000;
    }
    ar[k] = '\0';
    k = atoi(ar);
    return k;
}

Array<int> RecordChannelSelector::parseStringIntoRange(int rangeValue)
{
    String s = ",";
    s += rangeEditor->getText();

    Array<int> finalList, separator, rangeseparator;
    int i, j, a, b, k, openb, closeb, otherchar, x, y;
    s += ",";
    for (i = 0; i < s.length(); i++) //split string by ' , ' or ' ; '
    {
        if (s[i] == ';' || s[i] == ',')
        {
            separator.add(i);
        }
    }
    for (i = 0; i < separator.size() - 1; i++) // split ranges by ' : ' or ' - '
    {
        j = k = separator[i] + 1;
        openb = closeb = otherchar = 0;
        rangeseparator.clear();
        for (; j < separator[i + 1]; j++)
        {
            if (s[j] == '-' || s[j] == ':')
            {
                rangeseparator.add(j);
            }
            else if (((int)s[j] == 32))
            {
                continue;
            }
            else if (s[j] == '[' || s[j] == '{' || s[j] == '(')
            {
                openb++;
            }
            else if (s[j] == ']' || s[j] == '}' || s[j] == ')')
            {
                closeb++;
            }
            else if ((int)s[j] > 57 || (int)s[j] < 48)
            {
                otherchar++;
            }
        }

        if (openb != closeb || openb > 1 || closeb > 1 || otherchar > 0) //Invalid input
        {
            continue;
        }

        for (x = separator[i] + 1; x < separator[i + 1]; x++) //trim whitespace and brackets from front
        {
            if (((int)s[x] >= 48 && (int)s[x] <= 57) || s[x] == ':' || s[x] == '-')
            {
                break;
            }
        }
        for (y = separator[i + 1] - 1; y > separator[i]; y--) //trim whitespace and brackets from end
        {
            if (((int)s[y] >= 48 && (int)s[y] <= 57) || s[y] == ':' || s[y] == '-')
            {
                break;
            }
        }
        if (x > y)
        {
            continue;
        }

        if (rangeseparator.size() == 0) //syntax of form - x or [x]
        {
            a = convertStringToInteger(s.substring(x, y + 1));
            if (a == 0 || a > rangeValue)
            {
                continue;
            }
            finalList.add(a - 1);
            finalList.add(a - 1);
            finalList.add(1);
        }
        else if (rangeseparator.size() == 1) // syntax of type - x-y or [x-y]
        {
            a = convertStringToInteger(s.substring(x, rangeseparator[0]));
            b = convertStringToInteger(s.substring(rangeseparator[0] + 1, y + 1));
            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = rangeValue;
            }
            if (a > b || a > rangeValue || b > rangeValue)
            {
                continue;
            }
            finalList.add(a - 1);
            finalList.add(b - 1);
            finalList.add(1);
        }
        else if (rangeseparator.size() == 2) // syntax of type [x:y:z] or x-y-z
        {
            a = convertStringToInteger(s.substring(x, rangeseparator[0] + 1));
            k = convertStringToInteger(s.substring(rangeseparator[0] + 1, rangeseparator[1]));
            b = convertStringToInteger(s.substring(rangeseparator[1] + 1, y + 1));

            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = rangeValue;
            }
            if (k == 0)
            {
                k = 1;
            }
            if (a > b || a > rangeValue || b > rangeValue)
            {
                continue;
            }
            finalList.add(a - 1);
            finalList.add(b - 1);
            finalList.add(k);
        }
    }
    return finalList;
}
