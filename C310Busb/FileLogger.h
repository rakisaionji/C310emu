#pragma once

#include "ConsoleApp.h"

LPSTR BytesToString(PBYTE array, size_t size);

BOOL OpenLogFile(PHANDLE hFile, LPCSTR moduleName);

BOOL LogInfo(HANDLE hFile, LPCSTR lpszFunction, LPCSTR lpszMessage, ...);

DWORD LogError(HANDLE hFile, LPCSTR lpszFunction, DWORD errorCode, LPCSTR lpszMessage, ...);

DWORD WriteArrayToFile(LPCSTR lpOutputFilePath, LPVOID lpDataTemp, DWORD nDataSize, BOOL isAppend);

static HANDLE logFile;

#ifdef NDEBUG

#define OpenLogFileA(mod) ((int)0)
#define LogInfoA(msg, ...) ((void)0)
#define LogErrorA(msg, ...) ((void)0)
#define LogError1(func, msg, ...) ((void)0)
#define LogError2(code, msg, ...) ((void)0)
#define LogErrorEx(func, code, msg, ...) ((void)0)
#define CloseLogFile() ((void)0)

#else

#define OpenLogFileA(mod) OpenLogFile(&logFile, mod)
#define LogInfoA(msg, ...) LogInfo(logFile, __func__, msg, ##__VA_ARGS__)
#define LogErrorA(msg, ...) LogError(logFile, __func__, 0, msg, ##__VA_ARGS__)
#define LogError1(func, msg, ...) LogError(logFile, func, 0, msg, ##__VA_ARGS__)
#define LogError2(code, msg, ...) LogError(logFile, __func__, code, msg, ##__VA_ARGS__)
#define LogErrorEx(func, code, msg, ...) LogError(logFile, func, code, msg, ##__VA_ARGS__)
#define CloseLogFile() CloseHandle(logFile);

#endif
