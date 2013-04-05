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


#include "FileReaderThread.h"

FileReaderThread::FileReaderThread(SourceNode* sn) :
    DataThread(sn), lengthOfInputFile(0), bufferSize(0)
{
    //  FileChooser chooseFileReaderFile ("Please select the file you want to load...",
    //                            File::getSpecialLocation (File::userHomeDirectory),
    //                            "*");

    // if (chooseFileReaderFile.browseForFileToOpen())
    // {
    //     File fileToRead (chooseFileReaderFile.getResult());
    //     String fileName(fileToRead.getFullPathName());
    //     input = fopen(fileName.String::toCString(), "r");
    // }

    // FIXME stop hard-coding `path' once DataThread gives us a proper
    // mechanism for accepting arguments (the above commented-out code
    // is a layering violation that's best avoided).
#if JUCE_MAC
    const char* path = "/Users/Josh/Programming/open-ephys/GUI/Builds/Linux/build/data_stream_16ch_2";
#else
    const char* path = "./data_stream_16ch_2";
#endif

    input = fopen(path, "r");

    // Avoid a segfault if crock above fails.
    if (!input)
    {
        std::cout << "Can't find data file "
                  << '"' << path << "\", "
                  << "either make sure you're Josh on OS X, "
                  << "or run open-ephys from the build directory on Linux."
                  << std::endl;
        return;
    }

    fseek(input, 0, SEEK_END);
    lengthOfInputFile = ftell(input);
    rewind(input);

    bufferSize = 1600;
    dataBuffer = new DataBuffer(16, bufferSize*3);

    eventCode = 0;

    std::cout << "File Reader Thread initialized." << std::endl;

}

FileReaderThread::~FileReaderThread()
{
    if (input)
        fclose(input);
}

bool FileReaderThread::foundInputSource()
{
    return input != 0;
}

int FileReaderThread::getNumChannels()
{
    return 16;
}

float FileReaderThread::getSampleRate()
{
    return 28000.0f;
}

float FileReaderThread::getBitVolts()
{
    return 0.0305f;
}

bool FileReaderThread::startAcquisition()
{
    if (!input)
        return false;

    startThread();
    return true;
}

bool FileReaderThread::stopAcquisition()
{
    std::cout << "File reader received disable signal." << std::endl;
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }


    return true;

}

bool FileReaderThread::updateBuffer()
{
    if (!input)
        return false;
    if (dataBuffer->getNumSamples() < bufferSize)
    {
        //       // std::cout << dataBuffer->getNumSamples() << std::endl;

        if (ftell(input) >= lengthOfInputFile - bufferSize)
        {
            rewind(input);
        }

        fread(readBuffer, 2, bufferSize, input);

        int chan = 0;

        for (int n = 0; n < bufferSize; n++)
        {
            thisSample[chan] = float(-readBuffer[n])*0.0305f;

            if (chan == 15)
            {
                timestamp = timer.getHighResolutionTicks();
                dataBuffer->addToBuffer(thisSample, &timestamp, &eventCode, 1);
                chan = 0;
            }
            else
            {
                chan++;
            }


        }


    }
    else
    {
        wait(50); // pause for 50 ms to decrease sample rate
    }

    return true;
}
