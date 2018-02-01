// NanoCImp.h
//
//

#pragma once

#include "INanoCImp.h"

class NanoCImp : public INanoCImp{
public:
	NanoCImp();
	~NanoCImp();

public:
	virtual void Init();
	virtual void MainLoop();
	virtual void Sleep(INT32 n32MilliSecond);
};

extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp();