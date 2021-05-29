// NanoC.cpp : ���� DLL Ӧ�ó���ĵ���������
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

	//��load���dllʱ�������dll�ӿ�ʵ�ֵĹ��캯��
	//���ù��캯����ͨ��SetModelInstance���Լ�ע�ᵽ
	//�����iModel�У����ʹ��iModel���е��á�
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

	//�������������������Ϣ���ڶ�����
	GetNetListener()->Init();
	//��ȡNet�߳���
	this->hNetMutex = GetNetListener()->hMutex;

	//��ȡ��Ϣ��
	static CharStringPool pool;
	this->msgPool = &pool;
	//�����߳���
	__NANOC_THREAD_MUTEX_INIT__(hMutex, this);
	printf("%p == %p\n", GetNetListener()->msgPool, this->msgPool);
	//�����߳�
	__NANOC_THREAD_BEGIN__(m_sMainThread, NanoC::MainThread, this);
	if (0 == m_sMainThread) {
		printf("Thread start error\n");
	}
}

void NanoC::MainLoop() {
	if (NULL != iModel){
		//��ȡ�߳���
		iModel->hMutex = this->hMutex;
		iModel->hNetMutex = this->hNetMutex;
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
	//�������ȡ���������Ϣ
	MultiLinkList<CharString> * msgQueue = &GetNetListener()->msgQueue;
	while (true) {
		if (pThis->iModel->isRunning == 0) {
			GetNetListener()->isRunning = 0;
		}
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
		pThis->Sleep(100);
	}

	printf("MainThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}

void NanoC::SetModelInstance(INanoCModel * iModel) {
	this->iModel = iModel;
}


int NanoC::sendMessage(INetSession * session, const char * buf, int size) {
	return GetNetListener()->sendMessage(session, buf, size);
}

int NanoC::emitMessage(INetSession * session, const char * buf, int size) {
	return GetNetListener()->emitMessage(session, buf, size);
}


int NanoC::aliveSession(INetSession** sessions, int size) {
	return GetNetListener()->aliveSession(sessions, size);
}

int NanoC::closeSession(INetSession * session) {
	return GetNetListener()->closeConnection(session);
}

NanoC g_NanoCInstance;

extern "C" __NANOC_EXPORT__ INanoC * GetNanoC() {
	return &g_NanoCInstance;
}