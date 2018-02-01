//CharString.cpp
//


#include "CharString.h"

CharString::CharString() :
len(0){
	initialize();
}

CharString::CharString(const char* str) :
len(0){
	if (NULL == str) {
		len = 1;
		this->str[0] = 0;
	}
	else{
		for (; str[len] != 0; len++);
		if (len > 1024) {
			len = 1024;
		}
		for (int i = 0; i < len; i++) {
			this->str[i] = str[i];
		}
		this->str[len] = 0;
	}

	initialize();
}

CharString::~CharString()
{
}
