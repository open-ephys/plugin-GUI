/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "FileReader.h"
#include "FileReaderEditor.h"
#include "ScrubberInterface.h"

#include <stdio.h>

ScrubDrawerButton::ScrubDrawerButton(const String &name) : DrawerButton(name) {}

ScrubDrawerButton::~ScrubDrawerButton() {}

void ScrubDrawerButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
	g.setColour(Colour(110, 110, 110));
	if (isMouseOver)
		g.setColour(Colour(210, 210, 210));

	g.drawVerticalLine(3, 0.0f, getHeight());
	g.drawVerticalLine(5, 0.0f, getHeight());
	g.drawVerticalLine(7, 0.0f, getHeight());
}

FileReaderEditor::FileReaderEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode)
    , fileReader   (static_cast<FileReader*> (parentNode))
    , recTotalTime              (0)
    , m_isFileDragAndDropActive (false)
    , scrubInterfaceVisible (false)
    , scrubInterfaceAvailable(false)
{
    desiredWidth = 280;

    scrubberInterface = new ScrubberInterface(fileReader);
    scrubberInterface->setBounds(0, 0, 420, 140);
    addChildComponent(scrubberInterface);

    scrubDrawerButton = new ScrubDrawerButton(getNameAndId() + " Scrub Drawer Button");
	scrubDrawerButton->setBounds(4, 40, 10, 78);
    scrubDrawerButton->setToggleState(false, dontSendNotification);
	scrubDrawerButton->addListener(this);
    scrubDrawerButton->setEnabled(false);
	addAndMakeVisible(scrubDrawerButton);

    addPathParameterEditor (Parameter::PROCESSOR_SCOPE, "selected_file", 24, 29);
    addSelectedStreamParameterEditor (Parameter::PROCESSOR_SCOPE, "active_stream", 24, 54);
    addTimeParameterEditor (Parameter::PROCESSOR_SCOPE, "start_time", 24, 79);
    addTimeParameterEditor (Parameter::PROCESSOR_SCOPE, "end_time", 24, 104);

    for (auto& p : {"selected_file", "active_stream", "start_time", "end_time"})
    {
        auto* ed = getParameterEditor(p);
        ed->setBounds(ed->getX(), ed->getY(), desiredWidth, ed->getHeight());
    }

    lastFilePath = CoreServices::getDefaultUserSaveDirectory();

}

FileReaderEditor::~FileReaderEditor()
{
}

void FileReaderEditor::buttonClicked (Button* button)
{
    if (button == scrubDrawerButton)
        showScrubInterface(!scrubInterfaceVisible);
}

void FileReaderEditor::collapsedStateChanged()
{
    LOGD("Scrub interface visible: ", scrubInterfaceVisible);
    LOGD("Scrub interface available: ", scrubInterfaceAvailable);

    if (!getCollapsedState())
    {
        scrubberInterface->setVisible(scrubInterfaceVisible);
        scrubDrawerButton->setEnabled(scrubInterfaceAvailable);
    }
}

void FileReaderEditor::paintOverChildren (Graphics& g)
{
    /* Draws a frame if a file is currently being dragged over the editor */
    if (m_isFileDragAndDropActive)
    {
        g.setColour (Colours::aqua);
        g.drawRect (getLocalBounds(), 2.f);
    }

    if (scrubInterfaceVisible)
        scrubberInterface->paintOverChildren(g);

}

void FileReaderEditor::enableScrubDrawer(bool enable)
{
    scrubInterfaceAvailable = enable;

    scrubDrawerButton->setEnabled(scrubInterfaceAvailable);
    if (!scrubInterfaceAvailable) showScrubInterface(false);
}

ScrubberInterface* FileReaderEditor::getScrubberInterface()
{
    return scrubberInterface;
}

void FileReaderEditor::showScrubInterface(bool show)
{

    scrubberInterface->setVisible(show);

    int dX = scrubberInterface->getWidth();

    if (scrubInterfaceVisible && !show)
        dX = -dX;
    else if (!scrubInterfaceVisible && !show)
       return;

    desiredWidth += dX;

    /* Move all editor components to the right */
    scrubDrawerButton->setBounds(
        scrubDrawerButton->getX() + dX, scrubDrawerButton->getY(),
        scrubDrawerButton->getWidth(), scrubDrawerButton->getHeight()
    );

    for (auto& p : {"selected_file", "active_stream", "start_time", "end_time"})
    {
        auto* ed = getParameterEditor(p);
        ed->setBounds(ed->getX() + dX, ed->getY(), ed->getWidth(), ed->getHeight());
    }

    CoreServices::highlightEditor(this);
    deselect();

    scrubInterfaceVisible = show;

}

void FileReaderEditor::saveCustomParametersToXml (XmlElement* xml)
{
}

void FileReaderEditor::loadCustomParametersFromXml (XmlElement* xml)
{
}

bool FileReaderEditor::isInterestedInFileDrag (const StringArray& files)
{
    if (! acquisitionIsActive)
    {
        const bool isExtensionSupported = fileReader->isFileSupported (files[0]);
        m_isFileDragAndDropActive = true;

        return isExtensionSupported;
    }

    return false;
}

void FileReaderEditor::fileDragExit (const StringArray& files)
{
    m_isFileDragAndDropActive = false;

    repaint();
}

void FileReaderEditor::fileDragEnter (const StringArray& files, int x, int y)
{
    m_isFileDragAndDropActive = true;

    repaint();
}

void FileReaderEditor::filesDropped (const StringArray& files, int x, int y)
{
    //TODO: Use parameter 
    //setFile (files[0]);

    m_isFileDragAndDropActive = false;
    repaint();
}