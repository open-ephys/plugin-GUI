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


#ifdef _WIN32
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

#define PLUGIN_API_VER 9

typedef GenericProcessor*(*ProcessorCreator)();
typedef DataThread*(*DataThreadCreator)(SourceNode*);
typedef RecordEngineManager*(*EngineManagerCreator)();
typedef FileSource*(*FileSourceCreator)();

namespace Plugin
{
    /** High-level plugin type
     */
    enum Type
    {
        /** Compiled with the host application*/
        BUILT_IN = 0
        
        /** GenericProcessor plugin, can be dragged into the signal chain*/
        , PROCESSOR = 1
        
        /** RecordEngine, can be used by the RecordNode*/
        , RECORD_ENGINE = 2
        
        /** FileSource plugin, can be used by the FileReader*/
        , FILE_SOURCE = 3
        
        /** DataThread, nested inside a SourceNode*/
        , DATA_THREAD = 4
        
        /** Invalid plugin type*/
        , INVALID = -1
    };

    namespace Processor
    {
        /** If the high-level type is PLUGIN_TYPE_PROCESSOR,
            this specifies the type of processor to create
         */
        enum Type
        {
            /** Modifies incoming data*/
            FILTER = 1
            
            /** Generates data*/
            , SOURCE = 2
            
            /** Sends data outside the signal chain (but does not modify it)*/
            , SINK = 3

            /** Splits the incoming data down two paths*/
            , SPLITTER = 4
            
            /** Merges two sets of data streams*/
            , MERGER = 5
            
            /** Sends the incoming data to the computer's audio output*/
            , AUDIO_MONITOR = 6
            
            /** Other utilities (such as Record Control)*/
            , UTILITY = 7
            
            /** Writes incoming data to disk*/
            , RECORD_NODE = 8
            
            /** Not a valid processor type*/
            , INVALID = -1

			/** An empty source processor*/
            , EMPTY = -2
        };
    }
    
    struct Description {
        
        bool fromProcessorList = false;
        int index = -1;
        String name = "Default";
        String libName = "DefaultLibrary";
        String libVersion = "0.0.0";
        
        // built-in, processor, record engine, etc.
        Type type = Plugin::INVALID;
        
        // source, sink, filter, etc.
        Processor::Type processorType = Plugin::Processor::INVALID;

        int nodeId = -1;
    };

	struct ProcessorInfo
	{
		const char* name;
		ProcessorCreator creator;
		Processor::Type type;
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
		const char* libVersion;
		int apiVersion;
		int numPlugins;
	};

	struct PluginInfo
	{
		Type type;
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
