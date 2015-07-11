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
	sudo ln -s -f $BUILD_HOME/build/open-ephys /usr/bin/.
	echo "-----> GUI compile successful."
else
	echo "-----> GUI compile failed."
	exit
fi

# Step 2: Create GUI shared library
cd $BUILD_HOME/build/intermediate/Debug/
g++ -fPIC -shared -o libephys.so *.o
sudo rm /usr/lib/libephys.so
sudo cp libephys.so /usr/lib/.
ldconfig -n /usr/lib/

if [ $? -eq 0 ]; then
echo "-----> JUCE library installation sucessful."
else
echo "-----> JUCE library installation failed."
exit
fi

# Step 2: Compile plugins
PLUGIN_SRC_DIR="${PROC_DIR}/Source/Processors"
PLUGINS=`ls -d ${PLUGIN_SRC_DIR}/*`

cd $PLUGIN_SRC_DIR
for PLUGIN in ${PLUGINS}
do
	if [ -f $PLUGIN/Makefile ]; then
		cd $PLUGIN
		make clean
		make
		if [ $? -ne 0 ]; then
			echo "-----> Plugin compile failed."
			exit
		fi
		make install
		cd ..
	fi
done

if [ $? -eq 0 ]; then
	echo "-----> Plugin installation sucessful."
else
	echo "-----> Plugin installation failed."
	exit
fi
