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
				case 0://Unicode Stream
				{
							charString->transFromUnicode();

							printf("Unicode Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
							printf("%s\n", charString->getLastAsANSI());

							//回复
							if (GetNanoC()->sendMessage(charString->session, charString->getLastAsUTF8()) > 0) {
								printf("Unicode Send\n");
							}
							break;
				}
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
								CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "1::");
								if (CharString::makeWS(charString->_str, charString->__str, 101, _key, charString->str)) {
									//回复
									int sendCompleted = 0;
									if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
										printf("WS Header Send\n");
										//printf("%s", charString->_str);
										sendCompleted = 1;
									}
									if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
										printf("WS Content Send\n");
										//printf("%s", charString->__str);
										sendCompleted = 2;
									}

									if (sendCompleted == 2) {
									}
									else {
										printf("Http Msg send error\n");
									}
								}
							}
						   else if (CharString::match(url, "/socket.io/1")) {
							   //返回http信息
							   if (CharString::makeHTTP(charString->_str, charString->__str, 200, "96e1cc6b-b2c7-4372-967f-172b3f9a2a99:200:60:websocket,flashsocket")) {
								   //回复
								   int sendCompleted = 0;
								   if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
									   printf("Http Header Send\n");
									   sendCompleted = 1;
								   }
								   if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
									   printf("Http Content Send\n");
									   sendCompleted = 2;
								   }

								   if (sendCompleted == 2) {
									   //HTTP关闭连接
									   //TODO
									   //这里在多个线程中关闭连接可能会出现错误
									   //GetNanoC()->closeSession(charString->session);
								   }
								   else {
									   printf("Http Msg send error\n");
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
									   printf("Http Header Send\n");
									   sendCompleted = 1;
								   }
								   if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
									   printf("Http Content Send\n");
									   sendCompleted = 2;
								   }

								   if (sendCompleted == 2) {
									   //HTTP关闭连接
									   //TODO
									   //这里在多个线程中关闭连接可能会出现错误
									   //GetNanoC()->closeSession(charString->session);
								   }
								   else {
									   printf("Http Msg send error\n");
									   msgQueue->insertLink(charString);
								   }
							   }
						   }
						  break;
				}
				case 2: //WebSocket
				{
							const char * str = charString->getStr();

							CharString::decodeFrame(charString->_str, str);

							printf("WebSocket Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
							printf("%s\n", charString->_str);

							//回复
							CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"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\"]}");
							if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
								printf("WebSocket Send\n");
							}
							break;
				}
				default://Normal Stream
				{
							charString->Reflush();
						   const char * str = charString->getStr();

						   printf("Message Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
						   printf("%s\n", str);

						   //回复
						   if (GetNanoC()->sendMessage(charString->session, str) > 0) {
							   printf("Message Send\n");
						   }

						   break;
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