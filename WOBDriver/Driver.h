/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>

#include <Public.h>


#define LOG_KMSG(...) DbgPrint(__VA_ARGS__)

#define DRIVER_TAG              'DBOW'

#define OS_VERSION_KEY			L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion"
#define OS_VERSION_KEY_VALUE	L"UBR"



extern	UNICODE_STRING	ntDeviceName;
extern	UNICODE_STRING	ntDeviceSymbolicName;
extern  WDFDRIVER		wdfDriver;
extern  WDFDEVICE		wdfDevice;



typedef struct _WOB_OS_VERSION_INFO {
    DWORD   majorVersion;
    DWORD   minorVersion;
    DWORD   buildNumber;
    DWORD   updateRevision;
} WOB_OS_VERSION_INFO, * PWOB_OS_VERSION_INFO;



EXTERN_C_START

DRIVER_INITIALIZE                   DriverEntry;
EVT_WDF_DRIVER_UNLOAD               WOBDriverEvtDriverUnload;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  WOBDriverEvtIoQueueIoDeviceControl;

NTSTATUS IOCTLHandlerOpenProcess(
            _In_ WDFREQUEST Request,
            _In_ size_t OutputBufferLength,
            _In_ size_t InputBufferLength);

NTSTATUS IOCTLElevateHandle(
            _In_ WDFREQUEST Request,
            _In_ size_t OutputBufferLength,
            _In_ size_t InputBufferLength);


NTSTATUS GetSystemVersionInfor(PWOB_OS_VERSION_INFO pOsVersion);

NTSTATUS ReadRegistryKeyValue(UNICODE_STRING* key, UNICODE_STRING* valueName, PVOID outBuffer, SIZE_T szOutBuffer);

NTSTATUS FindHandleInHandleTable(HANDLE handle, PVOID handleTable, PVOID* outBuffer);

EXTERN_C_END


