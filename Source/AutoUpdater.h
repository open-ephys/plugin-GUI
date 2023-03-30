/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2023 Open Ephys

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

    Reference : https://github.com/juce-framework/JUCE/blob/6.0.8/extras/Projucer/Source/Application/jucer_AutoUpdater.h

*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DownloadAndInstallThread;

class LatestVersionCheckerAndUpdater   : public DeletedAtShutdown,
                                         private Thread
{
public:
    LatestVersionCheckerAndUpdater();
    ~LatestVersionCheckerAndUpdater() override;

    struct Asset
    {
        const String name;
        const String url;
        const int size;
    };

    void checkForNewVersion (bool isBackgroundCheck);

    //==============================================================================
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (LatestVersionCheckerAndUpdater)

private:
    //==============================================================================
    void run() override;
    void askUserAboutNewVersion (const String&, const String&, const Asset& asset);
    void askUserForLocationToDownload (const Asset& asset);
    void downloadAndInstall (const Asset& asset, const File& targetFolder);

    //==============================================================================
    bool backgroundCheck = false;

    std::unique_ptr<DownloadAndInstallThread> installer;
    std::unique_ptr<Component> dialogWindow;
};
