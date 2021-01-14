#include "ConsoleApp.h"
#include "SharpTypes.h"
#include "FileLogger.h"
#include "Bitmap.h"

#define IMAGE_SIZE 0x24FC00
#define HOLO_SIZE 0xC5400

static const int8 idNumber = 0x01;
static const int64 serialNo = 0x3837363534333231;

static const uint8 mainFirmware[] =
{
    0x45, 0x30, 0x32, 0x33, 0x33, 0x37, 0x30, 0x30, 0x43, 0x48, 0x43, 0x2D, 0x43, 0x33, 0x31, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x38, 0x30, 0x38, 0x32, 0x31, 0x31, 0x37,
    0x35, 0x39, 0x01, 0x19, 0x10, 0xDB  // E0233700 CHC-C310 1808211759 0119 MAIN
};
static const uint8 paramFirmware[] =
{
    0x44, 0x30, 0x35, 0x34, 0x35, 0x37, 0x30, 0x30, 0x43, 0x48, 0x43, 0x2D, 0x43, 0x33, 0x31, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x38, 0x30, 0x31, 0x31, 0x34, 0x31, 0x33,
    0x34, 0x35, 0x02, 0x01, 0x96, 0x4C  // D0545700 CHC-C310 1801141345 0201 PARAM
};

static uint8 STATUS = 0;
static uint16 WIDTH = 0;
static uint16 HEIGHT = 0;

static char PRINT_DIR[] = "print";

static int32 PAPERINFO[10];
static int32 CURVE[3][3];
static uint8 POLISH[2];
static int32 MTF[9];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        if (!CreateDirectoryA(PRINT_DIR, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
        {
            PRINT_DIR[0] = 0x2E;
            PRINT_DIR[1] = 0;
        }
        if (OpenLogFileA("C310Busb"))
        {
            LogInfoA("C310Busb: DLL_PROCESS_ATTACH success.");
        }
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        if (logFile != INVALID_HANDLE_VALUE)
        {
            LogInfoA("C310Busb: DLL_PROCESS_DETACH success.");
            CloseLogFile();
        }
    }
    return TRUE;
}

function chcusb_MakeThread(uint16 maxCount)
{
    LogInfoA("C310Busb: chcusb_MakeThread(%d)\n", maxCount);
    return 1;
}

function chcusb_open(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_open(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

procedure chcusb_close()
{
    LogInfoA("C310Busb: chcusb_close()\n");
}

function chcusb_ReleaseThread(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_ReleaseThread(%p)\n", rResult);
    return 1;
}

function chcusb_listupPrinter(uint8* rIdArray)
{
    LogInfoA("C310Busb: chcusb_listupPrinter(%p)\n", rIdArray);
    memset(rIdArray, 0xFF, 0x80);
    rIdArray[0] = idNumber;
    return 1;
}

function chcusb_listupPrinterSN(uint64* rSerialArray)
{
    LogInfoA("C310Busb: chcusb_listupPrinterSN(%p)\n", rSerialArray);
    memset(rSerialArray, 0xFF, 0x400);
    rSerialArray[0] = serialNo;
    return 1;
}

function chcusb_selectPrinter(uint8 printerId, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_selectPrinter(%d, %p)\n", printerId, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_selectPrinterSN(uint64* printerSN, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_selectPrinterSN(%p, %p)\n", printerSN, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_getPrinterInfo(uint16 tagNumber, uint8* rBuffer, uint32* rLen)
{
    LogInfoA("C310Busb: chcusb_getPrinterInfo(%d, %p, %d)\n", tagNumber, rBuffer, *rLen);

    switch (tagNumber)
    {
    case 0: //getPaperInfo
        if (*rLen != 0x67) *rLen = 0x67;
        if (rBuffer) memset(rBuffer, 0, *rLen);
        break;
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
        break;
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
    case 6:
        *rLen = 32;
        if (rBuffer)
        {
            memset(rBuffer, 0, 32);
            rBuffer[0] = 44;
        }
        break;
    case 8:
        *rLen = 1;
        if (rBuffer) memset(rBuffer, 0, 1);
        break;
    case 26: // getPrinterSerial
        if (*rLen != 8) *rLen = 8;
        if (rBuffer) memcpy(rBuffer, &serialNo, 8);
        break;
    case 40:
        *rLen = 10;
        if (rBuffer)
        {
            memset(rBuffer, 0, 10);
            rBuffer[0] = 1;
            rBuffer[1] = 2;
            rBuffer[2] = 3;
        }
        break;
    case 50:
        *rLen = 61;
        if (rBuffer) memset(rBuffer, 0, 61);
        break;
    default:
        LogErrorA("Unknown parameter 'tagNumber' value.");
        break;
    }
    return 1;
}

function chcusb_imageformat
(
    uint16 format, uint16 ncomp, uint16 depth, uint16 width, uint16 height,
    uint8* inputImage, uint16* rResult
)
{
    LogInfoA
    (
        "C310Busb: chcusb_getPrinterInfo(%d, %d, %d, %d, %d, %p, %p)\n",
        format, ncomp, depth, width, height, inputImage, rResult
    );

    WIDTH = width;
    HEIGHT = height;

    *rResult = 0;
    return 1;
}

function chcusb_setmtf(int32* mtf)
{
    LogInfoA("C310Busb: chcusb_setmtf(%p)\n", mtf);
    memcpy(MTF, mtf, sizeof(MTF));
    return 1;
}

function chcusb_makeGamma(uint16 k, uint8* intoneR, uint8* intoneG, uint8* intoneB)
{
    LogInfoA("C310Busb: chcusb_makeGamma(%d, %p, %p, %p)\n", k, intoneR, intoneG, intoneB);

    uint8 tone;
    int32 value;
    double power;

    double factor = (double)k / 100.0;

    for (int i = 0; i < 256; ++i)
    {
        power = pow((double)i, factor);
        value = (int)(power / pow(255.0, factor) * 255.0);

        if (value > 255)
            tone = 255;
        if (value >= 0)
            tone = value;
        else
            tone = 0;

        if (intoneR)
            *(uint8*)(intoneR + i) = tone;
        if (intoneG)
            *(uint8*)(intoneG + i) = tone;
        if (intoneB)
            *(uint8*)(intoneB + i) = tone;
    }

    return 1;
}

function chcusb_setIcctable
(
    LPCSTR icc1, LPCSTR icc2, uint16 intents,
    uint8* intoneR, uint8* intoneG, uint8* intoneB,
    uint8* outtoneR, uint8* outtoneG, uint8* outtoneB,
    uint16* rResult
)
{
    LogInfoA
    (
        "C310Busb: chcusb_setIcctable(%p, %p, %d, %p, %p, %p, %p, %p, %p, %p)\n",
        icc1, icc2, intents, intoneR, intoneG, intoneB,
        outtoneR, outtoneG, outtoneB, rResult
    );

    for (int i = 0; i < 256; ++i)
    {
        intoneR[i] = i;
        intoneG[i] = i;
        intoneB[i] = i;
        outtoneR[i] = i;
        outtoneG[i] = i;
        outtoneB[i] = i;
    }

    *rResult = 0;
    return 1;
}

function chcusb_copies(uint16 copies, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_copies(%d, %p)\n", copies, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_status(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_status(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function chcusb_statusAll(uint8* idArray, uint16* rResultArray)
{
    LogInfoA("C310Busb: chcusb_statusAll(%p, %p)\n", idArray, rResultArray);

    for (int i = 0; *(uint8*)(idArray + i) != 255 && i < 128; ++i)
    {
        *(uint16*)(rResultArray + 2i64 * i) = 0;
    }

    return 1;
}

function chcusb_startpage(uint16 postCardState, uint16* pageId, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_startpage(%d, %p, %p)\n", postCardState, pageId, rResult);

    STATUS = 2;

    *pageId = 1;

    *rResult = 0;
    return 1;
}

function chcusb_endpage(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_endpage(%p)\n", rResult);

    *rResult = 0;
    return 1;
}

function chcusb_write(uint8* data, uint32* writeSize, uint16* rResult)
{
    SYSTEMTIME t; GetLocalTime(&t);

#ifdef NDEBUG

    char dumpPath[0x80];
    sprintf_s
    (
        dumpPath, 0x80,
        "%s\\C310Busb_%04d%02d%02d_%02d%02d%02d_write.bmp",
        PRINT_DIR, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    WriteDataToBitmapFile(dumpPath, 24, WIDTH, HEIGHT, data, IMAGE_SIZE);

#else

    char dumpPath[0x80];
    sprintf_s
    (
        dumpPath, 0x80,
        "C310Busb_%04d%02d%02d_%02d%02d%02d_write.bin",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    WriteArrayToFile(dumpPath, data, IMAGE_SIZE, FALSE);
    LogInfoA("C310Busb: chcusb_write(%p, %p, %p) [%s]\n", data, writeSize, rResult, dumpPath);

#endif;

    *writeSize = IMAGE_SIZE;
    *rResult = 0;

    return 1;
}

function chcusb_writeLaminate(uint8* data, uint32* writeSize, uint16* rResult)
{
    SYSTEMTIME t; GetLocalTime(&t);

    char dumpPath[0x80];
    sprintf_s
    (
        dumpPath, 0x80,
        "C310Busb_%04d%02d%02d_%02d%02d%02d_writeLaminate.bin",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    WriteArrayToFile(dumpPath, data, *writeSize, FALSE);
    LogInfoA("C310Busb: chcusb_writeLaminate(%p, %p, %p) [%s]\n", data, writeSize, rResult, dumpPath);

    // *writeSize = written;
    *rResult = 0;

    return 1;
}

function chcusb_writeHolo(uint8* data, uint32* writeSize, uint16* rResult)
{
    SYSTEMTIME t; GetLocalTime(&t);

#ifdef NDEBUG

    char dumpPath[0x80];
    sprintf_s
    (
        dumpPath, 0x80,
        "%s\\C310Busb_%04d%02d%02d_%02d%02d%02d_writeHolo.bmp",
        PRINT_DIR, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    WriteDataToBitmapFile(dumpPath, 8, WIDTH, HEIGHT, data, HOLO_SIZE);

#else

    char dumpPath[0x80];
    sprintf_s
    (
        dumpPath, 0x80,
        "C310Busb_%04d%02d%02d_%02d%02d%02d_writeHolo.bin",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    WriteArrayToFile(dumpPath, data, HOLO_SIZE, FALSE);
    LogInfoA("C310Busb: chcusb_writeHolo(%p, %p, %p) [%s]\n", data, writeSize, rResult, dumpPath);

#endif;

    *writeSize = HOLO_SIZE;
    *rResult = 0;

    return 1;
}

function chcusb_setPrinterInfo(uint16 tagNumber, uint8* rBuffer, uint32* rLen, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_setPrinterInfo(%d, %p, %d, %p)\n", tagNumber, rBuffer, *rLen, rResult);

    switch (tagNumber)
    {
    case 0: //setPaperInfo
        memcpy(PAPERINFO, rBuffer, sizeof(PAPERINFO));
        break;
    case 20: // setPolishInfo
        memcpy(POLISH, rBuffer, sizeof(POLISH));
        break;
    default:
        break;
    }

    *rResult = 0;
    return 1;
}

function chcusb_getGamma(LPCSTR filename, uint8* r, uint8* g, uint8* b, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getGamma('%s', %p, %p, %p, %p)\n", filename, r, g, b, rResult);

    for (int i = 0; i < 256; ++i)
    {
        r[i] = i;
        g[i] = i;
        b[i] = i;
    }

    *rResult = 0;
    return 1;
}

function chcusb_getMtf(LPCSTR filename, int32* mtf, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getMtf('%s', %p, %p)\n", filename, mtf, rResult);

    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    DWORD read;
    uint8 buffer[0x40];
    BOOL result = ReadFile(hFile, buffer, 0x40, &read, NULL);
    if (!result) return 0;

    int a, c;
    int b = 1;
    int d = c = 0;

    memset(mtf, 0, sizeof(MTF));

    for (DWORD i = 0; i < read; i++)
    {
        a = buffer[i] - 0x30;
        if (a == -3 && c == 0)
        {
            b = -1;
        }
        else if (a >= 0 && a <= 9)
        {
            mtf[d] = mtf[d] * 10 + a;
            c++;
        }
        else if (c > 0)
        {
            mtf[d] *= b;
            b = 1;
            c = 0;
            d++;
        }
        if (d > 9) break;
    }

    *rResult = 0;
    return 1;
}

function chcusb_cancelCopies(uint16 pageId, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_cancelCopies(%d, %p)\n", pageId, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_setPrinterToneCurve(uint16 type, uint16 number, uint16* data, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_setPrinterToneCurve(%d, %d, %p, %p)\n", type, number, data, rResult);
    if (0 < type < 3 && 0 < number < 3) CURVE[type][number] = *data;
    *rResult = 0;
    return 1;
}

function chcusb_getPrinterToneCurve(uint16 type, uint16 number, uint16* data, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getPrinterToneCurve(%d, %d, %p, %p)\n", type, number, data, rResult);
    if (0 < type < 3 && 0 < number < 3) *data = CURVE[type][number];
    *rResult = 0;
    return 1;
}

function chcusb_blinkLED(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_blinkLED(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function chcusb_resetPrinter(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_resetPrinter(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function chcusb_AttachThreadCount(uint16* rCount, uint16* rMaxCount)
{
    LogInfoA("C310Busb: chcusb_AttachThreadCount(%p, %p)\n", rCount, rMaxCount);
    *rCount = 0;
    *rMaxCount = 1;
    return 1;
}

function chcusb_getPrintIDStatus(uint16 pageId, uint8* rBuffer, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getPrintIDStatus(%d, %p, %p)\n", pageId, rBuffer, rResult);

    memset(rBuffer, 0, 8);

    if (STATUS > 1)
    {
        STATUS = 0;
        *((uint16*)(rBuffer + 6)) = 2212; // Printing Complete
    }
    else
    {
        *((uint16*)(rBuffer + 6)) = 2300; // No Printting
    }

    *rResult = 0;
    return 1;
}

function chcusb_setPrintStandby(uint16 position, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_setPrintStandby(%d, %p)\n", position, rResult);

    if (STATUS == 0)
    {
        STATUS = 1;
        *rResult = 2100;
    }
    else
    {
        *rResult = 0;
    }

    return 1;
}

function chcusb_testCardFeed(uint16 mode, uint16 times, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_testCardFeed(%d, %d, %p)\n", mode, times, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_exitCard(uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_exitCard(%p)\n", rResult);
    *rResult = 0;
    return 1;
}

function chcusb_getCardRfidTID(uint8* rCardTID, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getCardRfidTID(%p, %p)\n", rCardTID, rResult);

    SYSTEMTIME t; GetLocalTime(&t);
    srand((UINT)(t.wMilliseconds + t.wSecond));
    PUINT16 pCardTID = (PUINT16)rCardTID;

    for (BYTE i = 0; i < 6; i++)
    {
        pCardTID[i] = (UINT16)(rand() + 1);
    }

    *rResult = 2405;
    return 1;
}

function chcusb_commCardRfidReader(uint8* sendData, uint8* rRecvData, uint32 sendSize, uint32* rRecvSize, uint16* rResult)
{
    DWORD dwRecvSize = 0;
    PUINT16 pbSendData = (PUINT16)sendData;
    SYSTEMTIME t; GetLocalTime(&t);

#ifdef NDEBUG

    LogInfoA
    (
        "C310Busb: chcusb_commCardRfidReader(%p { %d }, %p, %d, %p, %p)\n",
        sendData, pbSendData[0], rRecvData, sendSize, rRecvSize, rResult
    );

#else

    char dumpPath[0x80];
    sprintf_s
    (
        dumpPath, 0x80,
        "C310Busb_%04d%02d%02d_%02d%02d%02d_commCardRfidReader.bin",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond
    );

    WriteArrayToFile(dumpPath, sendData, sendSize, FALSE);
    LogInfoA
    (
        "C310Busb: chcusb_commCardRfidReader(%p { %d }, %p, %d, %p, %p) [%s]\n",
        sendData, pbSendData[0], rRecvData, sendSize, rRecvSize, rResult, dumpPath
    );

#endif

    switch (pbSendData[0])
    {
    case 65:
        dwRecvSize = 0x8;
        memset(rRecvData, 0, dwRecvSize);
        break;
    case 66:    // getRFIDFirmwareVersion
        dwRecvSize = 0x20;
        memset(rRecvData, 0, dwRecvSize);
        rRecvData[3] = 0x91;    // rfidAppFirmwareVer
        break;
    case 129:   // checkRFIDReaderStatus
        dwRecvSize = 0x8;
        memset(rRecvData, 0, dwRecvSize);
        break;
    case 132:
        dwRecvSize = 0x20;
        memset(rRecvData, 0, dwRecvSize);
        rRecvData[3] = 0x91;
        break;
    case 133:
        dwRecvSize = 0x3;
        memset(rRecvData, 0, dwRecvSize);
        rRecvData[2] = 0x91;
        break;
    default:
        break;
    }

    *rRecvSize = dwRecvSize;
    *rResult = 0;
    return 1;
}

function chcusb_updateCardRfidReader(uint8* data, uint32 size, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_updateCardRfidReader(%p, %d, %p)\n", data, size, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_getErrorLog(uint16 index, uint8* rData, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getErrorLog(%d, %p, %p)\n", index, rData, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_getErrorStatus(uint16* rBuffer)
{
    LogInfoA("C310Busb: chcusb_getErrorStatus(%p)\n", rBuffer);
    memset(rBuffer, 0, 0x80);
    return 1;
}

function chcusb_setCutList(uint8* rData, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_setCutList(%p, %p)\n", rData, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_setLaminatePattern(uint16 index, uint8* rData, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_setLaminatePattern(%d, %p, %p)\n", index, rData, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_color_adjustment(LPCSTR filename, int32 a2, int32 a3, int16 a4, int16 a5, int64 a6, int64 a7, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_color_adjustment('%s', %d, %d, %d, %d, %I64d, %I64d, %p)\n", filename, a2, a3, a4, a5, a6, a7, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_color_adjustmentEx(int32 a1, int32 a2, int32 a3, int16 a4, int16 a5, int64 a6, int64 a7, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_color_adjustmentEx(%d, %d, %d, %d, %d, %I64d, %I64d, %p)\n", a1, a2, a3, a4, a5, a6, a7, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_writeIred(uint8* a1, uint8* a2, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_writeIred(%p, %p, %p)\n", a1, a2, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_getEEPROM(uint8 index, uint8* rData, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getEEPROM(%d, %p, %p)\n", index, rData, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_setParameter(uint8 a1, uint32 a2, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_setParameter(%d, %d, %p)\n", a1, a2, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_getParameter(uint8 a1, uint8* a2, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_getParameter(%d, %p, %p)\n", a1, a2, rResult);
    *rResult = 0;
    return 1;
}

function chcusb_universal_command(int32 a1, uint8 a2, int32 a3, uint8* a4, uint16* rResult)
{
    LogInfoA("C310Busb: chcusb_universal_command(%d, %d, %d, %p, %p)\n", a1, a2, a3, a4, rResult);
    *rResult = 0;
    return 1;
}
