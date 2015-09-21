//------------------------------------------------------------------------
// okFrontPanelDLL.c/cpp
//
// This is the import source for the FrontPanel API DLL.  If you are 
// building an application using the DLL, this source should be included
// within your C++/C project.  It includes methods that will
// automatically load the DLL and map function calls to the DLL entry
// points.
//
// This library is not necessary when you call the DLL methods from 
// another application or language such as LabVIEW or VisualBasic.
//
// This methods in this DLL correspond closely with the C++ API.
// Therefore, the C++ API documentation serves as the documentation for
// this DLL.
//
//
// NOTE: Before any API function calls are made, you MUST call:
//    okFrontPanelDLL_LoadLib
//
// When you are finished using the API methods, you should call:
//    okFrontPanelDLL_FreeLib
//
// The current DLL version can be retrieved by calling:
//    okFrontPanelDLL_GetVersionString
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2012 Opal Kelly Incorporated
// $Rev: 1910 $ $Date: 2013-12-29 13:21:42 -0800 (Sun, 29 Dec 2013) $
//------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include <stdexcept>

#include "okFrontPanelDLL.h"

#if defined(_WIN32)
	#include "windows.h"
	#if !defined(okLIB_NAME)
		#if defined(_UNICODE)
			#define okLIB_NAME L"okFrontPanel.dll"
		#else
			#define okLIB_NAME "okFrontPanel.dll"
		#endif
	#endif
#elif defined(__APPLE__)
	#include <dlfcn.h>
	#define okLIB_NAME "libokFrontPanel.dylib"
#elif defined(__linux__)
	#include <dlfcn.h>
	#define okLIB_NAME "./libokFrontPanel.so"
#elif defined(__QNX__)
	#include <dlfcn.h>
	#define okLIB_NAME "./libokFrontPanel.so.1"
#endif

typedef void   DLL;
static DLL    *hLib = NULL;
static char    VERSION_STRING[32];


static DLL_EP
dll_entrypoint(DLL *dll, const char *name)
{
#if defined(_WIN32)
	FARPROC proc;
	proc = GetProcAddress((HMODULE) dll, (LPCSTR) name);
	if (NULL == proc) {
		printf( "Failed to load %s. Error code %d\n", name, GetLastError() );
	}
	return((DLL_EP)proc);
#else
	void *handle = (void *)dll;
	DLL_EP ep;
	ep = (DLL_EP)dlsym(handle, name);
	return( (dlerror()==0) ? (ep) : ((DLL_EP)NULL) );
#endif
}	


#if defined(_WIN32) && defined(_UNICODE)
static DLL *
dll_load(okFP_dll_pchar libname)
{
	return((DLL *) LoadLibraryW(libname));
}
#elif defined(_WIN32)
static DLL *
dll_load(okFP_dll_pchar libname)
{
	return((DLL *) LoadLibrary(libname));
}
#else
static DLL *
dll_load(okFP_dll_pchar libname)
{
	DLL *dll;
	dll = dlopen(libname, RTLD_NOW);
	if (!dll)
		printf("%s\n", (char *)dlerror());
	return(dll);
}
#endif


static void
dll_unload(DLL *dll)
{
#if defined(_WIN32)
	HINSTANCE hInst = (HINSTANCE) dll;
	FreeLibrary(hInst);
#else
	void *handle = (void *)dll;
	dlclose(handle);
#endif
}



#ifdef __cplusplus

//------------------------------------------------------------------------
// okCPLL22150 C++ wrapper class
//------------------------------------------------------------------------
bool okCPLL22150::to_bool(Bool x)
	{ return( (x==TRUE)?(true):(false) ); }
Bool okCPLL22150::from_bool(bool x)
	{ return( (x==true)?(TRUE):(FALSE) ); }
okCPLL22150::okCPLL22150()
	{ h=okPLL22150_Construct(); }
void okCPLL22150::SetCrystalLoad(double capload)
	{ okPLL22150_SetCrystalLoad(h, capload); }
void okCPLL22150::SetReference(double freq, bool extosc)
	{ okPLL22150_SetReference(h, freq, from_bool(extosc)); }
double okCPLL22150::GetReference()
	{ return(okPLL22150_GetReference(h)); }
bool okCPLL22150::SetVCOParameters(int p, int q)
	{ return(to_bool(okPLL22150_SetVCOParameters(h,p,q))); }
int okCPLL22150::GetVCOP()
	{ return(okPLL22150_GetVCOP(h)); }
int okCPLL22150::GetVCOQ()
	{ return(okPLL22150_GetVCOQ(h)); }
double okCPLL22150::GetVCOFrequency()
	{ return(okPLL22150_GetVCOFrequency(h)); }
void okCPLL22150::SetDiv1(DividerSource divsrc, int n)
	{ okPLL22150_SetDiv1(h, (ok_DividerSource)divsrc, n); }
void okCPLL22150::SetDiv2(DividerSource divsrc, int n)
	{ okPLL22150_SetDiv2(h, (ok_DividerSource)divsrc, n); }
okCPLL22150::DividerSource okCPLL22150::GetDiv1Source()
	{ return((DividerSource) okPLL22150_GetDiv1Source(h)); }
okCPLL22150::DividerSource okCPLL22150::GetDiv2Source()
	{ return((DividerSource) okPLL22150_GetDiv2Source(h)); }
int okCPLL22150::GetDiv1Divider()
	{ return(okPLL22150_GetDiv1Divider(h)); }
int okCPLL22150::GetDiv2Divider()
	{ return(okPLL22150_GetDiv2Divider(h)); }
void okCPLL22150::SetOutputSource(int output, okCPLL22150::ClockSource clksrc)
	{ okPLL22150_SetOutputSource(h, output, (ok_ClockSource_22150)clksrc); }
void okCPLL22150::SetOutputEnable(int output, bool enable)
	{ okPLL22150_SetOutputEnable(h, output, to_bool(enable)); }
okCPLL22150::ClockSource okCPLL22150::GetOutputSource(int output)
	{ return( (ClockSource)okPLL22150_GetOutputSource(h, output)); }
double okCPLL22150::GetOutputFrequency(int output)
	{ return(okPLL22150_GetOutputFrequency(h, output)); }
bool okCPLL22150::IsOutputEnabled(int output)
	{ return(to_bool(okPLL22150_IsOutputEnabled(h, output))); }
void okCPLL22150::InitFromProgrammingInfo(unsigned char *buf)
	{ okPLL22150_InitFromProgrammingInfo(h, buf); }
void okCPLL22150::GetProgrammingInfo(unsigned char *buf)
	{ okPLL22150_GetProgrammingInfo(h, buf); }

//------------------------------------------------------------------------
// okCPLL22393 C++ wrapper class
//------------------------------------------------------------------------
bool okCPLL22393::to_bool(Bool x)
	{ return( (x==TRUE)?(true):(false) ); }
Bool okCPLL22393::from_bool(bool x)
	{ return( (x==true)?(TRUE):(FALSE) ); }
okCPLL22393::okCPLL22393()
	{ h=okPLL22393_Construct(); }
void okCPLL22393::SetCrystalLoad(double capload)
	{ okPLL22393_SetCrystalLoad(h, capload); }
void okCPLL22393::SetReference(double freq)
	{ okPLL22393_SetReference(h, freq); }
double okCPLL22393::GetReference()
	{ return(okPLL22393_GetReference(h)); }
bool okCPLL22393::SetPLLParameters(int n, int p, int q, bool enable)
	{ return(to_bool(okPLL22393_SetPLLParameters(h, n, p, q, from_bool(enable)))); }
bool okCPLL22393::SetPLLLF(int n, int lf)
	{ return(to_bool(okPLL22393_SetPLLLF(h, n, lf))); }
bool okCPLL22393::SetOutputDivider(int n, int div)
	{ return(to_bool(okPLL22393_SetOutputDivider(h, n, div))); }
bool okCPLL22393::SetOutputSource(int n, okCPLL22393::ClockSource clksrc)
	{ return(to_bool(okPLL22393_SetOutputSource(h, n, (ok_ClockSource_22393)clksrc))); }
void okCPLL22393::SetOutputEnable(int n, bool enable)
	{ okPLL22393_SetOutputEnable(h, n, from_bool(enable)); }
int okCPLL22393::GetPLLP(int n)
	{ return(okPLL22393_GetPLLP(h, n)); }
int okCPLL22393::GetPLLQ(int n)
	{ return(okPLL22393_GetPLLQ(h, n)); }
double okCPLL22393::GetPLLFrequency(int n)
	{ return(okPLL22393_GetPLLFrequency(h, n)); }
int okCPLL22393::GetOutputDivider(int n)
	{ return(okPLL22393_GetOutputDivider(h, n)); }
okCPLL22393::ClockSource okCPLL22393::GetOutputSource(int n)
	{ return((ClockSource) okPLL22393_GetOutputSource(h, n)); }
double okCPLL22393::GetOutputFrequency(int n)
	{ return(okPLL22393_GetOutputFrequency(h, n)); }
bool okCPLL22393::IsOutputEnabled(int n)
	{ return(to_bool(okPLL22393_IsOutputEnabled(h, n))); }
bool okCPLL22393::IsPLLEnabled(int n)
	{ return(to_bool(okPLL22393_IsPLLEnabled(h, n))); }
void okCPLL22393::InitFromProgrammingInfo(unsigned char *buf)
	{ okPLL22393_InitFromProgrammingInfo(h, buf); }
void okCPLL22393::GetProgrammingInfo(unsigned char *buf)
	{ okPLL22393_GetProgrammingInfo(h, buf); }

//------------------------------------------------------------------------
// okCFrontPanel C++ wrapper class
//------------------------------------------------------------------------
bool okCFrontPanel::to_bool(Bool x)
	{ return( (x==TRUE)?(true):(false) ); }
Bool okCFrontPanel::from_bool(bool x)
	{ return( (x==true)?(TRUE):(FALSE) ); }
okCFrontPanel::okCFrontPanel(okFrontPanel_HANDLE hnd)
	{ h=hnd; }
okCFrontPanel::okCFrontPanel()
	{ h=okFrontPanel_Construct(); }
okCFrontPanel::~okCFrontPanel()
	{ okFrontPanel_Destruct(h); }
int okCFrontPanel::GetHostInterfaceWidth()
	{ return(okFrontPanel_GetHostInterfaceWidth(h)); }
bool okCFrontPanel::IsHighSpeed()
	{ return(to_bool(okFrontPanel_IsHighSpeed(h))); }
okCFrontPanel::BoardModel okCFrontPanel::GetBoardModel()
	{ return((okCFrontPanel::BoardModel)okFrontPanel_GetBoardModel(h)); }
std::string okCFrontPanel::GetBoardModelString(okCFrontPanel::BoardModel m)
	{
		char str[MAX_BOARDMODELSTRING_LENGTH];
		okFrontPanel_GetBoardModelString(h, (ok_BoardModel)m, str);
		return(std::string(str));
	}
int okCFrontPanel::GetDeviceCount()
	{ return(okFrontPanel_GetDeviceCount(h)); }
okCFrontPanel::BoardModel okCFrontPanel::GetDeviceListModel(int num)
	{ return((okCFrontPanel::BoardModel)okFrontPanel_GetDeviceListModel(h, num)); }
std::string okCFrontPanel::GetDeviceListSerial(int num)
	{
		char str[MAX_SERIALNUMBER_LENGTH+1];
		okFrontPanel_GetDeviceListSerial(h, num, str);
		str[MAX_SERIALNUMBER_LENGTH] = '\0';
		return(std::string(str));
	}
okCFrontPanel::ErrorCode okCFrontPanel::GetDeviceInfo(okTDeviceInfo *info)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetDeviceInfo(h, info)); }
void okCFrontPanel::EnableAsynchronousTransfers(bool enable)
	{ okFrontPanel_EnableAsynchronousTransfers(h, to_bool(enable)); }
okCFrontPanel::ErrorCode okCFrontPanel::OpenBySerial(std::string str)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_OpenBySerial(h, str.c_str())); }
bool okCFrontPanel::IsOpen()
	{ return(to_bool(okFrontPanel_IsOpen(h))); }
int okCFrontPanel::GetDeviceMajorVersion()
	{ return(okFrontPanel_GetDeviceMajorVersion(h)); }
int okCFrontPanel::GetDeviceMinorVersion()
	{ return(okFrontPanel_GetDeviceMinorVersion(h)); }
std::string okCFrontPanel::GetSerialNumber()
	{
		char str[MAX_SERIALNUMBER_LENGTH+1];
		okFrontPanel_GetSerialNumber(h, str);
		return(std::string(str));
	}
okCFrontPanel::ErrorCode okCFrontPanel::GetDeviceSettings(okCDeviceSettings& settings)
	{
		return (ErrorCode)okFrontPanel_GetDeviceSettings(h, settings.h);
	}
std::string okCFrontPanel::GetDeviceID()
	{
		char str[MAX_DEVICEID_LENGTH+1];
		okFrontPanel_GetDeviceID(h, str);
		return(std::string(str));
	}
void okCFrontPanel::SetDeviceID(const std::string str)
	{ okFrontPanel_SetDeviceID(h, str.c_str()); }
okCFrontPanel::ErrorCode okCFrontPanel::SetBTPipePollingInterval(int interval)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetBTPipePollingInterval(h, interval)); }
void okCFrontPanel::SetTimeout(int timeout)
	{ okFrontPanel_SetTimeout(h, timeout); }
okCFrontPanel::ErrorCode okCFrontPanel::ResetFPGA()
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ResetFPGA(h)); }
void okCFrontPanel::Close()
	{ okFrontPanel_Close(h); }
okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGAFromMemory(unsigned char *data, const unsigned long length, void(*callback)(int, int, void *), void *arg)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGAFromMemory(h, data, length)); }
okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGA(const std::string strFilename, void (*callback)(int, int, void *), void *arg)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGA(h, strFilename.c_str())); }
okCFrontPanel::ErrorCode okCFrontPanel::GetFPGAResetProfile(okEFPGAConfigurationMethod method, okTFPGAResetProfile *profile)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetFPGAResetProfile(h, method, profile)); }
okCFrontPanel::ErrorCode okCFrontPanel::ReadRegister(UINT32 addr, UINT32 *data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ReadRegister(h, addr, data)); }
okCFrontPanel::ErrorCode okCFrontPanel::ReadRegisters(std::vector<okTRegisterEntry>& regs)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ReadRegisters(h, (unsigned int)regs.size(), regs.empty() ? NULL : &regs[0])); }
okCFrontPanel::ErrorCode okCFrontPanel::SetFPGAResetProfile(okEFPGAConfigurationMethod method, const okTFPGAResetProfile *profile)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetFPGAResetProfile(h, method, profile)); }
okCFrontPanel::ErrorCode okCFrontPanel::FlashEraseSector(UINT32 address)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_FlashEraseSector(h, address)); }
okCFrontPanel::ErrorCode okCFrontPanel::FlashWrite(UINT32 address, UINT32 length, const UINT8 *buf)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_FlashWrite(h, address, length, buf)); }
okCFrontPanel::ErrorCode okCFrontPanel::FlashRead(UINT32 address, UINT32 length, UINT8 *buf)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_FlashRead(h, address, length, buf)); }
okCFrontPanel::ErrorCode okCFrontPanel::WriteRegister(UINT32 addr, UINT32 data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_WriteRegister(h, addr, data)); }
okCFrontPanel::ErrorCode okCFrontPanel::WriteRegisters(const std::vector<okTRegisterEntry>& regs)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_WriteRegisters(h, (unsigned int)regs.size(), regs.empty() ? NULL : &regs[0])); }
okCFrontPanel::ErrorCode okCFrontPanel::GetWireInValue(int epAddr, UINT32 *val)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetWireInValue(h, epAddr, val)); }
okCFrontPanel::ErrorCode okCFrontPanel::WriteI2C(const int addr, int length, unsigned char *data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_WriteI2C(h, addr, length, data)); }
okCFrontPanel::ErrorCode okCFrontPanel::ReadI2C(const int addr, int length, unsigned char *data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ReadI2C(h, addr, length, data)); }
okCFrontPanel::ErrorCode okCFrontPanel::GetPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetPLL22150Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::SetPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetPLL22150Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::GetEepromPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetEepromPLL22150Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::SetEepromPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetEepromPLL22150Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::GetPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetPLL22393Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::SetPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetPLL22393Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::GetEepromPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetEepromPLL22393Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::SetEepromPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetEepromPLL22393Configuration(h, pll.h)); }
okCFrontPanel::ErrorCode okCFrontPanel::LoadDefaultPLLConfiguration()
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_LoadDefaultPLLConfiguration(h)); }
bool okCFrontPanel::IsFrontPanelEnabled()
	{ return(to_bool(okFrontPanel_IsFrontPanelEnabled(h))); }
bool okCFrontPanel::IsFrontPanel3Supported()
	{ return(to_bool(okFrontPanel_IsFrontPanel3Supported(h))); }
//	void UnregisterAll();
//	void AddEventHandler(EventHandler *handler);
void okCFrontPanel::UpdateWireIns()
	{ okFrontPanel_UpdateWireIns(h); }
okCFrontPanel::ErrorCode okCFrontPanel::SetWireInValue(int ep, UINT32 val, UINT32 mask)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetWireInValue(h, ep, val, mask)); }
void okCFrontPanel::UpdateWireOuts()
	{ okFrontPanel_UpdateWireOuts(h); }
unsigned long okCFrontPanel::GetWireOutValue(int epAddr)
	{ return(okFrontPanel_GetWireOutValue(h, epAddr)); }
okCFrontPanel::ErrorCode okCFrontPanel::ActivateTriggerIn(int epAddr, int bit)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ActivateTriggerIn(h, epAddr, bit)); }
void okCFrontPanel::UpdateTriggerOuts()
	{ okFrontPanel_UpdateTriggerOuts(h); }
bool okCFrontPanel::IsTriggered(int epAddr, UINT32 mask)
	{ return(to_bool(okFrontPanel_IsTriggered(h, epAddr, mask))); }
long okCFrontPanel::GetLastTransferLength()
	{ return(okFrontPanel_GetLastTransferLength(h)); }
long okCFrontPanel::WriteToPipeIn(int epAddr, long length, unsigned char *data)
	{ return(okFrontPanel_WriteToPipeIn(h, epAddr, length, data)); }
long okCFrontPanel::ReadFromPipeOut(int epAddr, long length, unsigned char *data)
	{ return(okFrontPanel_ReadFromPipeOut(h, epAddr, length, data)); }
long okCFrontPanel::WriteToBlockPipeIn(int epAddr, int blockSize, long length, unsigned char *data)
	{ return(okFrontPanel_WriteToBlockPipeIn(h, epAddr, blockSize, length, data)); }
long okCFrontPanel::ReadFromBlockPipeOut(int epAddr, int blockSize, long length, unsigned char *data)
	{ return(okFrontPanel_ReadFromBlockPipeOut(h, epAddr, blockSize, length, data)); }

//------------------------------------------------------------------------
// FrontPanelManagerHandle C++ wrapper class
//------------------------------------------------------------------------

okCFrontPanelManager::okCFrontPanelManager()
	{ h=okFrontPanelManager_Construct(reinterpret_cast<okFrontPanelManager_HANDLE>(this)); }
okCFrontPanelManager::~okCFrontPanelManager()
	{ okFrontPanelManager_Destruct(h); }

void okCFrontPanelManager::StartMonitoring()
{
	if (okFrontPanelManager_StartMonitoring(h) != ok_NoError)
		throw std::runtime_error("Failed to start monitoring devices connection.");
}

okCFrontPanel* okCFrontPanelManager::Open(const char *serial)
{
	okFrontPanel_HANDLE hFP = okFrontPanelManager_Open(h, serial);
	return hFP ? new okCFrontPanel(hFP) : NULL;
}

#endif // __cplusplus


//------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------
typedef void                   (DLL_ENTRY *OKFRONTPANELDLL_GETVERSION_FN)                  (char *, char *);

typedef okPLL22150_HANDLE      (DLL_ENTRY *OKPLL22150_CONSTRUCT_FN)                        (void);
typedef void                   (DLL_ENTRY *OKPLL22150_DESTRUCT_FN)                         (okPLL22150_HANDLE);
typedef void                   (DLL_ENTRY *OKPLL22150_SETCRYSTALLOAD_FN)                   (okPLL22150_HANDLE, double);
typedef void                   (DLL_ENTRY *OKPLL22150_SETREFERENCE_FN)                     (okPLL22150_HANDLE, double, Bool);
typedef double                 (DLL_ENTRY *OKPLL22150_GETREFERENCE_FN)                     (okPLL22150_HANDLE);
typedef Bool                   (DLL_ENTRY *OKPLL22150_SETVCOPARAMETERS_FN)                 (okPLL22150_HANDLE, int, int);
typedef int                    (DLL_ENTRY *OKPLL22150_GETVCOP_FN)                          (okPLL22150_HANDLE);
typedef int                    (DLL_ENTRY *OKPLL22150_GETVCOQ_FN)                          (okPLL22150_HANDLE);
typedef double                 (DLL_ENTRY *OKPLL22150_GETVCOFREQUENCY_FN)                  (okPLL22150_HANDLE);
typedef void                   (DLL_ENTRY *OKPLL22150_SETDIV1_FN)                          (okPLL22150_HANDLE, ok_DividerSource, int);
typedef void                   (DLL_ENTRY *OKPLL22150_SETDIV2_FN)                          (okPLL22150_HANDLE, ok_DividerSource, int);
typedef ok_DividerSource       (DLL_ENTRY *OKPLL22150_GETDIV1SOURCE_FN)                    (okPLL22150_HANDLE);
typedef ok_DividerSource       (DLL_ENTRY *OKPLL22150_GETDIV2SOURCE_FN)                    (okPLL22150_HANDLE);
typedef int                    (DLL_ENTRY *OKPLL22150_GETDIV1DIVIDER_FN)                   (okPLL22150_HANDLE);
typedef int                    (DLL_ENTRY *OKPLL22150_GETDIV2DIVIDER_FN)                   (okPLL22150_HANDLE);
typedef void                   (DLL_ENTRY *OKPLL22150_SETOUTPUTSOURCE_FN)                  (okPLL22150_HANDLE, int, ok_ClockSource_22150);
typedef void                   (DLL_ENTRY *OKPLL22150_SETOUTPUTENABLE_FN)                  (okPLL22150_HANDLE, int, Bool);
typedef ok_ClockSource_22150   (DLL_ENTRY *OKPLL22150_GETOUTPUTSOURCE_FN)                  (okPLL22150_HANDLE, int);
typedef double                 (DLL_ENTRY *OKPLL22150_GETOUTPUTFREQUENCY_FN)               (okPLL22150_HANDLE, int);
typedef Bool                   (DLL_ENTRY *OKPLL22150_ISOUTPUTENABLED_FN)                  (okPLL22150_HANDLE, int);
typedef void                   (DLL_ENTRY *OKPLL22150_INITFROMPROGRAMMINGINFO_FN)          (okPLL22150_HANDLE, unsigned char *);
typedef void                   (DLL_ENTRY *OKPLL22150_GETPROGRAMMINGINFO_FN)               (okPLL22150_HANDLE, unsigned char *);

typedef okPLL22393_HANDLE      (DLL_ENTRY *OKPLL22393_CONSTRUCT_FN)                        (void);
typedef void                   (DLL_ENTRY *OKPLL22393_DESTRUCT_FN)                         (okPLL22393_HANDLE);
typedef void                   (DLL_ENTRY *OKPLL22393_SETCRYSTALLOAD_FN)                   (okPLL22393_HANDLE, double);
typedef void                   (DLL_ENTRY *OKPLL22393_SETREFERENCE_FN)                     (okPLL22393_HANDLE, double);
typedef double                 (DLL_ENTRY *OKPLL22393_GETREFERENCE_FN)                     (okPLL22393_HANDLE);
typedef Bool                   (DLL_ENTRY *OKPLL22393_SETPLLPARAMETERS_FN)                 (okPLL22393_HANDLE, int, int, int, Bool);
typedef Bool                   (DLL_ENTRY *OKPLL22393_SETPLLLF_FN)                         (okPLL22393_HANDLE, int, int);
typedef Bool                   (DLL_ENTRY *OKPLL22393_SETOUTPUTDIVIDER_FN)                 (okPLL22393_HANDLE, int, int);
typedef Bool                   (DLL_ENTRY *OKPLL22393_SETOUTPUTSOURCE_FN)                  (okPLL22393_HANDLE, int, ok_ClockSource_22393);
typedef void                   (DLL_ENTRY *OKPLL22393_SETOUTPUTENABLE_FN)                  (okPLL22393_HANDLE, int, Bool);
typedef int                    (DLL_ENTRY *OKPLL22393_GETPLLP_FN)                          (okPLL22393_HANDLE, int);
typedef int                    (DLL_ENTRY *OKPLL22393_GETPLLQ_FN)                          (okPLL22393_HANDLE, int);
typedef double                 (DLL_ENTRY *OKPLL22393_GETPLLFREQUENCY_FN)                  (okPLL22393_HANDLE, int);
typedef int                    (DLL_ENTRY *OKPLL22393_GETOUTPUTDIVIDER_FN)                 (okPLL22393_HANDLE, int);
typedef ok_ClockSource_22393   (DLL_ENTRY *OKPLL22393_GETOUTPUTSOURCE_FN)                  (okPLL22393_HANDLE, int);
typedef double                 (DLL_ENTRY *OKPLL22393_GETOUTPUTFREQUENCY_FN)               (okPLL22393_HANDLE, int);
typedef Bool                   (DLL_ENTRY *OKPLL22393_ISOUTPUTENABLED_FN)                  (okPLL22393_HANDLE, int);
typedef Bool                   (DLL_ENTRY *OKPLL22393_ISPLLENABLED_FN)                     (okPLL22393_HANDLE, int);
typedef void                   (DLL_ENTRY *OKPLL22393_INITFROMPROGRAMMINGINFO_FN)          (okPLL22393_HANDLE, unsigned char *);
typedef void                   (DLL_ENTRY *OKPLL22393_GETPROGRAMMINGINFO_FN)               (okPLL22393_HANDLE, unsigned char *);

typedef okDeviceSettings_HANDLE (DLL_ENTRY *okDeviceSettings_CONSTRUCT_FN)                 (void);
typedef void                    (DLL_ENTRY *okDeviceSettings_DESTRUCT_FN)                  (okDeviceSettings_HANDLE);
typedef ok_ErrorCode            (DLL_ENTRY *okDeviceSettings_GETSTRING_FN)                 (okDeviceSettings_HANDLE, const char *, int, char*);
typedef ok_ErrorCode            (DLL_ENTRY *okDeviceSettings_SETSTRING_FN)                 (okDeviceSettings_HANDLE, const char *, const char*);
typedef ok_ErrorCode            (DLL_ENTRY *okDeviceSettings_GETINT_FN)                    (okDeviceSettings_HANDLE, const char *, UINT32*);
typedef ok_ErrorCode            (DLL_ENTRY *okDeviceSettings_SETINT_FN)                    (okDeviceSettings_HANDLE, const char *, UINT32);
typedef ok_ErrorCode            (DLL_ENTRY *okDeviceSettings_DELETE_FN)                    (okDeviceSettings_HANDLE, const char *);
typedef ok_ErrorCode            (DLL_ENTRY *okDeviceSettings_SAVE_FN)                      (okDeviceSettings_HANDLE);

typedef okFrontPanel_HANDLE (DLL_ENTRY *okFrontPanel_CONSTRUCT_FN)                        (void);
typedef void                   (DLL_ENTRY *okFrontPanel_DESTRUCT_FN)                         (okFrontPanel_HANDLE);
typedef int                    (DLL_ENTRY *okFrontPanel_GETHOSTINTERFACEWIDTH_FN)            (okFrontPanel_HANDLE);
typedef Bool                   (DLL_ENTRY *okFrontPanel_ISHIGHSPEED_FN)                      (okFrontPanel_HANDLE);
typedef ok_BoardModel          (DLL_ENTRY *okFrontPanel_GETBOARDMODEL_FN)                    (okFrontPanel_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanel_GETBOARDMODELSTRING_FN)              (okFrontPanel_HANDLE, ok_BoardModel, char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_WRITEI2C_FN)                         (okFrontPanel_HANDLE, const int, int, unsigned char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_READI2C_FN)                          (okFrontPanel_HANDLE, const int, int, unsigned char *);
typedef int                    (DLL_ENTRY *okFrontPanel_GETDEVICECOUNT_FN)                   (okFrontPanel_HANDLE);
typedef ok_BoardModel          (DLL_ENTRY *okFrontPanel_GETDEVICELISTMODEL_FN)               (okFrontPanel_HANDLE, int);
typedef void                   (DLL_ENTRY *okFrontPanel_GETDEVICELISTSERIAL_FN)              (okFrontPanel_HANDLE, int, char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_OPENBYSERIAL_FN)                     (okFrontPanel_HANDLE, const char *);
typedef Bool                   (DLL_ENTRY *okFrontPanel_ISOPEN_FN)                           (okFrontPanel_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanel_ENABLEASYNCHRONOUSTRANSFERS_FN)      (okFrontPanel_HANDLE, Bool);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETBTPIPEPOLLINGINTERVAL_FN)         (okFrontPanel_HANDLE, int);
typedef void                   (DLL_ENTRY *okFrontPanel_SETTIMEOUT_FN)                       (okFrontPanel_HANDLE, int);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETDEVICEINFO_FN)                    (okFrontPanel_HANDLE, okTDeviceInfo *);
typedef int                    (DLL_ENTRY *okFrontPanel_GETDEVICEMAJORVERSION_FN)            (okFrontPanel_HANDLE);
typedef int                    (DLL_ENTRY *okFrontPanel_GETDEVICEMINORVERSION_FN)            (okFrontPanel_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_RESETFPGA_FN)                        (okFrontPanel_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanel_CLOSE_FN)                            (okFrontPanel_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanel_GETSERIALNUMBER_FN)                  (okFrontPanel_HANDLE, char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETDEVICESETTINGS_FN)                (okFrontPanel_HANDLE, okDeviceSettings_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanel_GETDEVICEID_FN)                      (okFrontPanel_HANDLE, char *);
typedef void                   (DLL_ENTRY *okFrontPanel_SETDEVICEID_FN)                      (okFrontPanel_HANDLE, const char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_CONFIGUREFPGA_FN)                    (okFrontPanel_HANDLE, const char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_CONFIGUREFPGAFROMMEMORY_FN)          (okFrontPanel_HANDLE, unsigned char *, unsigned long);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETPLL22150CONFIGURATION_FN)         (okFrontPanel_HANDLE, okPLL22150_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETPLL22150CONFIGURATION_FN)         (okFrontPanel_HANDLE, okPLL22150_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETEEPROMPLL22150CONFIGURATION_FN)   (okFrontPanel_HANDLE, okPLL22150_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETEEPROMPLL22150CONFIGURATION_FN)   (okFrontPanel_HANDLE, okPLL22150_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETPLL22393CONFIGURATION_FN)         (okFrontPanel_HANDLE, okPLL22393_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETPLL22393CONFIGURATION_FN)         (okFrontPanel_HANDLE, okPLL22393_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETEEPROMPLL22393CONFIGURATION_FN)   (okFrontPanel_HANDLE, okPLL22393_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETEEPROMPLL22393CONFIGURATION_FN)   (okFrontPanel_HANDLE, okPLL22393_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_LOADDEFAULTPLLCONFIGURATION_FN)      (okFrontPanel_HANDLE);
typedef Bool                   (DLL_ENTRY *okFrontPanel_ISFRONTPANELENABLED_FN)              (okFrontPanel_HANDLE);
typedef Bool                   (DLL_ENTRY *okFrontPanel_ISFRONTPANEL3SUPPORTED_FN)           (okFrontPanel_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanel_UPDATEWIREINS_FN)                    (okFrontPanel_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETWIREINVALUE_FN)                   (okFrontPanel_HANDLE, int, unsigned long, unsigned long);
typedef void                   (DLL_ENTRY *okFrontPanel_UPDATEWIREOUTS_FN)                   (okFrontPanel_HANDLE);
typedef unsigned long          (DLL_ENTRY *okFrontPanel_GETWIREOUTVALUE_FN)                  (okFrontPanel_HANDLE, int);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_ACTIVATETRIGGERIN_FN)                (okFrontPanel_HANDLE, int, int);
typedef void                   (DLL_ENTRY *okFrontPanel_UPDATETRIGGEROUTS_FN)                (okFrontPanel_HANDLE);
typedef Bool                   (DLL_ENTRY *okFrontPanel_ISTRIGGERED_FN)                      (okFrontPanel_HANDLE, int, unsigned long);
typedef long                   (DLL_ENTRY *okFrontPanel_GETLASTTRANSFERLENGTH_FN)            (okFrontPanel_HANDLE);
typedef long                   (DLL_ENTRY *okFrontPanel_WRITETOPIPEIN_FN)                    (okFrontPanel_HANDLE, int, long, unsigned char *);
typedef long                   (DLL_ENTRY *okFrontPanel_WRITETOBLOCKPIPEIN_FN)               (okFrontPanel_HANDLE, int, long, int, unsigned char *);
typedef long                   (DLL_ENTRY *okFrontPanel_READFROMPIPEOUT_FN)                  (okFrontPanel_HANDLE, int, long, unsigned char *);
typedef long                   (DLL_ENTRY *okFrontPanel_READFROMBLOCKPIPEOUT_FN)             (okFrontPanel_HANDLE, int, long, int, unsigned char *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_FLASHERASESECTOR_FN)                 (okFrontPanel_HANDLE, UINT32);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_FLASHWRITE_FN)                       (okFrontPanel_HANDLE, UINT32, UINT32, const UINT8 *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_FLASHREAD_FN)                        (okFrontPanel_HANDLE, UINT32, UINT32, UINT8 *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETFPGARESETPROFILE_FN)              (okFrontPanel_HANDLE, okEFPGAConfigurationMethod, okTFPGAResetProfile *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_SETFPGARESETPROFILE_FN)              (okFrontPanel_HANDLE, okEFPGAConfigurationMethod, const okTFPGAResetProfile *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_READREGISTER_FN)                     (okFrontPanel_HANDLE, UINT32, UINT32 *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_READREGISTERS_FN)                    (okFrontPanel_HANDLE, unsigned, okTRegisterEntry *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_WRITEREGISTER_FN)                    (okFrontPanel_HANDLE, UINT32, UINT32);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_WRITEREGISTERS_FN)                   (okFrontPanel_HANDLE, unsigned, const okTRegisterEntry *);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanel_GETWIREINVALUE_FN)                   (okFrontPanel_HANDLE, int, UINT32 *);

typedef okCFrontPanelManager_HANDLE (DLL_ENTRY *okFrontPanelManager_CONSTRUCT_FN)            (okFrontPanelManager_HANDLE);
typedef void                   (DLL_ENTRY *okFrontPanelManager_DESTRUCT_FN)                  (okCFrontPanelManager_HANDLE);
typedef ok_ErrorCode           (DLL_ENTRY *okFrontPanelManager_STARTMONITORING_FN)           (okCFrontPanelManager_HANDLE);
typedef okFrontPanel_HANDLE    (DLL_ENTRY *okFrontPanelManager_OPEN_FN)                      (okCFrontPanelManager_HANDLE, const char *);

//------------------------------------------------------------------------
// Function pointers
//------------------------------------------------------------------------
OKFRONTPANELDLL_GETVERSION_FN                  _okFrontPanelDLL_GetVersion = NULL;

OKPLL22393_CONSTRUCT_FN                        _okPLL22393_Construct = NULL;
OKPLL22393_DESTRUCT_FN                         _okPLL22393_Destruct = NULL;
OKPLL22393_SETCRYSTALLOAD_FN                   _okPLL22393_SetCrystalLoad = NULL;
OKPLL22393_SETREFERENCE_FN                     _okPLL22393_SetReference = NULL;
OKPLL22393_GETREFERENCE_FN                     _okPLL22393_GetReference = NULL;
OKPLL22393_SETPLLPARAMETERS_FN                 _okPLL22393_SetPLLParameters = NULL;
OKPLL22393_SETPLLLF_FN                         _okPLL22393_SetPLLLF = NULL;
OKPLL22393_SETOUTPUTDIVIDER_FN                 _okPLL22393_SetOutputDivider = NULL;
OKPLL22393_SETOUTPUTSOURCE_FN                  _okPLL22393_SetOutputSource = NULL;
OKPLL22393_SETOUTPUTENABLE_FN                  _okPLL22393_SetOutputEnable = NULL;
OKPLL22393_GETPLLP_FN                          _okPLL22393_GetPLLP = NULL;
OKPLL22393_GETPLLQ_FN                          _okPLL22393_GetPLLQ = NULL;
OKPLL22393_GETPLLFREQUENCY_FN                  _okPLL22393_GetPLLFrequency = NULL;
OKPLL22393_GETOUTPUTDIVIDER_FN                 _okPLL22393_GetOutputDivider = NULL;
OKPLL22393_GETOUTPUTSOURCE_FN                  _okPLL22393_GetOutputSource = NULL;
OKPLL22393_GETOUTPUTFREQUENCY_FN               _okPLL22393_GetOutputFrequency = NULL;
OKPLL22393_ISOUTPUTENABLED_FN                  _okPLL22393_IsOutputEnabled = NULL;
OKPLL22393_ISPLLENABLED_FN                     _okPLL22393_IsPLLEnabled = NULL;
OKPLL22393_INITFROMPROGRAMMINGINFO_FN          _okPLL22393_InitFromProgrammingInfo = NULL;
OKPLL22393_GETPROGRAMMINGINFO_FN               _okPLL22393_GetProgrammingInfo = NULL;

OKPLL22150_CONSTRUCT_FN                        _okPLL22150_Construct = NULL;
OKPLL22150_DESTRUCT_FN                         _okPLL22150_Destruct = NULL;
OKPLL22150_SETCRYSTALLOAD_FN                   _okPLL22150_SetCrystalLoad = NULL;
OKPLL22150_SETREFERENCE_FN                     _okPLL22150_SetReference = NULL;
OKPLL22150_GETREFERENCE_FN                     _okPLL22150_GetReference = NULL;
OKPLL22150_SETVCOPARAMETERS_FN                 _okPLL22150_SetVCOParameters = NULL;
OKPLL22150_GETVCOP_FN                          _okPLL22150_GetVCOP = NULL;
OKPLL22150_GETVCOQ_FN                          _okPLL22150_GetVCOQ = NULL;
OKPLL22150_GETVCOFREQUENCY_FN                  _okPLL22150_GetVCOFrequency = NULL;
OKPLL22150_SETDIV1_FN                          _okPLL22150_SetDiv1 = NULL;
OKPLL22150_SETDIV2_FN                          _okPLL22150_SetDiv2 = NULL;
OKPLL22150_GETDIV1SOURCE_FN                    _okPLL22150_GetDiv1Source = NULL;
OKPLL22150_GETDIV2SOURCE_FN                    _okPLL22150_GetDiv2Source = NULL;
OKPLL22150_GETDIV1DIVIDER_FN                   _okPLL22150_GetDiv1Divider = NULL;
OKPLL22150_GETDIV2DIVIDER_FN                   _okPLL22150_GetDiv2Divider = NULL;
OKPLL22150_SETOUTPUTSOURCE_FN                  _okPLL22150_SetOutputSource = NULL;
OKPLL22150_SETOUTPUTENABLE_FN                  _okPLL22150_SetOutputEnable = NULL;
OKPLL22150_GETOUTPUTSOURCE_FN                  _okPLL22150_GetOutputSource = NULL;
OKPLL22150_GETOUTPUTFREQUENCY_FN               _okPLL22150_GetOutputFrequency = NULL;
OKPLL22150_ISOUTPUTENABLED_FN                  _okPLL22150_IsOutputEnabled = NULL;
OKPLL22150_INITFROMPROGRAMMINGINFO_FN          _okPLL22150_InitFromProgrammingInfo = NULL;
OKPLL22150_GETPROGRAMMINGINFO_FN               _okPLL22150_GetProgrammingInfo = NULL;

okDeviceSettings_CONSTRUCT_FN                  _okDeviceSettings_Construct = NULL;
okDeviceSettings_DESTRUCT_FN                   _okDeviceSettings_Destruct = NULL;
okDeviceSettings_GETSTRING_FN                  _okDeviceSettings_GetString = NULL;
okDeviceSettings_SETSTRING_FN                  _okDeviceSettings_SetString = NULL;
okDeviceSettings_GETINT_FN                     _okDeviceSettings_GetInt = NULL;
okDeviceSettings_SETINT_FN                     _okDeviceSettings_SetInt = NULL;
okDeviceSettings_DELETE_FN                     _okDeviceSettings_Delete = NULL;
okDeviceSettings_SAVE_FN                       _okDeviceSettings_Save = NULL;

okFrontPanel_CONSTRUCT_FN                        _okFrontPanel_Construct = NULL;
okFrontPanel_DESTRUCT_FN                         _okFrontPanel_Destruct = NULL;
okFrontPanel_GETHOSTINTERFACEWIDTH_FN            _okFrontPanel_GetHostInterfaceWidth = NULL;
okFrontPanel_ISHIGHSPEED_FN                      _okFrontPanel_IsHighSpeed = NULL;
okFrontPanel_GETBOARDMODEL_FN                    _okFrontPanel_GetBoardModel = NULL;
okFrontPanel_GETBOARDMODELSTRING_FN              _okFrontPanel_GetBoardModelString = NULL;
okFrontPanel_WRITEI2C_FN                         _okFrontPanel_WriteI2C = NULL;
okFrontPanel_READI2C_FN                          _okFrontPanel_ReadI2C = NULL;
okFrontPanel_GETDEVICECOUNT_FN                   _okFrontPanel_GetDeviceCount = NULL;
okFrontPanel_GETDEVICELISTMODEL_FN               _okFrontPanel_GetDeviceListModel = NULL;
okFrontPanel_GETDEVICELISTSERIAL_FN              _okFrontPanel_GetDeviceListSerial = NULL;
okFrontPanel_OPENBYSERIAL_FN                     _okFrontPanel_OpenBySerial = NULL;
okFrontPanel_ISOPEN_FN                           _okFrontPanel_IsOpen = NULL; 
okFrontPanel_ENABLEASYNCHRONOUSTRANSFERS_FN      _okFrontPanel_EnableAsynchronousTransfers = NULL;
okFrontPanel_SETBTPIPEPOLLINGINTERVAL_FN         _okFrontPanel_SetBTPipePollingInterval = NULL;
okFrontPanel_SETTIMEOUT_FN                       _okFrontPanel_SetTimeout = NULL;
okFrontPanel_GETDEVICEINFO_FN                    _okFrontPanel_GetDeviceInfo = NULL;
okFrontPanel_GETDEVICEMAJORVERSION_FN            _okFrontPanel_GetDeviceMajorVersion = NULL;
okFrontPanel_GETDEVICEMINORVERSION_FN            _okFrontPanel_GetDeviceMinorVersion = NULL;
okFrontPanel_RESETFPGA_FN                        _okFrontPanel_ResetFPGA = NULL;
okFrontPanel_CLOSE_FN                            _okFrontPanel_Close = NULL;
okFrontPanel_GETSERIALNUMBER_FN                  _okFrontPanel_GetSerialNumber = NULL;
okFrontPanel_GETDEVICESETTINGS_FN                _okFrontPanel_GetDeviceSettings = NULL;
okFrontPanel_GETDEVICEID_FN                      _okFrontPanel_GetDeviceID = NULL;
okFrontPanel_SETDEVICEID_FN                      _okFrontPanel_SetDeviceID = NULL;
okFrontPanel_CONFIGUREFPGA_FN                    _okFrontPanel_ConfigureFPGA = NULL;
okFrontPanel_CONFIGUREFPGAFROMMEMORY_FN          _okFrontPanel_ConfigureFPGAFromMemory = NULL;
okFrontPanel_GETPLL22150CONFIGURATION_FN         _okFrontPanel_GetPLL22150Configuration = NULL;
okFrontPanel_SETPLL22150CONFIGURATION_FN         _okFrontPanel_SetPLL22150Configuration = NULL;
okFrontPanel_GETEEPROMPLL22150CONFIGURATION_FN   _okFrontPanel_GetEepromPLL22150Configuration = NULL;
okFrontPanel_SETEEPROMPLL22150CONFIGURATION_FN   _okFrontPanel_SetEepromPLL22150Configuration = NULL;
okFrontPanel_GETPLL22393CONFIGURATION_FN         _okFrontPanel_GetPLL22393Configuration = NULL;
okFrontPanel_SETPLL22393CONFIGURATION_FN         _okFrontPanel_SetPLL22393Configuration = NULL;
okFrontPanel_GETEEPROMPLL22393CONFIGURATION_FN   _okFrontPanel_GetEepromPLL22393Configuration = NULL;
okFrontPanel_SETEEPROMPLL22393CONFIGURATION_FN   _okFrontPanel_SetEepromPLL22393Configuration = NULL;
okFrontPanel_LOADDEFAULTPLLCONFIGURATION_FN      _okFrontPanel_LoadDefaultPLLConfiguration = NULL;
okFrontPanel_ISFRONTPANELENABLED_FN              _okFrontPanel_IsFrontPanelEnabled = NULL;
okFrontPanel_ISFRONTPANEL3SUPPORTED_FN           _okFrontPanel_IsFrontPanel3Supported = NULL;
okFrontPanel_UPDATEWIREINS_FN                    _okFrontPanel_UpdateWireIns = NULL;
okFrontPanel_SETWIREINVALUE_FN                   _okFrontPanel_SetWireInValue = NULL;
okFrontPanel_UPDATEWIREOUTS_FN                   _okFrontPanel_UpdateWireOuts = NULL;
okFrontPanel_GETWIREOUTVALUE_FN                  _okFrontPanel_GetWireOutValue = NULL;
okFrontPanel_ACTIVATETRIGGERIN_FN                _okFrontPanel_ActivateTriggerIn = NULL;
okFrontPanel_UPDATETRIGGEROUTS_FN                _okFrontPanel_UpdateTriggerOuts = NULL;
okFrontPanel_ISTRIGGERED_FN                      _okFrontPanel_IsTriggered = NULL;
okFrontPanel_GETLASTTRANSFERLENGTH_FN            _okFrontPanel_GetLastTransferLength = NULL;
okFrontPanel_WRITETOPIPEIN_FN                    _okFrontPanel_WriteToPipeIn = NULL;
okFrontPanel_WRITETOBLOCKPIPEIN_FN               _okFrontPanel_WriteToBlockPipeIn = NULL;
okFrontPanel_READFROMPIPEOUT_FN                  _okFrontPanel_ReadFromPipeOut = NULL;
okFrontPanel_READFROMBLOCKPIPEOUT_FN             _okFrontPanel_ReadFromBlockPipeOut = NULL;
okFrontPanel_FLASHERASESECTOR_FN                 _okFrontPanel_FlashEraseSector = NULL;
okFrontPanel_FLASHWRITE_FN                       _okFrontPanel_FlashWrite = NULL;
okFrontPanel_FLASHREAD_FN                        _okFrontPanel_FlashRead = NULL;
okFrontPanel_GETFPGARESETPROFILE_FN              _okFrontPanel_GetFPGAResetProfile = NULL;
okFrontPanel_SETFPGARESETPROFILE_FN              _okFrontPanel_SetFPGAResetProfile = NULL;
okFrontPanel_READREGISTER_FN                     _okFrontPanel_ReadRegister = NULL;
okFrontPanel_READREGISTERS_FN                    _okFrontPanel_ReadRegisters = NULL;
okFrontPanel_WRITEREGISTER_FN                    _okFrontPanel_WriteRegister = NULL;
okFrontPanel_WRITEREGISTERS_FN                   _okFrontPanel_WriteRegisters = NULL;
okFrontPanel_GETWIREINVALUE_FN                   _okFrontPanel_GetWireInValue = NULL;

okFrontPanelManager_CONSTRUCT_FN                 _okFrontPanelManager_Construct = NULL;
okFrontPanelManager_DESTRUCT_FN                  _okFrontPanelManager_Destruct = NULL;
okFrontPanelManager_STARTMONITORING_FN           _okFrontPanelManager_StartMonitoring = NULL;
okFrontPanelManager_OPEN_FN                      _okFrontPanelManager_Open = NULL;

//------------------------------------------------------------------------

/// Returns the version number of the DLL.
const char *
okFrontPanelDLL_GetVersionString()
{
	return(VERSION_STRING);
}


/// Loads the FrontPanel API DLL.  This function returns False if the 
/// DLL did not load for some reason, True otherwise.
Bool
okFrontPanelDLL_LoadLib(okFP_dll_pchar libname)
{
	// Return TRUE if the DLL is already loaded.
	if (hLib)
		return(TRUE);

	if (NULL == libname)
		hLib = dll_load(okLIB_NAME);
	else
		hLib = dll_load(libname);

	if (hLib) {
		_okFrontPanelDLL_GetVersion                   = ( OKFRONTPANELDLL_GETVERSION_FN )                    dll_entrypoint( hLib, "okFrontPanelDLL_GetVersion" );

		_okPLL22150_Construct                         = ( OKPLL22150_CONSTRUCT_FN )                          dll_entrypoint( hLib, "okPLL22150_Construct" );
		_okPLL22150_Destruct                          = ( OKPLL22150_DESTRUCT_FN )                           dll_entrypoint( hLib, "okPLL22150_Destruct" );
		_okPLL22150_SetCrystalLoad                    = ( OKPLL22150_SETCRYSTALLOAD_FN )                     dll_entrypoint( hLib, "okPLL22150_SetCrystalLoad" );
		_okPLL22150_SetReference                      = ( OKPLL22150_SETREFERENCE_FN )                       dll_entrypoint( hLib, "okPLL22150_SetReference" );
		_okPLL22150_GetReference                      = ( OKPLL22150_GETREFERENCE_FN )                       dll_entrypoint( hLib, "okPLL22150_GetReference" );
		_okPLL22150_SetVCOParameters                  = ( OKPLL22150_SETVCOPARAMETERS_FN )                   dll_entrypoint( hLib, "okPLL22150_SetVCOParameters" );
		_okPLL22150_GetVCOP                           = ( OKPLL22150_GETVCOP_FN )                            dll_entrypoint( hLib, "okPLL22150_GetVCOP" );
		_okPLL22150_GetVCOQ                           = ( OKPLL22150_GETVCOQ_FN )                            dll_entrypoint( hLib, "okPLL22150_GetVCOQ" );
		_okPLL22150_GetVCOFrequency                   = ( OKPLL22150_GETVCOFREQUENCY_FN )                    dll_entrypoint( hLib, "okPLL22150_GetVCOFrequency" );
		_okPLL22150_SetDiv1                           = ( OKPLL22150_SETDIV1_FN )                            dll_entrypoint( hLib, "okPLL22150_SetDiv1" );
		_okPLL22150_SetDiv2                           = ( OKPLL22150_SETDIV2_FN )                            dll_entrypoint( hLib, "okPLL22150_SetDiv2" );
		_okPLL22150_GetDiv1Source                     = ( OKPLL22150_GETDIV1SOURCE_FN )                      dll_entrypoint( hLib, "okPLL22150_GetDiv1Source" );
		_okPLL22150_GetDiv2Source                     = ( OKPLL22150_GETDIV2SOURCE_FN )                      dll_entrypoint( hLib, "okPLL22150_GetDiv2Source" );
		_okPLL22150_GetDiv1Divider                    = ( OKPLL22150_GETDIV1DIVIDER_FN )                     dll_entrypoint( hLib, "okPLL22150_GetDiv1Divider" );
		_okPLL22150_GetDiv2Divider                    = ( OKPLL22150_GETDIV2DIVIDER_FN )                     dll_entrypoint( hLib, "okPLL22150_GetDiv2Divider" );
		_okPLL22150_SetOutputSource                   = ( OKPLL22150_SETOUTPUTSOURCE_FN )                    dll_entrypoint( hLib, "okPLL22150_SetOutputSource" );
		_okPLL22150_SetOutputEnable                   = ( OKPLL22150_SETOUTPUTENABLE_FN )                    dll_entrypoint( hLib, "okPLL22150_SetOutputEnable" );
		_okPLL22150_GetOutputSource                   = ( OKPLL22150_GETOUTPUTSOURCE_FN )                    dll_entrypoint( hLib, "okPLL22150_GetOutputSource" );
		_okPLL22150_GetOutputFrequency                = ( OKPLL22150_GETOUTPUTFREQUENCY_FN )                 dll_entrypoint( hLib, "okPLL22150_GetOutputFrequency" );
		_okPLL22150_IsOutputEnabled                   = ( OKPLL22150_ISOUTPUTENABLED_FN )                    dll_entrypoint( hLib, "okPLL22150_IsOutputEnabled" );
		_okPLL22150_InitFromProgrammingInfo           = ( OKPLL22150_INITFROMPROGRAMMINGINFO_FN )            dll_entrypoint( hLib, "okPLL22150_InitFromProgrammingInfo" );
		_okPLL22150_GetProgrammingInfo                = ( OKPLL22150_GETPROGRAMMINGINFO_FN )                 dll_entrypoint( hLib, "okPLL22150_GetProgrammingInfo" );

		_okPLL22393_Construct                         = ( OKPLL22393_CONSTRUCT_FN )                          dll_entrypoint( hLib, "okPLL22393_Construct" );
		_okPLL22393_Destruct                          = ( OKPLL22393_DESTRUCT_FN )                           dll_entrypoint( hLib, "okPLL22393_Destruct" );
		_okPLL22393_SetCrystalLoad                    = ( OKPLL22393_SETCRYSTALLOAD_FN )                     dll_entrypoint( hLib, "okPLL22393_SetCrystalLoad" );
		_okPLL22393_SetReference                      = ( OKPLL22393_SETREFERENCE_FN )                       dll_entrypoint( hLib, "okPLL22393_SetReference" );
		_okPLL22393_GetReference                      = ( OKPLL22393_GETREFERENCE_FN )                       dll_entrypoint( hLib, "okPLL22393_GetReference" );
		_okPLL22393_SetPLLParameters                  = ( OKPLL22393_SETPLLPARAMETERS_FN )                   dll_entrypoint( hLib, "okPLL22393_SetPLLParameters" );
		_okPLL22393_SetPLLLF                          = ( OKPLL22393_SETPLLLF_FN )                           dll_entrypoint( hLib, "okPLL22393_SetPLLLF" );
		_okPLL22393_SetOutputDivider                  = ( OKPLL22393_SETOUTPUTDIVIDER_FN )                   dll_entrypoint( hLib, "okPLL22393_SetOutputDivider" );
		_okPLL22393_SetOutputSource                   = ( OKPLL22393_SETOUTPUTSOURCE_FN )                    dll_entrypoint( hLib, "okPLL22393_SetOutputSource" );
		_okPLL22393_SetOutputEnable                   = ( OKPLL22393_SETOUTPUTENABLE_FN )                    dll_entrypoint( hLib, "okPLL22393_SetOutputEnable" );
		_okPLL22393_GetPLLP                           = ( OKPLL22393_GETPLLP_FN )                            dll_entrypoint( hLib, "okPLL22393_GetPLLP" );
		_okPLL22393_GetPLLQ                           = ( OKPLL22393_GETPLLQ_FN )                            dll_entrypoint( hLib, "okPLL22393_GetPLLQ" );
		_okPLL22393_GetPLLFrequency                   = ( OKPLL22393_GETPLLFREQUENCY_FN )                    dll_entrypoint( hLib, "okPLL22393_GetPLLFrequency" );
		_okPLL22393_GetOutputDivider                  = ( OKPLL22393_GETOUTPUTDIVIDER_FN )                   dll_entrypoint( hLib, "okPLL22393_GetOutputDivider" );
		_okPLL22393_GetOutputSource                   = ( OKPLL22393_GETOUTPUTSOURCE_FN )                    dll_entrypoint( hLib, "okPLL22393_GetOutputSource" );
		_okPLL22393_GetOutputFrequency                = ( OKPLL22393_GETOUTPUTFREQUENCY_FN )                 dll_entrypoint( hLib, "okPLL22393_GetOutputFrequency" );
		_okPLL22393_IsOutputEnabled                   = ( OKPLL22393_ISOUTPUTENABLED_FN )                    dll_entrypoint( hLib, "okPLL22393_IsOutputEnabled" );
		_okPLL22393_IsPLLEnabled                      = ( OKPLL22393_ISPLLENABLED_FN )                       dll_entrypoint( hLib, "okPLL22393_IsPLLEnabled" );
		_okPLL22393_InitFromProgrammingInfo           = ( OKPLL22393_INITFROMPROGRAMMINGINFO_FN )            dll_entrypoint( hLib, "okPLL22393_InitFromProgrammingInfo" );
		_okPLL22393_GetProgrammingInfo                = ( OKPLL22393_GETPROGRAMMINGINFO_FN )                 dll_entrypoint( hLib, "okPLL22393_GetProgrammingInfo" );

		_okDeviceSettings_Construct                   = (okDeviceSettings_CONSTRUCT_FN)                      dll_entrypoint( hLib, "okDeviceSettings_Construct" );
		_okDeviceSettings_Destruct                    = (okDeviceSettings_DESTRUCT_FN)                       dll_entrypoint( hLib, "okDeviceSettings_Destruct" );
		_okDeviceSettings_GetString                   = (okDeviceSettings_GETSTRING_FN)                      dll_entrypoint( hLib, "okDeviceSettings_GetString" );
		_okDeviceSettings_SetString                   = (okDeviceSettings_SETSTRING_FN)                      dll_entrypoint( hLib, "okDeviceSettings_SetString" );
		_okDeviceSettings_GetInt                      = (okDeviceSettings_GETINT_FN)                         dll_entrypoint( hLib, "okDeviceSettings_GetInt" );
		_okDeviceSettings_SetInt                      = (okDeviceSettings_SETINT_FN)                         dll_entrypoint( hLib, "okDeviceSettings_SetInt" );
		_okDeviceSettings_Delete                      = (okDeviceSettings_DELETE_FN)                         dll_entrypoint( hLib, "okDeviceSettings_Delete" );
		_okDeviceSettings_Save                        = (okDeviceSettings_SAVE_FN)                           dll_entrypoint( hLib, "okDeviceSettings_Save" );

		_okFrontPanel_Construct                         = ( okFrontPanel_CONSTRUCT_FN )                          dll_entrypoint( hLib, "okFrontPanel_Construct" );
		_okFrontPanel_Destruct                          = ( okFrontPanel_DESTRUCT_FN )                           dll_entrypoint( hLib, "okFrontPanel_Destruct" );
		_okFrontPanel_GetHostInterfaceWidth             = ( okFrontPanel_GETHOSTINTERFACEWIDTH_FN )              dll_entrypoint( hLib, "okFrontPanel_GetHostInterfaceWidth" );
		_okFrontPanel_IsHighSpeed                       = ( okFrontPanel_ISHIGHSPEED_FN )                        dll_entrypoint( hLib, "okFrontPanel_IsHighSpeed" );
		_okFrontPanel_GetBoardModel                     = ( okFrontPanel_GETBOARDMODEL_FN )                      dll_entrypoint( hLib, "okFrontPanel_GetBoardModel" );
		_okFrontPanel_GetBoardModelString               = ( okFrontPanel_GETBOARDMODELSTRING_FN )                dll_entrypoint( hLib, "okFrontPanel_GetBoardModelString" );
		_okFrontPanel_WriteI2C                          = ( okFrontPanel_WRITEI2C_FN )                           dll_entrypoint( hLib, "okFrontPanel_WriteI2C" );
		_okFrontPanel_ReadI2C                           = ( okFrontPanel_READI2C_FN )                            dll_entrypoint( hLib, "okFrontPanel_ReadI2C" );
		_okFrontPanel_GetDeviceCount                    = ( okFrontPanel_GETDEVICECOUNT_FN )                     dll_entrypoint( hLib, "okFrontPanel_GetDeviceCount" );
		_okFrontPanel_GetDeviceListModel                = ( okFrontPanel_GETDEVICELISTMODEL_FN )                 dll_entrypoint( hLib, "okFrontPanel_GetDeviceListModel" );
		_okFrontPanel_GetDeviceListSerial               = ( okFrontPanel_GETDEVICELISTSERIAL_FN )                dll_entrypoint( hLib, "okFrontPanel_GetDeviceListSerial" );
		_okFrontPanel_OpenBySerial                      = ( okFrontPanel_OPENBYSERIAL_FN )                       dll_entrypoint( hLib, "okFrontPanel_OpenBySerial" );
		_okFrontPanel_IsOpen                            = ( okFrontPanel_ISOPEN_FN )                             dll_entrypoint( hLib, "okFrontPanel_IsOpen" );
		_okFrontPanel_SetBTPipePollingInterval          = ( okFrontPanel_SETBTPIPEPOLLINGINTERVAL_FN )           dll_entrypoint( hLib, "okFrontPanel_SetBTPipePollingInterval" );
		_okFrontPanel_SetTimeout                        = ( okFrontPanel_SETTIMEOUT_FN )                         dll_entrypoint( hLib, "okFrontPanel_SetTimeout" );
		_okFrontPanel_EnableAsynchronousTransfers       = ( okFrontPanel_ENABLEASYNCHRONOUSTRANSFERS_FN )        dll_entrypoint( hLib, "okFrontPanel_EnableAsynchronousTransfers" );
		_okFrontPanel_GetDeviceInfo                     = ( okFrontPanel_GETDEVICEINFO_FN )                      dll_entrypoint( hLib, "okFrontPanel_GetDeviceInfo" );
		_okFrontPanel_GetDeviceMajorVersion             = ( okFrontPanel_GETDEVICEMAJORVERSION_FN )              dll_entrypoint( hLib, "okFrontPanel_GetDeviceMajorVersion" );
		_okFrontPanel_GetDeviceMinorVersion             = ( okFrontPanel_GETDEVICEMINORVERSION_FN )              dll_entrypoint( hLib, "okFrontPanel_GetDeviceMinorVersion" );
		_okFrontPanel_ResetFPGA                         = ( okFrontPanel_RESETFPGA_FN )                          dll_entrypoint( hLib, "okFrontPanel_ResetFPGA" );
		_okFrontPanel_Close                             = ( okFrontPanel_CLOSE_FN )                              dll_entrypoint( hLib, "okFrontPanel_Close" );
		_okFrontPanel_GetSerialNumber                   = ( okFrontPanel_GETSERIALNUMBER_FN )                    dll_entrypoint( hLib, "okFrontPanel_GetSerialNumber" );
		_okFrontPanel_GetDeviceSettings                 = ( okFrontPanel_GETDEVICESETTINGS_FN )                  dll_entrypoint( hLib, "okFrontPanel_GetDeviceSettings" );
		_okFrontPanel_GetDeviceID                       = ( okFrontPanel_GETDEVICEID_FN )                        dll_entrypoint( hLib, "okFrontPanel_GetDeviceID" );
		_okFrontPanel_SetDeviceID                       = ( okFrontPanel_SETDEVICEID_FN )                        dll_entrypoint( hLib, "okFrontPanel_SetDeviceID" );
		_okFrontPanel_ConfigureFPGA                     = ( okFrontPanel_CONFIGUREFPGA_FN )                      dll_entrypoint( hLib, "okFrontPanel_ConfigureFPGA" );
		_okFrontPanel_ConfigureFPGAFromMemory           = ( okFrontPanel_CONFIGUREFPGAFROMMEMORY_FN )            dll_entrypoint( hLib, "okFrontPanel_ConfigureFPGAFromMemory" );
		_okFrontPanel_GetPLL22150Configuration          = ( okFrontPanel_GETPLL22150CONFIGURATION_FN )           dll_entrypoint( hLib, "okFrontPanel_GetPLL22150Configuration" );
		_okFrontPanel_SetPLL22150Configuration          = ( okFrontPanel_SETPLL22150CONFIGURATION_FN )           dll_entrypoint( hLib, "okFrontPanel_SetPLL22150Configuration" );
		_okFrontPanel_GetEepromPLL22150Configuration    = ( okFrontPanel_GETEEPROMPLL22150CONFIGURATION_FN )     dll_entrypoint( hLib, "okFrontPanel_GetEepromPLL22150Configuration" );
		_okFrontPanel_SetEepromPLL22150Configuration    = ( okFrontPanel_SETEEPROMPLL22150CONFIGURATION_FN )     dll_entrypoint( hLib, "okFrontPanel_SetEepromPLL22150Configuration" );
		_okFrontPanel_GetPLL22393Configuration          = ( okFrontPanel_GETPLL22393CONFIGURATION_FN )           dll_entrypoint( hLib, "okFrontPanel_GetPLL22393Configuration" );
		_okFrontPanel_SetPLL22393Configuration          = ( okFrontPanel_SETPLL22393CONFIGURATION_FN )           dll_entrypoint( hLib, "okFrontPanel_SetPLL22393Configuration" );
		_okFrontPanel_GetEepromPLL22393Configuration    = ( okFrontPanel_GETEEPROMPLL22393CONFIGURATION_FN )     dll_entrypoint( hLib, "okFrontPanel_GetEepromPLL22393Configuration" );
		_okFrontPanel_SetEepromPLL22393Configuration    = ( okFrontPanel_SETEEPROMPLL22393CONFIGURATION_FN )     dll_entrypoint( hLib, "okFrontPanel_SetEepromPLL22393Configuration" );
		_okFrontPanel_LoadDefaultPLLConfiguration       = ( okFrontPanel_LOADDEFAULTPLLCONFIGURATION_FN )        dll_entrypoint( hLib, "okFrontPanel_LoadDefaultPLLConfiguration" );
		_okFrontPanel_IsFrontPanelEnabled               = ( okFrontPanel_ISFRONTPANELENABLED_FN )                dll_entrypoint( hLib, "okFrontPanel_IsFrontPanelEnabled" );
		_okFrontPanel_IsFrontPanel3Supported            = ( okFrontPanel_ISFRONTPANEL3SUPPORTED_FN )             dll_entrypoint( hLib, "okFrontPanel_IsFrontPanel3Supported" );
		_okFrontPanel_UpdateWireIns                     = ( okFrontPanel_UPDATEWIREINS_FN )                      dll_entrypoint( hLib, "okFrontPanel_UpdateWireIns" );
		_okFrontPanel_SetWireInValue                    = ( okFrontPanel_SETWIREINVALUE_FN )                     dll_entrypoint( hLib, "okFrontPanel_SetWireInValue" );
		_okFrontPanel_UpdateWireOuts                    = ( okFrontPanel_UPDATEWIREOUTS_FN )                     dll_entrypoint( hLib, "okFrontPanel_UpdateWireOuts" );
		_okFrontPanel_GetWireOutValue                   = ( okFrontPanel_GETWIREOUTVALUE_FN )                    dll_entrypoint( hLib, "okFrontPanel_GetWireOutValue" );
		_okFrontPanel_ActivateTriggerIn                 = ( okFrontPanel_ACTIVATETRIGGERIN_FN )                  dll_entrypoint( hLib, "okFrontPanel_ActivateTriggerIn" );
		_okFrontPanel_UpdateTriggerOuts                 = ( okFrontPanel_UPDATETRIGGEROUTS_FN )                  dll_entrypoint( hLib, "okFrontPanel_UpdateTriggerOuts" );
		_okFrontPanel_IsTriggered                       = ( okFrontPanel_ISTRIGGERED_FN )                        dll_entrypoint( hLib, "okFrontPanel_IsTriggered" );
		_okFrontPanel_GetLastTransferLength             = ( okFrontPanel_GETLASTTRANSFERLENGTH_FN )              dll_entrypoint( hLib, "okFrontPanel_GetLastTransferLength" );
		_okFrontPanel_WriteToPipeIn                     = ( okFrontPanel_WRITETOPIPEIN_FN )                      dll_entrypoint( hLib, "okFrontPanel_WriteToPipeIn" );
		_okFrontPanel_WriteToBlockPipeIn                = ( okFrontPanel_WRITETOBLOCKPIPEIN_FN )                 dll_entrypoint( hLib, "okFrontPanel_WriteToBlockPipeIn" );
		_okFrontPanel_ReadFromPipeOut                   = ( okFrontPanel_READFROMPIPEOUT_FN )                    dll_entrypoint( hLib, "okFrontPanel_ReadFromPipeOut" );
		_okFrontPanel_ReadFromBlockPipeOut              = ( okFrontPanel_READFROMBLOCKPIPEOUT_FN )               dll_entrypoint( hLib, "okFrontPanel_ReadFromBlockPipeOut" );
		_okFrontPanel_FlashEraseSector                  = ( okFrontPanel_FLASHERASESECTOR_FN )                   dll_entrypoint( hLib, "okFrontPanel_FlashEraseSector" );
		_okFrontPanel_FlashWrite                        = ( okFrontPanel_FLASHWRITE_FN )                         dll_entrypoint( hLib, "okFrontPanel_FlashWrite" );
		_okFrontPanel_FlashRead                         = ( okFrontPanel_FLASHREAD_FN )                          dll_entrypoint( hLib, "okFrontPanel_FlashRead" );
		_okFrontPanel_GetFPGAResetProfile               = ( okFrontPanel_GETFPGARESETPROFILE_FN )                dll_entrypoint( hLib, "okFrontPanel_GetFPGAResetProfile" );
		_okFrontPanel_SetFPGAResetProfile               = ( okFrontPanel_SETFPGARESETPROFILE_FN )                dll_entrypoint( hLib, "okFrontPanel_SetFPGAResetProfile" );
		_okFrontPanel_ReadRegister                      = ( okFrontPanel_READREGISTER_FN )                       dll_entrypoint( hLib, "okFrontPanel_ReadRegister" );
		_okFrontPanel_ReadRegisters                     = ( okFrontPanel_READREGISTERS_FN )                      dll_entrypoint( hLib, "okFrontPanel_ReadRegisters" );
		_okFrontPanel_WriteRegister                     = ( okFrontPanel_WRITEREGISTER_FN )                      dll_entrypoint( hLib, "okFrontPanel_WriteRegister" );
		_okFrontPanel_WriteRegisters                    = ( okFrontPanel_WRITEREGISTERS_FN )                     dll_entrypoint( hLib, "okFrontPanel_WriteRegisters" );
		_okFrontPanel_GetWireInValue                    = ( okFrontPanel_GETWIREINVALUE_FN )                     dll_entrypoint( hLib, "okFrontPanel_GetWireInValue" );

		_okFrontPanelManager_Construct                  = ( okFrontPanelManager_CONSTRUCT_FN )                   dll_entrypoint( hLib, "okFrontPanelManager_Construct" );
		_okFrontPanelManager_Destruct                   = ( okFrontPanelManager_DESTRUCT_FN )                    dll_entrypoint( hLib, "okFrontPanelManager_Destruct" );
		_okFrontPanelManager_StartMonitoring            = ( okFrontPanelManager_STARTMONITORING_FN )             dll_entrypoint( hLib, "okFrontPanelManager_StartMonitoring" );
		_okFrontPanelManager_Open                       = ( okFrontPanelManager_OPEN_FN )                        dll_entrypoint( hLib, "okFrontPanelManager_Open" );
	}

    if (NULL == hLib) {
		return(FALSE);
    }
    
	return(TRUE);
}


void
okFrontPanelDLL_FreeLib(void)
{
	_okFrontPanelDLL_GetVersion                        = NULL;

	_okFrontPanel_Construct                         = NULL;
	_okFrontPanel_Destruct                          = NULL;
	_okFrontPanel_GetHostInterfaceWidth             = NULL;
	_okFrontPanel_IsHighSpeed                       = NULL;
	_okFrontPanel_GetBoardModel                     = NULL;
	_okFrontPanel_GetBoardModelString               = NULL;
	_okFrontPanel_WriteI2C                          = NULL;
	_okFrontPanel_ReadI2C                           = NULL;
	_okFrontPanel_GetDeviceCount                    = NULL;
	_okFrontPanel_GetDeviceListModel                = NULL;
	_okFrontPanel_GetDeviceListSerial               = NULL;
	_okFrontPanel_OpenBySerial                      = NULL;
	_okFrontPanel_IsOpen                            = NULL;
	_okFrontPanel_SetBTPipePollingInterval          = NULL;
	_okFrontPanel_SetTimeout                        = NULL;
	_okFrontPanel_EnableAsynchronousTransfers       = NULL;
	_okFrontPanel_GetDeviceInfo                     = NULL;
	_okFrontPanel_GetDeviceMajorVersion             = NULL;
	_okFrontPanel_GetDeviceMinorVersion             = NULL;
	_okFrontPanel_ResetFPGA                         = NULL;
	_okFrontPanel_Close                             = NULL;
	_okFrontPanel_GetSerialNumber                   = NULL;
	_okFrontPanel_GetDeviceSettings                 = NULL;
	_okFrontPanel_GetDeviceID                       = NULL;
	_okFrontPanel_SetDeviceID                       = NULL;
	_okFrontPanel_ConfigureFPGA                     = NULL;
	_okFrontPanel_ConfigureFPGAFromMemory           = NULL;
	_okFrontPanel_GetPLL22150Configuration          = NULL;
	_okFrontPanel_SetPLL22150Configuration          = NULL;
	_okFrontPanel_GetEepromPLL22150Configuration    = NULL;
	_okFrontPanel_SetEepromPLL22150Configuration    = NULL;
	_okFrontPanel_GetPLL22393Configuration          = NULL;
	_okFrontPanel_SetPLL22393Configuration          = NULL;
	_okFrontPanel_GetEepromPLL22393Configuration    = NULL;
	_okFrontPanel_SetEepromPLL22393Configuration    = NULL;
	_okFrontPanel_IsFrontPanelEnabled               = NULL;
	_okFrontPanel_IsFrontPanel3Supported            = NULL;
	_okFrontPanel_UpdateWireIns                     = NULL;
	_okFrontPanel_SetWireInValue                    = NULL;
	_okFrontPanel_UpdateWireOuts                    = NULL;
	_okFrontPanel_GetWireOutValue                   = NULL;
	_okFrontPanel_ActivateTriggerIn                 = NULL;
	_okFrontPanel_UpdateTriggerOuts                 = NULL;
	_okFrontPanel_IsTriggered                       = NULL;
	_okFrontPanel_GetLastTransferLength             = NULL;
	_okFrontPanel_WriteToPipeIn                     = NULL;
	_okFrontPanel_WriteToBlockPipeIn                = NULL;
	_okFrontPanel_ReadFromPipeOut                   = NULL;
	_okFrontPanel_ReadFromBlockPipeOut              = NULL;
    _okFrontPanel_FlashEraseSector                  = NULL;
    _okFrontPanel_FlashWrite                        = NULL;
    _okFrontPanel_FlashRead                         = NULL;
    _okFrontPanel_GetFPGAResetProfile               = NULL;
    _okFrontPanel_SetFPGAResetProfile               = NULL;
    _okFrontPanel_ReadRegister                      = NULL;
    _okFrontPanel_ReadRegisters                   = NULL;
    _okFrontPanel_WriteRegister                     = NULL;
    _okFrontPanel_WriteRegisters                  = NULL;
    _okFrontPanel_GetWireInValue                    = NULL;

	_okPLL22393_Construct                         = NULL;
	_okPLL22393_Destruct                          = NULL;
	_okPLL22393_SetCrystalLoad                    = NULL;
	_okPLL22393_SetReference                      = NULL;
	_okPLL22393_GetReference                      = NULL;
	_okPLL22393_SetPLLParameters                  = NULL;
	_okPLL22393_SetPLLLF                          = NULL;
	_okPLL22393_SetOutputDivider                  = NULL;
	_okPLL22393_SetOutputSource                   = NULL;
	_okPLL22393_SetOutputEnable                   = NULL;
	_okPLL22393_GetPLLP                           = NULL;
	_okPLL22393_GetPLLQ                           = NULL;
	_okPLL22393_GetPLLFrequency                   = NULL;
	_okPLL22393_GetOutputDivider                  = NULL;
	_okPLL22393_GetOutputSource                   = NULL;
	_okPLL22393_GetOutputFrequency                = NULL;
	_okPLL22393_IsOutputEnabled                   = NULL;
	_okPLL22393_IsPLLEnabled                      = NULL;
	_okPLL22393_InitFromProgrammingInfo           = NULL;
	_okPLL22393_GetProgrammingInfo                = NULL;

	_okPLL22150_Construct                         = NULL;
	_okPLL22150_Destruct                          = NULL;
	_okPLL22150_SetCrystalLoad                    = NULL;
	_okPLL22150_SetReference                      = NULL;
	_okPLL22150_GetReference                      = NULL;
	_okPLL22150_SetVCOParameters                  = NULL;
	_okPLL22150_GetVCOP                           = NULL;
	_okPLL22150_GetVCOQ                           = NULL;
	_okPLL22150_GetVCOFrequency                   = NULL;
	_okPLL22150_SetDiv1                           = NULL;
	_okPLL22150_SetDiv2                           = NULL;
	_okPLL22150_GetDiv1Source                     = NULL;
	_okPLL22150_GetDiv2Source                     = NULL;
	_okPLL22150_GetDiv1Divider                    = NULL;
	_okPLL22150_GetDiv2Divider                    = NULL;
	_okPLL22150_SetOutputSource                   = NULL;
	_okPLL22150_SetOutputEnable                   = NULL;
	_okPLL22150_GetOutputSource                   = NULL;
	_okPLL22150_GetOutputFrequency                = NULL;
	_okPLL22150_IsOutputEnabled                   = NULL;
	_okPLL22150_InitFromProgrammingInfo           = NULL;
	_okPLL22150_GetProgrammingInfo                = NULL;

	if (hLib) {
		dll_unload(hLib);
		hLib = NULL;
    }
}


//------------------------------------------------------------------------
// Function calls - General
//------------------------------------------------------------------------
okDLLEXPORT void DLL_ENTRY
okFrontPanelDLL_GetVersion(char *date, char *time) {

	if (_okFrontPanelDLL_GetVersion)
		(*_okFrontPanelDLL_GetVersion)(date, time);
}

//------------------------------------------------------------------------
// Function calls - okCPLL22393
//------------------------------------------------------------------------
okDLLEXPORT okPLL22393_HANDLE DLL_ENTRY
okPLL22393_Construct() {
	if (_okPLL22393_Construct)
		return((*_okPLL22393_Construct)());

	return(NULL);
}

okDLLEXPORT void DLL_ENTRY
okPLL22393_Destruct(okPLL22393_HANDLE pll) {
	if (_okPLL22393_Destruct)
		(*_okPLL22393_Destruct)(pll);
}

okDLLEXPORT void DLL_ENTRY
okPLL22393_SetCrystalLoad(okPLL22393_HANDLE pll, double capload) {
	if (_okPLL22393_SetCrystalLoad)
		(*_okPLL22393_SetCrystalLoad)(pll, capload);
}

okDLLEXPORT void DLL_ENTRY
okPLL22393_SetReference(okPLL22393_HANDLE pll, double freq) {
	if (_okPLL22393_SetReference)
		(*_okPLL22393_SetReference)(pll, freq);
}

okDLLEXPORT double DLL_ENTRY
okPLL22393_GetReference(okPLL22393_HANDLE pll) {
	if (_okPLL22393_GetReference)
		return((*_okPLL22393_GetReference)(pll));
	return(0.0);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22393_SetPLLParameters(okPLL22393_HANDLE pll, int n, int p, int q, Bool enable) {
	if (_okPLL22393_SetPLLParameters)
		return((*_okPLL22393_SetPLLParameters)(pll, n, p, q, enable));
	return(FALSE);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22393_SetPLLLF(okPLL22393_HANDLE pll, int n, int lf) {
	if (_okPLL22393_SetPLLLF)
		return((*_okPLL22393_SetPLLLF)(pll, n, lf));
	return(FALSE);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22393_SetOutputDivider(okPLL22393_HANDLE pll, int n, int div) {
	if (_okPLL22393_SetOutputDivider)
		return((*_okPLL22393_SetOutputDivider)(pll, n, div));
	return(FALSE);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22393_SetOutputSource(okPLL22393_HANDLE pll, int n, ok_ClockSource_22393 clksrc) {
	if (_okPLL22393_SetOutputSource)
		return((*_okPLL22393_SetOutputSource)(pll, n, clksrc));
	return(FALSE);
}

okDLLEXPORT void DLL_ENTRY
okPLL22393_SetOutputEnable(okPLL22393_HANDLE pll, int n, Bool enable) {
	if (_okPLL22393_SetOutputEnable)
		(*_okPLL22393_SetOutputEnable)(pll, n, enable);
}

okDLLEXPORT int DLL_ENTRY
okPLL22393_GetPLLP(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_GetPLLP)
		return((*_okPLL22393_GetPLLP)(pll, n));
	return(0);
}

okDLLEXPORT int DLL_ENTRY
okPLL22393_GetPLLQ(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_GetPLLQ)
		return((*_okPLL22393_GetPLLQ)(pll, n));
	return(0);
}

okDLLEXPORT double DLL_ENTRY
okPLL22393_GetPLLFrequency(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_GetPLLFrequency)
		return((*_okPLL22393_GetPLLFrequency)(pll, n));
	return(0.0);
}

okDLLEXPORT int DLL_ENTRY
okPLL22393_GetOutputDivider(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_GetOutputDivider)
		return((*_okPLL22393_GetOutputDivider)(pll, n));
	return(0);
}

okDLLEXPORT ok_ClockSource_22393 DLL_ENTRY
okPLL22393_GetOutputSource(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_GetOutputSource)
		return((*_okPLL22393_GetOutputSource)(pll, n));
	return(ok_ClkSrc22393_Ref);
}

okDLLEXPORT double DLL_ENTRY
okPLL22393_GetOutputFrequency(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_GetOutputFrequency)
		return((*_okPLL22393_GetOutputFrequency)(pll, n));
	return(0.0);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22393_IsOutputEnabled(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_IsOutputEnabled)
		return((*_okPLL22393_IsOutputEnabled)(pll, n));
	return(FALSE);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22393_IsPLLEnabled(okPLL22393_HANDLE pll, int n) {
	if (_okPLL22393_IsPLLEnabled)
		return((*_okPLL22393_IsPLLEnabled)(pll, n));
	return(FALSE);
}

okDLLEXPORT void DLL_ENTRY
okPLL22393_InitFromProgrammingInfo(okPLL22393_HANDLE pll, unsigned char *buf) {
	if (_okPLL22393_InitFromProgrammingInfo)
		(*_okPLL22393_InitFromProgrammingInfo)(pll, buf);
}

okDLLEXPORT void DLL_ENTRY
okPLL22393_GetProgrammingInfo(okPLL22393_HANDLE pll, unsigned char *buf) {
	if (_okPLL22393_GetProgrammingInfo)
		(*_okPLL22393_GetProgrammingInfo)(pll, buf);
}


//------------------------------------------------------------------------
// Function calls - okCPLL22150
//------------------------------------------------------------------------
okDLLEXPORT okPLL22150_HANDLE DLL_ENTRY
okPLL22150_Construct()
{
	if (_okPLL22150_Construct)
		return((*_okPLL22150_Construct)());

	return(NULL);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_Destruct(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_Destruct)
		(*_okPLL22150_Destruct)(pll);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_SetCrystalLoad(okPLL22150_HANDLE pll, double capload)
{
	if (_okPLL22150_SetCrystalLoad)
		(*_okPLL22150_SetCrystalLoad)(pll, capload);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_SetReference(okPLL22150_HANDLE pll, double freq, Bool extosc)
{
	if (_okPLL22150_SetReference)
		(*_okPLL22150_SetReference)(pll, freq, extosc);
}

okDLLEXPORT double DLL_ENTRY
okPLL22150_GetReference(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetReference)
		return((*_okPLL22150_GetReference)(pll));

	return(0.0);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22150_SetVCOParameters(okPLL22150_HANDLE pll, int p, int q)
{
	if (_okPLL22150_SetVCOParameters)
		return((*_okPLL22150_SetVCOParameters)(pll, p, q));

	return(FALSE);
}

okDLLEXPORT int DLL_ENTRY
okPLL22150_GetVCOP(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetVCOP)
		return((*_okPLL22150_GetVCOP)(pll));

	return(0);
}

okDLLEXPORT int DLL_ENTRY
okPLL22150_GetVCOQ(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetVCOQ)
		return((*_okPLL22150_GetVCOQ)(pll));

	return(0);
}

okDLLEXPORT double DLL_ENTRY
okPLL22150_GetVCOFrequency(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetVCOFrequency)
		return((*_okPLL22150_GetVCOFrequency)(pll));

	return(0.0);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_SetDiv1(okPLL22150_HANDLE pll, ok_DividerSource divsrc, int n)
{
	if (_okPLL22150_SetDiv1)
		(*_okPLL22150_SetDiv1)(pll, divsrc, n);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_SetDiv2(okPLL22150_HANDLE pll, ok_DividerSource divsrc, int n)
{
	if (_okPLL22150_SetDiv2)
		(*_okPLL22150_SetDiv2)(pll, divsrc, n);
}

okDLLEXPORT ok_DividerSource  DLL_ENTRY
okPLL22150_GetDiv1Source(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetDiv1Source)
		return((*_okPLL22150_GetDiv1Source)(pll));

	return(ok_DivSrc_Ref);
}

okDLLEXPORT ok_DividerSource DLL_ENTRY
okPLL22150_GetDiv2Source(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetDiv2Source)
		return((*_okPLL22150_GetDiv2Source)(pll));

	return(ok_DivSrc_Ref);
}

okDLLEXPORT int DLL_ENTRY
okPLL22150_GetDiv1Divider(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetDiv1Divider)
		return((*_okPLL22150_GetDiv1Divider)(pll));

	return(0);
}

okDLLEXPORT int DLL_ENTRY
okPLL22150_GetDiv2Divider(okPLL22150_HANDLE pll)
{
	if (_okPLL22150_GetDiv2Divider)
		return((*_okPLL22150_GetDiv2Divider)(pll));

	return(0);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_SetOutputSource(okPLL22150_HANDLE pll, int output, ok_ClockSource_22150 clksrc)
{
	if (_okPLL22150_SetOutputSource)
		(*_okPLL22150_SetOutputSource)(pll, output, clksrc);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_SetOutputEnable(okPLL22150_HANDLE pll, int output, Bool enable)
{
	if (_okPLL22150_SetOutputEnable)
		(*_okPLL22150_SetOutputEnable)(pll, output, enable);
}

okDLLEXPORT ok_ClockSource_22150 DLL_ENTRY
okPLL22150_GetOutputSource(okPLL22150_HANDLE pll, int output)
{
	if (_okPLL22150_GetOutputSource)
		return((*_okPLL22150_GetOutputSource)(pll, output));

	return(ok_ClkSrc22150_Ref);
}

okDLLEXPORT double DLL_ENTRY
okPLL22150_GetOutputFrequency(okPLL22150_HANDLE pll, int output)
{
	if (_okPLL22150_GetOutputFrequency)
		return((*_okPLL22150_GetOutputFrequency)(pll, output));

	return(0.0);
}

okDLLEXPORT Bool DLL_ENTRY
okPLL22150_IsOutputEnabled(okPLL22150_HANDLE pll, int output)
{
	if (_okPLL22150_IsOutputEnabled)
		return((*_okPLL22150_IsOutputEnabled)(pll, output));

	return(FALSE);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_InitFromProgrammingInfo(okPLL22150_HANDLE pll, unsigned char *buf)
{
	if (_okPLL22150_InitFromProgrammingInfo)
		(*_okPLL22150_InitFromProgrammingInfo)(pll, buf);
}

okDLLEXPORT void DLL_ENTRY
okPLL22150_GetProgrammingInfo(okPLL22150_HANDLE pll, unsigned char *buf)
{
	if (_okPLL22150_GetProgrammingInfo)
		(*_okPLL22150_GetProgrammingInfo)(pll, buf);
}


//------------------------------------------------------------------------
// Function calls - okCDeviceSettings
//------------------------------------------------------------------------

okDLLEXPORT okDeviceSettings_HANDLE DLL_ENTRY
okDeviceSettings_Construct()
{
	if (_okDeviceSettings_Construct)
		return (*_okDeviceSettings_Construct)();

	return NULL;
}

okDLLEXPORT void DLL_ENTRY
okDeviceSettings_Destruct(okDeviceSettings_HANDLE hnd)
{
	if (_okDeviceSettings_Destruct)
		(*_okDeviceSettings_Destruct)(hnd);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okDeviceSettings_GetString(okDeviceSettings_HANDLE hnd, const char *key, int length, char *buf)
{
	if (_okDeviceSettings_GetString)
		return (*_okDeviceSettings_GetString)(hnd, key, length, buf);

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okDeviceSettings_SetString(okDeviceSettings_HANDLE hnd, const char *key, const char *buf)
{
	if (_okDeviceSettings_SetString)
		return (*_okDeviceSettings_SetString)(hnd, key, buf);

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okDeviceSettings_GetInt(okDeviceSettings_HANDLE hnd, const char *key, UINT32 *value)
{
	if (_okDeviceSettings_GetInt)
		return (*_okDeviceSettings_GetInt)(hnd, key, value);

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okDeviceSettings_SetInt(okDeviceSettings_HANDLE hnd, const char *key, UINT32 value)
{
	if (_okDeviceSettings_SetInt)
		return (*_okDeviceSettings_SetInt)(hnd, key, value);

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okDeviceSettings_Delete(okDeviceSettings_HANDLE hnd, const char *key)
{
	if (_okDeviceSettings_Delete)
		return (*_okDeviceSettings_Delete)(hnd, key);

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okDeviceSettings_Save(okDeviceSettings_HANDLE hnd)
{
	if (_okDeviceSettings_Save)
		return (*_okDeviceSettings_Save)(hnd);

	return(ok_UnsupportedFeature);
}

//------------------------------------------------------------------------
// Function calls - okCFrontPanel
//------------------------------------------------------------------------
okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY
okFrontPanel_Construct()
{
	if (_okFrontPanel_Construct)
		return((*_okFrontPanel_Construct)());

	return(NULL);
}


okDLLEXPORT void DLL_ENTRY
okFrontPanel_Destruct(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_Destruct)
		(*_okFrontPanel_Destruct)(hnd);
}


okDLLEXPORT int DLL_ENTRY
okFrontPanel_GetHostInterfaceWidth(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_GetHostInterfaceWidth)
		return((*_okFrontPanel_GetHostInterfaceWidth)(hnd));

	return(FALSE);
}


okDLLEXPORT Bool DLL_ENTRY
okFrontPanel_IsHighSpeed(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_IsHighSpeed)
		return((*_okFrontPanel_IsHighSpeed)(hnd));

	return(FALSE);
}


okDLLEXPORT ok_BoardModel DLL_ENTRY
okFrontPanel_GetBoardModel(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_GetBoardModel)
		return((*_okFrontPanel_GetBoardModel)(hnd));

	return(ok_brdUnknown);
}


okDLLEXPORT void DLL_ENTRY
okFrontPanel_GetBoardModelString(okFrontPanel_HANDLE hnd, ok_BoardModel m, char *str)
{
	if (_okFrontPanel_GetBoardModelString)
		(*_okFrontPanel_GetBoardModelString)(hnd, m, str);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_WriteI2C(okFrontPanel_HANDLE hnd, const int addr, int length, unsigned char *data)
{
	if (_okFrontPanel_WriteI2C)
		return((*_okFrontPanel_WriteI2C)(hnd, addr, length, data));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ReadI2C(okFrontPanel_HANDLE hnd, const int addr, int length, unsigned char *data)
{
	if (_okFrontPanel_ReadI2C)
		return((*_okFrontPanel_ReadI2C)(hnd, addr, length, data));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT int DLL_ENTRY
okFrontPanel_GetDeviceCount(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_GetDeviceCount)
		return((*_okFrontPanel_GetDeviceCount)(hnd));

	return(0);
}


okDLLEXPORT ok_BoardModel DLL_ENTRY
okFrontPanel_GetDeviceListModel(okFrontPanel_HANDLE hnd, int num)
{
	if (_okFrontPanel_GetDeviceListModel)
		return((*_okFrontPanel_GetDeviceListModel)(hnd, num));

	return(ok_brdUnknown);
}


okDLLEXPORT void DLL_ENTRY
okFrontPanel_GetDeviceListSerial(okFrontPanel_HANDLE hnd, int num, char *str)
{
	if (_okFrontPanel_GetDeviceListSerial)
		(*_okFrontPanel_GetDeviceListSerial)(hnd, num, str);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_OpenBySerial(okFrontPanel_HANDLE hnd, const char *serial)
{
	if (_okFrontPanel_OpenBySerial)
		return((*_okFrontPanel_OpenBySerial)(hnd, serial));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT Bool DLL_ENTRY
okFrontPanel_IsOpen(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_IsOpen)
		return((*_okFrontPanel_IsOpen)(hnd));

	return(FALSE);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_EnableAsynchronousTransfers(okFrontPanel_HANDLE hnd, Bool enable)
{
	if (_okFrontPanel_EnableAsynchronousTransfers)
		(*_okFrontPanel_EnableAsynchronousTransfers)(hnd, enable);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetBTPipePollingInterval(okFrontPanel_HANDLE hnd, int interval)
{
	if (_okFrontPanel_SetBTPipePollingInterval)
		return((*_okFrontPanel_SetBTPipePollingInterval)(hnd, interval));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_SetTimeout(okFrontPanel_HANDLE hnd, int timeout)
{
	if (_okFrontPanel_SetTimeout)
		(*_okFrontPanel_SetTimeout)(hnd, timeout);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetDeviceInfo(okFrontPanel_HANDLE hnd, okTDeviceInfo *info)
{
	if (_okFrontPanel_GetDeviceInfo)
		return((*_okFrontPanel_GetDeviceInfo)(hnd, info));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT int DLL_ENTRY
okFrontPanel_GetDeviceMajorVersion(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_GetDeviceMajorVersion)
		return((*_okFrontPanel_GetDeviceMajorVersion)(hnd));

	return(0);
}

okDLLEXPORT int DLL_ENTRY
okFrontPanel_GetDeviceMinorVersion(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_GetDeviceMinorVersion)
		return((*_okFrontPanel_GetDeviceMinorVersion)(hnd));

	return(0);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ResetFPGA(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_ResetFPGA)
		return((*_okFrontPanel_ResetFPGA)(hnd));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_Close(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_Close)
		((*_okFrontPanel_Close)(hnd));
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_GetSerialNumber(okFrontPanel_HANDLE hnd, char *buf)
{
	if (_okFrontPanel_GetSerialNumber)
		(*_okFrontPanel_GetSerialNumber)(hnd, buf);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetDeviceSettings(okFrontPanel_HANDLE hnd, okDeviceSettings_HANDLE settings)
{
	if (_okFrontPanel_GetDeviceSettings)
		(*_okFrontPanel_GetDeviceSettings)(hnd, settings);

	return(ok_UnsupportedFeature);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_GetDeviceID(okFrontPanel_HANDLE hnd, char *buf)
{
	if (_okFrontPanel_GetDeviceID)
		(*_okFrontPanel_GetDeviceID)(hnd, buf);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_SetDeviceID(okFrontPanel_HANDLE hnd, const char *strID)
{
	if (_okFrontPanel_SetDeviceID)
		(*_okFrontPanel_SetDeviceID)(hnd, strID);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ConfigureFPGA(okFrontPanel_HANDLE hnd, const char *strFilename)
{
	if (_okFrontPanel_ConfigureFPGA)
		return((*_okFrontPanel_ConfigureFPGA)(hnd, strFilename));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ConfigureFPGAFromMemory(okFrontPanel_HANDLE hnd, unsigned char *data, unsigned long length)
{
	if (_okFrontPanel_ConfigureFPGAFromMemory)
		return((*_okFrontPanel_ConfigureFPGAFromMemory)(hnd, data, length));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll)
{
	if (_okFrontPanel_GetPLL22150Configuration)
		return((*_okFrontPanel_GetPLL22150Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll)
{
	if (_okFrontPanel_SetPLL22150Configuration)
		return((*_okFrontPanel_SetPLL22150Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetEepromPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll)
{
	if (_okFrontPanel_GetEepromPLL22150Configuration)
		return((*_okFrontPanel_GetEepromPLL22150Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetEepromPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll)
{
	if (_okFrontPanel_SetEepromPLL22150Configuration)
		return((*_okFrontPanel_SetEepromPLL22150Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll)
{
	if (_okFrontPanel_GetPLL22393Configuration)
		return((*_okFrontPanel_GetPLL22393Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll)
{
	if (_okFrontPanel_SetPLL22393Configuration)
		return((*_okFrontPanel_SetPLL22393Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetEepromPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll)
{
	if (_okFrontPanel_GetEepromPLL22393Configuration)
		return((*_okFrontPanel_GetEepromPLL22393Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetEepromPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll)
{
	if (_okFrontPanel_SetEepromPLL22393Configuration)
		return((*_okFrontPanel_SetEepromPLL22393Configuration)(hnd, pll));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_LoadDefaultPLLConfiguration(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_LoadDefaultPLLConfiguration)
		return((*_okFrontPanel_LoadDefaultPLLConfiguration)(hnd));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT Bool DLL_ENTRY
okFrontPanel_IsFrontPanelEnabled(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_IsFrontPanelEnabled)
		return((*_okFrontPanel_IsFrontPanelEnabled)(hnd));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT Bool DLL_ENTRY
okFrontPanel_IsFrontPanel3Supported(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_IsFrontPanel3Supported)
		return((*_okFrontPanel_IsFrontPanel3Supported)(hnd));

	return(FALSE);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_UpdateWireIns(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_UpdateWireIns)
		(*_okFrontPanel_UpdateWireIns)(hnd);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetWireInValue(okFrontPanel_HANDLE hnd, int ep, unsigned long val, unsigned long mask)
{
	if (_okFrontPanel_SetWireInValue)
		return((*_okFrontPanel_SetWireInValue)(hnd, ep, val, mask));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_UpdateWireOuts(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_UpdateWireOuts)
		(*_okFrontPanel_UpdateWireOuts)(hnd);
}

okDLLEXPORT unsigned long DLL_ENTRY
okFrontPanel_GetWireOutValue(okFrontPanel_HANDLE hnd, int epAddr)
{
	if (_okFrontPanel_GetWireOutValue)
		return((*_okFrontPanel_GetWireOutValue)(hnd, epAddr));

	return(0);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ActivateTriggerIn(okFrontPanel_HANDLE hnd, int epAddr, int bit)
{
	if (_okFrontPanel_ActivateTriggerIn)
		return((*_okFrontPanel_ActivateTriggerIn)(hnd, epAddr, bit));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT void DLL_ENTRY
okFrontPanel_UpdateTriggerOuts(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_UpdateTriggerOuts)
		(*_okFrontPanel_UpdateTriggerOuts)(hnd);
}

okDLLEXPORT Bool DLL_ENTRY
okFrontPanel_IsTriggered(okFrontPanel_HANDLE hnd, int epAddr, unsigned long mask)
{
	if (_okFrontPanel_IsTriggered)
		return((*_okFrontPanel_IsTriggered)(hnd, epAddr, mask));

	return(FALSE);
}

okDLLEXPORT long DLL_ENTRY
okFrontPanel_GetLastTransferLength(okFrontPanel_HANDLE hnd)
{
	if (_okFrontPanel_GetLastTransferLength)
		return((*_okFrontPanel_GetLastTransferLength)(hnd));

	return(0);
}

okDLLEXPORT long DLL_ENTRY
okFrontPanel_WriteToPipeIn(okFrontPanel_HANDLE hnd, int epAddr, long length, unsigned char *data)
{
	if (_okFrontPanel_WriteToPipeIn)
		return((*_okFrontPanel_WriteToPipeIn)(hnd, epAddr, length, data));

	return(0);
}

okDLLEXPORT long DLL_ENTRY
okFrontPanel_WriteToBlockPipeIn(okFrontPanel_HANDLE hnd, int epAddr, int blocksize, long length, unsigned char *data)
{
	if (_okFrontPanel_WriteToBlockPipeIn)
		return((*_okFrontPanel_WriteToBlockPipeIn)(hnd, epAddr, blocksize, length, data));

	return(0);
}

okDLLEXPORT long DLL_ENTRY
okFrontPanel_ReadFromPipeOut(okFrontPanel_HANDLE hnd, int epAddr, long length, unsigned char *data)
{
	if (_okFrontPanel_ReadFromPipeOut)
		return((*_okFrontPanel_ReadFromPipeOut)(hnd, epAddr, length, data));

	return(0);
}

okDLLEXPORT long DLL_ENTRY
okFrontPanel_ReadFromBlockPipeOut(okFrontPanel_HANDLE hnd, int epAddr, int blocksize, long length, unsigned char *data)
{
	if (_okFrontPanel_ReadFromBlockPipeOut)
		return((*_okFrontPanel_ReadFromBlockPipeOut)(hnd, epAddr, blocksize, length, data));

	return(0);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_FlashEraseSector(okFrontPanel_HANDLE hnd, UINT32 address)
{
	if (_okFrontPanel_FlashEraseSector)
		return((*_okFrontPanel_FlashEraseSector)(hnd, address));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_FlashWrite(okFrontPanel_HANDLE hnd, UINT32 address, UINT32 length, const UINT8 *buf)
{
	if (_okFrontPanel_FlashWrite)
		return((*_okFrontPanel_FlashWrite)(hnd, address, length, buf));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_FlashRead(okFrontPanel_HANDLE hnd, UINT32 address, UINT32 length, UINT8 *buf)
{
	if (_okFrontPanel_FlashRead)
		return((*_okFrontPanel_FlashRead)(hnd, address, length, buf));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetFPGAResetProfile(okFrontPanel_HANDLE hnd, okEFPGAConfigurationMethod method, okTFPGAResetProfile *profile)
{
	if (_okFrontPanel_GetFPGAResetProfile)
		return((*_okFrontPanel_GetFPGAResetProfile)(hnd, method, profile));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_SetFPGAResetProfile(okFrontPanel_HANDLE hnd, okEFPGAConfigurationMethod method, const okTFPGAResetProfile *profile)
{
	if (_okFrontPanel_SetFPGAResetProfile)
		return((*_okFrontPanel_SetFPGAResetProfile)(hnd, method, profile));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ReadRegister(okFrontPanel_HANDLE hnd, UINT32 addr, UINT32 *data)
{
	if (_okFrontPanel_ReadRegister)
		return((*_okFrontPanel_ReadRegister)(hnd, addr, data));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_ReadRegisters(okFrontPanel_HANDLE hnd, unsigned num, okTRegisterEntry* regs)
{
	if (_okFrontPanel_ReadRegisters)
		return((*_okFrontPanel_ReadRegisters)(hnd, num, regs));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_WriteRegister(okFrontPanel_HANDLE hnd, UINT32 addr, UINT32 data)
{
	if (_okFrontPanel_WriteRegister)
		return((*_okFrontPanel_WriteRegister)(hnd, addr, data));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_WriteRegisters(okFrontPanel_HANDLE hnd, unsigned num, const okTRegisterEntry* regs)
{
	if (_okFrontPanel_WriteRegisters)
		return((*_okFrontPanel_WriteRegisters)(hnd, num, regs));

	return(ok_UnsupportedFeature);
}


okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanel_GetWireInValue(okFrontPanel_HANDLE hnd, int epAddr, UINT32 *val)
{
	if (_okFrontPanel_GetWireInValue)
		return((*_okFrontPanel_GetWireInValue)(hnd, epAddr, val));

	return(ok_UnsupportedFeature);
}


//------------------------------------------------------------------------
// Function calls - okCFrontPanelManager
//------------------------------------------------------------------------
okDLLEXPORT okCFrontPanelManager_HANDLE DLL_ENTRY
okFrontPanelManager_Construct(okFrontPanelManager_HANDLE self)
{
	if (_okFrontPanelManager_Construct)
		return((*_okFrontPanelManager_Construct)(self));

	return(NULL);
}


okDLLEXPORT void DLL_ENTRY
okFrontPanelManager_Destruct(okCFrontPanelManager_HANDLE hnd)
{
	if (_okFrontPanelManager_Destruct)
		(*_okFrontPanelManager_Destruct)(hnd);
}

okDLLEXPORT ok_ErrorCode DLL_ENTRY
okFrontPanelManager_StartMonitoring(okCFrontPanelManager_HANDLE hnd)
{
	if (_okFrontPanelManager_StartMonitoring)
		return((*_okFrontPanelManager_StartMonitoring)(hnd));

	return(ok_UnsupportedFeature);
}

okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY
okFrontPanelManager_Open(okCFrontPanelManager_HANDLE hnd, const char *serial)
{
	if (_okFrontPanelManager_Open)
		return((*_okFrontPanelManager_Open)(hnd, serial));

	return(NULL);
}
