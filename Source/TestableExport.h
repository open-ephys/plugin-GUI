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
#define TESTABLE __attribute__((visibility("default")))
#endif
#else
#define TESTABLE
#endif



#endif
