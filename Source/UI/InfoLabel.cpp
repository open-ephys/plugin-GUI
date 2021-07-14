/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "InfoLabel.h"

InfoLabel::InfoLabel()
{

#if defined(__APPLE__)
    File appBundle = File::getSpecialLocation(File::currentApplicationFile);
    const String indexHTML = appBundle.getChildFile("Contents/Resources/Assets/InfoLabel/index.html").getFullPathName();
#else
    File appDirectory = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
    const String indexHTML = appDirectory.getChildFile("assets").getChildFile("InfoLabel").getChildFile("index.html").getFullPathName();
#endif
    
    goToURL(indexHTML);
}

InfoLabel::~InfoLabel()
{

}

bool InfoLabel::pageAboutToLoad(const String & newURL)
{
    if (newURL.compare("about:blank") == 0)
    {
        return false;
    } else if (newURL.startsWith("http"))
    {
        URL url = URL(newURL);
        url.launchInDefaultBrowser();
        return false;
    }
    return true;
}
