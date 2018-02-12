//INanoC.h
//
//

#pragma once

#include "Platform.h"

class INanoCModel {
public:
	virtual void Init() = 0;
	virtual void MainLoop() = 0;
	virtual void Sleep(INT32 n32MilliSecond) = 0;
	HANDLE_MUTEX hMutex;
	HANDLE_MUTEX hNetMutex;
	INT isRunning;
};

class INanoC {
public:
	INanoC() : msgQueue(1) {
	}
	~INanoC() {
	}
public:
	virtual void Init() = 0;
	virtual void Sleep(INT32 n32MilliSecond) = 0;
	virtual void SetModelInstance(INanoCModel * model) = 0;

	virtual void MainLoop() = 0;

public:
	INanoCModel * iModel;
public:
	HANDLE_MUTEX hNetMutex;
	HANDLE_MUTEX hMutex;
	MultiLinkList<CharString> msgQueue;
	CharStringPool * msgPool;

	virtual int sendMessage(INetSession * session, const char * buf, int size = 0) = 0;
	virtual int closeSession(INetSession * session) = 0;
};