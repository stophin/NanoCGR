//INanoCImp.h
//

#pragma once

#include "../NanoC/INanoC.h"

class INanoCImp : public INanoCModel {
public:
	virtual void Init() = 0;
	virtual void MainLoop() = 0;
};