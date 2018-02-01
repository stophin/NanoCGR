// Platform.h
//
//

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "NanoType.h"

#ifdef _NANOC_WINDOWS_

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>


// TODO:  在此处引用程序需要的其他头文件
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

#define __NANOC_THREAD_MUTEX_INIT__(hMutex, obj) obj->hMutex = CreateMutex(NULL, FALSE, NULL)
#define __NANOC_THREAD_MUTEX_LOCK__(hMutex) WaitForSingleObject(hMutex, INFINITE)
#define __NANOC_THREAD_MUTEX_UNLOCK__(hMutex) ReleaseMutex(hMutex);
#else


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

#define __NANOC_THREAD_MUTEX_INIT__(hMutex, obj) pthread_mutex_init(&obj->hMutex, NULL)
#define __NANOC_THREAD_MUTEX_LOCK__(hMutex)  pthread_mutex_lock(&hMutex)
#define __NANOC_THREAD_MUTEX_UNLOCK__(hMutex) pthread_mutex_unlock(&hMutex)

//#define __NANOC_THREAD_MUTEX_INIT__(hMutex, obj) sem_init(&obj->hMutex, 0, 0);sem_post(&obj->hMutex)
//#define __NANOC_THREAD_MUTEX_LOCK__(hMutex)  sem_wait(&hMutex)
//#define __NANOC_THREAD_MUTEX_UNLOCK__(hMutex) sem_post(&hMutex)

#endif

#include "MultiLinkList.h"

#include "CharString.h"

#include "NanoC.h"

#endif