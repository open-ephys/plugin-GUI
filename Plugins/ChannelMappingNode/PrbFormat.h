/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __PRBFORMAT_H_330E50E0__
#define __PRBFORMAT_H_330E50E0__


#include <ProcessorHeaders.h>

#include "ChannelMappingNode.h"

class PrbFormat
{
public:

    static void write(File filename, ChannelMapSettings& settings)
    {
        /*FileOutputStream outputStream(filename);
        outputStream.setPosition(0);
        outputStream.truncate();

        std::unique_ptr<DynamicObject> info = std::make_unique<DynamicObject>();
        std::unique_ptr<DynamicObject> nestedObj = std::make_unique<DynamicObject>();

        Array<var> arr;

        for (int i = 0; i < settings.channelOrder.size(); i++)
        {
            arr.add(var(settings.channelOrder[i]));
        }
        nestedObj->setProperty("mapping", var(arr));

        Array<var> arr2;
        for (int i = 0; i < settings.referenceArray.size(); i++)
        {
            arr2.add(var(settings.referenceArray[settings.channelArray[i] - 1]));
        }
        nestedObj->setProperty("reference", var(arr2));

        Array<var> arr3;
        for (int i = 0; i < settings.enabledChannelArray.size(); i++)
        {
            arr3.add(var(enabledChannelArray[settings.channelArray[i] - 1]));
        }
        nestedObj->setProperty("enabled", var(arr3));

        info->setProperty("0", nestedObj);

        DynamicObject* nestedObj2 = new DynamicObject();
        Array<var> arr4;
        for (int i = 0; i < settings.referenceChannels.size(); i++)
        {
            arr4.add(var(settings.referenceChannels[i]));
        }
        nestedObj2->setProperty("channels", var(arr4));

        info->setProperty("refs", nestedObj2);

        info->writeAsJSON(outputStream, 2, false, 3);*/
    }

    static void read(File filename, ChannelMapSettings& settings)
    {
        /*FileInputStream inputStream(filename);

        var json = JSON::parse(inputStream);

        var returnVal = -255;

        var channelGroup = json.getProperty(Identifier("0"), returnVal);

        if (channelGroup.equalsWithSameType(returnVal))
        {
            return "Not a valid .prb file.";
        }

        var mapping = channelGroup[Identifier("mapping")];
        Array<var>* map = mapping.getArray();

        var reference = channelGroup[Identifier("reference")];
        Array<var>* ref = reference.getArray();

        var enabled = channelGroup[Identifier("enabled")];
        Array<var>* enbl = enabled.getArray();

        std::cout << "We found this many: " << map->size() << std::endl;

        if (map->size() > previousChannelCount)
            createElectrodeButtons(map->size(), false);

        for (int i = 0; i < map->size(); i++)
        {
            int ch = map->getUnchecked(i);
            channelArray.set(i, ch);

            int rf = ref->getUnchecked(i);
            referenceArray.set(ch - 1, rf);

            bool en = enbl->getUnchecked(i);
            enabledChannelArray.set(ch - 1, en);

            electrodeButtons[i]->setChannelNum(ch);
            electrodeButtons[i]->setEnabled(en);

            getProcessor()->setCurrentChannel(i);
            getProcessor()->setParameter(0, ch - 1);
            getProcessor()->setCurrentChannel(ch - 1);
            getProcessor()->setParameter(1, rf);
            getProcessor()->setParameter(3, en ? 1 : 0);
        }
        checkUnusedChannels();

        var refChans = json[Identifier("refs")];
        var channels = refChans[Identifier("channels")];
        Array<var>* chans = channels.getArray();

        for (int i = 0; i < chans->size(); i++)
        {
            int ch = chans->getUnchecked(i);
            referenceChannels.set(i, ch);
            getProcessor()->setCurrentChannel(ch);
            getProcessor()->setParameter(2, i);
        }

        referenceButtons[0]->setToggleState(true, sendNotificationSync);

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (referenceArray[electrodeButtons[i]->getChannelNum() - 1] == 0)
            {
                electrodeButtons[i]->setToggleState(true, dontSendNotification);
            }
            else
            {
                electrodeButtons[i]->setToggleState(false, dontSendNotification);
            }
        }

        setConfigured(true);
        CoreServices::updateSignalChain(this);*/
    }
};


#endif  // __PRBFORMAT_H_330E50E0__
