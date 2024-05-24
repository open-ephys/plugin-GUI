/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

/*
    DLL Import/Export Macro needed for Open Ephys component testing.
    Required for Windows testing.

    TESTABLE should be added to any class in the gui_testable_source target that is used in a Unit Test.

    Test executable targets that link gui_testable_source need to define the flag TEST_RUNNER=1 to import
    TESTABLE symbols.
*/

#ifndef TESTABLEEXPORT_H
#define TESTABLEEXPORT_H

#ifdef BUILD_TESTS
#ifdef _WIN32
#ifdef TEST_RUNNER
#define TESTABLE __declspec(dllimport)
#else
#define TESTABLE __declspec(dllexport)
#endif
#else
#define TESTABLE __attribute__ ((visibility ("default")))
#endif
#else
#define TESTABLE
#endif

#endif
