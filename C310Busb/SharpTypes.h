#pragma once

typedef signed __int8       int8;
typedef signed __int16      int16;
typedef signed __int32      int32;
typedef signed __int64      int64;

typedef unsigned __int8     uint8;
typedef unsigned __int16    uint16;
typedef unsigned __int32    uint32;
typedef unsigned __int64    uint64;

#define procedure   void __fastcall

#if defined(_M_AMD64) || defined (_WIN64)
#define function    __int64 __fastcall
#else
#define function    __int32 __fastcall
#endif
