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
#ifdef _NANOC_WINDOWS_
	NetSessionManager netSession;

	INT32 MakeFreeIOCompletionPort(NetSession* session);
#else
#endif
public:
	virtual void addMsgQueue(const char * buf);
};

__NANOC_EXPORT__ INetListener * GetNetListener();