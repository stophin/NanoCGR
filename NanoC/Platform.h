// Platform.h
//
//

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define _NANOC_WINDOWS_

#ifdef _NANOC_WINDOWS_

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

#define __NANOC_EXPORT__ __declspec(dllexport)
#define __NANOC_DLLOAD__(hInstance, cpszDLName) HINSTANCE hInstance = LoadLibraryA( cpszDLName);
#define __NANOC_DLLUNLOAD__(hInstance) FreeLibrary(hInstance);

#define __NANOC_THREAD_FUNC_DECLARE(hHandle, pFuncName)\
	public:\
	HANDLE hHandle;\
	static unsigned int __stdcall pFuncName(void *pv);
#define __NANOC_THREAD_FUNC_BEGIN__(pFuncName) unsigned int __stdcall pFuncName(void *pv)
#define __NANOC_THREAD_FUNC_END__(nReturn) return nReturn

#define __NANOC_THREAD_BEGIN__(hHandle, pFuncName, pParam) hHandle = (HANDLE)_beginthreadex(NULL, 0, &pFuncName, pParam, 0, 0)
#define __NANOC_THREAD_WAIT__(hHandle) WaitForSingleObject(hHandle, 1000)
#define __NANOC_THREAD_END__(hHandle) TerminateThread(hHandle, 0)

#include <conio.h>
#define getch _getch

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
#include <netinet/in.h>			/* socket�ඨ����Ҫ*/
#include <sys/epoll.h>			/* epollͷ�ļ� */
#include <fcntl.h>	            /* nonblocking��Ҫ */
#include <sys/resource.h>		/* ����������������Ҫsetrlimit */

#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>

#include <math.h>

#define __NANOC_EXPORT__ 

#define __NANOC_DLLOAD__(hInstance, cpszDLlName)\
	CHAR aszDLLName[1024]; \
	UINT32 un32NameLen = strlen(cpszDLlName); \
	printf("%s, strlen %d\n", cpszDLlName, un32NameLen); \
	if (un32NameLen <1000)\
	{\
	sprintf(aszDLLName, "lib%s.so", cpszDLlName); \
	printf("Loading %s\n", aszDLLName);\
	hInstance = dlopen(aszDLLName, RTLD_LAZY); \
	}
#define __NANOC_DLLUNLOAD__(hInstance) dlclose(hInstance);


#define __NANOC_THREAD_FUNC_DECLARE(hHandle, pFuncName)\
	private:\
	HANDLE hHandle; \
	static void * pFuncName(void *pv);
#define __NANOC_THREAD_FUNC_BEGIN__(pFuncName) void * pFuncName(void *pv)
#define __NANOC_THREAD_FUNC_END__(nReturn) pthread_exit((void *)nReturn)

#define __NANOC_THREAD_BEGIN__(hHandle, pFuncName, pParam) pthread_create(&hHandle, NULL, pFuncName, pParam)
#define __NANOC_THREAD_WAIT__(hHandle) pthread_cancel(hHandle)
#define __NANOC_THREAD_END__(hHandle) pthread_kill(hHandle, 0)

#endif

#include "NanoType.h"

#include "NanoC.h"

#endif