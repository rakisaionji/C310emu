#include "ConsoleApp.h"
#include "SharpTypes.h"
#include "FileLogger.h"

static const int8 idNumber = 0x01;
static const int64 serialNo = 0x3837363534333231;

static const int8 mainFirmware[] =
{
    0x45, 0x30, 0x32, 0x33, 0x33, 0x37, 0x30, 0x30, 0x43, 0x48, 0x43, 0x2D, 0x43, 0x33, 0x31, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x38, 0x30, 0x38, 0x32, 0x31, 0x31, 0x37,
    0x35, 0x39, 0x01, 0x19, 0x10, 0xDB  // E0233700 CHC-C310 1808211759 0119 MAIN
};
static const int8 paramFirmware[] =
{
    0x44, 0x30, 0x35, 0x34, 0x35, 0x37, 0x30, 0x30, 0x43, 0x48, 0x43, 0x2D, 0x43, 0x33, 0x31, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x38, 0x30, 0x31, 0x31, 0x34, 0x31, 0x33,
    0x34, 0x35, 0x02, 0x01, 0x96, 0x4C  // D0545700 CHC-C310 1801141345 0201 PARAM
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        if (OpenLogFileA("C310BFWDLusb"))
        {
            LogInfoA("C310BFWDLusb: DLL_PROCESS_ATTACH success.");
        }
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        if (logFile != INVALID_HANDLE_VALUE)
        {
            LogInfoA("C310BFWDLusb: DLL_PROCESS_DETACH success.");
            CloseLogFile();
        }
    }
    return TRUE;
}

function fwdlusb_open(uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_open(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

procedure fwdlusb_close()
{
    LogInfoA("C310BFWDLusb: fwdlusb_close()\n");
}

function fwdlusb_listupPrinter(uint8* rIdArray)
{
    LogInfoA("C310BFWDLusb: fwdlusb_listupPrinter(%p)\n", rIdArray);
    memset(rIdArray, 0xFF, 0x80);
    rIdArray[0] = idNumber;
    return 1;
}

function fwdlusb_listupPrinterSN(uint64* rSerialArray)
{
    LogInfoA("C310BFWDLusb: fwdlusb_listupPrinterSN(%p)\n", rSerialArray);
    memset(rSerialArray, 0xFF, 0x400);
    rSerialArray[0] = serialNo;
    return 1;
}

function fwdlusb_selectPrinter(uint8 printerId, uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_selectPrinter(%d, %p)\n", printerId, rResult);
    *rResult = 0;
    return 1;
}

function fwdlusb_selectPrinterSN(uint64 printerSN, uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_selectPrinterSN(%I64d, %p)\n", printerSN, rResult);
    *rResult = 0;
    return 1;
}

function fwdlusb_getPrinterInfo(uint16 tagNumber, uint8* rBuffer, uint32* rLen)
{
    LogInfoA("C310BFWDLusb: fwdlusb_getPrinterInfo(%d, %p, %d)\n", tagNumber, rBuffer, *rLen);

    switch (tagNumber)
    {
    case 0: //getPaperInfo
        if (*rLen != 0x67) *rLen = 0x67;
        if (rBuffer) memset(rBuffer, 0, *rLen);
        return 1;
    case 3: // getFirmwareVersion
        if (*rLen != 0x99) *rLen = 0x99;
        if (rBuffer)
        {
            memset(rBuffer, 0, *rLen);
            // bootFirmware
            int i = 1;
            memcpy(rBuffer + i, mainFirmware, sizeof(mainFirmware));
            // mainFirmware
            i += 0x26;
            memcpy(rBuffer + i, mainFirmware, sizeof(mainFirmware));
            // printParameterTable
            i += 0x26;
            memcpy(rBuffer + i, paramFirmware, sizeof(paramFirmware));
        }
        return 1;
    case 5: // getPrintCountInfo
        if (!rBuffer)
        {
            *rLen = 0x28;
            return 1;
        }
        int32 bInfo[10] = { 0 };
        bInfo[0] = 22;      // printCounter0
        bInfo[1] = 23;      // printCounter1
        bInfo[2] = 33;      // feedRollerCount
        bInfo[3] = 55;      // cutterCount
        bInfo[4] = 88;      // headCount
        bInfo[5] = 999;     // ribbonRemain
        bInfo[6] = 888;     // holoHeadCount
        if (*rLen <= 0x1Cu)
        {
            memcpy(rBuffer, bInfo, *rLen);
        }
        else
        {
            bInfo[7] = 69;  // paperCount
            bInfo[8] = 21;  // printCounter2
            bInfo[9] = 20;  // holoPrintCounter
            if (*rLen > 0x28u) *rLen = 0x28;
            memcpy(rBuffer, bInfo, *rLen);
        }
        break;
    case 26: // getPrinterSerial
        if (*rLen != 8) *rLen = 8;
        if (rBuffer) memcpy(rBuffer, &serialNo, 8);
        return 1;
    default:
        LogErrorA("Unknown parameter 'tagNumber' value.");
        break;
    }
    return 1;
}

function fwdlusb_status(uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_status(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function fwdlusb_statusAll(uint8* idArray, uint16* rResultArray)
{
    LogInfoA("C310BFWDLusb: fwdlusb_statusAll(%p, %p)\n", idArray, rResultArray);

    for (int i = 0; *(uint8*)(idArray + i) != 255 && i < 128; ++i)
    {
        *(uint16*)(rResultArray + 2i64 * i) = 0;
    }

    return 1;
}

function fwdlusb_resetPrinter(uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_resetPrinter(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function fwdlusb_updateFirmware(uint8 update, LPCSTR filename, uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_updateFirmware(%d, '%s', %p)\n", update, filename, rResult);
    *rResult = 0;
    return 1;
}

function fwdlusb_getFirmwareVersion(uint8* buffer, int size)
{
    int8 a;
    uint32 b = 0;
    for (int32 i = 0; i < size; ++i)
    {
        if (*(int8*)(buffer + i) < 0x30 || *(int8*)(buffer + i) > 0x39)
        {
            if (*(int8*)(buffer + i) < 0x41 || *(int8*)(buffer + i) > 0x46)
            {
                if (*(int8*)(buffer + i) < 0x61 || *(int8*)(buffer + i) > 0x66)
                {
                    return 0;
                }
                a = *(int8*)(buffer + i) - 0x57;
            }
            else
            {
                a = *(int8*)(buffer + i) - 0x37;
            }
        }
        else
        {
            a = *(int8*)(buffer + i) - 0x30;
        }
        b = a + 0x10 * b;
    }
    return b;
}

function fwdlusb_getFirmwareInfo_main(LPCSTR filename, uint8* rBuffer, uint32* rLen, uint16* rResult)
{
    DWORD result;

    if (filename)
    {
        HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return 0;
        {
            if (rResult) *rResult = 1005;
            result = 0;
        }

        DWORD read;
        uint8 buffer[0x40];
        BOOL result = ReadFile(hFile, buffer, 0x40, &read, NULL);
        if (result && read > 0x24)
        {
            memcpy(rBuffer, buffer + 0x2, 0x8);
            memcpy(rBuffer + 0x8, buffer + 0xA, 0x10);
            memcpy(rBuffer + 0x18, buffer + 0x20, 0xA);
            *(rBuffer + 0x22) = (uint8)fwdlusb_getFirmwareVersion(buffer + 0x1A, 0x2);
            *(rBuffer + 0x23) = (uint8)fwdlusb_getFirmwareVersion(buffer + 0x1C, 0x2);
            memcpy(rBuffer + 0x24, buffer + 0x2A, 0x2);

            // memcpy(&mainFirmware, rBuffer, sizeof(mainFirmware));

            if (rResult) *rResult = 0;
            result = 1;
        }
        else
        {
            if (rResult) *rResult = 1005;
            result = 0;
        }
    }
    else
    {
        if (rResult) *rResult = 1006;
        result = 0;
    }

    return result;
}

function fwdlusb_getFirmwareInfo_param(LPCSTR filename, uint8* rBuffer, uint32* rLen, uint16* rResult)
{
    DWORD result;

    if (filename)
    {
        HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return 0;
        {
            if (rResult) *rResult = 1005;
            result = 0;
        }

        DWORD read;
        uint8 buffer[0x40];
        BOOL result = ReadFile(hFile, buffer, 0x40, &read, NULL);
        if (result && read > 0x24)
        {
            memcpy(rBuffer, buffer + 0x2, 8);
            memcpy(rBuffer + 0x8, buffer + 0xA, 0x10);
            memcpy(rBuffer + 0x18, buffer + 0x20, 0xA);
            memcpy(rBuffer + 0x22, buffer + 0x1A, 0x1);
            memcpy(rBuffer + 0x23, buffer + 0x1C, 0x1);
            memcpy(rBuffer + 0x24, buffer + 0x2A, 0x2);

            // memcpy(&paramFirmware, rBuffer, sizeof(paramFirmware));

            if (rResult) *rResult = 0;
            result = 1;
        }
        else
        {
            if (rResult) *rResult = 1005;
            result = 0;
        }
    }
    else
    {
        if (rResult) *rResult = 1006;
        result = 0;
    }

    return result;
}

function fwdlusb_getFirmwareInfo(uint8 update, LPCSTR filename, uint8* rBuffer, uint32* rLen, uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_getFirmwareInfo(%d, '%s', %p, %p, %p)\n", update, filename, rBuffer, rLen, rResult);

    if (!rBuffer)
    {
        *rLen = 38;
        return 1;
    }
    if (*rLen > 38) *rLen = 38;
    if (update == 1)
    {
        return fwdlusb_getFirmwareInfo_main(filename, rBuffer, rLen, rResult);
    }
    else if (update == 3)
    {
        return fwdlusb_getFirmwareInfo_param(filename, rBuffer, rLen, rResult);
    }
    else
    {
        if (rResult) *rResult = 1006;
        return 0;
    }
}

function fwdlusb_MakeThread(uint16 maxCount)
{
    LogInfoA("C310BFWDLusb: fwdlusb_MakeThread(%d)\n", maxCount);
    return 1;
}

function fwdlusb_ReleaseThread(uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_ReleaseThread(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function fwdlusb_AttachThreadCount(uint16* rCount, uint16* rMaxCount)
{
    LogInfoA("C310BFWDLusb: fwdlusb_AttachThreadCount(%p, %p)\n", rCount, rMaxCount);
    *rCount = 0;
    *rMaxCount = 1;
    return 1;
}

function fwdlusb_getErrorLog(uint16 index, uint8* rData, uint16* rResult)
{
    LogInfoA("C310BFWDLusb: fwdlusb_getErrorLog(%d, %p, %p)\n", index, rData, rResult);
    *rResult = 0;
    return 1;
}
