#include "Utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cctype>

std::string OELogger::getModuleName() {
#ifdef _WIN32
    HMODULE hModule = nullptr;
    // Get handle to the module containing the current instruction pointer
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCTSTR)(void*)_ReturnAddress(),  // Cast return address properly
        &hModule);
    
    WCHAR modulePath[MAX_PATH + 256];
    if (GetModuleFileNameW(hModule, modulePath, (DWORD)(MAX_PATH + 256))) {
        // Convert WCHAR to std::string using UTF-8 encoding
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, modulePath, -1, NULL, 0, NULL, NULL);
        std::string result(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, modulePath, -1, &result[0], sizeNeeded, NULL, NULL);
        return formatModuleName(result);
    }
    return "[unknown]";
#else
    // macOS/Linux implementation
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(__builtin_return_address(0)), &info)) {
        if (info.dli_fname) {
            return formatModuleName(std::string(info.dli_fname));
        }
    }
    return "[unknown]";
#endif
}

std::string OELogger::formatModuleName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    std::string basename = path.substr(lastSlash + 1);
    
    // Remove .exe or .dll extension on Windows
    #ifdef _WIN32
    size_t lastDot = basename.find_last_of('.');
    if (lastDot != std::string::npos) {
        std::string ext = basename.substr(lastDot);
        if (_stricmp(ext.c_str(), ".exe") == 0 || _stricmp(ext.c_str(), ".dll") == 0) {
            basename = basename.substr(0, lastDot);
        }
    }
    #endif

    std::string formatted;
    for (size_t i = 0; i < basename.length(); ++i) {
        char ch = basename[i];
        if (std::isupper(ch)) {
            if (i > 0) {
                formatted += '-';
            }
            formatted += std::tolower(ch);
        } else {
            formatted += ch;
        }
    }
    return "[" + formatted + "]";
}