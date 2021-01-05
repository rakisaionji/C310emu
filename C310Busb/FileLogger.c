#include "ConsoleApp.h"

LPSTR BytesToString(PBYTE array, size_t size)
{
    size_t count = size * 3;
    LPSTR buffer = malloc(count);
    if (!buffer) return NULL;

    for (size_t i = 0; i < size; i++)
    {
        sprintf_s(buffer + (i * 3), size - (i * 3), "%02x ", array[i]);
    }

    return buffer;
}

BOOL OpenLogFile(PHANDLE hFile, LPCSTR moduleName)
{
#ifdef NDEBUG

    return FALSE;

#else

    SYSTEMTIME t; GetLocalTime(&t);

    char logFilePath[0x80];

    sprintf_s
    (
        logFilePath, 0x80, "%s_%04d%02d%02d_%02d%02d%02d.log", moduleName,
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    *hFile = CreateFileA
    (
        logFilePath,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        NULL
    );

    return (hFile != INVALID_HANDLE_VALUE);

#endif
}

BOOL LogInfo(HANDLE hFile, LPCSTR lpszFunction, LPCSTR lpszMessage, ...)
{
#ifdef NDEBUG

    return FALSE;

#else

    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    SYSTEMTIME t; GetLocalTime(&t);

    DWORD bytesWritten;
    PSTR buffer1 = calloc(0x1000, 1);
    PSTR buffer2 = calloc(0x100, 1);
    if (!buffer1 || !buffer2) return FALSE;

    va_list args;
    va_start(args, lpszMessage);
    vsprintf_s(buffer2, 0x100, lpszMessage, args);
    va_end(args);

    sprintf_s(buffer1, 0x1000,
        "<< %04d/%02d/%02d %02d:%02d:%02d >>\nFunction: %s\nMessage: %s\n",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
        lpszFunction, buffer2);

    WriteFile(hFile, buffer1, (DWORD)strlen(buffer1), &bytesWritten, NULL);

    free(buffer2);
    free(buffer1);

    return TRUE;

#endif
}

DWORD LogError(HANDLE hFile, LPCSTR lpszFunction, DWORD errorCode, LPCSTR lpszMessage, ...)
{
#ifdef NDEBUG

    return 0;

#else

    errorCode = (errorCode == 0) ? GetLastError() : errorCode;
    if (hFile == INVALID_HANDLE_VALUE) return errorCode;

    SYSTEMTIME t; GetLocalTime(&t);

    DWORD bytesWritten;
    PSTR buffer1 = calloc(0x1000, 1);
    PSTR buffer2 = calloc(0x100, 1);
    if (!buffer1 || !buffer2) return errorCode;

    va_list args;
    va_start(args, lpszMessage);
    vsprintf_s(buffer2, 0x100, lpszMessage, args);
    va_end(args);

    sprintf_s(buffer1, 0x1000,
        "<< %04d/%02d/%02d %02d:%02d:%02d >>\nFunction: %s\nMessage: %s\n",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
        lpszFunction, buffer2);

    if (errorCode)
    {
        size_t lenBuffer = strlen(buffer1);
        sprintf_s(buffer1 + lenBuffer, 0x1000 - lenBuffer, "ErrCode: %ld (0x%X)\n\0", errorCode, errorCode);
    }

    WriteFile(hFile, buffer1, (DWORD)strlen(buffer1), &bytesWritten, NULL);

    free(buffer2);
    free(buffer1);

    return errorCode;

#endif
}

DWORD WriteArrayToFile(LPCSTR lpOutputFilePath, LPVOID lpDataTemp, DWORD nDataSize, BOOL isAppend)
{
#ifdef NDEBUG

    return nDataSize;

#else

    HANDLE hFile;
    DWORD dwBytesWritten;
    DWORD dwDesiredAccess;
    DWORD dwCreationDisposition;

    if (isAppend)
    {
        dwDesiredAccess = FILE_APPEND_DATA;
        dwCreationDisposition = OPEN_ALWAYS;
    }
    else
    {
        dwDesiredAccess = GENERIC_WRITE;
        dwCreationDisposition = CREATE_ALWAYS;
    }

    hFile = CreateFileA
    (
        lpOutputFilePath,
        dwDesiredAccess,
        FILE_SHARE_READ,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    WriteFile(hFile, lpDataTemp, nDataSize, &dwBytesWritten, NULL);
    CloseHandle(hFile);

    return dwBytesWritten;

#endif
}
