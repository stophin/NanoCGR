//NanoType.h
//
//

#pragma once

//#define _NANOC_WINDOWS_

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

typedef INT32 INT;
typedef void VOID;

#else

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include<signal.h>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>			/* socket类定义需要*/
#include <sys/epoll.h>			/* epoll头文件 */
#include <fcntl.h>	            /* nonblocking需要 */
#include <sys/resource.h>		/* 设置最大的连接数需要setrlimit */

#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>

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

typedef INT32 INT;

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define WAIT_TIMEOUT			-1

#define WSAGetLastError() errno

#endif
