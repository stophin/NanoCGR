// Platform.h
//
//

#pragma once

#include "../NanoC/Platform.h"

#ifdef _NANOC_WINDOWS_

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

#else

#endif

#include "INetListener.h"

#include "NetListener.h"
