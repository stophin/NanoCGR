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

#include <time.h>
#include <string>
using namespace std;


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
				case 1://OnConnect
				{
						   printf("Http:\n");
						   const char * str = charString->getStr();
						   printf("%s\n", str);

						   //返回http信息
						   string statusCode("200 OK");
						   string contentType("text/html");
						   string content("96e1cc6b-b2c7-4372-967f-172b3f9a2a99:200:60:websocket,flashsocket");
						   string contentSize("65");
						   string head("\r\nHTTP/1.1 ");
						   string ContentType("\r\nContent-Type: ");
						   string ServerHead("\r\nServer: localhost");
						   string ContentLength("\r\nContent-Length: ");
						   string Date("\r\nDate: ");
						   string Newline("\r\n");
						   time_t rawtime;
						   time(&rawtime);
						   string message;
						   message += head;
						   message += statusCode;
						   message += ContentType;
						   message += contentType;
						   message += ServerHead;
						   message += ContentLength;
						   message += contentSize;
						   message += Date;
						   message += (string)ctime(&rawtime);
						   message += Newline;

						   //回复
						   INT send = 0;
						   if (GetNanoC()->sendMessage(charString->session, message.c_str()) > 0) {
							   printf("NanoCImp Header Send\n");
							   send = 1;
						   }
						   if (GetNanoC()->sendMessage(charString->session, content.c_str()) > 0) {
							   printf("NanoCImp Content Send\n");
							   send = 2;
						   }

						   if (send == 2) {
							   //HTTP关闭连接
							   //GetNanoC()->closeSession(charString->session);
						   }
						   else {
							   printf("NanoCImp Msg send error\n");
							   msgQueue->insertLink(charString);
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