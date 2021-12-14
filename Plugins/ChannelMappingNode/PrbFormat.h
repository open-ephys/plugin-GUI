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
        FileOutputStream outputStream(filename);
        outputStream.setPosition(0);
        outputStream.truncate();

        std::unique_ptr<DynamicObject> info = std::make_unique<DynamicObject>();
        std::unique_ptr<DynamicObject> nestedObj = std::make_unique<DynamicObject>();

        Array<var> arr;
        Array<var> arr2;
        Array<var> arr3;
        
        for (int i = 0; i < settings.channelOrder.size(); i++)
        {
            arr.add(var(settings.channelOrder[i]));
            arr2.add(var(settings.referenceIndex[i]));
            arr3.add(var(settings.isEnabled[i]));
        }
        
        nestedObj->setProperty("mapping", var(arr));
        nestedObj->setProperty("reference", var(arr2));
        nestedObj->setProperty("enabled", var(arr3));

        info->setProperty("0", nestedObj.get());

        DynamicObject* nestedObj2 = new DynamicObject();
        Array<var> arr4;
        for (int i = 0; i < settings.referenceChannels.size(); i++)
        {
            arr4.add(var(settings.referenceChannels[i]));
        }
        nestedObj2->setProperty("channels", var(arr4));

        info->setProperty("refs", nestedObj2);

        info->writeAsJSON(outputStream, 2, false, 3);
    }

    static void read(File filename, ChannelMapSettings& settings)
    {
        FileInputStream inputStream(filename);

        var json = JSON::parse(inputStream);

        var returnVal = -255;

        var channelGroup = json.getProperty(Identifier("0"), returnVal);

        if (channelGroup.equalsWithSameType(returnVal))
        {
            std::cout << "Not a valid .prb file." << std::endl;
            return;
        }

        var mapping = channelGroup[Identifier("mapping")];
        Array<var>* map = mapping.getArray();

        var reference = channelGroup[Identifier("reference")];
        Array<var>* ref = reference.getArray();

        var enabled = channelGroup[Identifier("enabled")];
        Array<var>* enbl = enabled.getArray();

        std::cout << "We found this many: " << map->size() << std::endl;

        for (int i = 0; i < map->size(); i++)
        {
            int ch = map->getUnchecked(i);
            settings.channelOrder.set(i, ch);

            int rf = ref->getUnchecked(i);
            settings.referenceIndex.set(i, rf);

            bool en = enbl->getUnchecked(i);
            settings.isEnabled.set(i, en);
        }
        
        var refChans = json[Identifier("refs")];
        var channels = refChans[Identifier("channels")];
        Array<var>* chans = channels.getArray();

        settings.referenceChannels.clear();
        
        for (int i = 0; i < chans->size(); i++)
        {
            int ch = chans->getUnchecked(i);
            settings.referenceChannels.add(ch);
        }
    }
};


#endif  // __PRBFORMAT_H_330E50E0__
