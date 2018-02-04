//CharString.h
//
#ifndef _CharString_H_
#define _CharString_H_

#include "../NanoC/NanoType.h"

typedef class CharString
{
public:
	CharString() :
		len(0){
		initialize();
	}

	CharString(const char* str) :
		len(0){
		set(str);
		initialize();
	}

	~CharString()
	{
	}

	void set(const char * str) {
		if (NULL == str) {
			len = 1;
			this->str[0] = 0;
		}
		else{
			for (; str[len] != 0 && len < 1024; len++);
			for (int i = 0; i < len; i++) {
				this->str[i] = str[i];
			}
			this->str[len] = 0;
		}
	}

	char str[1025];
	int len;

	int used;
	int f;
	int uniqueID;
#define CharString_Max 3
	void initialize() {
		for (int i = 0; i < CharString_Max; i++)
		{
			this->prev[i] = NULL;
			this->next[i] = NULL;
		}
	}
	CharString * prev[CharString_Max];
	CharString * next[CharString_Max];
}CharString, *PCharString;

typedef unsigned char UMAP;
#define POOL_MAX	128
#define MAP_SHIFT	8
#define MAP_MASK	0xFF
#define GET_MAP_SIZE(x) (x / MAP_SHIFT + 1)
class ICharStringPool {
public:
	virtual CharString * get() = 0;
	virtual void back(CharString * o) = 0;
};

class CharStringPool : public ICharStringPool {
public:
	CharStringPool():
		size(POOL_MAX),
		pool(NULL),
		map(NULL){
		pool = new CharString[size];
		mapsize = GET_MAP_SIZE(size);
		map = new UMAP[mapsize];
		for (int i = 0; i < mapsize; i++){
			map[i] = MAP_MASK;
		}
		used = 0;
	}
	~CharStringPool() {
		if (pool) {
			delete[] pool;
			pool = NULL;
		}
		if (map) {
			delete[] map;
			map = NULL;
		}
	}

	int size;
	CharString * pool;
	UMAP * map;
	int mapsize;
	int used;

	void gc(int index) {
		if (index < 0 || index >= this->size) {
			return;
		}
		CharString * _ptr = &this->pool[index];
		int i, j;
		i = index / MAP_SHIFT;
		j = index - i * MAP_SHIFT;
		this->map[i] |= (0x01 << j);
		used--;
		_ptr->used = 0;
		memset(_ptr, 0, sizeof(CharString));
		//printf("CharString released\n");
	}

	int gc(CharString * _ptr) {
		int i;
		for (i = 0; i < CharString_Max; i++)
		{
			if (_ptr->prev[i] != NULL || _ptr->next[i] != NULL)
			{
				return 0;
			}
		}
		return 1;
	}

	void gc() {
		int i, j, index;
		for (i = 0, index = 0; i < this->mapsize && index < this->size; i++) {
			for (j = 0; j < MAP_SHIFT && index < this->size; j++, index++) {
				if (this->map[i] & (0x01 << j)) {
					continue;
				}
				if (this->pool[index].used == 1) {
					if (this->gc(&this->pool[index]) == 1) {
						this->gc(index);
					}
				}
			}
		}
	}

	CharString * get() {
		int i, j, index;
		for (i = 0, index = 0; i < this->mapsize && index < this->size; i++, index += MAP_SHIFT) {
			if (this->map[i] & MAP_MASK) {
				for (j = 0; j < MAP_SHIFT && index < this->size; j++, index++) {
					if (this->map[i] & (0x01 << j)) {
						this->map[i] &= ~(0x01 << j);
						used++;
						this->pool[index].used = 1;
						//printf("CharString got\n");
						return &this->pool[index];
					}
				}
			}
		}
		return NULL;
	}

	void back(CharString * o) {
		int index;
		if (o == NULL) {
			return;
		}
		//printf("Trying to releases-->");
		for (index = 0; index < this->size; index++) {
			if (&this->pool[index] == o) {
				this->gc(index);
				return;
			}
		}
		//printf("Fail!\n");
		o = NULL;
		return;
	}
};

extern "C" __NANOC_EXPORT__ ICharStringPool  * GetPool(); 

#endif	//end of _CharString_H_
