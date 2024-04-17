/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

Logger::Logger() {}

Logger::~Logger()
{
    // You're deleting this logger while it's still being used!
    // Always call Logger::setCurrentLogger (nullptr) before deleting the active logger.
    jassert (currentLogger != this);
}

Logger* Logger::currentLogger = nullptr;

void Logger::setCurrentLogger (Logger* const newLogger) noexcept    { currentLogger = newLogger; }
Logger* Logger::getCurrentLogger()  noexcept                        { return currentLogger; }

void Logger::writeToLog (const String& message)
{
    if (currentLogger != nullptr)
        currentLogger->logMessage (message);
    else
        outputDebugString (message);
}

#if JUCE_LOG_ASSERTIONS || JUCE_DEBUG
void JUCE_API JUCE_CALLTYPE logAssertion (const char* const filename, const int lineNum) noexcept
{
    String m ("JUCE Assertion failure in ");
    m << File::createFileWithoutCheckingPath (CharPointer_UTF8 (filename)).getFileName() << ':' << lineNum;

   #if JUCE_LOG_ASSERTIONS
    Logger::writeToLog (m);
   #else
    DBG (m);
   #endif
}
#endif

} // namespace juce
