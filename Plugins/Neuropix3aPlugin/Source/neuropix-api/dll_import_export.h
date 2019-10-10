#ifndef dll_import_export_h_
#define dll_import_export_h_

#ifdef MSVC
#ifdef EXPORTING
#define DLL_IMPORT_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_IMPORT_EXPORT
#endif

#endif
