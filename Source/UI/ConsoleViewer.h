/*
    This contains code from: git.fh-muenster.de/NTLab/CodeReUse/LogComponent4JUCE 

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

#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Visualization/Visualizer.h"
#include <thread>


/**
 * A component that takes over stdout and stderr and displays it inside
 */
class LogComponent : public juce::Component, private juce::AsyncUpdater {
    
public:
    
    /**
     * Creates a new log component. By default, stderr and stdout are captured immediately after
     * construction
     */
    LogComponent (bool captureStdErrImmediately = true, bool captureStdOutImmediately = true);
    
    ~LogComponent();
    
    /** Clears the log */
    void clear();

    /** Copies the content of the log to the clipboard */
    void copyToClipBoard();

    /** Sets the colour of the log messages */
    void colourChanged() override;
    
    /** Redirects stdout to this component. Call releaseStdOut to restore the prior state */
    bool captureStdOut();
    
    /** Redirects stderr to this component. Call releaseStdErr to restore the prior state */
    bool captureStdErr();
    
    /** Restores the original stdout */
    void releaseStdOut (bool printRestoreMessage = true);
    
    /** Restores the original stderr */
    void releaseStdErr (bool printRestoreMessage = true);

    void resized() override;
    
private:
    
    // filedescriptors to restore the standard console output streams
    static int originalStdout, originalStderr;
    
    // pipes to redirect the streams to this component
    static int logStdOutputPipe[2];
    static int logErrOutputPipe[2];
    
    static std::unique_ptr<std::thread> stdOutReaderThread;
    static std::unique_ptr<std::thread> stdErrReaderThread;
    
    // indicates if it is the current stdout
    static LogComponent* currentStdOutTarget;
    static LogComponent* currentStdErrTarget;
    
    static const int tmpBufLen = 512;
    juce::TextEditor consoleOutputEditor;
    juce::Colour stdOutColour = juce::Colours::black;
    juce::Colour stdErrColour = juce::Colours::red;
    
    // this is where the text is stored
    int numLinesToStore = 2000;
    int numLinesToRemoveWhenFull = 20;
    int numLinesStored = 0;
    int numNewLinesSinceUpdate = 0;
    juce::StringArray lines;
    juce::Array<juce::Colour> lineColours;
    juce::CriticalSection linesLock;
    
    static bool createAndAssignPipe (int *pipeIDs, FILE *stream);
    static void deletePipeAndEndThread (int original, FILE *stream, std::unique_ptr<std::thread>& thread);
    
    static void readStdOut();
    
    static void readStdErr();
    
    void addFromStd (char *stringBufferToAdd, size_t bufferSize, juce::Colour colourOfString);
    
    void handleAsyncUpdate() override;
};


class ConsoleViewer : public Visualizer
{
public:
    ConsoleViewer();
    ~ConsoleViewer();
    
    void paint (Graphics& g) override;
    void resized() override;

    /** Visualizer virtual functions */
    void refresh() override {}
    void refreshState() override {}


private:
    std::unique_ptr<LogComponent> logComponent;

    std::unique_ptr<UtilityButton> clearButton;
    std::unique_ptr<UtilityButton> copyButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleViewer)
};
