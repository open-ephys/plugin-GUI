///------------------------------------------------------------------------
// okFrontPanelDLL.h
//
// This is the header file for C/C++ compilation of the FrontPanel DLL
// import library.  This file must be included within any C/C++ source
// which references the FrontPanel DLL methods.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------

#ifndef __okFrontPanelDLL_h__
#define __okFrontPanelDLL_h__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FRONTPANELDLL_EXPORTS
// symbol defined on the command line.  This symbol should not be defined on any project
// that uses this DLL.
#if defined(_WIN32)
	#ifdef FRONTPANELDLL_EXPORTS
		#define okDLLEXPORT __declspec(dllexport)
	#else
		#define okDLLEXPORT __declspec(dllimport)

		#ifdef _MSC_VER
			#pragma comment(lib, "okFrontPanel")

			// If the user code delay loads this DLL, we provide a way to
			// verify if loading it during run-time succeeded. Predefine
			// okNO_DELAY_LOAD_FRONTPANEL to completely disable this code.
			#ifndef okNO_DELAY_LOAD_FRONTPANEL
				#define okDELAY_LOAD_FRONTPANEL
			#endif

			#ifdef okDELAY_LOAD_FRONTPANEL
				#include <windows.h>
				#include <delayimp.h>

				// Exception filter used for handling exceptions thrown by MSVC
				// CRT when delay-loading DLLs.
				inline
				LONG WINAPI ok_DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pExcPointers)
				{
					switch (pExcPointers->ExceptionRecord->ExceptionCode) {
						case 0xc06d007e: // FACILITY_VISUALCPP ERROR_MOD_NOT_FOUND
						case 0xc06d007f: // FACILITY_VISUALCPP ERROR_PROC_NOT_FOUND
							return EXCEPTION_EXECUTE_HANDLER;
					}

					return EXCEPTION_CONTINUE_SEARCH;
				}
			#endif
		#endif
	#endif
	#define DLL_ENTRY   __stdcall
	typedef unsigned int  UINT32;
	typedef unsigned char UINT8;
#elif defined(__linux__) || defined(__APPLE__) || defined(__QNX__)
	#define okDLLEXPORT __attribute__ ((visibility("default")))
	#define DLL_ENTRY
	typedef unsigned int  UINT32;
	typedef unsigned char UINT8;
#endif


#if defined(_WIN32) && defined(_UNICODE)
	typedef wchar_t const * okFP_dll_pchar;
#elif defined(_WIN32)
	typedef const char * okFP_dll_pchar;
#else
	typedef const char * okFP_dll_pchar;
#endif

#ifdef __cplusplus
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

extern "C" {
#endif // __cplusplus

typedef void* okPLL22150_HANDLE;
typedef void* okPLL22393_HANDLE;
typedef struct okDeviceSensorsHandle* okDeviceSensors_HANDLE;
typedef struct okDeviceSettingsHandle* okDeviceSettings_HANDLE;
typedef struct okFirmwareHandle* okFirmware_HANDLE;
typedef struct okFirmwarePackageHandle* okFirmwarePackage_HANDLE;
typedef void* okFrontPanel_HANDLE;
typedef struct okFrontPanelManagerHandle* okFrontPanelManager_HANDLE;
typedef struct okCFrontPanelManagerHandle* okCFrontPanelManager_HANDLE;
typedef struct okCFrontPanelDevicesHandle* okCFrontPanelDevices_HANDLE;
typedef int Bool;
#ifdef __cplusplus
	typedef bool okBool;
#else
	typedef char okBool;
#endif

#define MAX_SERIALNUMBER_LENGTH      10       // 10 characters + Does NOT include termination NULL.
#define MAX_DEVICEID_LENGTH          32       // 32 characters + Does NOT include termination NULL.
#define MAX_BOARDMODELSTRING_LENGTH  64       // 64 characters including termination NULL.

#ifndef TRUE
	#define TRUE    1
	#define FALSE   0
#endif

typedef enum {
	ok_ClkSrc22150_Ref=0,
	ok_ClkSrc22150_Div1ByN=1,
	ok_ClkSrc22150_Div1By2=2,
	ok_ClkSrc22150_Div1By3=3,
	ok_ClkSrc22150_Div2ByN=4,
	ok_ClkSrc22150_Div2By2=5,
	ok_ClkSrc22150_Div2By4=6
} ok_ClockSource_22150;

typedef enum {
	ok_ClkSrc22393_Ref=0,
	ok_ClkSrc22393_PLL0_0=2,
	ok_ClkSrc22393_PLL0_180=3,
	ok_ClkSrc22393_PLL1_0=4,
	ok_ClkSrc22393_PLL1_180=5,
	ok_ClkSrc22393_PLL2_0=6,
	ok_ClkSrc22393_PLL2_180=7
} ok_ClockSource_22393;

typedef enum {
	ok_DivSrc_Ref = 0, 
	ok_DivSrc_VCO = 1
} ok_DividerSource;

typedef enum {
	ok_brdUnknown = 0,
	ok_brdXEM3001v1 = 1,
	ok_brdXEM3001v2 = 2,
	ok_brdXEM3010 = 3,
	ok_brdXEM3005 = 4,
	ok_brdXEM3001CL = 5,
	ok_brdXEM3020 = 6,
	ok_brdXEM3050 = 7,
	ok_brdXEM9002 = 8,
	ok_brdXEM3001RB = 9,
	ok_brdXEM5010 = 10,
	ok_brdXEM6110LX45 = 11,
	ok_brdXEM6110LX150 = 15,
	ok_brdXEM6001 = 12,
	ok_brdXEM6010LX45 = 13,
	ok_brdXEM6010LX150 = 14,
	ok_brdXEM6006LX9 = 16,
	ok_brdXEM6006LX16 = 17,
	ok_brdXEM6006LX25 = 18,
	ok_brdXEM5010LX110 = 19,
	ok_brdZEM4310=20,
	ok_brdXEM6310LX45=21,
	ok_brdXEM6310LX150=22,
	ok_brdXEM6110v2LX45=23,
	ok_brdXEM6110v2LX150=24,
	ok_brdXEM6002LX9=25,
	ok_brdXEM6310MTLX45T=26,
	ok_brdXEM6320LX130T=27,
	ok_brdXEM7350K70T=28,
	ok_brdXEM7350K160T=29,
	ok_brdXEM7350K410T=30,
	ok_brdXEM6310MTLX150T=31,
	ok_brdZEM5305A2 = 32,
	ok_brdZEM5305A7 = 33,
	ok_brdXEM7001A15 = 34,
	ok_brdXEM7001A35 = 35,
	ok_brdXEM7360K160T = 36,
	ok_brdXEM7360K410T = 37,
	ok_brdZEM5310A4 = 38,
	ok_brdZEM5310A7 = 39,
	ok_brdZEM5370A5 = 40,
	ok_brdXEM7010A50 = 41,
	ok_brdXEM7010A200 = 42,
	ok_brdXEM7310A75 = 43,
	ok_brdXEM7310A200 = 44
} ok_BoardModel;

typedef enum {
	ok_NoError                    = 0,
	ok_Failed                     = -1,
	ok_Timeout                    = -2,
	ok_DoneNotHigh                = -3,
	ok_TransferError              = -4,
	ok_CommunicationError         = -5,
	ok_InvalidBitstream           = -6,
	ok_FileError                  = -7,
	ok_DeviceNotOpen              = -8,
	ok_InvalidEndpoint            = -9,
	ok_InvalidBlockSize           = -10,
	ok_I2CRestrictedAddress       = -11,
	ok_I2CBitError                = -12,
	ok_I2CNack                    = -13,
	ok_I2CUnknownStatus           = -14,
	ok_UnsupportedFeature         = -15,
	ok_FIFOUnderflow              = -16,
	ok_FIFOOverflow               = -17,
	ok_DataAlignmentError         = -18,
	ok_InvalidResetProfile        = -19,
	ok_InvalidParameter           = -20
} ok_ErrorCode;

#ifndef FRONTPANELDLL_EXPORTS

#define OK_MAX_DEVICEID_LENGTH              (33)     // 32-byte content + NULL termination
#define OK_MAX_SERIALNUMBER_LENGTH          (11)     // 10-byte content + NULL termination
#define OK_MAX_PRODUCT_NAME_LENGTH          (128)    // 127-byte content + NULL termination
#define OK_MAX_BOARD_MODEL_STRING_LENGTH    (128)

// ok_USBSpeed types
#define OK_USBSPEED_UNKNOWN                 (0)
#define OK_USBSPEED_FULL                    (1)
#define OK_USBSPEED_HIGH                    (2)
#define OK_USBSPEED_SUPER                   (3)

// ok_Interface types
#define OK_INTERFACE_UNKNOWN                (0)
#define OK_INTERFACE_USB2                   (1)
#define OK_INTERFACE_PCIE                   (2)
#define OK_INTERFACE_USB3                   (3)

// ok_Product types
#define OK_PRODUCT_UNKNOWN                  (0)
#define OK_PRODUCT_XEM3001V1                (1)
#define OK_PRODUCT_XEM3001V2                (2)
#define OK_PRODUCT_XEM3010                  (3)
#define OK_PRODUCT_XEM3005                  (4)
#define OK_PRODUCT_XEM3001CL                (5)
#define OK_PRODUCT_XEM3020                  (6)
#define OK_PRODUCT_XEM3050                  (7)
#define OK_PRODUCT_XEM9002                  (8)
#define OK_PRODUCT_XEM3001RB                (9)
#define OK_PRODUCT_XEM5010                  (10)
#define OK_PRODUCT_XEM6110LX45              (11)
#define OK_PRODUCT_XEM6001                  (12)
#define OK_PRODUCT_XEM6010LX45              (13)
#define OK_PRODUCT_XEM6010LX150             (14)
#define OK_PRODUCT_XEM6110LX150             (15)
#define OK_PRODUCT_XEM6006LX9               (16)
#define OK_PRODUCT_XEM6006LX16              (17)
#define OK_PRODUCT_XEM6006LX25              (18)
#define OK_PRODUCT_XEM5010LX110             (19)
#define OK_PRODUCT_ZEM4310                  (20)
#define OK_PRODUCT_XEM6310LX45              (21)
#define OK_PRODUCT_XEM6310LX150             (22)
#define OK_PRODUCT_XEM6110V2LX45            (23)
#define OK_PRODUCT_XEM6110V2LX150           (24)
#define OK_PRODUCT_XEM6002LX9               (25)
#define OK_PRODUCT_XEM6310MTLX45T           (26)
#define OK_PRODUCT_XEM6320LX130T            (27)
#define OK_PRODUCT_XEM7350K70T              (28)
#define OK_PRODUCT_XEM7350K160T             (29)
#define OK_PRODUCT_XEM7350K410T             (30)
#define OK_PRODUCT_XEM6310MTLX150T          (31)
#define OK_PRODUCT_ZEM5305A2                (32)
#define OK_PRODUCT_ZEM5305A7                (33)
#define OK_PRODUCT_XEM7001A15               (34)
#define OK_PRODUCT_XEM7001A35               (35)
#define OK_PRODUCT_XEM7360K160T             (36)
#define OK_PRODUCT_XEM7360K410T             (37)
#define OK_PRODUCT_ZEM5310A4                (38)
#define OK_PRODUCT_ZEM5310A7                (39)
#define OK_PRODUCT_ZEM5370A5                (40)
#define OK_PRODUCT_XEM7010A50               (41)
#define OK_PRODUCT_XEM7010A200              (42)
#define OK_PRODUCT_XEM7310A75               (43)
#define OK_PRODUCT_XEM7310A200              (44)

#define OK_PRODUCT_OEM_START                11000

#define OK_FPGACONFIGURATIONMETHOD_NVRAM    (0)
#define OK_FPGACONFIGURATIONMETHOD_JTAG     (1)
enum okEFPGAConfigurationMethod {
	ok_FPGAConfigurationMethod_NVRAM = OK_FPGACONFIGURATIONMETHOD_NVRAM,
	ok_FPGAConfigurationMethod_JTAG  = OK_FPGACONFIGURATIONMETHOD_JTAG
};


typedef struct {
	UINT32   address;
	UINT32   data;
} okTRegisterEntry;

#ifdef __cplusplus
typedef std::vector<okTRegisterEntry> okTRegisterEntries;
#endif // __cplusplus

typedef struct {
	UINT32   address;
	UINT32   mask;
} okTTriggerEntry;


typedef struct {
	// Magic number indicating the profile is valid.  (4 byte = 0xBE097C3D)
	UINT32                     magic;

	// Location of the configuration file (Flash boot).  (4 bytes)
	UINT32                     configFileLocation;

	// Length of the configuration file.  (4 bytes)
	UINT32                     configFileLength;

	// Number of microseconds to wait after DONE goes high before
	// starting the reset profile.  (4 bytes)
	UINT32                     doneWaitUS;

	// Number of microseconds to wait after wires are updated
	// before deasserting logic RESET.  (4 bytes)
	UINT32                     resetWaitUS;

	// Number of microseconds to wait after RESET is deasserted
	// before loading registers.  (4 bytes)
	UINT32                     registerWaitUS;

	// Future expansion  (112 bytes)
	UINT32                     padBytes1[28];

	// Initial values of WireIns.  These are loaded prior to
	// deasserting logic RESET.  (32*4 = 128 bytes)
	UINT32                     wireInValues[32];

	// Number of valid Register Entries (4 bytes)
	UINT32                     registerEntryCount;

	// Initial register loads.  (256*8 = 2048 bytes)
	okTRegisterEntry           registerEntries[256];

	// Number of valid Trigger Entries (4 bytes)
	UINT32                     triggerEntryCount;

	// Initial trigger assertions.  These are performed last.
	// (32*8 = 256 bytes)
	okTTriggerEntry            triggerEntries[32];

	// Padding to a 4096-byte size for future expansion
	UINT8                      padBytes2[1520];
} okTFPGAResetProfile;


// Describes the layout of an available Flash memory on the device
typedef struct {
	UINT32             sectorCount;
	UINT32             sectorSize;
	UINT32             pageSize;
	UINT32             minUserSector;
	UINT32             maxUserSector;
} okTFlashLayout;


typedef struct {
	char            deviceID[OK_MAX_DEVICEID_LENGTH];
	char            serialNumber[OK_MAX_SERIALNUMBER_LENGTH];
	char            productName[OK_MAX_BOARD_MODEL_STRING_LENGTH];
	int             productID;
	int             deviceInterface;
	int             usbSpeed;
	int             deviceMajorVersion;
	int             deviceMinorVersion;
	int             hostInterfaceMajorVersion;
	int             hostInterfaceMinorVersion;
	okBool          isPLL22150Supported;
	okBool          isPLL22393Supported;
	okBool          isFrontPanelEnabled;
	int             wireWidth;
	int             triggerWidth;
	int             pipeWidth;
	int             registerAddressWidth;
	int             registerDataWidth;

	okTFlashLayout  flashSystem;
	okTFlashLayout  flashFPGA;

	okBool          hasFMCEEPROM;
	okBool          hasResetProfiles;
} okTDeviceInfo;


typedef struct {
	char            productName[OK_MAX_PRODUCT_NAME_LENGTH];
	int             productBaseID;
	int             productID;
	UINT32          usbVID;
	UINT32          usbPID;
	UINT32          pciDID;
} okTDeviceMatchInfo;


typedef enum {
	okDEVICESENSOR_INVALID,
	okDEVICESENSOR_BOOL,
	okDEVICESENSOR_INTEGER,
	okDEVICESENSOR_FLOAT,
	okDEVICESENSOR_VOLTAGE,
	okDEVICESENSOR_CURRENT,
	okDEVICESENSOR_TEMPERATURE,
	okDEVICESENSOR_FAN_RPM
} okEDeviceSensorType;

#define OK_MAX_DEVICE_SENSOR_NAME_LENGTH   (64)   // including NULL termination
#define OK_MAX_DEVICE_SENSOR_DESCRIPTION_LENGTH   (256)   // including NULL termination
typedef struct {
    int                    id;
    okEDeviceSensorType    type;
    char                   name[OK_MAX_DEVICE_SENSOR_NAME_LENGTH];
    char                   description[OK_MAX_DEVICE_SENSOR_DESCRIPTION_LENGTH];
    double                 min;
    double                 max;
    double                 step;
    double                 value;
} okTDeviceSensor;

#endif // ! FRONTPANELDLL_EXPORTS

typedef enum okEFPGAConfigurationMethod ok_FPGAConfigurationMethod;

//
// General
//
okDLLEXPORT Bool DLL_ENTRY okFrontPanelDLL_LoadLib(okFP_dll_pchar libname);
okDLLEXPORT void DLL_ENTRY okFrontPanelDLL_FreeLib(void);
okDLLEXPORT void DLL_ENTRY okFrontPanelDLL_GetVersion(char *date, char *time);

// This function is special and doesn't really need to be called at all if the
// DLL is always available, as it will be always loaded anyhow. It is mostly
// useful when using delay-loading, i.e. using "/DELAYLOAD:okFrontPanel.dll"
// linker option as then it will return false if the DLL couldn't be loaded.
inline Bool okFrontPanelDLL_TryLoadLib()
{
#ifdef okDELAY_LOAD_FRONTPANEL
	__try {
		return okFrontPanelDLL_LoadLib(NULL);
	} __except (ok_DelayLoadDllExceptionFilter(GetExceptionInformation())) {
		return FALSE;
	}
#endif // okDELAY_LOAD_FRONTPANEL

	return TRUE;
}

//
// okPLL22393
//
okDLLEXPORT okPLL22393_HANDLE DLL_ENTRY okPLL22393_Construct();
okDLLEXPORT void DLL_ENTRY okPLL22393_Destruct(okPLL22393_HANDLE pll);
okDLLEXPORT void DLL_ENTRY okPLL22393_SetCrystalLoad(okPLL22393_HANDLE pll, double capload);
okDLLEXPORT void DLL_ENTRY okPLL22393_SetReference(okPLL22393_HANDLE pll, double freq);
okDLLEXPORT double DLL_ENTRY okPLL22393_GetReference(okPLL22393_HANDLE pll);
okDLLEXPORT Bool DLL_ENTRY okPLL22393_SetPLLParameters(okPLL22393_HANDLE pll, int n, int p, int q, Bool enable);
okDLLEXPORT Bool DLL_ENTRY okPLL22393_SetPLLLF(okPLL22393_HANDLE pll, int n, int lf);
okDLLEXPORT Bool DLL_ENTRY okPLL22393_SetOutputDivider(okPLL22393_HANDLE pll, int n, int div);
okDLLEXPORT Bool DLL_ENTRY okPLL22393_SetOutputSource(okPLL22393_HANDLE pll, int n, ok_ClockSource_22393 clksrc);
okDLLEXPORT void DLL_ENTRY okPLL22393_SetOutputEnable(okPLL22393_HANDLE pll, int n, Bool enable);
okDLLEXPORT int DLL_ENTRY okPLL22393_GetPLLP(okPLL22393_HANDLE pll, int n);
okDLLEXPORT int DLL_ENTRY okPLL22393_GetPLLQ(okPLL22393_HANDLE pll, int n);
okDLLEXPORT double DLL_ENTRY okPLL22393_GetPLLFrequency(okPLL22393_HANDLE pll, int n);
okDLLEXPORT int DLL_ENTRY okPLL22393_GetOutputDivider(okPLL22393_HANDLE pll, int n);
okDLLEXPORT ok_ClockSource_22393 DLL_ENTRY okPLL22393_GetOutputSource(okPLL22393_HANDLE pll, int n);
okDLLEXPORT double DLL_ENTRY okPLL22393_GetOutputFrequency(okPLL22393_HANDLE pll, int n);
okDLLEXPORT Bool DLL_ENTRY okPLL22393_IsOutputEnabled(okPLL22393_HANDLE pll, int n);
okDLLEXPORT Bool DLL_ENTRY okPLL22393_IsPLLEnabled(okPLL22393_HANDLE pll, int n);
okDLLEXPORT void DLL_ENTRY okPLL22393_InitFromProgrammingInfo(okPLL22393_HANDLE pll, unsigned char *buf);
okDLLEXPORT void DLL_ENTRY okPLL22393_GetProgrammingInfo(okPLL22393_HANDLE pll, unsigned char *buf);


//
// okPLL22150
//
okDLLEXPORT okPLL22150_HANDLE DLL_ENTRY okPLL22150_Construct();
okDLLEXPORT void DLL_ENTRY okPLL22150_Destruct(okPLL22150_HANDLE pll);
okDLLEXPORT void DLL_ENTRY okPLL22150_SetCrystalLoad(okPLL22150_HANDLE pll, double capload);
okDLLEXPORT void DLL_ENTRY okPLL22150_SetReference(okPLL22150_HANDLE pll, double freq, Bool extosc);
okDLLEXPORT double DLL_ENTRY okPLL22150_GetReference(okPLL22150_HANDLE pll);
okDLLEXPORT Bool DLL_ENTRY okPLL22150_SetVCOParameters(okPLL22150_HANDLE pll, int p, int q);
okDLLEXPORT int DLL_ENTRY okPLL22150_GetVCOP(okPLL22150_HANDLE pll);
okDLLEXPORT int DLL_ENTRY okPLL22150_GetVCOQ(okPLL22150_HANDLE pll);
okDLLEXPORT double DLL_ENTRY okPLL22150_GetVCOFrequency(okPLL22150_HANDLE pll);
okDLLEXPORT void DLL_ENTRY okPLL22150_SetDiv1(okPLL22150_HANDLE pll, ok_DividerSource divsrc, int n);
okDLLEXPORT void DLL_ENTRY okPLL22150_SetDiv2(okPLL22150_HANDLE pll, ok_DividerSource divsrc, int n);
okDLLEXPORT ok_DividerSource DLL_ENTRY okPLL22150_GetDiv1Source(okPLL22150_HANDLE pll);
okDLLEXPORT ok_DividerSource DLL_ENTRY okPLL22150_GetDiv2Source(okPLL22150_HANDLE pll);
okDLLEXPORT int DLL_ENTRY okPLL22150_GetDiv1Divider(okPLL22150_HANDLE pll);
okDLLEXPORT int DLL_ENTRY okPLL22150_GetDiv2Divider(okPLL22150_HANDLE pll);
okDLLEXPORT void DLL_ENTRY okPLL22150_SetOutputSource(okPLL22150_HANDLE pll, int output, ok_ClockSource_22150 clksrc);
okDLLEXPORT void DLL_ENTRY okPLL22150_SetOutputEnable(okPLL22150_HANDLE pll, int output, Bool enable);
okDLLEXPORT ok_ClockSource_22150 DLL_ENTRY okPLL22150_GetOutputSource(okPLL22150_HANDLE pll, int output);
okDLLEXPORT double DLL_ENTRY okPLL22150_GetOutputFrequency(okPLL22150_HANDLE pll, int output);
okDLLEXPORT Bool DLL_ENTRY okPLL22150_IsOutputEnabled(okPLL22150_HANDLE pll, int output);
okDLLEXPORT void DLL_ENTRY okPLL22150_InitFromProgrammingInfo(okPLL22150_HANDLE pll, unsigned char *buf);
okDLLEXPORT void DLL_ENTRY okPLL22150_GetProgrammingInfo(okPLL22150_HANDLE pll, unsigned char *buf);


//
// okDeviceSensors
//
okDLLEXPORT okDeviceSensors_HANDLE DLL_ENTRY okDeviceSensors_Construct();
okDLLEXPORT void DLL_ENTRY okDeviceSensors_Destruct(okDeviceSensors_HANDLE hnd);
okDLLEXPORT int DLL_ENTRY okDeviceSensors_GetSensorCount(okDeviceSensors_HANDLE hnd);
okDLLEXPORT okTDeviceSensor DLL_ENTRY okDeviceSensors_GetSensor(okDeviceSensors_HANDLE hnd, int n);

//
// okDeviceSettings
//
okDLLEXPORT okDeviceSettings_HANDLE DLL_ENTRY okDeviceSettings_Construct();
okDLLEXPORT void DLL_ENTRY okDeviceSettings_Destruct(okDeviceSettings_HANDLE hnd);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okDeviceSettings_GetString(okDeviceSettings_HANDLE hnd, const char *key, int length, char *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okDeviceSettings_SetString(okDeviceSettings_HANDLE hnd, const char *key, const char *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okDeviceSettings_GetInt(okDeviceSettings_HANDLE hnd, const char *key, UINT32 *value);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okDeviceSettings_SetInt(okDeviceSettings_HANDLE hnd, const char *key, UINT32 value);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okDeviceSettings_Delete(okDeviceSettings_HANDLE hnd, const char *key);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okDeviceSettings_Save(okDeviceSettings_HANDLE hnd);

//
// okFirmware and okFirmwarePackage
//
okDLLEXPORT okFirmwarePackage_HANDLE DLL_ENTRY okFirmwarePackage_Load(const char *filename);
okDLLEXPORT void DLL_ENTRY okFirmwarePackage_Destruct(okFirmwarePackage_HANDLE hnd);
okDLLEXPORT int DLL_ENTRY okFirmwarePackage_GetFirmwareCount(okFirmwarePackage_HANDLE hnd);
okDLLEXPORT okFirmware_HANDLE DLL_ENTRY okFirmwarePackage_GetFirmware(okFirmwarePackage_HANDLE hnd, int num);

typedef void (*okFirmware_PerformTasks_Callback)(void*, int, const char*);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFirmware_PerformTasks(okFirmware_HANDLE hnd, const char *serial, okFirmware_PerformTasks_Callback callback, void *arg);

//
// okFrontPanel
//
okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY okFrontPanel_Construct();
okDLLEXPORT void DLL_ENTRY okFrontPanel_Destruct(okFrontPanel_HANDLE hnd);
okDLLEXPORT int DLL_ENTRY okFrontPanel_GetErrorString(int ec, char *buf, int length);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_AddCustomDevice(const okTDeviceMatchInfo* matchInfo, const okTDeviceInfo* devInfo);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_RemoveCustomDevice(int productID);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_WriteI2C(okFrontPanel_HANDLE hnd, const int addr, int length, unsigned char *data);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ReadI2C(okFrontPanel_HANDLE hnd, const int addr, int length, unsigned char *data);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_FlashEraseSector(okFrontPanel_HANDLE hnd, UINT32 address);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_FlashWrite(okFrontPanel_HANDLE hnd, UINT32 address, UINT32 length, const UINT8 *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_FlashRead(okFrontPanel_HANDLE hnd, UINT32 address, UINT32 length, UINT8 *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetFPGAResetProfile(okFrontPanel_HANDLE hnd, ok_FPGAConfigurationMethod method, okTFPGAResetProfile *profile);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetFPGAResetProfileWithSize(okFrontPanel_HANDLE hnd, ok_FPGAConfigurationMethod method, okTFPGAResetProfile *profile, unsigned size);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetFPGAResetProfile(okFrontPanel_HANDLE hnd, ok_FPGAConfigurationMethod method, const okTFPGAResetProfile *profile);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetFPGAResetProfileWithSize(okFrontPanel_HANDLE hnd, ok_FPGAConfigurationMethod method, const okTFPGAResetProfile *profile, unsigned size);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ReadRegister(okFrontPanel_HANDLE hnd, UINT32 addr, UINT32 *data);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ReadRegisters(okFrontPanel_HANDLE hnd, unsigned num, okTRegisterEntry* regs);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_WriteRegister(okFrontPanel_HANDLE hnd, UINT32 addr, UINT32 data);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_WriteRegisters(okFrontPanel_HANDLE hnd, unsigned num, const okTRegisterEntry* regs);
okDLLEXPORT int DLL_ENTRY okFrontPanel_GetHostInterfaceWidth(okFrontPanel_HANDLE hnd);
okDLLEXPORT Bool DLL_ENTRY okFrontPanel_IsHighSpeed(okFrontPanel_HANDLE hnd);
okDLLEXPORT ok_BoardModel DLL_ENTRY okFrontPanel_GetBoardModel(okFrontPanel_HANDLE hnd);
okDLLEXPORT void DLL_ENTRY okFrontPanel_GetBoardModelString(okFrontPanel_HANDLE hnd, ok_BoardModel m, char *buf);
okDLLEXPORT int DLL_ENTRY okFrontPanel_GetDeviceCount(okFrontPanel_HANDLE hnd);
okDLLEXPORT ok_BoardModel DLL_ENTRY okFrontPanel_GetDeviceListModel(okFrontPanel_HANDLE hnd, int num);
okDLLEXPORT void DLL_ENTRY okFrontPanel_GetDeviceListSerial(okFrontPanel_HANDLE hnd, int num, char *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_OpenBySerial(okFrontPanel_HANDLE hnd, const char *serial);
okDLLEXPORT Bool DLL_ENTRY okFrontPanel_IsOpen(okFrontPanel_HANDLE hnd);
okDLLEXPORT void DLL_ENTRY okFrontPanel_EnableAsynchronousTransfers(okFrontPanel_HANDLE hnd, Bool enable);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetBTPipePollingInterval(okFrontPanel_HANDLE hnd, int interval);
okDLLEXPORT void DLL_ENTRY okFrontPanel_SetTimeout(okFrontPanel_HANDLE hnd, int timeout);
okDLLEXPORT int DLL_ENTRY okFrontPanel_GetDeviceMajorVersion(okFrontPanel_HANDLE hnd);
okDLLEXPORT int DLL_ENTRY okFrontPanel_GetDeviceMinorVersion(okFrontPanel_HANDLE hnd);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ResetFPGA(okFrontPanel_HANDLE hnd);
okDLLEXPORT void DLL_ENTRY okFrontPanel_Close(okFrontPanel_HANDLE hnd);
okDLLEXPORT void DLL_ENTRY okFrontPanel_GetSerialNumber(okFrontPanel_HANDLE hnd, char *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetDeviceSensors(okFrontPanel_HANDLE hnd, okDeviceSensors_HANDLE settings);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetDeviceSettings(okFrontPanel_HANDLE hnd, okDeviceSettings_HANDLE settings);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetDeviceInfo(okFrontPanel_HANDLE hnd, okTDeviceInfo *info);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetDeviceInfoWithSize(okFrontPanel_HANDLE hnd, okTDeviceInfo *info, unsigned size);
okDLLEXPORT void DLL_ENTRY okFrontPanel_GetDeviceID(okFrontPanel_HANDLE hnd, char *buf);
okDLLEXPORT void DLL_ENTRY okFrontPanel_SetDeviceID(okFrontPanel_HANDLE hnd, const char *strID);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGA(okFrontPanel_HANDLE hnd, const char *strFilename);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGAWithReset(okFrontPanel_HANDLE hnd, const char *strFilename, const okTFPGAResetProfile *reset);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGAFromMemory(okFrontPanel_HANDLE hnd, unsigned char *data, unsigned long length);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGAFromMemoryWithReset(okFrontPanel_HANDLE hnd, unsigned char *data, unsigned long length, const okTFPGAResetProfile *reset);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGAFromFlash(okFrontPanel_HANDLE hnd, unsigned long configIndex);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetEepromPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetEepromPLL22150Configuration(okFrontPanel_HANDLE hnd, okPLL22150_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetEepromPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetEepromPLL22393Configuration(okFrontPanel_HANDLE hnd, okPLL22393_HANDLE pll);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_LoadDefaultPLLConfiguration(okFrontPanel_HANDLE hnd);
okDLLEXPORT Bool DLL_ENTRY okFrontPanel_IsFrontPanelEnabled(okFrontPanel_HANDLE hnd);
okDLLEXPORT Bool DLL_ENTRY okFrontPanel_IsFrontPanel3Supported(okFrontPanel_HANDLE hnd);
okDLLEXPORT void DLL_ENTRY okFrontPanel_UpdateWireIns(okFrontPanel_HANDLE hnd);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetWireInValue(okFrontPanel_HANDLE hnd, int epAddr, UINT32 *val);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetWireInValue(okFrontPanel_HANDLE hnd, int ep, unsigned long val, unsigned long mask);
okDLLEXPORT void DLL_ENTRY okFrontPanel_UpdateWireOuts(okFrontPanel_HANDLE hnd);
okDLLEXPORT unsigned long DLL_ENTRY okFrontPanel_GetWireOutValue(okFrontPanel_HANDLE hnd, int epAddr);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ActivateTriggerIn(okFrontPanel_HANDLE hnd, int epAddr, int bit);
okDLLEXPORT void DLL_ENTRY okFrontPanel_UpdateTriggerOuts(okFrontPanel_HANDLE hnd);
okDLLEXPORT Bool DLL_ENTRY okFrontPanel_IsTriggered(okFrontPanel_HANDLE hnd, int epAddr, unsigned long mask);
okDLLEXPORT long DLL_ENTRY okFrontPanel_GetLastTransferLength(okFrontPanel_HANDLE hnd);
okDLLEXPORT long DLL_ENTRY okFrontPanel_WriteToPipeIn(okFrontPanel_HANDLE hnd, int epAddr, long length, unsigned char *data);
okDLLEXPORT long DLL_ENTRY okFrontPanel_ReadFromPipeOut(okFrontPanel_HANDLE hnd, int epAddr, long length, unsigned char *data);
okDLLEXPORT long DLL_ENTRY okFrontPanel_WriteToBlockPipeIn(okFrontPanel_HANDLE hnd, int epAddr, int blockSize, long length, unsigned char *data);
okDLLEXPORT long DLL_ENTRY okFrontPanel_ReadFromBlockPipeOut(okFrontPanel_HANDLE hnd, int epAddr, int blockSize, long length, unsigned char *data);


//
// okFrontPanelManager
//
okDLLEXPORT okCFrontPanelManager_HANDLE DLL_ENTRY okFrontPanelManager_Construct(okFrontPanelManager_HANDLE self, const char* realm);
okDLLEXPORT void DLL_ENTRY okFrontPanelManager_Destruct(okCFrontPanelManager_HANDLE hnd);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanelManager_StartMonitoring(okCFrontPanelManager_HANDLE hnd);
okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY okFrontPanelManager_Open(okCFrontPanelManager_HANDLE hnd, const char *serial);

//
// FrontPanelDevices
//
okDLLEXPORT okCFrontPanelDevices_HANDLE DLL_ENTRY okFrontPanelDevices_Construct(const char* realm);
okDLLEXPORT void DLL_ENTRY okFrontPanelDevices_Destruct(okCFrontPanelDevices_HANDLE hnd);
okDLLEXPORT int DLL_ENTRY okFrontPanelDevices_GetCount(okCFrontPanelDevices_HANDLE hnd);
okDLLEXPORT void DLL_ENTRY okFrontPanelDevices_GetSerial(okCFrontPanelDevices_HANDLE hnd, int num, char* buf);
okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY okFrontPanelDevices_Open(okCFrontPanelDevices_HANDLE hnd, const char* serial);

#ifdef __cplusplus
} // extern "C"

namespace OpalKellyLegacy
{
    class okCFrontPanel;
}

#if !defined(FRONTPANELDLL_EXPORTS)

namespace OpalKelly
{

class FrontPanelManager;
class FrontPanelDevices;

} // namespace OpalKelly

namespace OpalKellyLegacy
{

//------------------------------------------------------------------------
// okCPLL22150 C++ wrapper class
//------------------------------------------------------------------------
class okCPLL22150
{
public:
	okPLL22150_HANDLE h;
	enum ClockSource {
			ClkSrc_Ref=0,
			ClkSrc_Div1ByN=1,
			ClkSrc_Div1By2=2,
			ClkSrc_Div1By3=3,
            ClkSrc_Div2ByN=4,
			ClkSrc_Div2By2=5,
			ClkSrc_Div2By4=6 };

	enum DividerSource {
			DivSrc_Ref = 0, 
			DivSrc_VCO = 1 };
public:
	okCPLL22150();
	void SetCrystalLoad(double capload);
	void SetReference(double freq, bool extosc);
	double GetReference();
	bool SetVCOParameters(int p, int q);
	int GetVCOP();
	int GetVCOQ();
	double GetVCOFrequency();
	void SetDiv1(DividerSource divsrc, int n);
	void SetDiv2(DividerSource divsrc, int n);
	DividerSource GetDiv1Source();
	DividerSource GetDiv2Source();
	int GetDiv1Divider();
	int GetDiv2Divider();
	void SetOutputSource(int output, ClockSource clksrc);
	void SetOutputEnable(int output, bool enable);
	ClockSource GetOutputSource(int output);
	double GetOutputFrequency(int output);
	bool IsOutputEnabled(int output);
	void InitFromProgrammingInfo(unsigned char *buf);
	void GetProgrammingInfo(unsigned char *buf);
};

//------------------------------------------------------------------------
// okCPLL22150 C++ wrapper class
//------------------------------------------------------------------------
class okCPLL22393
{
public:
	okPLL22393_HANDLE h;
	enum ClockSource {
			ClkSrc_Ref=0,
			ClkSrc_PLL0_0=2,
			ClkSrc_PLL0_180=3,
			ClkSrc_PLL1_0=4,
			ClkSrc_PLL1_180=5,
			ClkSrc_PLL2_0=6,
			ClkSrc_PLL2_180=7 };
private:
	bool to_bool(Bool x);
	Bool from_bool(bool x);
public:
	okCPLL22393();
	void SetCrystalLoad(double capload);
	void SetReference(double freq);
	double GetReference();
	bool SetPLLParameters(int n, int p, int q, bool enable=true);
	bool SetPLLLF(int n, int lf);
	bool SetOutputDivider(int n, int div);
	bool SetOutputSource(int n, ClockSource clksrc);
	void SetOutputEnable(int n, bool enable);
	int GetPLLP(int n);
	int GetPLLQ(int n);
	double GetPLLFrequency(int n);
	int GetOutputDivider(int n);
	ClockSource GetOutputSource(int n);
	double GetOutputFrequency(int n);
	bool IsOutputEnabled(int n);
	bool IsPLLEnabled(int n);
	void InitFromProgrammingInfo(unsigned char *buf);
	void GetProgrammingInfo(unsigned char *buf);
};

//------------------------------------------------------------------------
// okCFrontPanel C++ wrapper class
//------------------------------------------------------------------------

class okCDeviceSensors;
class okCDeviceSettings;

class okCFrontPanel
{
public:
	okFrontPanel_HANDLE h;
	enum BoardModel {
		brdUnknown=0,
		brdXEM3001v1=1,
		brdXEM3001v2=2,
		brdXEM3010=3,
		brdXEM3005=4,
		brdXEM3001CL=5,
		brdXEM3020=6,
		brdXEM3050=7,
		brdXEM9002=8,
		brdXEM3001RB=9,
		brdXEM5010=10,
		brdXEM6110LX45=11,
		brdXEM6110LX150=15,
		brdXEM6001=12,
		brdXEM6010LX45=13,
		brdXEM6010LX150=14,
		brdXEM6006LX9=16,
		brdXEM6006LX16=17,
		brdXEM6006LX25=18,
		brdXEM5010LX110=19,
		brdZEM4310=20,
		brdXEM6310LX45=21,
		brdXEM6310LX150=22,
		brdXEM6110v2LX45=23,
		brdXEM6110v2LX150=24,
		brdXEM6002LX9=25,
		brdXEM6310MTLX45T=26,
		brdXEM6320LX130T=27,
		brdXEM7350K70T=28,
		brdXEM7350K160T=29,
		brdXEM7350K410T=30,
		brdXEM6310MTLX150T=31,
		brdZEM5305A2=32,
		brdZEM5305A7=33,
		brdXEM7001A15=34,
		brdXEM7001A35=35,
		brdXEM7360K160T=36,
		brdXEM7360K410T=37,
		brdZEM5310A4=38,
		brdZEM5310A7=39,
		brdZEM5370A5=40,
		brdXEM7010A50=41,
		brdXEM7010A200=42,
		brdXEM7310A75=43,
		brdXEM7310A200=44
	};
	enum ErrorCode {
		NoError                    = 0,
		Failed                     = -1,
		Timeout                    = -2,
		DoneNotHigh                = -3,
		TransferError              = -4,
		CommunicationError         = -5,
		InvalidBitstream           = -6,
		FileError                  = -7,
		DeviceNotOpen              = -8,
		InvalidEndpoint            = -9,
		InvalidBlockSize           = -10,
		I2CRestrictedAddress       = -11,
		I2CBitError                = -12,
		I2CNack                    = -13,
		I2CUnknownStatus           = -14,
		UnsupportedFeature         = -15,
		FIFOUnderflow              = -16,
		FIFOOverflow               = -17,
		DataAlignmentError         = -18,
		InvalidResetProfile        = -19,
		InvalidParameter           = -20
	};
private:
	bool to_bool(Bool x);
	Bool from_bool(bool x);

	explicit okCFrontPanel(okFrontPanel_HANDLE hnd);
	friend class OpalKelly::FrontPanelManager;
	friend class OpalKelly::FrontPanelDevices;

public:
	okCFrontPanel();
	~okCFrontPanel();
	static std::string GetErrorString(int errorCode);
	static ErrorCode AddCustomDevice(const okTDeviceMatchInfo& matchInfo, const okTDeviceInfo* devInfo = NULL);
	static ErrorCode RemoveCustomDevice(int productID);
	int GetHostInterfaceWidth();
	BoardModel GetBoardModel();
	std::string GetBoardModelString(BoardModel m);
	int GetDeviceCount();
	ErrorCode GetFPGAResetProfile(okEFPGAConfigurationMethod method, okTFPGAResetProfile *profile);
	ErrorCode SetFPGAResetProfile(okEFPGAConfigurationMethod method, const okTFPGAResetProfile *profile);
	ErrorCode FlashEraseSector(UINT32 address);
	ErrorCode FlashWrite(UINT32 address, UINT32 length, const UINT8 *buf);
	ErrorCode FlashRead(UINT32 address, UINT32 length, UINT8 *buf);
	ErrorCode ReadRegister(UINT32 addr, UINT32 *data);
	ErrorCode ReadRegisters(okTRegisterEntries& regs);
	ErrorCode WriteRegister(UINT32 addr, UINT32 data);
	ErrorCode WriteRegisters(const okTRegisterEntries& regs);
	BoardModel GetDeviceListModel(int num);
	std::string GetDeviceListSerial(int num);
	void EnableAsynchronousTransfers(bool enable);
	ErrorCode OpenBySerial(std::string str = "");
	bool IsOpen();
	ErrorCode GetDeviceInfo(okTDeviceInfo *info);
	int GetDeviceMajorVersion();
	int GetDeviceMinorVersion();
	std::string GetSerialNumber();
	ErrorCode GetDeviceSettings(okCDeviceSettings& settings);
	ErrorCode GetDeviceSensors(okCDeviceSensors& sensors);
	std::string GetDeviceID();
	void SetDeviceID(const std::string& str);
	ErrorCode SetBTPipePollingInterval(int interval);
	void SetTimeout(int timeout);
	ErrorCode ResetFPGA();
	void Close();
	ErrorCode ConfigureFPGA(const std::string& strFilename);
	ErrorCode ConfigureFPGAWithReset(const std::string& strFilename, const okTFPGAResetProfile *reset);
	ErrorCode ConfigureFPGAFromMemory(unsigned char *data, const unsigned long length);
	ErrorCode ConfigureFPGAFromMemoryWithReset(unsigned char *data, const unsigned long length, const okTFPGAResetProfile *reset);
	ErrorCode ConfigureFPGAFromFlash(unsigned long configIndex);
	ErrorCode WriteI2C(const int addr, int length, unsigned char *data);
	ErrorCode ReadI2C(const int addr, int length, unsigned char *data);
	ErrorCode GetPLL22150Configuration(okCPLL22150& pll);
	ErrorCode SetPLL22150Configuration(okCPLL22150& pll);
	ErrorCode GetEepromPLL22150Configuration(okCPLL22150& pll);
	ErrorCode SetEepromPLL22150Configuration(okCPLL22150& pll);
	ErrorCode GetPLL22393Configuration(okCPLL22393& pll);
	ErrorCode SetPLL22393Configuration(okCPLL22393& pll);
	ErrorCode GetEepromPLL22393Configuration(okCPLL22393& pll);
	ErrorCode SetEepromPLL22393Configuration(okCPLL22393& pll);
	ErrorCode LoadDefaultPLLConfiguration();
	bool IsHighSpeed();
	bool IsFrontPanelEnabled();
	bool IsFrontPanel3Supported();
	void UpdateWireIns();
	ErrorCode GetWireInValue(int epAddr, UINT32 *val);
	ErrorCode SetWireInValue(int ep, UINT32 val, UINT32 mask = 0xffffffff);
	void UpdateWireOuts();
	unsigned long GetWireOutValue(int epAddr);
	ErrorCode ActivateTriggerIn(int epAddr, int bit);
	void UpdateTriggerOuts();
	bool IsTriggered(int epAddr, UINT32 mask);
	long GetLastTransferLength();
	long WriteToPipeIn(int epAddr, long length, unsigned char *data);
	long ReadFromPipeOut(int epAddr, long length, unsigned char *data);
	long WriteToBlockPipeIn(int epAddr, int blockSize, long length, unsigned char *data);
	long ReadFromBlockPipeOut(int epAddr, int blockSize, long length, unsigned char *data);
};

class okCDeviceSettings
{
public:
	okCDeviceSettings()
	{
		h = okDeviceSettings_Construct();
	}

	~okCDeviceSettings()
	{
		okDeviceSettings_Destruct(h);
	}

	okCFrontPanel::ErrorCode GetString(const std::string& key, std::string* value)
	{
		char buf[256];
		ok_ErrorCode rc = okDeviceSettings_GetString(h, key.c_str(), sizeof(buf), buf);
		if (rc == ok_NoError) {
			if (!value)
				rc = ok_InvalidParameter;
			else
				value->assign(buf);
		}

		return static_cast<okCFrontPanel::ErrorCode>(rc);
	}

	okCFrontPanel::ErrorCode GetInt(const std::string& key, UINT32* value)
	{
		return static_cast<okCFrontPanel::ErrorCode>(
					okDeviceSettings_GetInt(h, key.c_str(), value)
				);
	}

	okCFrontPanel::ErrorCode SetString(const std::string& key, const std::string& value)
	{
		return static_cast<okCFrontPanel::ErrorCode>(
					okDeviceSettings_SetString(h, key.c_str(), value.c_str())
				);
	}

	okCFrontPanel::ErrorCode SetInt(const std::string& key, UINT32 value)
	{
		return static_cast<okCFrontPanel::ErrorCode>(
					okDeviceSettings_SetInt(h, key.c_str(), value)
				);
	}

	okCFrontPanel::ErrorCode Delete(const std::string& key)
	{
		return static_cast<okCFrontPanel::ErrorCode>(
					okDeviceSettings_Delete(h, key.c_str())
				);
	}

	okCFrontPanel::ErrorCode Save()
	{
		return static_cast<okCFrontPanel::ErrorCode>(okDeviceSettings_Save(h));
	}

private:
	okDeviceSettings_HANDLE h;

	friend class okCFrontPanel;
};

class okCDeviceSensors
{
public:
	okCDeviceSensors()
	{
		h = okDeviceSensors_Construct();
	}

	~okCDeviceSensors()
	{
		okDeviceSensors_Destruct(h);
	}

	int GetSensorCount() const
	{
		return okDeviceSensors_GetSensorCount(h);
	}

	okTDeviceSensor GetSensor(int n) const
	{
		return okDeviceSensors_GetSensor(h, n);
	}

private:
	okDeviceSensors_HANDLE h;

	friend class OpalKellyLegacy::okCFrontPanel;
};

} // namespace OpalKellyLegacy

#endif // !defined(FRONTPANELDLL_EXPORTS)

namespace OpalKelly
{

/// \brief Represents a single firmware defined inside a firmware package.
///
/// Objects of this class are only created by OpalKelly::FirmwarePackage.
class Firmware
{
public:
	/// \brief Perform the firmware download to the given device.
	///
	/// If @a callback is non-null, it will be called to notify about the
	/// progress of the operation with the library provided progress indicator
	/// (in 0..100 range) and the status message and an extra @a arg argument
	/// passed to this function.
	bool PerformTasks(
			const std::string& serial,
			okFirmware_PerformTasks_Callback callback = NULL,
			void* arg = NULL
		)
	{
		return h && okFirmware_PerformTasks(h, serial.c_str(), callback, arg) == ok_NoError;
	}

private:
	Firmware(okFirmware_HANDLE h) : h(h) {}

	// This class can't be assigned to once it's created.
	Firmware& operator=(const Firmware&);

	const okFirmware_HANDLE h;

	friend class FirmwarePackage;
};


/// \brief Represents a firmware package file.
class FirmwarePackage
{
public:
	/// \brief Load firmware package from file.
	///
	/// If unsuccessful, GetFirmwareCount() will return 0 after loading.
	explicit FirmwarePackage(const std::string& filename) :
		h(okFirmwarePackage_Load(filename.c_str()))
	{
	}

	/// \brief Return the number of firmwares defined in this package.
	int GetFirmwareCount() const
	{
		return h ? okFirmwarePackage_GetFirmwareCount(h) : 0;
	}

	/// \brief Return the firmware object at the given index.
	///
	/// Index must be valid, i.e. less than GetFirmwareCount(), otherwise an
	/// invalid firmware is returned.
	Firmware GetFirmware(int num = 0) const
	{
		return Firmware(h ? okFirmwarePackage_GetFirmware(h, num) : NULL);
	}


	~FirmwarePackage()
	{
		if (h)
			okFirmwarePackage_Destruct(h);
	}

private:
	// This class can't be copied.
	FirmwarePackage(const FirmwarePackage&);
	FirmwarePackage& operator=(const FirmwarePackage&);

	const okFirmwarePackage_HANDLE h;
};


/// \brief Class for managing device connections and disconnections.
///
/// This class allows to be notified about devices connections and
/// disconnections.
///
/// To receive these notifications, you need to derive your own manager
/// subclass from this class and override its virtual OnDeviceAdded() and
/// OnDeviceRemoved() methods.
/// Alternatively, use FrontPanelDevices if you need to simply open a connected
/// device and are not interested in getting notifications for the devices
/// connections and disconnections.
///
/// See Open() method documentation for a simple example of doing this.
class FrontPanelManager
{
public:
	explicit FrontPanelManager(const std::string& realm = "local");
	virtual ~FrontPanelManager();

	void StartMonitoring();

	virtual void OnDeviceAdded(const char *serial) = 0;
	virtual void OnDeviceRemoved(const char *serial) = 0;

    OpalKellyLegacy::okCFrontPanel* Open(const char *serial);

	okCFrontPanelManager_HANDLE h;
};

typedef std::auto_ptr<OpalKellyLegacy::okCFrontPanel> FrontPanelPtr;

/// \brief Allows to easily open devices in any realms.
class FrontPanelDevices
{
public:
	explicit FrontPanelDevices(const std::string& realm = std::string());

	int GetCount() const;
	std::string GetSerial(int num) const;
	FrontPanelPtr Open(const char *serial = NULL) const;

	~FrontPanelDevices();

	okCFrontPanelDevices_HANDLE h;
};

} // namespace OpalKelly

#if !defined(FRONTPANELDLL_EXPORTS)

namespace OpalKellyLegacy
{

//------------------------------------------------------------------------
// okCPLL22150 C++ wrapper class
//------------------------------------------------------------------------
inline okCPLL22150::okCPLL22150()
	{ h=okPLL22150_Construct(); }
inline void okCPLL22150::SetCrystalLoad(double capload)
	{ okPLL22150_SetCrystalLoad(h, capload); }
inline void okCPLL22150::SetReference(double freq, bool extosc)
	{ okPLL22150_SetReference(h, freq, extosc); }
inline double okCPLL22150::GetReference()
	{ return(okPLL22150_GetReference(h)); }
inline bool okCPLL22150::SetVCOParameters(int p, int q)
	{ return okPLL22150_SetVCOParameters(h,p,q) == TRUE; }
inline int okCPLL22150::GetVCOP()
	{ return(okPLL22150_GetVCOP(h)); }
inline int okCPLL22150::GetVCOQ()
	{ return(okPLL22150_GetVCOQ(h)); }
inline double okCPLL22150::GetVCOFrequency()
	{ return(okPLL22150_GetVCOFrequency(h)); }
inline void okCPLL22150::SetDiv1(DividerSource divsrc, int n)
	{ okPLL22150_SetDiv1(h, (ok_DividerSource)divsrc, n); }
inline void okCPLL22150::SetDiv2(DividerSource divsrc, int n)
	{ okPLL22150_SetDiv2(h, (ok_DividerSource)divsrc, n); }
inline okCPLL22150::DividerSource okCPLL22150::GetDiv1Source()
	{ return((DividerSource) okPLL22150_GetDiv1Source(h)); }
inline okCPLL22150::DividerSource okCPLL22150::GetDiv2Source()
	{ return((DividerSource) okPLL22150_GetDiv2Source(h)); }
inline int okCPLL22150::GetDiv1Divider()
	{ return(okPLL22150_GetDiv1Divider(h)); }
inline int okCPLL22150::GetDiv2Divider()
	{ return(okPLL22150_GetDiv2Divider(h)); }
inline void okCPLL22150::SetOutputSource(int output, okCPLL22150::ClockSource clksrc)
	{ okPLL22150_SetOutputSource(h, output, (ok_ClockSource_22150)clksrc); }
inline void okCPLL22150::SetOutputEnable(int output, bool enable)
	{ okPLL22150_SetOutputEnable(h, output, enable); }
inline okCPLL22150::ClockSource okCPLL22150::GetOutputSource(int output)
	{ return( (ClockSource)okPLL22150_GetOutputSource(h, output)); }
inline double okCPLL22150::GetOutputFrequency(int output)
	{ return(okPLL22150_GetOutputFrequency(h, output)); }
inline bool okCPLL22150::IsOutputEnabled(int output)
	{ return okPLL22150_IsOutputEnabled(h, output) == TRUE; }
inline void okCPLL22150::InitFromProgrammingInfo(unsigned char *buf)
	{ okPLL22150_InitFromProgrammingInfo(h, buf); }
inline void okCPLL22150::GetProgrammingInfo(unsigned char *buf)
	{ okPLL22150_GetProgrammingInfo(h, buf); }

//------------------------------------------------------------------------
// okCPLL22393 C++ wrapper class
//------------------------------------------------------------------------
inline okCPLL22393::okCPLL22393()
	{ h=okPLL22393_Construct(); }
inline void okCPLL22393::SetCrystalLoad(double capload)
	{ okPLL22393_SetCrystalLoad(h, capload); }
inline void okCPLL22393::SetReference(double freq)
	{ okPLL22393_SetReference(h, freq); }
inline double okCPLL22393::GetReference()
	{ return(okPLL22393_GetReference(h)); }
inline bool okCPLL22393::SetPLLParameters(int n, int p, int q, bool enable)
	{ return okPLL22393_SetPLLParameters(h, n, p, q, enable) == TRUE; }
inline bool okCPLL22393::SetPLLLF(int n, int lf)
	{ return okPLL22393_SetPLLLF(h, n, lf) == TRUE; }
inline bool okCPLL22393::SetOutputDivider(int n, int div)
	{ return okPLL22393_SetOutputDivider(h, n, div) == TRUE; }
inline bool okCPLL22393::SetOutputSource(int n, okCPLL22393::ClockSource clksrc)
	{ return okPLL22393_SetOutputSource(h, n, (ok_ClockSource_22393)clksrc) == TRUE; }
inline void okCPLL22393::SetOutputEnable(int n, bool enable)
	{ okPLL22393_SetOutputEnable(h, n, enable); }
inline int okCPLL22393::GetPLLP(int n)
	{ return(okPLL22393_GetPLLP(h, n)); }
inline int okCPLL22393::GetPLLQ(int n)
	{ return(okPLL22393_GetPLLQ(h, n)); }
inline double okCPLL22393::GetPLLFrequency(int n)
	{ return(okPLL22393_GetPLLFrequency(h, n)); }
inline int okCPLL22393::GetOutputDivider(int n)
	{ return(okPLL22393_GetOutputDivider(h, n)); }
inline okCPLL22393::ClockSource okCPLL22393::GetOutputSource(int n)
	{ return((ClockSource) okPLL22393_GetOutputSource(h, n)); }
inline double okCPLL22393::GetOutputFrequency(int n)
	{ return(okPLL22393_GetOutputFrequency(h, n)); }
inline bool okCPLL22393::IsOutputEnabled(int n)
	{ return okPLL22393_IsOutputEnabled(h, n) == TRUE; }
inline bool okCPLL22393::IsPLLEnabled(int n)
	{ return okPLL22393_IsPLLEnabled(h, n) == TRUE; }
inline void okCPLL22393::InitFromProgrammingInfo(unsigned char *buf)
	{ okPLL22393_InitFromProgrammingInfo(h, buf); }
inline void okCPLL22393::GetProgrammingInfo(unsigned char *buf)
	{ okPLL22393_GetProgrammingInfo(h, buf); }

//------------------------------------------------------------------------
// okCFrontPanel C++ wrapper class
//------------------------------------------------------------------------
inline okCFrontPanel::okCFrontPanel(okFrontPanel_HANDLE hnd)
	{ h=hnd; }
inline okCFrontPanel::okCFrontPanel()
	{ h=okFrontPanel_Construct(); }
inline okCFrontPanel::~okCFrontPanel()
	{ okFrontPanel_Destruct(h); }
inline std::string okCFrontPanel::GetErrorString(int ec)
	{
		int len = okFrontPanel_GetErrorString(ec, NULL, 0);
		std::string s;
		if (len > 0) {
			s.resize(len);
			okFrontPanel_GetErrorString(ec, &s[0], len + 1);
		}
		return s;
	}
inline okCFrontPanel::ErrorCode okCFrontPanel::AddCustomDevice(const okTDeviceMatchInfo& matchInfo, const okTDeviceInfo* devInfo)
	{ return (okCFrontPanel::ErrorCode)okFrontPanel_AddCustomDevice(&matchInfo, devInfo); }
inline okCFrontPanel::ErrorCode okCFrontPanel::RemoveCustomDevice(int productID)
	{ return (okCFrontPanel::ErrorCode)okFrontPanel_RemoveCustomDevice(productID); }
inline int okCFrontPanel::GetHostInterfaceWidth()
	{ return(okFrontPanel_GetHostInterfaceWidth(h)); }
inline bool okCFrontPanel::IsHighSpeed()
	{ return okFrontPanel_IsHighSpeed(h) == TRUE; }
inline okCFrontPanel::BoardModel okCFrontPanel::GetBoardModel()
	{ return((okCFrontPanel::BoardModel)okFrontPanel_GetBoardModel(h)); }
inline std::string okCFrontPanel::GetBoardModelString(okCFrontPanel::BoardModel m)
	{
		char str[MAX_BOARDMODELSTRING_LENGTH];
		okFrontPanel_GetBoardModelString(h, (ok_BoardModel)m, str);
		return(std::string(str));
	}
inline int okCFrontPanel::GetDeviceCount()
	{ return(okFrontPanel_GetDeviceCount(h)); }
inline okCFrontPanel::BoardModel okCFrontPanel::GetDeviceListModel(int num)
	{ return((okCFrontPanel::BoardModel)okFrontPanel_GetDeviceListModel(h, num)); }
inline std::string okCFrontPanel::GetDeviceListSerial(int num)
	{
		char str[MAX_SERIALNUMBER_LENGTH+1];
		okFrontPanel_GetDeviceListSerial(h, num, str);
		str[MAX_SERIALNUMBER_LENGTH] = '\0';
		return(std::string(str));
	}
inline okCFrontPanel::ErrorCode okCFrontPanel::GetDeviceInfo(okTDeviceInfo *info)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetDeviceInfoWithSize(h, info, sizeof(*info))); }
inline void okCFrontPanel::EnableAsynchronousTransfers(bool enable)
	{ okFrontPanel_EnableAsynchronousTransfers(h, enable); }
inline okCFrontPanel::ErrorCode okCFrontPanel::OpenBySerial(std::string str)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_OpenBySerial(h, str.c_str())); }
inline bool okCFrontPanel::IsOpen()
	{ return okFrontPanel_IsOpen(h) == TRUE; }
inline int okCFrontPanel::GetDeviceMajorVersion()
	{ return(okFrontPanel_GetDeviceMajorVersion(h)); }
inline int okCFrontPanel::GetDeviceMinorVersion()
	{ return(okFrontPanel_GetDeviceMinorVersion(h)); }
inline std::string okCFrontPanel::GetSerialNumber()
	{
		char str[MAX_SERIALNUMBER_LENGTH+1];
		okFrontPanel_GetSerialNumber(h, str);
		return(std::string(str));
	}
inline okCFrontPanel::ErrorCode okCFrontPanel::GetDeviceSettings(okCDeviceSettings& settings)
	{
		return (ErrorCode)okFrontPanel_GetDeviceSettings(h, settings.h);
	}
inline okCFrontPanel::ErrorCode okCFrontPanel::GetDeviceSensors(okCDeviceSensors& sensors)
	{
		return (ErrorCode)okFrontPanel_GetDeviceSensors(h, sensors.h);
	}
inline std::string okCFrontPanel::GetDeviceID()
	{
		char str[MAX_DEVICEID_LENGTH+1];
		okFrontPanel_GetDeviceID(h, str);
		return(std::string(str));
	}
inline void okCFrontPanel::SetDeviceID(const std::string& str)
	{ okFrontPanel_SetDeviceID(h, str.c_str()); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetBTPipePollingInterval(int interval)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetBTPipePollingInterval(h, interval)); }
inline void okCFrontPanel::SetTimeout(int timeout)
	{ okFrontPanel_SetTimeout(h, timeout); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ResetFPGA()
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ResetFPGA(h)); }
inline void okCFrontPanel::Close()
	{ okFrontPanel_Close(h); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGA(const std::string& strFilename)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGA(h, strFilename.c_str())); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGAWithReset(const std::string& strFilename, const okTFPGAResetProfile *reset)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGAWithReset(h, strFilename.c_str(), reset)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGAFromMemory(unsigned char *data, const unsigned long length)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGAFromMemory(h, data, length)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGAFromMemoryWithReset(unsigned char *data, const unsigned long length, const okTFPGAResetProfile* reset)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGAFromMemoryWithReset(h, data, length, reset)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ConfigureFPGAFromFlash(unsigned long configIndex)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ConfigureFPGAFromFlash(h, configIndex)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::GetFPGAResetProfile(okEFPGAConfigurationMethod method, okTFPGAResetProfile *profile)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetFPGAResetProfileWithSize(h, method, profile, sizeof(*profile))); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ReadRegister(UINT32 addr, UINT32 *data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ReadRegister(h, addr, data)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ReadRegisters(std::vector<okTRegisterEntry>& regs)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ReadRegisters(h, (unsigned int)regs.size(), regs.empty() ? NULL : &regs[0])); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetFPGAResetProfile(okEFPGAConfigurationMethod method, const okTFPGAResetProfile *profile)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetFPGAResetProfileWithSize(h, method, profile, sizeof(*profile))); }
inline okCFrontPanel::ErrorCode okCFrontPanel::FlashEraseSector(UINT32 address)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_FlashEraseSector(h, address)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::FlashWrite(UINT32 address, UINT32 length, const UINT8 *buf)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_FlashWrite(h, address, length, buf)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::FlashRead(UINT32 address, UINT32 length, UINT8 *buf)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_FlashRead(h, address, length, buf)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::WriteRegister(UINT32 addr, UINT32 data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_WriteRegister(h, addr, data)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::WriteRegisters(const std::vector<okTRegisterEntry>& regs)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_WriteRegisters(h, (unsigned int)regs.size(), regs.empty() ? NULL : &regs[0])); }
inline okCFrontPanel::ErrorCode okCFrontPanel::GetWireInValue(int epAddr, UINT32 *val)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetWireInValue(h, epAddr, val)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::WriteI2C(const int addr, int length, unsigned char *data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_WriteI2C(h, addr, length, data)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ReadI2C(const int addr, int length, unsigned char *data)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ReadI2C(h, addr, length, data)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::GetPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetPLL22150Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetPLL22150Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::GetEepromPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetEepromPLL22150Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetEepromPLL22150Configuration(okCPLL22150& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetEepromPLL22150Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::GetPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetPLL22393Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetPLL22393Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::GetEepromPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_GetEepromPLL22393Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetEepromPLL22393Configuration(okCPLL22393& pll)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetEepromPLL22393Configuration(h, pll.h)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::LoadDefaultPLLConfiguration()
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_LoadDefaultPLLConfiguration(h)); }
inline bool okCFrontPanel::IsFrontPanelEnabled()
	{ return okFrontPanel_IsFrontPanelEnabled(h) == TRUE; }
inline bool okCFrontPanel::IsFrontPanel3Supported()
	{ return okFrontPanel_IsFrontPanel3Supported(h) == TRUE; }
//	void UnregisterAll();
//	void AddEventHandler(EventHandler *handler);
inline void okCFrontPanel::UpdateWireIns()
	{ okFrontPanel_UpdateWireIns(h); }
inline okCFrontPanel::ErrorCode okCFrontPanel::SetWireInValue(int ep, UINT32 val, UINT32 mask)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_SetWireInValue(h, ep, val, mask)); }
inline void okCFrontPanel::UpdateWireOuts()
	{ okFrontPanel_UpdateWireOuts(h); }
inline unsigned long okCFrontPanel::GetWireOutValue(int epAddr)
	{ return(okFrontPanel_GetWireOutValue(h, epAddr)); }
inline okCFrontPanel::ErrorCode okCFrontPanel::ActivateTriggerIn(int epAddr, int bit)
	{ return((okCFrontPanel::ErrorCode) okFrontPanel_ActivateTriggerIn(h, epAddr, bit)); }
inline void okCFrontPanel::UpdateTriggerOuts()
	{ okFrontPanel_UpdateTriggerOuts(h); }
inline bool okCFrontPanel::IsTriggered(int epAddr, UINT32 mask)
	{ return okFrontPanel_IsTriggered(h, epAddr, mask) == TRUE; }
inline long okCFrontPanel::GetLastTransferLength()
	{ return(okFrontPanel_GetLastTransferLength(h)); }
inline long okCFrontPanel::WriteToPipeIn(int epAddr, long length, unsigned char *data)
	{ return(okFrontPanel_WriteToPipeIn(h, epAddr, length, data)); }
inline long okCFrontPanel::ReadFromPipeOut(int epAddr, long length, unsigned char *data)
	{ return(okFrontPanel_ReadFromPipeOut(h, epAddr, length, data)); }
inline long okCFrontPanel::WriteToBlockPipeIn(int epAddr, int blockSize, long length, unsigned char *data)
	{ return(okFrontPanel_WriteToBlockPipeIn(h, epAddr, blockSize, length, data)); }
inline long okCFrontPanel::ReadFromBlockPipeOut(int epAddr, int blockSize, long length, unsigned char *data)
	{ return(okFrontPanel_ReadFromBlockPipeOut(h, epAddr, blockSize, length, data)); }

} // namespace OpalKellyLegacy

//------------------------------------------------------------------------
// FrontPanelManagerHandle C++ wrapper class
//------------------------------------------------------------------------

namespace OpalKelly
{

inline FrontPanelManager::FrontPanelManager(const std::string& realm)
	{ h=okFrontPanelManager_Construct(reinterpret_cast<okFrontPanelManager_HANDLE>(this), realm.c_str()); }
inline FrontPanelManager::~FrontPanelManager()
	{ okFrontPanelManager_Destruct(h); }

inline void FrontPanelManager::StartMonitoring()
{
	if (okFrontPanelManager_StartMonitoring(h) != ok_NoError)
		throw std::runtime_error("Failed to start monitoring devices connection.");
}

inline OpalKellyLegacy::okCFrontPanel* FrontPanelManager::Open(const char *serial)
{
	okFrontPanel_HANDLE hFP = okFrontPanelManager_Open(h, serial);
	return hFP ? new OpalKellyLegacy::okCFrontPanel(hFP) : NULL;
}

inline FrontPanelDevices::FrontPanelDevices(const std::string& realm)
	{ h = okFrontPanelDevices_Construct(realm.c_str()); }
inline FrontPanelDevices::~FrontPanelDevices()
	{ okFrontPanelDevices_Destruct(h); }

inline int FrontPanelDevices::GetCount() const
	{ return okFrontPanelDevices_GetCount(h); }
inline std::string FrontPanelDevices::GetSerial(int num) const
	{
		char str[MAX_SERIALNUMBER_LENGTH+1];
		okFrontPanelDevices_GetSerial(h, num, str);
		return str;
	}

inline FrontPanelPtr FrontPanelDevices::Open(const char* serial) const
	{
		okFrontPanel_HANDLE hFP = okFrontPanelDevices_Open(h, serial);
		FrontPanelPtr p;
		if (hFP)
			p.reset(new OpalKellyLegacy::okCFrontPanel(hFP));
		return p;
	}

} // namespace OpalKelly

// For compatibility with the existing user code, all classes in this namespace
// are currently made globally accessible, this will change in the future API
// versions when their counterparts in OpalKelly namespace are provided.
using namespace OpalKellyLegacy;

#endif // !defined(FRONTPANELDLL_EXPORTS)

#endif // __cplusplus

#endif // __okFrontPanelDLL_h__
