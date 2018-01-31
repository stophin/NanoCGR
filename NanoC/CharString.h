//CharString.h
//
#ifndef _CharString_H_
#define _CharString_H_

#include <stdlib.h>

typedef class CharString
{
public:
	CharString();
	CharString(const char * str);
	~CharString();

	char * str;
	int len;

	int f;
	int uniqueID;
#define CharString_Max 1
	void initialize() {
		for (int i = 0; i < CharString_Max; i++)
		{
			this->prev[i] = NULL;
			this->next[i] = NULL;
		}
	}
	CharString * prev[CharString_Max];
	CharString * next[CharString_Max];
	void operator delete(void * _ptr){
		if (_ptr == NULL)
		{
			return;
		}
		for (int i = 0; i < CharString_Max; i++)
		{
			if (((CharString*)_ptr)->prev[i] != NULL || ((CharString*)_ptr)->next[i] != NULL)
			{
				return;
			}
		}
		delete(_ptr);
	}
}CharString, *PCharString;

#endif	//end of _CharString_H_
