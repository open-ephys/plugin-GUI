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

#ifndef __PLUGINMGR__
#define __PLUGINMGR__

#ifdef WIN32
#include <Windows.h>
#endif
#include <list>
#include <string>
#include <sys/types.h>
#include "../../../JuceLibraryCode/JuceHeader.h"

namespace PluginManager {

	class Plugin;

	class Manager {
		friend class Plugin;

		public:
		static Manager *getInstance(void);
		Plugin *loadPlugin(const String&);
		void unloadPlugin(Plugin *);
		void removeAllPlugins();

		private:
		Manager(void) {};
		~Manager(void) {};
		Manager(const Manager &) {};
		Manager &operator=(const Manager &) {return *getInstance();};

		void insertListPlugin(PluginManager::Plugin *);
		void removeListPlugin(PluginManager::Plugin *);
		std::list<PluginManager::Plugin *> pluginList;
		static Manager *instance;
	};

	class Plugin {
		friend class Manager;

		public:
		Plugin();
		virtual ~Plugin();
		void unload();

		private:
#ifdef WIN32
			HINSTANCE processorHandle;
#else
		void *processorHandle;
#endif
	};
};
#endif
