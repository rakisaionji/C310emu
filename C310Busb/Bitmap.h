#pragma once

#include "ConsoleApp.h"

DWORD ConvertDataToBitmap
(
    DWORD dwBitCount,
    DWORD dwWidth, DWORD dwHeight,
    PBYTE pbInput, DWORD cbInput,
    PBYTE pbOutput, DWORD cbOutput,
    PDWORD pcbResult
);

DWORD WriteDataToBitmapFile
(
    LPCSTR lpFilePath, DWORD dwBitCount,
    DWORD dwWidth, DWORD dwHeight,
    PBYTE pbInput, DWORD cbInput
);
