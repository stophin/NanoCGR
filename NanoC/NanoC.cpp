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

	//当load这个dll时，会调用dll接口实现的构造函数
	//而该构造函数会通过SetModelInstance把自己注册到
	//本类的iModel中，最后使用iModel进行调用。
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

	//开启网络监听，并将消息放在队列中
	GetNetListener()->Init();
	//获取Net线程锁
	this->hNetMutex = GetNetListener()->hMutex;

	//获取消息池
	static CharStringPool pool;
	this->msgPool = &pool;
	//创建线程锁
	__NANOC_THREAD_MUTEX_INIT__(hMutex, this);
	printf("%p == %p\n", GetNetListener()->msgPool, this->msgPool);
	//开启线程
	__NANOC_THREAD_BEGIN__(m_sMainThread, NanoC::MainThread, this);
	if (0 == m_sMainThread) {
		printf("Thread start error\n");
	}
}

void NanoC::MainLoop() {
	if (NULL != iModel){
		//获取线程锁
		iModel->hMutex = this->hMutex;
		iModel->hNetMutex = this->hNetMutex;
		iModel->isRunning = 1;
		iModel->MainLoop();
	}
}

__NANOC_THREAD_FUNC_BEGIN__(NanoC::MainThread) {
	printf("This is MainThread\n");

	NanoC * pThis = (NanoC*)pv;
	if (NULL == pThis) {
		__NANOC_THREAD_FUNC_END__(0);
	}
	printf("NetListener msg pool: %p\n", GetNetListener()->msgPool);

	CharString * charString, *_charString;
	//从这里获取网络监听消息
	MultiLinkList<CharString> * msgQueue = &GetNetListener()->msgQueue;
	while (true) {
		charString = NULL;
		_charString = NULL;
		__NANOC_THREAD_MUTEX_LOCK__(pThis->hNetMutex);
		if (msgQueue->linkcount > 0) {
			charString = msgQueue->getPos(0);
			if (NULL != charString) {
				msgQueue->removeLink(charString);

				//printf("NanoC Get(%d/%d): %s\n", msgQueue->linkcount, pThis->msgPool->used, charString->str);

				__NANOC_THREAD_MUTEX_LOCK__(pThis->hMutex);
				if (pThis->msgPool->used >= POOL_MAX) {
					pThis->msgPool->gc();
				}
				_charString = pThis->msgPool->get();
				if (_charString != NULL) {
					_charString->set(charString->str);
					_charString->session = charString->session;
					_charString->f = charString->f;
					pThis->msgQueue.insertLink(_charString);
				}
				__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hMutex);

			}
		}
		__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hNetMutex);
	}

	printf("MainThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}

void NanoC::SetModelInstance(INanoCModel * iModel) {
	this->iModel = iModel;
}


int NanoC::sendMessage(INetSession * session, const char * buf) {
	return GetNetListener()->sendMessage(session, buf);
}

NanoC g_NanoCInstance;

extern "C" __NANOC_EXPORT__ INanoC * GetNanoC() {
	return &g_NanoCInstance;
}