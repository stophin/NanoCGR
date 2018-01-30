// NanoCImp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Platform.h"

NanoCImp::NanoCImp() {
	printf("NanoCImp is registered\n");
	GetNanoC()->SetModelInstance(this);
}
NanoCImp::~NanoCImp() {

}

void NanoCImp::Init() {
	printf("This is NanoCImp Init\n");
}

void NanoCImp::MainLoop() {
	printf("This is NanoCImp MainLoop\n");

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
					break;
				}
			}
		}
	}

	printf("This is NanoCImp MainLoop End\n");
}

NanoCImp g_NanoCImp;
extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp() {
	return &g_NanoCImp;
}