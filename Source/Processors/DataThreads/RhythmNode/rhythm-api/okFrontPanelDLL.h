///------------------------------------------------------------------------
// okFrontPanelDLL.h
//
// This is the header file for C/C++ compilation of the FrontPanel DLL
// import library.  This file must be included within any C/C++ source
// which references the FrontPanel DLL methods.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev: 1911 $ $Date: 2013-12-29 14:03:17 -0800 (Sun, 29 Dec 2013) $
//------------------------------------------------------------------------

#ifndef __okFrontPanelDLL_h__
#define __okFrontPanelDLL_h__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FRONTPANELDLL_EXPORTS
// symbol defined on the command line.  This symbol should not be defined on any project
// that uses this DLL.
#if defined(_WIN32)
	#define okDLLEXPORT
	#define DLL_ENTRY   __stdcall
	typedef unsigned int  UINT32;
	typedef unsigned char UINT8;
#elif defined(__linux__) || defined(__APPLE__) || defined(__QNX__)
	#define okDLLEXPORT
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

typedef void (* DLL_EP)(void);

#ifdef __cplusplus
#include <string>
#include <vector>

extern "C" {
#endif // __cplusplus

typedef void* okPLL22150_HANDLE;
typedef void* okPLL22393_HANDLE;
typedef struct okDeviceSettingsHandle* okDeviceSettings_HANDLE;
typedef void* okFrontPanel_HANDLE;
typedef struct okFrontPanelManagerHandle* okFrontPanelManager_HANDLE;
typedef struct okCFrontPanelManagerHandle* okCFrontPanelManager_HANDLE;
typedef int Bool;

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
	ok_brdXEM7350K410T=30
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
#define OK_PRODUCT_XEM6310MTLX45            (26)
#define OK_PRODUCT_XEM6320LX130T            (27)
#define OK_PRODUCT_XEM7350K70T              (28)
#define OK_PRODUCT_XEM7350K160T             (29)
#define OK_PRODUCT_XEM7350K410T             (30)

// ok_FPGAConfigurationMethod types
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
#ifdef __cplusplus
	bool            isPLL22150Supported;
	bool            isPLL22393Supported;
	bool            isFrontPanelEnabled;
#else // __cplusplus
	char            isPLL22150Supported;
	char            isPLL22393Supported;
	char            isFrontPanelEnabled;
#endif // __cplusplus
	int             wireWidth;
	int             triggerWidth;
	int             pipeWidth;
	int             registerAddressWidth;
	int             registerDataWidth;

	okTFlashLayout  flashSystem;
	okTFlashLayout  flashFPGA;
} okTDeviceInfo;

#endif // ! FRONTPANELDLL_EXPORTS

//
// Define the LoadLib and FreeLib methods for the IMPORT side.
//
#ifndef FRONTPANELDLL_EXPORTS
	Bool okFrontPanelDLL_LoadLib(okFP_dll_pchar libname);
	void okFrontPanelDLL_FreeLib(void);
#endif // ! FRONTPANELDLL_EXPORTS

//
// General
//
okDLLEXPORT void DLL_ENTRY okFrontPanelDLL_GetVersion(char *date, char *time);

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
// okFrontPanel
//
okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY okFrontPanel_Construct();
okDLLEXPORT void DLL_ENTRY okFrontPanel_Destruct(okFrontPanel_HANDLE hnd);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_WriteI2C(okFrontPanel_HANDLE hnd, const int addr, int length, unsigned char *data);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ReadI2C(okFrontPanel_HANDLE hnd, const int addr, int length, unsigned char *data);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_FlashEraseSector(okFrontPanel_HANDLE hnd, UINT32 address);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_FlashWrite(okFrontPanel_HANDLE hnd, UINT32 address, UINT32 length, const UINT8 *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_FlashRead(okFrontPanel_HANDLE hnd, UINT32 address, UINT32 length, UINT8 *buf);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetFPGAResetProfile(okFrontPanel_HANDLE hnd, okEFPGAConfigurationMethod method, okTFPGAResetProfile *profile);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_SetFPGAResetProfile(okFrontPanel_HANDLE hnd, okEFPGAConfigurationMethod method, const okTFPGAResetProfile *profile);
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
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetDeviceSettings(okFrontPanel_HANDLE hnd, okDeviceSettings_HANDLE settings);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_GetDeviceInfo(okFrontPanel_HANDLE hnd, okTDeviceInfo *info);
okDLLEXPORT void DLL_ENTRY okFrontPanel_GetDeviceID(okFrontPanel_HANDLE hnd, char *buf);
okDLLEXPORT void DLL_ENTRY okFrontPanel_SetDeviceID(okFrontPanel_HANDLE hnd, const char *strID);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGA(okFrontPanel_HANDLE hnd, const char *strFilename);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanel_ConfigureFPGAFromMemory(okFrontPanel_HANDLE hnd, unsigned char *data, unsigned long length);
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
okDLLEXPORT okCFrontPanelManager_HANDLE DLL_ENTRY okFrontPanelManager_Construct(okFrontPanelManager_HANDLE self);
okDLLEXPORT void DLL_ENTRY okFrontPanelManager_Destruct(okCFrontPanelManager_HANDLE hnd);
okDLLEXPORT ok_ErrorCode DLL_ENTRY okFrontPanelManager_StartMonitoring(okCFrontPanelManager_HANDLE hnd);
okDLLEXPORT okFrontPanel_HANDLE DLL_ENTRY okFrontPanelManager_Open(okCFrontPanelManager_HANDLE hnd, const char *serial);

#ifdef __cplusplus
#if !defined(FRONTPANELDLL_EXPORTS)

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
private:
	bool to_bool(Bool x);
	Bool from_bool(bool x);
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
		brdXEM6310MTLX45=26,
		brdXEM6320LX130T=27,
		brdXEM7350K70T=28,
		brdXEM7350K160T=29,
		brdXEM7350K410T=30
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
	friend class okCFrontPanelManager;

public:
	okCFrontPanel();
	~okCFrontPanel();
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
	std::string GetDeviceID();
	void SetDeviceID(const std::string str);
	ErrorCode SetBTPipePollingInterval(int interval);
	void SetTimeout(int timeout);
	ErrorCode ResetFPGA();
	void Close();
	ErrorCode ConfigureFPGAFromMemory(unsigned char *data, const unsigned long length,
				void(*callback)(int, int, void *) = NULL, void *arg = NULL);
	ErrorCode ConfigureFPGA(const std::string strFilename,
				void (*callback)(int, int, void *) = NULL, void *arg = NULL);
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


/// \brief Class for managing device connections and disconnections.
///
/// This class allows to be notified about devices connections and
/// disconnections.
///
/// To receive these notifications, you need to derive your own manager
/// subclass from this class and override its virtual OnDeviceAdded() and
/// OnDeviceRemoved() methods.
///
/// See Open() method documentation for a simple example of doing this.
class okCFrontPanelManager
{
public:
	/// \brief Default constructor.
	///
	/// This constructor doesn't do anything special, call StartMonitoring()
	/// to enable calls to OnDeviceAdded() and OnDeviceRemoved() methods.
	okCFrontPanelManager();

	/// Destructor stops monitoring the devices.
	virtual ~okCFrontPanelManager();

	/// \brief Start monitoring for Opal Kelly devices connections and
	/// disconnections.
	///
	/// Call this method to subscribe to device connection and disconnection
	/// messages. If subscription fails, an \c std::runtime_error exception is
	/// thrown.
	///
	/// Otherwise, OnDeviceAdded() will be called for each device connected at
	/// the moment of this call, allowing the application to build a full list
	/// of them.
	///
	/// Notice that this method relies on having a working event loop in the
	/// calling application which should dispatch OS-level notifications that
	/// are generated when the devices are added to or removed from the system,
	/// the notifications won't be generated without a running event loop.
	void StartMonitoring();

	/// \brief Called when a new device is connected to the system.
	///
	/// The application will typically call okCFrontPanelManager::Open() to open
	/// the newly connected device.
	///
	/// \param[in] serial Unique identifier of the new device.
	virtual void OnDeviceAdded(const char *serial) = 0;

	/// \brief Called when a device is disconnected from the system.
	///
	/// This callback can be used to remove the currently shown device from the
	/// application UI, for example.
	///
	/// Notice that \a serial will normally correspond to the same parameter of
	/// a previous call to OnDeviceAdded().
	///
	/// \param[in] serial Unique identifier of the disconnected device.
	virtual void OnDeviceRemoved(const char *serial) = 0;

	/// \brief Create a new device object from the serial number.
	///
	/// Notice that the caller must delete the pointer returned by this method.
	/// The best way to do it is to assign its result to \c std::shared_ptr<>,
	/// \c std::unique_ptr<> or, if your C++ compiler doesn't support these
	/// classes yet, to \c std::auto_ptr<>. Otherwise you need to do it
	/// manually, e.g:
	///
	/// \code
	/// // Simple manager class always storing the last connected device
	/// // (works for single device only, use a std::map of strings to
	/// // pointers to have more than one).
	/// class MySimpleManager : public okCFrontPanelManager {
	/// public:
	///     MySimpleManager() {
	///         m_device = NULL;
	///         StartMonitoring();
	///     }
	///
	///     virtual void OnDeviceAdded(const char *serial) {
	///         delete m_device;
	///         m_device = Open(serial);
	///     }
	///
	///     virtual void OnDeviceRemoved(const char *serial) {
	///         delete m_device;
	///         m_device = NULL;
	///     }
	///
	/// private:
	///     okCFrontPanel *m_device;
	/// };
	/// \endcode
	///
	/// \param[in] serial The serial number of the device to open, typically
	///   obtained from a call to OnDeviceAdded().
	/// \return A new device object to be deleted by the caller or \c NULL if
	///   opening the device failed.
	okCFrontPanel* Open(const char *serial);

	okCFrontPanelManager_HANDLE h;
};

#endif // !defined(FRONTPANELDLL_EXPORTS)

}
#endif // __cplusplus

#endif // __okFrontPanelDLL_h__
