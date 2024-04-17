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

FileLogger::FileLogger (const File& file,
                        const String& welcomeMessage,
                        const int64 maxInitialFileSizeBytes)
    : logFile (file)
{
    if (maxInitialFileSizeBytes >= 0)
        trimFileSize (logFile, maxInitialFileSizeBytes);

    if (! file.exists())
        file.create();  // (to create the parent directories)

    String welcome;
    welcome << newLine
            << "**********************************************************" << newLine
            << welcomeMessage << newLine
            << "Log started: " << Time::getCurrentTime().toString (true, true) << newLine;

    FileLogger::logMessage (welcome);
}

FileLogger::~FileLogger() {}

//==============================================================================
void FileLogger::logMessage (const String& message)
{
    const ScopedLock sl (logLock);
    DBG (message);
    FileOutputStream out (logFile, 256);
    out << message << newLine;
}

void FileLogger::trimFileSize (const File& file, int64 maxFileSizeBytes)
{
    if (maxFileSizeBytes <= 0)
    {
        file.deleteFile();
    }
    else
    {
        const int64 fileSize = file.getSize();

        if (fileSize > maxFileSizeBytes)
        {
            TemporaryFile tempFile (file);

            {
                FileOutputStream out (tempFile.getFile());
                FileInputStream in (file);

                if (! (out.openedOk() && in.openedOk()))
                    return;

                in.setPosition (fileSize - maxFileSizeBytes);

                for (;;)
                {
                    const char c = in.readByte();
                    if (c == 0)
                        return;

                    if (c == '\n' || c == '\r')
                    {
                        out << c;
                        break;
                    }
                }

                out.writeFromInputStream (in, -1);
            }

            tempFile.overwriteTargetFileWithTemporary();
        }
    }
}

//==============================================================================
File FileLogger::getSystemLogFileFolder()
{
   #if JUCE_MAC
    return File ("~/Library/Logs");
   #else
    return File::getSpecialLocation (File::userApplicationDataDirectory);
   #endif
}

FileLogger* FileLogger::createDefaultAppLogger (const String& logFileSubDirectoryName,
                                                const String& logFileName,
                                                const String& welcomeMessage,
                                                const int64 maxInitialFileSizeBytes)
{
    return new FileLogger (getSystemLogFileFolder().getChildFile (logFileSubDirectoryName)
                                                   .getChildFile (logFileName),
                           welcomeMessage, maxInitialFileSizeBytes);
}

FileLogger* FileLogger::createDateStampedLogger (const String& logFileSubDirectoryName,
                                                 const String& logFileNameRoot,
                                                 const String& logFileNameSuffix,
                                                 const String& welcomeMessage)
{
    return new FileLogger (getSystemLogFileFolder().getChildFile (logFileSubDirectoryName)
                                                   .getChildFile (logFileNameRoot + Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S"))
                                                   .withFileExtension (logFileNameSuffix)
                                                   .getNonexistentSibling(),
                           welcomeMessage, 0);
}

} // namespace juce
