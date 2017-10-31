// NanoCGR.cpp : 定义控制台应用程序的入口点。
//

#include "../NanoC/NanoC.h"

int main()
{
	GetNanoC()->Init();

	GetNanoC()->MainLoop();

	GetNanoC()->Sleep(3000);
	return 0;
}

