// Platform.h
//
//

#pragma once

#include "../NanoC/Platform.h"

#ifdef _NANOC_WINDOWS_

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�

#else

#endif

#include "../NanoC/CharString.h"
#include "../NanoC/CharString.cpp"

#include "INanoCImp.h"

#include "NanoCImp.h"
