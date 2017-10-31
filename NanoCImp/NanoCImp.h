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
};

extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp();