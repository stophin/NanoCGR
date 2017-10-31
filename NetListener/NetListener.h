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
	virtual void MainLoop();

	__NANOC_THREAD_FUNC_DECLARE(m_phIOThread, IOCPThread);
public:
	BOOL bIfInitialized;
#ifdef _NANOC_WINDOWS_
	NetSessionManager netSession;
#else
#endif
};

__NANOC_EXPORT__ INetListener * GetNetListener();