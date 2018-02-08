// NanoCImp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Platform.h"

NanoCImp::NanoCImp() {
	printf("NanoCImp is registered\n");
	this->isRunning = 1;
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
	printf("NanoC msg pool: %p\n", GetNanoC()->msgPool);

	int count = 0;
	MultiLinkList<CharString> * msgQueue = &GetNanoC()->msgQueue;
	while (true) {
		//结束主线程，其余子线程也结束
		if (pThis->isRunning == 0) {
			break;
		}
		__NANOC_THREAD_MUTEX_LOCK__(pThis->hMutex);
		if (msgQueue->linkcount > 0) {
			CharString * charString = msgQueue->getPos(0);
			if (NULL != charString) {
				msgQueue->removeLink(charString);

				UINT32 n32Protocol = charString->getInt();

				switch (n32Protocol) {
				case 1://Http
				{
						   printf("Http:\n");
						   const char * str = charString->getStr();
						   printf("%s\n", str);

						   //解析地址
						   const char * url = str;
						   for (int i = 0; url[i]; i++) {
							   if (CharString::match(&url[i], "GET ")) {
								   url = url + i + strlen("GET ");
								   break;
							   }
						   }

							if (CharString::match(url, "/socket.io/1/websocket/96e1cc6b-b2c7-4372-967f-172b3f9a2a99")) {

								char _key[128];
								const char * key = str;
								for (int i = 0; key[i]; i++) {
									if (CharString::match(&key[i], "Sec-WebSocket-Key: ")) {
										key = key + i + strlen("Sec-WebSocket-Key: ");
										break;
									}
								}
								int len;
								for (len = 0; key[len]; len++) {
									if (key[len] == '\r') {
										break;
									}
									_key[len] = key[len];
								}
								_key[len] = 0;

								//返回http信息
								if (CharString::makeWS(charString->_str, charString->__str,101, _key, "")) {
									//回复
									int sendCompleted = 0;
									if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
										printf("NanoCImp Header Send\n");
										//printf("%s", charString->_str);
										sendCompleted = 1;
									}
									if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
										printf("NanoCImp Content Send\n");
										//printf("%s", charString->__str);
										sendCompleted = 2;
									}

									if (sendCompleted == 2) {
									}
									else {
										printf("NanoCImp Msg send error\n");
									}
								}
							}
						   else if (CharString::match(url, "/socket.io/1")) {
							   //返回http信息
							   if (CharString::makeHTTP(charString->_str, charString->__str, 200, "96e1cc6b-b2c7-4372-967f-172b3f9a2a99:200:60:websocket,flashsocket")) {
								   //回复
								   int sendCompleted = 0;
								   if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
									   printf("NanoCImp Header Send\n");
									   sendCompleted = 1;
								   }
								   if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
									   printf("NanoCImp Content Send\n");
									   sendCompleted = 2;
								   }

								   if (sendCompleted == 2) {
									   //HTTP关闭连接
									   //TODO
									   //这里在多个线程中关闭连接可能会出现错误
									   //GetNanoC()->closeSession(charString->session);
								   }
								   else {
									   printf("NanoCImp Msg send error\n");
									   msgQueue->insertLink(charString);
								   }
							   }
						   }
						   else {
							   //返回http信息
							   if (CharString::makeHTTP(charString->_str, charString->__str, 200, "NO DATA CONTENT")) {
								   //回复
								   int sendCompleted = 0;
								   if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
									   printf("NanoCImp Header Send\n");
									   sendCompleted = 1;
								   }
								   if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
									   printf("NanoCImp Content Send\n");
									   sendCompleted = 2;
								   }

								   if (sendCompleted == 2) {
									   //HTTP关闭连接
									   //TODO
									   //这里在多个线程中关闭连接可能会出现错误
									   //GetNanoC()->closeSession(charString->session);
								   }
								   else {
									   printf("NanoCImp Msg send error\n");
									   msgQueue->insertLink(charString);
								   }
							   }
						   }
						  break;
				}
				case 2: {//WebSocket
							const char * str = charString->getStr();

							CharString::decodeFrame(charString->_str, str);

							printf("NanoCImp Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
							printf("%s\n", charString->_str);

							//回复
							if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
								printf("NanoCImp Send\n");
							}
							break;
				}
				default:
				{
						   charString->transFromUnicode();

						   printf("NanoCImp Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
						   printf("%s\n", charString->getLastAsANSI());

						   //回复
						   if (GetNanoC()->sendMessage(charString->session, charString->getLastAsUTF8()) > 0) {
							   printf("NanoCImp Send\n");
						   }
				}
				}
			}
		}
		__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hMutex);
		pThis->Sleep(100);
	}

	printf("This is NanoCImp MainLoop End\n");
}

NanoCImp g_NanoCImp;
extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp() {
	return &g_NanoCImp;
}