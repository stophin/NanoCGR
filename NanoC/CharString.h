//CharString.h
//
#ifndef _CharString_H_
#define _CharString_H_

#include "../NanoC/NanoType.h"

class INetSession;
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

	/////////////////////////////////////
	int pos;
	int len;
	void Reflush() {
		this->pos = 0;
		this->len = this->getInt();
		if (this->len > 1024) {
			this->len = 1024;
		}
		this->str[this->len] = 0;
	}
	int getInt() {
		if (this->pos > this->len) {
			return 0;
		}
		int ret = *(int*)(this->str + this->pos);
		this->pos += sizeof(int);
		return ret;
	}
	const char * getStrUnicode() {
		int len;
		const char * str = this->getStr(&len);

#ifdef _NANOC_WINDOWS_

		//返回需要的buffer大小
		int _len = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)str, len, _str, 1024, "△", NULL);
		if (_len > 0) {
			//计算中非双字节字符的个数
			int cc = 0;
			for (int i = 0; i < len && i < _len; i++) {
				if (_str[i] & 0x80) {
					continue;
				}
				cc++;
			}
			_str[len - cc] = 0;
		} else {
			_str[0] = 0;
			printf("Trans failed: %d\n", _len);
		}
		return _str;
#else 
		//Linux下wchar占4个字节，需要对2字节字符串进行扩充
		for (int i = len * 2; i >= 0; i--){
			if (i > 1024) {
				continue;
			}
			this->_str[i] = 0;
		}
		for (int i = 0; i < len; i++) {
			int ind = 0;
			if (i % 2 == 0) {
				ind = i * 2;
			}
			else {
				ind = i * 2 - 1;
			}
			if (ind > 1024) {
				continue;
			}
			this->_str[ind] = str[i];
		}
		char * dest = &this->_str[len * 3];

		setlocale(LC_ALL, "");
		int _len = wcstombs(dest, (wchar_t*)_str, 1024 - len * 3);
		if (_len > 0) {
		}
		else {
			//dest[0] = 0;
			printf("Trans failed: %d\n", _len);
		}
		return dest;
#endif
	}
	const char * getStr(int * len = NULL) {
		int _len;
		if (len == NULL) {
			len = &_len;
		}
		if (this->pos > this->len) {
			return NULL;
		}
		*len = getInt();
		if (this->len > 1024) {
			this->len = 1024;
		}
		if (this->pos + *len > this->len) {
			*len = this->len - this->pos;
		}
		char * str = (this->str + this->pos);
		str[*len] = 0;
		this->pos += *len;
		return str;
	}
	/////////////////////////////////////

	void set(const char * str) {
		if (NULL == str) {
			len = 1;
			this->str[0] = 0;
		}
		else{
			for (len = 0; str[len] != 0 && len < 1024; len++);
			int _len = *(int*)(str);
			if (_len <= 1024 && _len > len) {
				len = _len;
			}
			for (int i = 0; i < len; i++) {
				this->str[i] = str[i];
			}
			this->str[len] = 0;
		}
		this->Reflush();
	}

	INetSession * session;

	char _str[1025];
	char str[1025];

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
		//printf("Get failed\n");
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
