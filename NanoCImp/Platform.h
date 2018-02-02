// Platform.h
//
//

#pragma once

#include "../NanoC/Platform.h"

#ifdef _NANOC_WINDOWS_

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

// TODO:  在此处引用程序需要的其他头文件

#else

#endif

#include "../NanoC/CharString.h"
#include "../NanoC/CharString.cpp"

#include "INanoCImp.h"

#include "NanoCImp.h"
