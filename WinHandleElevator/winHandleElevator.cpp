

#include <Windows.h>
#include <iostream>
#include <cstdarg>

#include <Public.h>

#include "WinHandleEvelevator.h"



int main()
{
    DWORD   error   = NULL;
    HANDLE  hDevice = INVALID_HANDLE_VALUE;

   error = getDriverHandle(&hDevice);
    if (hDevice == INVALID_HANDLE_VALUE) {
        LogError("Failed to obtain driver handle. ErrorCode [0x%x]\n", error);
        return -1;
    }

    LogInfo("Obtained driver handle. HValue [0x%x]\n", hDevice);

    BOOL    bRc;
    ULONG   bytesReturned;
    DWORD   procPID = FindProcessByName("explorer.exe");
    HANDLE  hProc   = INVALID_HANDLE_VALUE;

    if (!procPID) {
        LogError("Failed to find process \n");
        return -1;
    }

    LogInfo("found process: %d \n", procPID);


    // with control code
   LogInfo("Sending IOCTL:0x%x\n", IOCTL_OPEN_PROCESS_METHOD_BUFFERED);

   bRc = DeviceIoControl( hDevice, IOCTL_OPEN_PROCESS_METHOD_BUFFERED, 
                            &procPID, sizeof(procPID),
                            &hProc, sizeof(hProc), 
                            &bytesReturned, NULL );

   if (!bRc)
   {
       error = GetLastError();
       LogError("Error handling IOCTL, 0x%x, ErrorCode [0x%x]\n", IOCTL_OPEN_PROCESS_METHOD_BUFFERED,  error);
       return -1;
   }


   LogInfo("Got Handle: [0x%x]\n", hProc);


   // with control code
       // without control code

    DWORD       myPID   = FindProcessByName("WinHandleElevator.exe");
    NTSTATUS    IOCTLStatus;
    HANDLE      hManual = OpenProcess(GENERIC_READ, false, myPID);
    ELEVATE_HANDLE_PROCESS_INFO eProcInfo = { myPID, hManual, STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL  };

   LogInfo("Manual Handle: [0x%x]\n", hManual);
   LogInfo("Sending IOCTL:0x%x\n", IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED);

   bRc = DeviceIoControl(hDevice, IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED,
       &eProcInfo, sizeof(eProcInfo),
       &IOCTLStatus, sizeof(IOCTLStatus),
       &bytesReturned, NULL);

   if (!bRc) {
       error = GetLastError();
       LogError("Error handling IOCTL, 0x%x, ErrorCode [0x%x]\n", IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED, error);
       return -1;
   }

   LogInfo("IOCTL Status: [0x%x]", IOCTLStatus);


   char a[100];
   std::cout << "> ";
   std::cin >> a;


    closeDriverHandle(hDevice);

}



DWORD getDriverHandle(PHANDLE handle) {

    HANDLE hDevice = CreateFile( WOBDRIVER_DEVICE_NAME,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
    *handle = hDevice;

    if (hDevice == INVALID_HANDLE_VALUE) 
        return GetLastError();
    
    return NULL;
}


DWORD closeDriverHandle(HANDLE hDriver) {

    if (CloseHandle(hDriver) == 0) 
        return GetLastError();

    return NULL;
}


