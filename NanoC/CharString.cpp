//CharString.cpp
//


#include "CharString.h"

CharString::CharString() :
len(0),
str(NULL){
	initialize();
}

CharString::CharString(const char* str) :
len(0),
str(NULL) {
	if (NULL == str) {
		return;
	}
	for (; str[len] != 0; len++);
	this->str = new char[len + 1];
	for (int i = 0; i < len; i++) {
		this->str[i] = str[i];
	}
	this->str[len] = 0;

	initialize();
}

CharString::~CharString()
{
	delete[] this->str;
}
