//INetListener.h
//
//

#pragma once

#include "Platform.h"

class INetListener  {
public:
	INetListener() : msgQueue(0) {
	}
	~INetListener() {
	}
public:
	virtual void Init() = 0;
public:
#ifdef _NANOC_WINDOWS_
	HANDLE hCompletionPort; //listen completion port
	SOCKET hListenSocket; //listen socket port
	SOCKET hSessionSocket;
	LPFN_ACCEPTEX lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS lpfnAcceptExSocketAddrs;
	INT8 bOutputBuffer[1024];
	WSAOVERLAPPED	sOverLapped;
#else
	SOCKET epoll_fd;	//epoll
	SOCKET hListenSocket; //listen socket port
	SOCKET hSessionSocket;
#endif

public:
	MultiLinkList<CharString> msgQueue;
};