// NanoCGR.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "../NanoC/NanoC.h"

int main()
{
	GetNanoC()->Init();

	GetNanoC()->MainLoop();

	GetNanoC()->Sleep(3000);
	return 0;
}

