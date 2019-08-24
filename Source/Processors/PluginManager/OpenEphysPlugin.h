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

#ifndef OPENEPHYSPLUGIN_H_INCLUDED
#define OPENEPHYSPLUGIN_H_INCLUDED

#include <JuceHeader.h>

#include "PluginIDs.h"


#ifdef WIN32
    #ifdef OEPLUGIN
        #define PLUGIN_API __declspec(dllimport)
#else
    #define PLUGIN_API __declspec(dllexport)
#endif
#else
    #define PLUGIN_API __attribute__((visibility("default")))
#endif

struct ProcessorInfo;
struct LibraryInfo;
struct PluginInfo;
class GenericProcessor;
class DataThread;
class SourceNode;
class RecordEngineManager;
class FileSource;

#define PLUGIN_API_VER 6

typedef GenericProcessor*(*ProcessorCreator)();
typedef DataThread*(*DataThreadCreator)(SourceNode*);
typedef RecordEngineManager*(*EngineManagerCreator)();
typedef FileSource*(*FileSourceCreator)();

namespace Plugin
{
	enum ProcessorType
	{
		SourceProcessor = 0, FilterProcessor = 1, SinkProcessor = 2, UtilityProcessor = 3, InvalidProcessor = -1
	};

	struct ProcessorInfo
	{
		const char* name;
		ProcessorCreator creator;
		ProcessorType type;
	};

	struct DataThreadInfo
	{
		const char* name;
		DataThreadCreator creator;
	};

	struct RecordEngineInfo
	{
		const char* name;
		EngineManagerCreator creator;
	};

	struct FileSourceInfo
	{
		const char* name;
		FileSourceCreator creator;
		const char* extensions; //Semicolon separated list of extensions. Eg: "txt;dat;info;kwd"
	};

	struct LibraryInfo
	{
		const char* name;
		int libVersion;
		int apiVersion;
		int numPlugins;
	};

	struct PluginInfo
	{
		PluginType type;
		union
		{
			ProcessorInfo processor;
			DataThreadInfo dataThread;
			RecordEngineInfo recordEngine;
			FileSourceInfo fileSource;
		};
	};

	template<class T>
	GenericProcessor* createProcessor()
	{
		return new T;
	}

	template<class T>
	DataThread* createDataThread(SourceNode* sn)
	{
		return new T(sn);
	}

	template<class T>
	RecordEngineManager* createRecordEngine()
	{
		return T::getEngineManager();
	}

	template<class T>
	FileSource* createFileSource()
	{
		return new T;
	}

};

typedef void(*LibraryInfoFunction)(Plugin::LibraryInfo*);
typedef int(*PluginInfoFunction)(int, Plugin::PluginInfo*);



#endif  // OPENEPHYSPLUGIN_H_INCLUDED
