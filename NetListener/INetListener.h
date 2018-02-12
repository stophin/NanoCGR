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
	WSAOVERLAPPED	sOverLapped;
#else
	SOCKET epoll_fd;	//epoll
	SOCKET hListenSocket; //listen socket port
	SOCKET hSessionSocket;
	INT cur_fds;
#endif
	INT isRunning;

public:
	HANDLE_MUTEX hMutex;
	MultiLinkList<CharString> msgQueue;
	CharStringPool * msgPool;

	virtual int addMsgQueue(INetSession * session, const char * buf, int size = 0) = 0;
	virtual int sendMessage(INetSession * session, const char * buf, int size = 0) = 0;
	virtual int closeConnection(INetSession * session) = 0;
};