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
};

extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp();