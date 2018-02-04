// NanoCImp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Platform.h"

NanoCImp::NanoCImp() {
	printf("NanoCImp is registered\n");
	GetNanoC()->SetModelInstance(this);
}
NanoCImp::~NanoCImp() {

}

void NanoCImp::Init() {
	printf("This is NanoCImp Init\n");
}

void NanoCImp::Sleep(INT32 n32MilliSecond) {
#ifdef _WINDOWS
	::Sleep(n32MilliSecond);
#else
	usleep(n32MilliSecond * 1000);
#endif
}

void NanoCImp::MainLoop() {
	printf("This is NanoCImp MainLoop\n");
	NanoCImp * pThis = (NanoCImp*)this;
	if (NULL == pThis) {
		return;
	}

	printf("%p == %p\n", GetNanoC()->msgPool, GetPool());

	int count = 0;
	MultiLinkList<CharString> * msgQueue = &GetNanoC()->msgQueue;
	while (true) {
		//结束主线程，其余子线程也结束
		if (pThis->isRunning == 0) {
			break;
		}
		__NANOC_THREAD_MUTEX_LOCK__(pThis->hNetMutex);
		if (msgQueue->linkcount > 0) {
			CharString * charString = msgQueue->getPos(0);
			if (NULL != charString) {
				msgQueue->removeLink(charString);

				//printf("NanoCImp Get(%d/%d): %s\n", msgQueue->linkcount, GetNanoC()->msgPool->used, charString->str);
				printf("NanoCImp Get(%d/%d)\n", msgQueue->linkcount, GetNanoC()->msgPool->used);

				//GetNanoC()->msgPool->back(charString);
			}
		}
		__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hNetMutex);
	}

	printf("This is NanoCImp MainLoop End\n");
}

NanoCImp g_NanoCImp;
extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp() {
	return &g_NanoCImp;
}