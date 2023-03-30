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

    Reference : https://github.com/juce-framework/JUCE/blob/6.0.8/extras/Projucer/Source/Application/jucer_AutoUpdater.cpp

*/

#include "AutoUpdater.h"
#include "CoreServices.h"

//==============================================================================
LatestVersionCheckerAndUpdater::LatestVersionCheckerAndUpdater()
    : Thread ("VersionChecker")
{
}

LatestVersionCheckerAndUpdater::~LatestVersionCheckerAndUpdater()
{
    stopThread (6000);
    clearSingletonInstance();
}

void LatestVersionCheckerAndUpdater::checkForNewVersion (bool background)
{
    if (! isThreadRunning())
    {
        backgroundCheck = background;
        startThread (3);
    }
}

//==============================================================================
void LatestVersionCheckerAndUpdater::run()
{
    LOGC("Checking for a new version....");
    URL latestVersionURL ("https://api.github.com/repos/open-ephys/plugin-GUI/releases/latest");

    std::unique_ptr<InputStream> inStream (latestVersionURL.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                                                 .withConnectionTimeoutMs (5000)));
    const String commErr = "Failed to communicate with the Open Ephys update server.\n"
                           "Please try again in a few minutes.\n\n"
                           "If this problem persists you can download the latest version of Open Ephys GUI from open-ephys.org/gui";

    if (inStream == nullptr)
    {
        if (! backgroundCheck)
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Update Server Communication Error",
                                              commErr);

        return;
    }

    auto content = inStream->readEntireStreamAsString();
    auto latestReleaseDetails = JSON::parse (content);

    auto* json = latestReleaseDetails.getDynamicObject();

    if (json == nullptr)
    {
        if (! backgroundCheck)
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Update Server Communication Error",
                                              commErr);

        return;
    }

    auto versionString = json->getProperty ("tag_name").toString();

    if (versionString.isEmpty())
        return;

    auto* assets = json->getProperty ("assets").getArray();

    if (assets == nullptr)
        return;

    auto releaseNotes = json->getProperty ("body").toString();

    std::vector<Asset> parsedAssets;

    for (auto& asset : *assets)
    {
        if (auto* assetJson = asset.getDynamicObject())
        {
            parsedAssets.push_back ({ assetJson->getProperty ("name").toString(),
                                      assetJson->getProperty ("url").toString(),
                                      (int)assetJson->getProperty("size")});
            jassert (parsedAssets.back().name.isNotEmpty());
            jassert (parsedAssets.back().url.isNotEmpty());
            jassert (parsedAssets.back().size != 0);

        }
        else
        {
            jassertfalse;
        }
    }

    String latestVer = versionString.substring(1);


    if (latestVer.compareNatural(CoreServices::getGUIVersion()) <= 0)
    {
        if (! backgroundCheck)
            AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                              "No New Version Available",
                                              "Your GUI version is up to date.");
        return;
    }

    auto osString = []
    {
       #if JUCE_MAC
        return "mac";
       #elif JUCE_WINDOWS
        return "windows";
       #elif JUCE_LINUX
        return "linux";
       #else
        jassertfalse;
        return "Unknown";
       #endif
    }();

#if JUCE_WINDOWS 
    String requiredFilename ("open-ephys-" + versionString + "-" + osString + ".zip");
    File exeDir = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    if(exeDir.findChildFiles(File::findFiles, false, "unins*").size() > 0)
    {
        requiredFilename = "Install-Open-Ephys-GUI-" + versionString + ".exe";
    }
#elif
    String requiredFilename ("open-ephys-" + versionString + "-" + osString + ".zip");
#endif
    for (auto& asset : parsedAssets)
    {
        if (asset.name == requiredFilename)
        {

            MessageManager::callAsync ([this, versionString, releaseNotes, asset]
            {
                askUserAboutNewVersion (versionString, releaseNotes, asset);
            });

            return;
        }
    }

    if (! backgroundCheck)
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Failed to find any new downloads",
                                          "Please try again in a few minutes.");
}

//==============================================================================
class UpdateDialog  : public Component
{
public:
    UpdateDialog (const String& newVersion, const String& releaseNotes)
    {
        titleLabel.setText ("Open Ephys GUI version " + newVersion, dontSendNotification);
        titleLabel.setFont (Font("Fira Sans", "SemiBold", 18.0f));
        titleLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (titleLabel);

        contentLabel.setText ("A new version of Open Ephys GUI is available - would you like to download it?", dontSendNotification);
        contentLabel.setFont (Font("Fira Sans", "Regular", 16.0f));
        contentLabel.setJustificationType (Justification::topLeft);
        contentLabel.setMinimumHorizontalScale(1.0);
        addAndMakeVisible (contentLabel);

        releaseNotesEditor.setMultiLine (true);
        releaseNotesEditor.setReadOnly (true);
        releaseNotesEditor.setText (releaseNotes);
        addAndMakeVisible (releaseNotesEditor);

        addAndMakeVisible (chooseButton);
        chooseButton.onClick = [this] { exitModalStateWithResult (1); };

        addAndMakeVisible (cancelButton);
        cancelButton.onClick = [this]
        {
            // ProjucerApplication::getApp().setAutomaticVersionCheckingEnabled (! dontAskAgainButton.getToggleState());
            exitModalStateWithResult (-1);
        };

        dontAskAgainButton.setToggleState (false, dontSendNotification);
        addAndMakeVisible (dontAskAgainButton);

#if JUCE_MAC
    	File iconDir = File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/Resources");
#else
		File iconDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();
#endif
        juceIcon = Drawable::createFromImageFile(iconDir.getChildFile("icon-small.png"));
        lookAndFeelChanged();

        setSize (640, 480);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (10);

        auto topSlice = b.removeFromTop (juceIconBounds.getHeight())
                         .withTrimmedLeft (juceIconBounds.getWidth());

        titleLabel.setBounds (topSlice.removeFromTop (25));
        topSlice.removeFromTop (5);
        contentLabel.setBounds (topSlice.removeFromTop (25));

        auto buttonBounds = b.removeFromBottom (60);
        buttonBounds.removeFromBottom (25);
        chooseButton.setBounds (buttonBounds.removeFromLeft (buttonBounds.getWidth() / 2).reduced (20, 0));
        cancelButton.setBounds (buttonBounds.reduced (20, 0));
        dontAskAgainButton.setBounds (cancelButton.getBounds().withY (cancelButton.getBottom() + 5).withHeight (20));

        releaseNotesEditor.setBounds (b.reduced (0, 10));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);

        if (juceIcon != nullptr)
            juceIcon->drawWithin (g, juceIconBounds.toFloat(),
                                  RectanglePlacement::stretchToFit, 1.0f);
    }

    static std::unique_ptr<DialogWindow> launchDialog (const String& newVersionString,
                                                       const String& releaseNotes)
    {
        DialogWindow::LaunchOptions options;

        options.dialogTitle = "Download Open Ephys GUI version " + newVersionString + "?";
        options.resizable = false;

        auto* content = new UpdateDialog (newVersionString, releaseNotes);
        options.content.set (content, true);

        std::unique_ptr<DialogWindow> dialog (options.create());

        content->setParentWindow (dialog.get());
        dialog->enterModalState (true, nullptr, true);

        return dialog;
    }

private:
    void lookAndFeelChanged() override
    {
        cancelButton.setColour (TextButton::buttonColourId, Colours::crimson);
        releaseNotesEditor.applyFontToAllText (Font("Fira Sans", "Regular", 16.0f));
    }

    void setParentWindow (DialogWindow* parent)
    {
        parentWindow = parent;
    }

    void exitModalStateWithResult (int result)
    {
        if (parentWindow != nullptr)
            parentWindow->exitModalState (result);
    }

    Label titleLabel, contentLabel, releaseNotesLabel;
    TextEditor releaseNotesEditor;
    TextButton chooseButton { "Download" }, cancelButton { "Cancel" };
    ToggleButton dontAskAgainButton { "Don't ask again" };
    std::unique_ptr<Drawable> juceIcon;
    Rectangle<int> juceIconBounds { 10, 10, 64, 64 };

    DialogWindow* parentWindow = nullptr;
};

void LatestVersionCheckerAndUpdater::askUserForLocationToDownload (const Asset& asset)
{
    FileChooser chooser ("Please select the location into which you would like to install the new version",
                         { File::getSpecialLocation(File::userDesktopDirectory) },
                         "*.exe;*.zip");

    if (chooser.browseForDirectory())
    {
        auto targetFolder = chooser.getResult();
        if (targetFolder == File{})
            return;

        downloadAndInstall (asset, targetFolder);
    }
}

void LatestVersionCheckerAndUpdater::askUserAboutNewVersion (const String& newVersionString,
                                                             const String& releaseNotes,
                                                             const Asset& asset)
{
    dialogWindow = UpdateDialog::launchDialog (newVersionString, releaseNotes);

    if (auto* mm = ModalComponentManager::getInstance())
    {
        mm->attachCallback (dialogWindow.get(),
                            ModalCallbackFunction::create ([this, asset] (int result)
                                                           {
                                                               if (result == 1)
                                                                    askUserForLocationToDownload (asset);

                                                                dialogWindow.reset();
                                                            }));
    }
}

//==============================================================================
class DownloadAndInstallThread   : private ThreadWithProgressWindow
{
public:
    DownloadAndInstallThread  (const LatestVersionCheckerAndUpdater::Asset& a,
                               const File& t,
                               std::function<void()>&& cb)
        : ThreadWithProgressWindow ("Downloading New Version", true, true),
          asset (a), targetFolder (t), completionCallback (std::move (cb))
    {
        launchThread (3);
    }

private:
    void run() override
    {
        setProgress (0.0);

        File targetFile = targetFolder.getChildFile(asset.name).getNonexistentSibling();
        auto result = download (targetFile);

        // if (result.wasOk() && ! threadShouldExit())
        //     result = install (zipData);

        if (result.failed())
        {
            MessageManager::callAsync ([result] { AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                                                    "Downloading Failed",
                                                                                    result.getErrorMessage()); });
        }
        else
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                "Download finished!",
                                                "New binaries can be found at: " + 
                                                targetFile.getFullPathName());
            MessageManager::callAsync (completionCallback);
        }
    }

    Result download (File& dest)
    {
        setStatusMessage ("Downloading...");

        int statusCode = 0;
        URL downloadUrl (asset.url);
        StringPairArray responseHeaders;

        auto inStream = downloadUrl.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                        .withExtraHeaders ("Accept: application/octet-stream")
                                                        .withConnectionTimeoutMs (5000)
                                                        .withResponseHeaders (&responseHeaders)
                                                        .withStatusCode (&statusCode)
                                                        .withNumRedirectsToFollow (1));

        if (inStream != nullptr && statusCode == 200)
        {
            int64 total = 0;

            //Use the Url's input stream and write it to a file using output stream
            std::unique_ptr<FileOutputStream> out = dest.createOutputStream();

            for (;;)
            {
                if (threadShouldExit())
                    return Result::fail ("Cancelled");



                auto written = out->writeFromInputStream(*inStream, 8192);

                if (written == 0)
                    break;

                total += written;

                setProgress((double)total / (double)asset.size);

                setStatusMessage ("Downloading... " + File::descriptionOfSizeInBytes (total));
            }

            out->flush();
            return Result::ok();
        }

        return Result::fail ("Failed to download from: " + asset.url);
    }

    Result install (const File& dest)
    {
        setStatusMessage ("Installing...");

        ZipFile zip (dest);

    //     if (zip.getNumEntries() == 0)
    //         return Result::fail ("The downloaded file was not a valid JUCE file!");

    //     struct ScopedDownloadFolder
    //     {
    //         explicit ScopedDownloadFolder (const File& installTargetFolder)
    //         {
    //             folder = installTargetFolder.getSiblingFile (installTargetFolder.getFileNameWithoutExtension() + "_download").getNonexistentSibling();
    //             jassert (folder.createDirectory());
    //         }

    //         ~ScopedDownloadFolder()   { folder.deleteRecursively(); }

    //         File folder;
    //     };

    //     ScopedDownloadFolder unzipTarget (targetFolder);

    //     if (! unzipTarget.folder.isDirectory())
    //         return Result::fail ("Couldn't create a temporary folder to unzip the new version!");

    //     auto r = zip.uncompressTo (unzipTarget.folder);

    //     if (r.failed())
    //         return r;

    //     if (threadShouldExit())
    //         return Result::fail ("Cancelled");

    //    #if JUCE_LINUX || JUCE_BSD || JUCE_MAC
    //     r = setFilePermissions (unzipTarget.folder, zip);

    //     if (r.failed())
    //         return r;

    //     if (threadShouldExit())
    //         return Result::fail ("Cancelled");
    //    #endif

    //     if (targetFolder.exists())
    //     {
    //         auto oldFolder = targetFolder.getSiblingFile (targetFolder.getFileNameWithoutExtension() + "_old").getNonexistentSibling();

    //         if (! targetFolder.moveFileTo (oldFolder))
    //             return Result::fail ("Could not remove the existing folder!\n\n"
    //                                  "This may happen if you are trying to download into a directory that requires administrator privileges to modify.\n"
    //                                  "Please select a folder that is writable by the current user.");
    //     }

    //     if (! unzipTarget.folder.getChildFile ("JUCE").moveFileTo (targetFolder))
    //         return Result::fail ("Could not overwrite the existing folder!\n\n"
    //                              "This may happen if you are trying to download into a directory that requires administrator privileges to modify.\n"
    //                              "Please select a folder that is writable by the current user.");

    //     return Result::ok();
    }

    Result setFilePermissions (const File& root, const ZipFile& zip)
    {
        // constexpr uint32 executableFlag = (1 << 22);

        // for (int i = 0; i < zip.getNumEntries(); ++i)
        // {
        //     auto* entry = zip.getEntry (i);

        //     if ((entry->externalFileAttributes & executableFlag) != 0 && entry->filename.getLastCharacter() != '/')
        //     {
        //         auto exeFile = root.getChildFile (entry->filename);

        //         if (! exeFile.exists())
        //             return Result::fail ("Failed to find executable file when setting permissions " + exeFile.getFileName());

        //         if (! exeFile.setExecutePermission (true))
        //             return Result::fail ("Failed to set executable file permission for " + exeFile.getFileName());
        //     }
        // }

        // return Result::ok();
    }

    const LatestVersionCheckerAndUpdater::Asset asset;
    File targetFolder;
    std::function<void()> completionCallback;
};

static void restartProcess (const File& targetFolder)
{
   #if JUCE_MAC || JUCE_LINUX || JUCE_BSD
    #if JUCE_MAC
     auto newProcess = targetFolder.getChildFile ("Projucer.app").getChildFile ("Contents").getChildFile ("MacOS").getChildFile ("Projucer");
    #elif JUCE_LINUX || JUCE_BSD
     auto newProcess = targetFolder.getChildFile ("Projucer");
    #endif

    StringArray command ("/bin/sh", "-c", "while killall -0 Projucer; do sleep 5; done; " + newProcess.getFullPathName().quoted());
   #elif JUCE_WINDOWS
    auto newProcess = targetFolder.getChildFile ("Projucer.exe");

    auto command = "cmd.exe /c\"@echo off & for /l %a in (0) do ( tasklist | find \"Projucer\" >nul & ( if errorlevel 1 ( "
                    + targetFolder.getChildFile ("Projucer.exe").getFullPathName().quoted() + " & exit /b ) else ( timeout /t 10 >nul ) ) )\"";
   #endif

    if (newProcess.existsAsFile())
    {
        ChildProcess restartProcess;
        restartProcess.start (command, 0);

        JUCEApplication::getInstance()->systemRequestedQuit();
    }
}

void LatestVersionCheckerAndUpdater::downloadAndInstall (const Asset& asset, const File& targetFolder)
{
    installer.reset (new DownloadAndInstallThread (asset, targetFolder,
                                                   [this, targetFolder]
                                                   {
                                                       installer.reset();
                                
                                                    //    restartProcess (targetFolder);
                                                   }));
}

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (LatestVersionCheckerAndUpdater)
