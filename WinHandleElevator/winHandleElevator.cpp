
#define NOMINMAX

#include <Windows.h>
#include <cstdarg>

#include <iostream>
#include <ios>
#include <limits>

#include <Public.h>

#include "WinHandleEvelevator.h"


void ClearSTDIN() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


int main()
{
    DWORD   choice = 0;
    DWORD   procId = NULL;
    DWORD   desiredAccess = NULL;
    HANDLE  hProc = NULL;

    DWORD   error = NULL;
    HANDLE  hDevice = INVALID_HANDLE_VALUE;


    BOOL                        devCtrlStatus = JOB_STATUS_ERROR;
    NTSTATUS                    ioctlStatus = JOB_STATUS_ERROR;
    ULONG                       bytesReturned = NULL;
    ELEVATE_HANDLE_PROCESS_INFO eProcInfo = { 0 };



    error = getDriverHandle(&hDevice);
    if (hDevice == INVALID_HANDLE_VALUE) {
        LogError("Failed to obtain driver handle. ErrorCode [0x%x]\n", error);

        std::cout << "Press [enter] to continue > ";
        std::cin.putback('\n');
        ClearSTDIN();
        ClearSTDIN();
        return -1;
    }


    while (true) {

        procId = NULL;

        system("cls");

        std::cout << "================= WinHandleElevator ================" << std::endl;
        std::cout << "Find process by:" << std::endl;
        std::cout << "    1. name" << std::endl;
        std::cout << "    2. pid" << std::endl;
        std::cout << "    3. quit" << std::endl;
        std::cout << "[1] > ";

        std::cin >> choice;
        ClearSTDIN();

        if (choice < 1 || choice > 3) {
            LogError("Invalid choice %d.\n", choice);
            goto _RUN_END;
        }

        if (choice == 3) {
            LogInfo("Good Bye.\n");
            break;
        }


        if (choice == 1) {
            std::string procName;
            std::cout << "Enter process name > ";
            std::cin >> procName;
            ClearSTDIN();

            procId = FindProcessByName(procName.c_str());
        }

        if (choice == 2) {
            std::cout << "Enter process id > ";
            std::cin >> procId;
            ClearSTDIN();
        }


        LogInfo("Current Process set to: %d \n", procId);

        hProc = NULL;

        std::cout << "Enter handle hex value [C0]> ";
        std::cin >> std::hex >> hProc;
        ClearSTDIN();


        LogInfo("Target handle set to: 0x%x \n", hProc);


        desiredAccess = NULL;

        std::cout << "Enter desiredAccess [" << std::hex << (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL) << std::dec << "]>" ;
        std::cin >> std::hex >> desiredAccess;
        std::cin >> std::dec;
        ClearSTDIN();

        LogInfo("Desired access for handle: 0x%x set to 0x%x \n", hProc, desiredAccess);


        devCtrlStatus   = FALSE;
        ioctlStatus = JOB_STATUS_ERROR;
        bytesReturned   = NULL;
        eProcInfo       = { procId, hProc, desiredAccess };

        
        devCtrlStatus = DeviceIoControl(hDevice, IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED,
                                &eProcInfo, sizeof(eProcInfo), &ioctlStatus, sizeof(ioctlStatus), &bytesReturned, NULL);

        if (!devCtrlStatus) {
            error = GetLastError();
            LogError("Error handling IOCTL, 0x%x, ErrorCode [0x%x]\n", IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED, error);
            goto _RUN_END;
        }

        if (ioctlStatus != 0 ) {
            LogInfo("Request failed with status: [0x%x]\n", ioctlStatus);
            goto _RUN_END;
        }

        LogInfo("Request success\n\n");

_RUN_END:
        std::cout << "Press [enter] to continue > ";
        std::cin.putback('\n');
        ClearSTDIN();
        ClearSTDIN();
    }

    closeDriverHandle(hDevice);
    exit (0);
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


