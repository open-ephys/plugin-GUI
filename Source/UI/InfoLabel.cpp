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

#include "InfoLabel.h"

InfoLabelTabButton::InfoLabelTabButton(const String& name) : Button(name)
{
    setClickingTogglesState(true);
    setRadioGroupId(87, dontSendNotification);
}

void InfoLabelTabButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    
    g.setColour(Colours::lightgrey);

    g.setFont(20);
    g.drawText(getName(), 0, 0, getWidth(), getHeight(), Justification::centred);

    if (isMouseOver)
        g.setColour(Colours::yellow);
    
    if (getToggleState())
        g.setColour(Colours::orange);

    g.fillRect(0, getHeight() - 3, getWidth(), 3);

}

InfoLabel::InfoLabel()
{

    StringArray names = { "About", "Authors", "License" };

    for (int i = 0; i < 3; i++)
    {
        tabButtons.add(new InfoLabelTabButton(names[i]));
        
        tabButtons.getLast()->addListener(this);
        addAndMakeVisible(tabButtons.getLast());
    }

    textEditor.reset(new TextEditor());
    textEditor->setScrollbarsShown(true);
    textEditor->setMultiLine(true, true);
    textEditor->setReadOnly(true);
    textEditor->setPopupMenuEnabled(false);
    textEditor->setLineSpacing(1.1f);
    textEditor->setIndents(5, 20);
    textEditor->setColour(TextEditor::ColourIds::outlineColourId, Colour(0).withAlpha(0.0f));
    textEditor->setColour(TextEditor::ColourIds::focusedOutlineColourId, Colour(0).withAlpha(0.0f));
    textEditor->setColour(TextEditor::ColourIds::backgroundColourId, Colour(0).withAlpha(0.0f));
    textEditor->setColour(TextEditor::ColourIds::shadowColourId, Colour(0).withAlpha(0.0f));
    textEditor->setColour(TextEditor::ColourIds::textColourId, Colours::antiquewhite);
    textEditor->setColour(TextEditor::ColourIds::highlightColourId, Colours::white.withAlpha(0.0f));
    addAndMakeVisible(textEditor.get());

    tabButtons.getFirst()->setToggleState(true, sendNotification);

    addMouseListener(this, true);

}

InfoLabel::~InfoLabel()
{

}

void InfoLabel::buttonClicked(Button* button)

{
    if (button->getName().equalsIgnoreCase("about"))
    {
        setAboutText();
    }
    else if (button->getName().equalsIgnoreCase("authors"))
    {
        setAuthorsText();
    }
    else if (button->getName().equalsIgnoreCase("license"))
    {
        setLicenseText();
    }
}

void InfoLabel::resized()
{
    textEditor->setBounds(20, 85, getWidth() -40, getHeight() - 105);

    for (int i = 0; i < tabButtons.size(); i++)
         tabButtons[i]->setBounds(20 + 130 * i, 20, 90, 40);
}


void InfoLabel::paint(Graphics& g)
{
    g.fillAll(Colour(20,20,20));

    //g.setFont(16.0f); // labelFont);

    //g.setColour(Colours::lightgrey);

    //g.drawFittedText(infoString, 20, 20, getWidth()-20, getHeight()-20, Justification::left, 100);

}

void InfoLabel::mouseMove(const MouseEvent& originalEvent)
{

    const MouseEvent& event = originalEvent.getEventRelativeTo(textEditor.get());

    int i = textEditor->getTextIndexAt(event.x, event.y);
    //std::cout << i << std::endl;

    if (textEditor->getMouseCursor() != MouseCursor::NormalCursor)
    {
        textEditor->setMouseCursor(MouseCursor::NormalCursor);
        //hoverLink = false;
        //repaint();
    }

    for (auto link : hyperlinks)
    {
        if (link.position.contains(i))
        {
            textEditor->setMouseCursor(MouseCursor::PointingHandCursor);
            //if (!l.url.startsWithIgnoreCase("http") || !showAnchorPopup) break;

            //hoverLink = true;
           // hoverLinkText = l.url;
           // toolTipWidth = toolTipFont.getStringWidth(hoverLinkText) + 10;
           // hoverPosition = event.getPosition() + juce::Point<int>(15, 15);
            //repaint();
            break;
        }
    }

}

void InfoLabel::mouseUp(const MouseEvent& originalEvent)
{
    const MouseEvent& event = originalEvent.getEventRelativeTo(textEditor.get());

    int i = textEditor->getTextIndexAt(event.x, event.y);


    for (auto link : hyperlinks)
    {

        if (link.position.contains(i))
        {
            URL url(link.url);
            url.launchInDefaultBrowser();
            break;
        }
    }
}

void InfoLabel::setAboutText()
{

    String infoString = "Open Ephys is an employee-owned cooperative with members "
        "distributed around the world. Its "
        "mission is to advance our understanding of the brain by promoting "
        "community ownership of the tools we use to study it. \n\n"

        "The Open Ephys GUI is free, collaboratively "
        "developed, open - source software for scientific research. It includes " 
        "many features designed to make extracellular electrophysiology data "
        "easier to acquire; however, it is not guaranteed to work as "
        "advertised. Before you use it for your own experiments, you should "
        "test any capabilities you plan to use.The use of a plugin - based "
        "architecture provides the flexibility to customize your signal "
        "chain, but it also makes it difficult to test every possible "
        "combination of processors in advance.Whenever you download or "
        "upgrade the GUI, be sure to test your desired configuration in a "
        "safe environment before using it to collect real data.\n\n"
        ;

    textEditor->clear();
    textEditor->setFont(20.0f);
    textEditor->insertTextAtCaret("Welcome to the Open Ephys GUI!\n\n");
    textEditor->setFont(15.0f);
    textEditor->insertTextAtCaret(infoString);
    textEditor->setCaretPosition(0);
    textEditor->scrollEditorToPositionCaret(0, 0);

    hyperlinks.clear();

    hyperlink.url = "https://open-ephys.org";
    hyperlink.position.setStart(1);
    hyperlink.position.setEnd(6);

    hyperlinks.add(hyperlink);
}

void InfoLabel::setAuthorsText()
{
    textEditor->clear();

    textEditor->setFont(18.0f);
    textEditor->insertTextAtCaret("Core development team\n\n");
    textEditor->setFont(15.0f);

    String coreDevString = "Josh Siegle\n"
        "Aaron Cuevas Lopez\n"
        "Jakob Voigts\n"
        "Pavel Kulik\n"
        "Anjal Doshi\n\n";

    textEditor->insertTextAtCaret(coreDevString);

    textEditor->setFont(18.0f);
    textEditor->insertTextAtCaret("Other contributors\n\n");
    textEditor->setFont(15.0f);

    String otherDevString = "Kirill Abramov\n"
        "Ben Acland\n"
        "Ananya Bahadur\n"
        "Clayton Barnes\n"
        "Francesco Battaglia\n"
        "Edgar Bermudez - Contreras\n"
        "Max Bernstein\n"
        "Ethan Blackwood\n"
        "Kevin Michael Boergens\n"
        "Melodie Borel\n"
        "Michael Borisov\n"
        "Alessio Buccino\n"
        "Ariel Burman\n"
        "Godwin Charan DSouza\n"
        "Priyanjit Dey\n"
        "Svenn-Arne Dragly\n"
        "Ronnie Eichler\n"
        "Florian Franzen\n"
        "K.Michael Fox\n"
        "@jialinzou\n"
        "@jonaskn\n"
        "@koreign\n"
        "Nikolas Karalis\n"
        "Caleb Kemere\n"
        "Stuart Layton\n"
        "Galen Lynch\n"
        "@metatari\n"
        "Arne Meyer\n"
        "@msvdgoes\n"
        "Jon Newman\n"
        "Shay Ohayon\n"
        "Chris Rodgers\n"
        "Mark Schatza\n"
        "Martin Spacek\n"
        "Christopher Stawarz\n"
        "Daniel Wagenaar\n"
        "@whitepine\n\n";

    textEditor->insertTextAtCaret(otherDevString);
    textEditor->setCaretPosition(0);
    textEditor->scrollEditorToPositionCaret(0, 0);

    hyperlinks.clear();

}

void InfoLabel::setLicenseText()
{
    textEditor->clear();
    textEditor->setFont(18.0f);
    textEditor->insertTextAtCaret("GNU GENERAL PUBLIC LICENSE\n\n");
    textEditor->setFont(15.0f);
    textEditor->insertTextAtCaret("Version 3, 29 June 2007\n\n");
    textEditor->insertTextAtCaret("Copyright 2007 Free Software Foundation, Inc. "
        "<https://fsf.org/>\n\n");
    textEditor->insertTextAtCaret(
        "Everyone is permitted to copy and distribute verbatim copies of this "
        "license document, but changing it is not allowed.\n\n");

    textEditor->setFont(18.0f);
    textEditor->insertTextAtCaret("Preamble\n\n");
    textEditor->setFont(15.0f);
    textEditor->insertTextAtCaret(
        "The GNU General Public License is a free, copyleft license for "
        "software and other kinds of works.");

    textEditor->setCaretPosition(0);
    textEditor->scrollEditorToPositionCaret(0, 0);

    hyperlinks.clear();

}