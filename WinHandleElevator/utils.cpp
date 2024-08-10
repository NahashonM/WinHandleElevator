

#include <Windows.h>
#include <iostream>
#include <cstdarg>

#include <Public.h>

#include "WinHandleEvelevator.h"



DWORD strToWstr(wchar_t* dst, size_t szDest, const char* src, size_t szSrc) {
    size_t  szConverted = 0;
    return mbstowcs_s(&szConverted, dst, szDest, src, szSrc);
}





DWORD FindProcessByName(const char* procName) {

    BOOL            moreEntries = false;
    DWORD           procPID = NULL;
    PROCESSENTRY32W entryW;
    HANDLE          hSnap = INVALID_HANDLE_VALUE;

    const size_t    szProcName = strlen(procName);
    wchar_t* wProcName = new wchar_t[szProcName + 1];

    errno_t error = strToWstr(wProcName, szProcName + 1, procName, szProcName);
    if (error) {
        delete[] wProcName;
        LogError("Failed to convert [%s] to wide character string. Error: 0x%x\n", procName, error);
        return procPID;
    }

    Log("Looking for: %s\n", procName);

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        LogError("Failed to get process snapshot. Error: 0x%x\n", GetLastError());
        return procPID;
    }

    entryW.dwSize = sizeof(PROCESSENTRY32W);

    while (Process32NextW(hSnap, &entryW)) {
        if (_wcsicmp(entryW.szExeFile, wProcName) != 0)
            continue;

        procPID = entryW.th32ProcessID;
        break;
    }

    delete[] wProcName;

    return procPID;
}