//NetListenr.h
//
//

#pragma once

#include "INetListener.h"
#include "NetSession.h"

class NetListener : public INetListener {
public:
	NetListener();
	~NetListener();
	void CleanUp();
public:
	virtual void Init();

	__NANOC_THREAD_FUNC_DECLARE(m_phIOThread, IOCPThread);
public:
	BOOL bIfInitialized;

	NetSessionManager netSession;
#ifdef _NANOC_WINDOWS_

	INT32 MakeFreeIOCompletionPort(INetSession* session);
#else
#endif
public:
	virtual int addMsgQueue(INetSession * session, const char * buf, int size = 0);
	virtual int sendMessage(INetSession * session, const char * buf);
	virtual int closeConnection(INetSession * session);
};

__NANOC_EXPORT__ INetListener * GetNetListener();