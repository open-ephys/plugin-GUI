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

#include "UIComponent.h"
#include <stdio.h>

UIComponent::UIComponent(MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio_)
    : mainWindow(mainWindow_), processorGraph(pgraph), audio(audio_)

{

    processorGraph->setUIComponent(this);

    infoLabel = new InfoLabel();

    dataViewport = new DataViewport();
    addChildComponent(dataViewport);
    dataViewport->addTabToDataViewport("Info", infoLabel,0);

    std::cout << "Created data viewport." << std::endl;

    editorViewport = new EditorViewport();

    addAndMakeVisible(editorViewport);

    std::cout << "Created filter viewport." << std::endl;

    editorViewportButton = new EditorViewportButton(this);
    addAndMakeVisible(editorViewportButton);

    controlPanel = new ControlPanel(processorGraph, audio);
    addAndMakeVisible(controlPanel);

    std::cout << "Created control panel." << std::endl;

    processorList = new ProcessorList();
    addAndMakeVisible(processorList);

    std::cout << "Created filter list." << std::endl;

    messageCenter = new MessageCenter();
    addActionListener(messageCenter);
    addAndMakeVisible(messageCenter);

    std::cout << "Created message center." << std::endl;

    setBounds(0,0,500,400);

    std::cout << "Component width = " << getWidth() << std::endl;
    std::cout << "Component height = " << getHeight() << std::endl;

    std::cout << "UI component data viewport: " << dataViewport << std::endl;

    std::cout << "Finished UI stuff." << std::endl << std::endl << std::endl;

    processorGraph->setUIComponent(this);
    processorGraph->updatePointers(); // needs to happen after processorGraph gets the right pointers
    processorList->setUIComponent(this);
    editorViewport->setUIComponent(this);
    dataViewport->setUIComponent(this);
    controlPanel->getAudioEditor()->setUIComponent(this);
    controlPanel->setUIComponent(this);

    //processorGraph->sendActionMessage("Test.");

    //processorGraph->loadState();

#if JUCE_MAC
    MenuBarModel::setMacMainMenu(this);
    mainWindow->setMenuBar(0);
#else
    mainWindow->setMenuBar(this);
#endif

    //getEditorViewport()->loadState(File("/home/jsiegle/Programming/GUI/Builds/Linux/build/test.xml"));

}

UIComponent::~UIComponent()
{
    dataViewport->destroyTab(0); // get rid of tab for InfoLabel
}

void UIComponent::resized()
{

    int w = getWidth();
    int h = getHeight();

    if (dataViewport != 0)
    {
        int left, top, width, height;
        left = 6;
        top = 40;

        if (processorList->isOpen())
            left = 202;
        else
            left = 6;

        if (controlPanel->isOpen())
            top = 72;
        else
            top = 40;

        if (editorViewportButton->isOpen())
            height = h - top - 195;
        else
            height = h - top - 45;

        width = w - left - 5;

        dataViewport->setBounds(left, top, width, height);
    }

    if (editorViewportButton != 0)
    {
        editorViewportButton->setBounds(w-230, h-40, 225, 35);
    }

    if (editorViewport != 0)
    {
        if (editorViewportButton->isOpen() && !editorViewport->isVisible())
            editorViewport->setVisible(true);
        else if (!editorViewportButton->isOpen() && editorViewport->isVisible())
            editorViewport->setVisible(false);

        editorViewport->setBounds(6,h-190,w-11,150);
    }

    if (controlPanel != 0)
    {
        if (controlPanel->isOpen())
            controlPanel->setBounds(201,6,w-210,64);
        else
            controlPanel->setBounds(201,6,w-210,32);
    }

    if (processorList != 0)
    {
        if (processorList->isOpen())
            if (editorViewportButton->isOpen())
                processorList->setBounds(5,5,195,h-200);
            else
                processorList->setBounds(5,5,195,h-50);
        else
            processorList->setBounds(5,5,195,34);
    }

    if (messageCenter != 0)
        messageCenter->setBounds(6,h-35,w-241,30);

    // for debugging qpurposes:
    if (false)
    {
        dataViewport->setVisible(false);
        editorViewport->setVisible(false);
        processorList->setVisible(false);
        messageCenter->setVisible(false);
        controlPanel->setVisible(false);
        editorViewportButton->setVisible(false);
    }

}

void UIComponent::disableCallbacks()
{
    //sendActionMessage("Data acquisition terminated.");
    controlPanel->disableCallbacks();
}

void UIComponent::disableDataViewport()
{
    dataViewport->disableConnectionToEditorViewport();
}

void UIComponent::childComponentChanged()
{
    resized();
}




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// MENU BAR METHODS

StringArray UIComponent::getMenuBarNames()
{

    // StringArray names;
    // names.add("File");
    // names.add("Edit");
    // names.add("Help");

    const char* const names[] = { "File", "Edit", "View", "Help", 0 };

    return StringArray(names);

}

PopupMenu UIComponent::getMenuForIndex(int menuIndex, const String& menuName)
{
    ApplicationCommandManager* commandManager = &(mainWindow->commandManager);

    PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(commandManager, openConfiguration);
        menu.addCommandItem(commandManager, saveConfiguration);

#if !JUCE_MAC
        menu.addSeparator();
        menu.addCommandItem(commandManager, StandardApplicationCommandIDs::quit);
#endif

    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem(commandManager, undo);
        menu.addCommandItem(commandManager, redo);
        menu.addSeparator();
        menu.addCommandItem(commandManager, copySignalChain);
        menu.addCommandItem(commandManager, pasteSignalChain);
        menu.addSeparator();
        menu.addCommandItem(commandManager, clearSignalChain);

    }
    else if (menuIndex == 2)
    {

        menu.addCommandItem(commandManager, toggleProcessorList);
        menu.addCommandItem(commandManager, toggleSignalChain);
        menu.addCommandItem(commandManager, toggleFileInfo);

    }
    else if (menuIndex == 3)
    {
        menu.addCommandItem(commandManager, showHelp);
    }

    return menu;

}

void UIComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{

    //
}

// ApplicationCommandTarget methods

ApplicationCommandTarget* UIComponent::getNextCommandTarget()
{
    // this will return the next parent component that is an ApplicationCommandTarget (in this
    // case, there probably isn't one, but it's best to use this method anyway).
    return findFirstTargetParentComponent();
}

void UIComponent::getAllCommands(Array <CommandID>& commands)
{
    const CommandID ids[] = {openConfiguration,
                             saveConfiguration,
                             undo,
                             redo,
                             copySignalChain,
                             pasteSignalChain,
                             clearSignalChain,
                             toggleProcessorList,
                             toggleSignalChain,
                             toggleFileInfo,
                             showHelp
                            };

    commands.addArray(ids, numElementsInArray(ids));

}

void UIComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{

    bool acquisitionStarted = getAudioComponent()->callbacksAreActive();

    switch (commandID)
    {
        case openConfiguration:
            result.setInfo("Open configuration", "Load a saved processor graph.", "General", 0);
            result.addDefaultKeypress('O', ModifierKeys::commandModifier);
            result.setActive(!acquisitionStarted);
            break;

        case saveConfiguration:
            result.setInfo("Save configuration", "Save the current processor graph.", "General", 0);
            result.addDefaultKeypress('S', ModifierKeys::commandModifier);
            result.setActive(!acquisitionStarted);
            break;

        case undo:
            result.setInfo("Undo", "Undo the last action.", "General", 0);
            result.addDefaultKeypress('Z', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case redo:
            result.setInfo("Redo", "Undo the last action.", "General", 0);
            result.addDefaultKeypress('Y', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case copySignalChain:
            result.setInfo("Copy", "Copy a portion of the signal chain.", "General", 0);
            result.addDefaultKeypress('C', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case pasteSignalChain:
            result.setInfo("Paste", "Paste a portion of the signal chain.", "General", 0);
            result.addDefaultKeypress('V', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case clearSignalChain:
            result.setInfo("Clear signal chain", "Clear the current signal chain.", "General", 0);
            result.addDefaultKeypress(KeyPress::backspaceKey, ModifierKeys::commandModifier);
            result.setActive(!getEditorViewport()->isSignalChainEmpty() && !acquisitionStarted);
            break;

        case toggleProcessorList:
            result.setInfo("Processor List", "Show/hide Processor List.", "General", 0);
            result.addDefaultKeypress('P', ModifierKeys::shiftModifier);
            result.setTicked(processorList->isOpen());
            break;

        case toggleSignalChain:
            result.setInfo("Signal Chain", "Show/hide Signal Chain.", "General", 0);
            result.addDefaultKeypress('S', ModifierKeys::shiftModifier);
            result.setTicked(editorViewportButton->isOpen());
            break;

        case toggleFileInfo:
            result.setInfo("File Info", "Show/hide File Info.", "General", 0);
            result.addDefaultKeypress('F', ModifierKeys::shiftModifier);
            result.setTicked(controlPanel->isOpen());
            break;

        case showHelp:
            result.setInfo("Show help...", "Show some freakin' help.", "General", 0);
            result.setActive(false);
            break;

        default:
            break;
    };

}

bool UIComponent::perform(const InvocationInfo& info)
{

    switch (info.commandID)
    {
        case openConfiguration:
            {
                FileChooser fc("Choose a file to load...",
                               File::getCurrentWorkingDirectory(),
                               "*.xml",
                               true);

                if (fc.browseForFileToOpen())
                {
                    File currentFile = fc.getResult();
                    sendActionMessage(getEditorViewport()->loadState(currentFile));
                }
                else
                {
                    sendActionMessage("No configuration selected.");
                }

                break;
            }
        case saveConfiguration:
            {

                FileChooser fc("Choose the file to save...",
                               File::getCurrentWorkingDirectory(),
                               "*",
                               true);

                if (fc.browseForFileToSave(true))
                {
                    File currentFile = fc.getResult();
                    std::cout << currentFile.getFileName() << std::endl;
                    sendActionMessage(getEditorViewport()->saveState(currentFile));
                }
                else
                {
                    sendActionMessage("No file chosen.");
                }

                break;
            }
        case clearSignalChain:
            getEditorViewport()->clearSignalChain();
            break;

        case showHelp:
            std::cout << "SHOW ME SOME HELP!" << std::endl;
            break;

        case toggleProcessorList:
            processorList->toggleState();
            break;

        case toggleFileInfo:
            controlPanel->toggleState();
            break;

        case toggleSignalChain:
            editorViewportButton->toggleState();
            break;

        default:
            break;

    }

    return true;

}


void UIComponent::saveStateToXml(XmlElement* xml)
{
    XmlElement* uiComponentState = xml->createNewChildElement("UICOMPONENT");
    uiComponentState->setAttribute("isProcessorListOpen",processorList->isOpen());
    uiComponentState->setAttribute("isEditorViewportOpen",editorViewportButton->isOpen());
}

void UIComponent::loadStateFromXml(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("UICOMPONENT"))
        {

            bool isProcessorListOpen = xmlNode->getBoolAttribute("isProcessorListOpen");
            bool isEditorViewportOpen = xmlNode->getBoolAttribute("isEditorViewportOpen");

            if (!isProcessorListOpen)
            {
                processorList->toggleState();
            }

            if (!isEditorViewportOpen)
            {
                editorViewportButton->toggleState();
            }

        }
    }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

EditorViewportButton::EditorViewportButton(UIComponent* ui) : UI(ui)
{
    open = true;

    buttonFont = Font("Default Light", 25, Font::plain);

    // MemoryInputStream mis1(BinaryData::cpmonolightserialized,
    //                        BinaryData::cpmonolightserializedSize,
    //                        false);
    // Typeface::Ptr tp1 = new CustomTypeface(mis1);
    // buttonFont = Font(tp1);
    // buttonFont.setHeight(25);

}

EditorViewportButton::~EditorViewportButton()
{

}


void EditorViewportButton::paint(Graphics& g)
{

    g.fillAll(Colour(58,58,58));

    g.setColour(Colours::white);
    g.setFont(buttonFont);
    g.drawText("SIGNAL CHAIN", 10, 0, getWidth(), getHeight(), Justification::left, false);

    g.setColour(Colours::white);

    Path p;

    float h = getHeight();
    float w = getWidth()-5;

    if (open)
    {
        p.addTriangle(w-h+0.3f*h, 0.7f*h,
                      w-h+0.5f*h, 0.3f*h,
                      w-h+0.7f*h, 0.7f*h);
    }
    else
    {
        p.addTriangle(w-h+0.3f*h, 0.5f*h,
                      w-h+0.7f*h, 0.3f*h,
                      w-h+0.7f*h, 0.7f*h);
    }

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    g.strokePath(p, pst);

}


void EditorViewportButton::mouseDown(const MouseEvent& e)
{
    open = !open;
    UI->childComponentChanged();
    repaint();

}

void EditorViewportButton::toggleState()
{
    open = !open;
    UI->childComponentChanged();
    repaint();
}
