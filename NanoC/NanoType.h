//NanoType.h
//
//

#pragma once

#ifdef _NANOC_WINDOWS_

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

typedef INT64 SIZE_INT;

#else

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed long 		INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long		 UINT64, *PUINT64;

typedef void * HINSTANCE;
typedef pthread_t HANDLE;

typedef char CHAR;
typedef void VOID;

typedef INT32 SOCKET;
typedef INT8 BOOL;

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define WAIT_TIMEOUT			-1

#define WSAGetLastError() errno

#endif
