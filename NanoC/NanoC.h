//NanoC.h
//
//

#pragma once

#include "INanoC.h"

class NanoC : public INanoC {
public:
	NanoC();
	~NanoC();

public:
	virtual void Init();
	virtual void Sleep(INT32 n32MilliSecond);
	virtual void SetModelInstance(INanoCModel * model);

	virtual void MainLoop();

	__NANOC_THREAD_FUNC_DECLARE(m_sMainThread, MainThread);
public:
	HINSTANCE hInstance;

	virtual int sendMessage(INetSession * session, const char * buf, int size = 0);
	virtual int closeSession(INetSession * session);
};

extern "C" __NANOC_EXPORT__ INanoC * GetNanoC();