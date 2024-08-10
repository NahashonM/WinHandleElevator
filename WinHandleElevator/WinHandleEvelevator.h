#pragma once

#include <Windows.h>
#include <TlHelp32.h>


typedef enum {
	WHE_LOG_DEFAULT = 0,
	WHE_LOG_INFO = 0,
	WHE_LOG_WARNING,
	WHE_LOG_ERROR,
} WHE_LOG_LEVEL, * PWHE_LOG_LEVEL;

template <typename T>
extern void LogMessageRaw(WHE_LOG_LEVEL loglevel, T format, ...);

#define Log(__msg__,...)			LogMessageRaw(WHE_LOG_DEFAULT, __msg__, __VA_ARGS__);
#define LogInfo(__msg__,...)		LogMessageRaw(WHE_LOG_INFO, __msg__, __VA_ARGS__);
#define LogError(__msg__,...)		LogMessageRaw(WHE_LOG_ERROR, __msg__, __VA_ARGS__);
#define LogWarning(__msg__,...)		LogMessageRaw(WHE_LOG_WARNING, __msg__, __VA_ARGS__);


DWORD getDriverHandle(PHANDLE handle);
DWORD closeDriverHandle(HANDLE hDriver);

extern DWORD FindProcessByName(const char* procName);
extern DWORD strToWstr(wchar_t* dst, size_t szDest, const char* src, size_t szSrc);


