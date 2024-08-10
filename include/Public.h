/*++
Module Name:
    public.h

Abstract:
    This module contains the common declarations shared by driver
    and user applications.

Environment:
    user and kernel
--*/


#define DRIVER_NAME     L"WOBDriver"
#define DRIVER_NAME_A   "WOBDriver"

#ifdef _DRIVER_PROJECT_
    #define		WOBDRIVER_DEVICE_NAME		    L"\\Device\\"       DRIVER_NAME
    #define		WOBDRIVER_SYMBOLIC_DEVICE_NAME  L"\\??\\"           DRIVER_NAME
#else
    #define		WOBDRIVER_DEVICE_NAME           L"\\\\.\\"          DRIVER_NAME
#endif 



#define IOCTL_OPEN_PROCESS_METHOD_BUFFERED        CTL_CODE( FILE_DEVICE_UNKNOWN , 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS  )
#define IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED      CTL_CODE( FILE_DEVICE_UNKNOWN , 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS  )


#ifndef DWORD
    typedef unsigned long   DWORD;
    typedef unsigned long*  PDWORD;
#endif

#ifndef BOOL
    typedef int             BOOL;
    typedef int*            PBOOL;
#endif

typedef struct _ELEVATE_HANDLE_PROCESS_INFO {
    DWORD   tPID;           // target process ID
    HANDLE  tHandle;        // handle owned by target process
    DWORD   newAccess;      // handle owned by target process
}ELEVATE_HANDLE_PROCESS_INFO, *PELEVATE_HANDLE_PROCESS_INFO;

