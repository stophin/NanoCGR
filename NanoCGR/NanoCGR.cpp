// NanoCGR.cpp : 定义控制台应用程序的入口点。
//

#include "../NanoC/NanoC.h"

__NANOC_THREAD_FUNC_BEGIN__(pThreadMain) {

	char str[512];
	int ind = 0;
	while (true) {
		if (ind < 512) {
			str[ind++] = getchar();

			if (str[ind - 1] == '\n') {
				str[ind - 1] = '\0';
				ind = 0;
				printf("%s\n", str);

				if (!strcmp(str, "exit")) {
					GetNanoC()->iModel->isRunning = 0;
					break;
				}
			}
		}
	}

	__NANOC_THREAD_FUNC_END__(0);
}

HANDLE pThread;

int main()
{
	GetNanoC()->Init();

	__NANOC_THREAD_BEGIN__(pThread, pThreadMain, NULL);
	if (NULL == pThread) {
		printf("Main thread start error\n");
	}

	GetNanoC()->MainLoop();

	GetNanoC()->Sleep(3000);
	return 0;
}

