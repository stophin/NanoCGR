// NanoC.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Platform.h"


#include "../NetListener/NetListener.h"

NanoC::NanoC() {
	hInstance = NULL;
	iModel = NULL;
}

NanoC::~NanoC() {
	if (0 != m_sMainThread) {
		printf("Terminating thread\n");
		if (WAIT_TIMEOUT == __NANOC_THREAD_WAIT__(m_sMainThread)) {
			__NANOC_THREAD_END__(m_sMainThread);
		}
	}
	if (NULL != hInstance) {
		printf("Unloading lib\n");
		__NANOC_DLLUNLOAD__(hInstance);
	}
}

void NanoC::Sleep(INT32 n32MilliSecond) {
#ifdef _WINDOWS
		::Sleep(n32MilliSecond);
#else
		usleep(n32MilliSecond * 1000);
#endif
}

void NanoC::Init() {
	printf("This is NanoC Init\n");

	__NANOC_DLLOAD__(hInstance, "nanocimp");
	if (NULL == hInstance) {
		printf("Instance load failed\n");
	}
	else {
		printf("Instance loaded successfully\n");
	}

	printf("IModel is %p\n", iModel);
	if (NULL != iModel) {
		iModel->Init();
	}

	__NANOC_THREAD_BEGIN__(m_sMainThread, NanoC::MainThread, this);
	if (0 == m_sMainThread) {
		printf("Thread start error\n");
	}
}

void NanoC::MainLoop() {
	if (NULL != iModel) {
		iModel->MainLoop();
	}
}

__NANOC_THREAD_FUNC_BEGIN__(NanoC::MainThread) {
	printf("This is MainThread\n");

	//TODO
	//开启网络监听，并将消息放在队列中，从这里获取消息，将消息发送给模块
	GetNetListener()->Init();
	GetNetListener()->MainLoop();

	printf("MainThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}

void NanoC::SetModelInstance(INanoCModel * iModel) {
	this->iModel = iModel;
}

NanoC g_NanoCInstance;

extern "C" __NANOC_EXPORT__ INanoC * GetNanoC() {
	return &g_NanoCInstance;
}