#/*
#	 ------------------------------------------------------------------
#
#	 This file is part of the Open Ephys GUI
#	 Copyright (C) 2013 Open Ephys
#
#	 ------------------------------------------------------------------
#
#	 This program is free software: you can redistribute it and/or modify
#	 it under the terms of the GNU General Public License as published by
#	 the Free Software Foundation, either version 3 of the License, or
#	 (at your option) any later version.
#
#	 This program is distributed in the hope that it will be useful,
#	 but WITHOUT ANY WARRANTY; without even the implied warranty of
#	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	 GNU General Public License for more details.
#
#	 You should have received a copy of the GNU General Public License
#	 along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#*/

#!/bin/bash

BUILD_HOME=$(pwd)
PLUGIN_DIR="/usr/local/lib"
PROC_DIR=${BUILD_HOME%/*/*}

# Step 1: Compile GUI source
make -j4

if [ $? -eq 0 ]; then
	sudo ln -s -f $BUILD_HOME/build/open-ephys /usr/bin/open-ephys
	echo "-----> GUI compile successful."
else
	echo "-----> GUI compile failed."
	exit
fi

# Step 2: Compile plugins
make -j4 -f Makefile.plugins

if [ $? -eq 0 ]; then
	echo "-----> Plugin installation sucessful."
else
	echo "-----> Plugin installation failed."
	exit
fi
