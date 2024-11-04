/*
    This file contains code from: git.fh-muenster.de/NTLab/CodeReUse/LogComponent4JUCE 

    Copyright (c) 2018 Janos Buttgereit

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    ------------------------------------------------------------------

    This file is modified and a part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "ConsoleViewer.h"

#if JUCE_WINDOWS
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#define fileno _fileno
#define dup _dup
#define dup2 _dup2
#define read _read
#define close _close
#elif JUCE_MAC || JUCE_LINUX
#include <unistd.h>
#endif

ConsoleViewer::ConsoleViewer()
    : Visualizer (nullptr)
{
    logComponent = std::make_unique<LogComponent> (true, true);
    addAndMakeVisible (logComponent.get());

    copyButton = std::make_unique<UtilityButton> ("Copy All");
    copyButton->setFont (FontOptions (13.0f));
    copyButton->setTooltip ("Copy the console output to the clipboard");
    copyButton->onClick = [this]
    { logComponent->copyToClipBoard(); };
    addAndMakeVisible (copyButton.get());

    clearButton = std::make_unique<UtilityButton> ("Clear");
    clearButton->setFont (FontOptions (13.0f));
    clearButton->setTooltip ("Clear the console output");
    clearButton->onClick = [this]
    { logComponent->clear(); };
    addAndMakeVisible (clearButton.get());
}

ConsoleViewer::~ConsoleViewer()
{
}

void ConsoleViewer::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::windowBackground));
    g.setColour (findColour (ThemeColours::controlPanelText));
    g.setFont (FontOptions ("Inter", "Semi Bold", 20.0f));
    g.drawText ("Console", 20, 0, getWidth() - 40, 40, Justification::centredLeft);
}

void ConsoleViewer::resized()
{
    logComponent->setBounds (10, 40, getWidth() - 20, getHeight() - 50);
    copyButton->setBounds (getWidth() - 140, 10, 70, 22);
    clearButton->setBounds (getWidth() - 65, 10, 55, 22);
}

LogComponent::LogComponent (bool captureStdErrImmediately, bool captureStdOutImmediately)
    : stdOutColour (findColour (ThemeColours::controlPanelText)), stdErrColour (Colours::red.darker (0.25f))
{
    consoleEditor.reset (new ConsoleEditor (codeDocument));
    consoleEditor->setFont (FontOptions (14.0f));
    consoleEditor->setScrollbarThickness (12);
    consoleEditor->setTabSize (4, true);
    addAndMakeVisible (consoleEditor.get());

    // save the original stdout and stderr to restore it later
    if (originalStdout == -3)
        originalStdout = dup (fileno (stdout));
    if (originalStderr == -4)
        originalStderr = dup (fileno (stderr));
    std::ios::sync_with_stdio();

    if (captureStdErrImmediately)
        captureStdErr();

    if (captureStdOutImmediately)
        captureStdOut();
}

LogComponent::~LogComponent()
{
    releaseStdOut (false);
    releaseStdErr (false);
}

void LogComponent::clear()
{
    const juce::ScopedLock scopedLock (linesLock);

    consoleEditor->loadContent ("");
    lines.clear();
    numLinesStored = 0;
    numNewLinesSinceUpdate = 0;
}

void LogComponent::copyToClipBoard()
{
    SystemClipboard::copyTextToClipboard (codeDocument.getAllContent());
}

bool LogComponent::captureStdOut()
{
    if (currentStdOutTarget == this)
        return true;

    if (createAndAssignPipe (logStdOutputPipe, stdout) == false)
        return false;

    currentStdOutTarget = this;

    if (stdOutReaderThread == nullptr)
        stdOutReaderThread.reset (new std::thread (LogComponent::readStdOut));

    return true;
}

bool LogComponent::captureStdErr()
{
    if (currentStdErrTarget == this)
        return true;

    if (createAndAssignPipe (logErrOutputPipe, stderr) == false)
        return false;

    currentStdErrTarget = this;

    if (stdErrReaderThread == nullptr)
        stdErrReaderThread.reset (new std::thread (LogComponent::readStdErr));

    return true;
}

void LogComponent::releaseStdOut (bool printRestoreMessage)
{
    if (currentStdOutTarget != this)
        return;

    currentStdOutTarget = nullptr;
    deletePipeAndEndThread (originalStdout, stdout, stdOutReaderThread);

    if (printRestoreMessage)
        LOGC ("ConsoleViewer restored stdout");
}

void LogComponent::releaseStdErr (bool printRestoreMessage)
{
    if (currentStdErrTarget != this)
        return;

    currentStdErrTarget = nullptr;
    deletePipeAndEndThread (originalStderr, stderr, stdErrReaderThread);

    if (printRestoreMessage)
        LOGC ("ConsoleViewer restored stderr");
}

void LogComponent::resized()
{
    consoleEditor->setBounds (getLocalBounds());
}

bool LogComponent::createAndAssignPipe (int* pipeIDs, FILE* stream)
{
    fflush (stream);

#ifdef JUCE_WINDOWS
    int retValue = _pipe (pipeIDs, tmpBufLen, _O_TEXT);
#else
    int retValue = pipe (pipeIDs);
#endif
    if (retValue != 0)
        return false;

    dup2 (pipeIDs[1], fileno (stream));
    close (pipeIDs[1]);

    std::ios::sync_with_stdio();

    // no buffering - will make new content appear as soon as possible
    setvbuf (stream, NULL, _IONBF, 0);

    return true;
}

void LogComponent::deletePipeAndEndThread (int original, FILE* stream, std::unique_ptr<std::thread>& thread)
{
    // send some empty string to trigger the read thread and let it come to an end
    fprintf (stream, " ");
    fflush (stream);
    thread->join();

    // redirect stdout to its original destination
    dup2 (original, fileno (stream));

    // delete the read thread
    thread.reset (nullptr);
}

void LogComponent::readStdOut()
{
    char tmpStdOutBuf[tmpBufLen];

    while (true)
    {
        fflush (stdout);
        size_t numCharsRead = read (logStdOutputPipe[0], tmpStdOutBuf, tmpBufLen - 1);
        if (currentStdOutTarget == nullptr)
            break;
        tmpStdOutBuf[numCharsRead] = '\0';
        currentStdOutTarget->addFromStd (tmpStdOutBuf, numCharsRead);
    }
}

void LogComponent::readStdErr()
{
    char tmpStdErrBuf[tmpBufLen];

    while (true)
    {
        fflush (stderr);
        size_t numCharsRead = read (logErrOutputPipe[0], tmpStdErrBuf, tmpBufLen - 1);
        if (currentStdErrTarget == nullptr)
            break;
        tmpStdErrBuf[numCharsRead] = '\0';
        currentStdErrTarget->addFromStd (tmpStdErrBuf, numCharsRead);
    }
}

void LogComponent::addFromStd (char* stringBufferToAdd, size_t bufferSize)
{
    const juce::ScopedLock scopedLock (linesLock);

    int numNewLines = lines.addTokens (stringBufferToAdd, "\n", "");

    numNewLinesSinceUpdate += numNewLines;
    numLinesStored += numNewLines;

    for (int i = numLinesStored - numNewLines; i < numLinesStored - 1; i++)
    {
        lines.getReference (i) += "\n";
    }

    triggerAsyncUpdate();
}

void LogComponent::handleAsyncUpdate()
{
    const juce::ScopedLock scopedLock (linesLock);
    // check if the lines should be cleared
    if (numLinesStored > numLinesToStore)
    {
        // remove 20 or more lines at the beginning
        int numLinesToRemove = numLinesStored - numLinesToStore;
        if (numLinesToRemove < numLinesToRemoveWhenFull)
            numLinesToRemove = numLinesToRemoveWhenFull;
        lines.removeRange (0, numLinesToRemove);
        numLinesStored = lines.size();

        // clear the editor and flag all lines as new lines
        codeDocument.replaceAllContent (String());
        numNewLinesSinceUpdate = numLinesStored;
    }

    // append new lines
    juce::Colour lastColour = stdOutColour;
    consoleEditor->moveCaretToEnd (false);

    for (int i = numLinesStored - numNewLinesSinceUpdate; i < numLinesStored; i++)
    {
        consoleEditor->insertTextAtCaret (lines[i]);
    }

    numNewLinesSinceUpdate = 0;
}

// static members
int LogComponent::originalStdout = -3;
int LogComponent::originalStderr = -4;
int LogComponent::logStdOutputPipe[2];
int LogComponent::logErrOutputPipe[2];
std::unique_ptr<std::thread> LogComponent::stdOutReaderThread;
std::unique_ptr<std::thread> LogComponent::stdErrReaderThread;

LogComponent* LogComponent::currentStdOutTarget = nullptr;
LogComponent* LogComponent::currentStdErrTarget = nullptr;
