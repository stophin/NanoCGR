// NetSession.h
//

#pragma once

#include "Platform.h"

#ifdef _NANOC_WINDOWS_

class INetSession {
public:
	INetSession() : bIfUse(false) {
	}
	INT iSessionID;
	BOOL bIfUse;
};

/**
* 结构体名称：PER_IO_DATA
* 结构体功能：重叠I/O需要用到的结构体，临时记录IO数据
**/
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[1025];
	int BufferLen;
	int operationType;
	INetSession * netSession;
	SOCKET client;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;
/**
* 结构体名称：PER_HANDLE_DATA
* 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
* 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

class NetSession : public INetSession {
public:
	NetSession();
	~NetSession();

	PER_HANDLE_DATA handleData;;
	PER_IO_OPERATEION_DATA operationData;
};

class NetSessionManager {
public:
	NetSessionManager();
	~NetSessionManager();

	NetSession * GetFreeSession();

	INT32 getSize();
private:
	INT32 n32Size;
	NetSession * netSession;
};

#else

#define MAXEPOLL 1000
#define MAXLINE 1024

#endif