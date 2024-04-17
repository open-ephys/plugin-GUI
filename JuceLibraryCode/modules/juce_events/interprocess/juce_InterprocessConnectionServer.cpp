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

InterprocessConnectionServer::InterprocessConnectionServer() : Thread ("JUCE IPC server")
{
}

InterprocessConnectionServer::~InterprocessConnectionServer()
{
    stop();
}

//==============================================================================
bool InterprocessConnectionServer::beginWaitingForSocket (const int portNumber, const String& bindAddress)
{
    stop();

    socket.reset (new StreamingSocket());

    if (socket->createListener (portNumber, bindAddress))
    {
        startThread();
        return true;
    }

    socket.reset();
    return false;
}

void InterprocessConnectionServer::stop()
{
    signalThreadShouldExit();

    if (socket != nullptr)
        socket->close();

    stopThread (4000);
    socket.reset();
}

int InterprocessConnectionServer::getBoundPort() const noexcept
{
    return (socket == nullptr) ? -1 : socket->getBoundPort();
}

void InterprocessConnectionServer::run()
{
    while ((! threadShouldExit()) && socket != nullptr)
    {
        std::unique_ptr<StreamingSocket> clientSocket (socket->waitForNextConnection());

        if (clientSocket != nullptr)
            if (auto* newConnection = createConnectionObject())
                newConnection->initialiseWithSocket (std::move (clientSocket));
    }
}

} // namespace juce
