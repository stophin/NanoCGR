//CharString.h
//
#ifndef _CharString_H_
#define _CharString_H_

#include "../NanoC/NanoType.h"

#include "sha1.h"

#define MAX_BUFFERSIZE 8192


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
	enum WS_FrameType
	{
		WS_EMPTY_FRAME = 0xF0,
		WS_ERROR_FRAME = 0xF1,
		WS_TEXT_FRAME = 0x01,
		WS_BINARY_FRAME = 0x02,
		WS_PING_FRAME = 0x09,
		WS_PONG_FRAME = 0x0A,
		WS_OPENING_FRAME = 0xF3,
		WS_CLOSING_FRAME = 0x08
	};
	static int decodeFrame(char * out, const char * inFrame) {
		if (NULL == out) {
			return 0;
		}
		out[0] = 0;
		if (NULL == inFrame) {
			return 0;
		}
		unsigned int frameLength;
		for (frameLength = 0; inFrame[frameLength]; frameLength++);
		if (frameLength < 2) {
			return 0;
		}
		//检查扩展为并忽略
		if ((inFrame[0] & 0x70) != 0x0) {
			return 0;
		}
		//fin位：为1表示已接收完整保温，为0表示继续监听后续报文
		if ((inFrame[0] & 0x80) != 0x80) {
			return 0;
		}
		//mask位：位1表示数据被加密
		if ((inFrame[1] & 0x80) != 0x80) {
			return 0;
		}
		//操作码
		unsigned short payloadLength = 0;
		unsigned char payLoadExtraBytes = 0;
		unsigned char opcode = (unsigned char)(inFrame[0] & 0x0f);
		if (WS_TEXT_FRAME == opcode) {
			//处理utf-8编码的文本帧
			payloadLength = (unsigned short)(inFrame[1] & 0x7f);
			if (0x7e == payloadLength) {
				//unsigned short payloadLength16b = 0;
				//unsigned short payloadFieldExtraBytes = 2;
				unsigned short payloadLength16b = 0;
				payloadLength16b = (unsigned short)(inFrame[2]);
				payloadLength = ntohs(payloadLength16b);
			}
			else if (0x7f == payloadLength) {
				//数据过长暂不支持
				return 0;
			}
		}
		else if (opcode == WS_BINARY_FRAME ||
			opcode == WS_PING_FRAME ||
			opcode == WS_PONG_FRAME) {
			//二进制/ping/pong帧暂不处理
		}
		else if (opcode == WS_CLOSING_FRAME) {
			return 0;
		}
		else {
			return 0;
		}
		//数据解码
		if (payloadLength > 0) {
			//header: 2字节，masking key: 4字节
			const char * maskingKey = &inFrame[2 + payLoadExtraBytes];
			const char * frameData = &inFrame[2 + payLoadExtraBytes + 4];
			char * payloadData = out;
			if (payloadLength > MAX_BUFFERSIZE) {
				payloadLength = MAX_BUFFERSIZE;
			}
			for (int i = 0; i < payloadLength; i++) {
				payloadData[i] = frameData[i];
			}
			payloadData[payloadLength] = 0;
			for (int i = 0; i < payloadLength; i++)  {
				payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
			}
		}

		return 0;
	}

	static int makeHTTP(char * header, char * content, int statusCode, const char * msg) {
		if (NULL == content) {
			return 0;
		}
		if (NULL == header) {
			return 0;
		}
		if (NULL == msg) {
			return 0;
		}

		int contentSize;
		for (contentSize = 0; msg[contentSize] && contentSize < MAX_BUFFERSIZE; contentSize++) {
			content[contentSize] = msg[contentSize];
		}
		content[contentSize] = 0;
		time_t rawtime;
		time(&rawtime);
		int headerPos = 0;
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "\r\nHTTP/1.1 ");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "%d OK", statusCode);
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "\r\nContent-Type: ");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "text/html");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "\r\nServer: localhost");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "\r\nContent-Length: ");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "%d", contentSize);
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "\r\nDate: ");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, ctime(&rawtime));
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "\r\n");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) header[headerPos] = 0;

		return contentSize + headerPos;
	}

	static int makeWS(char * header, char * content, int statusCode, const char * key, const char * msg) {
		if (NULL == content) {
			return 0;
		}
		if (NULL == header) {
			return 0;
		}
		if (NULL == msg) {
			return 0;
		}
		//计算key
		const char * MAGIC_KEY = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		string serverKey = string(key) + MAGIC_KEY;
		unsigned char message_digest[20];
		SHA1_String((const unsigned char*)serverKey.c_str(), serverKey.size(), message_digest);
		serverKey = base64_encode(message_digest, 20);

		int contentSize;
		for (contentSize = 0; msg[contentSize] && contentSize < MAX_BUFFERSIZE; contentSize++) {
			content[contentSize] = msg[contentSize];
		}
		content[contentSize] = 0;
		time_t rawtime;
		time(&rawtime);
		int headerPos = 0;
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "HTTP/1.1 %d Switching Protocols\r\n", statusCode);
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "Connection: upgrade\r\n");
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "Sec-WebSocket-Accept: %s\r\n", serverKey.c_str());
		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) headerPos += sprintf(header + headerPos, "Upgrade: websocket\r\n\r\n");

		if (headerPos >= 0 && headerPos < MAX_BUFFERSIZE) header[headerPos] = 0;

		return contentSize + headerPos;
	}

	static int match(const char * src, const char * dest) {
		if (NULL == src) {
			return 0;
		}
		if (NULL == dest) {
			return 0;
		}
		int i;
		for (i = 0; src[i] && dest[i]; i++) {
			if (src[i] == dest[i]) {
				continue;
			}
			return 0;
		}
		return i;
	}

	/////////////////////////////////////
	unsigned int pos;
	unsigned int len;
	void Reflush() {
		this->pos = 0;
		this->len = this->getInt();
		if (this->len > MAX_BUFFERSIZE) {
			this->len = MAX_BUFFERSIZE;
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
	const char * getLastAsUTF8() {
		return this->__str;
	}
	const char * getLastAsANSI() {
		return this->_str;
	}
	const char * transFromUnicode() {
		unsigned int len;
		const char * str = this->getStr(&len);

		if (NULL == str) {
			__str[0] = 0;
			return __str;
		}

#ifdef _NANOC_WINDOWS_

		int _len;
		//获取ANSI编码
		_len = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)str, len, NULL, 0, "△", NULL);
		if (_len > MAX_BUFFERSIZE) {
			_len = MAX_BUFFERSIZE;
		}
		//memset(_str, 0, _len + 1);
		WideCharToMultiByte(CP_ACP, 0, (wchar_t*)str, len, _str, _len, "△", NULL);
		int cc = 0;
		if (_len > 0) {
			//计算中ascii字符个数
			for (int i = 0; i < len && i < _len; i++) {
				if (_str[i] & 0x80) {
					i++;
					continue;
				}
				cc++;
			}
			//设置实际大小
			_str[len - cc] = 0;
		} else {
			_str[0] = 0;
			printf("Trans failed(ANSI): %d\n", _len);
		}

		//获取UTF-8编码
		_len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)str, len, NULL, 0, NULL, NULL);
		if (_len > MAX_BUFFERSIZE) {
			_len = MAX_BUFFERSIZE;
		}
		//memset(__str, 0, _len + 1);
		WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)str, len, __str, _len, NULL, NULL);
		if (_len > 0) {
			//设置实际大小
			__str[len - cc + (len - 2 * cc) / 2] = 0;
		}
		else {
			__str[0] = 0;
			printf("Trans failed(UTF-8): %d\n", _len);
		}

		return _str;
#else 
		//Linux下wchar占4个字节，需要对2字节字符串进行扩充
		for (int i = len * 2; i >= 0; i--){
			if (i >= MAX_BUFFERSIZE) {
				continue;
			}
			this->_str[i] = 0;
			this->__str[i] = 0;
		}
		for (unsigned int i = 0; i < len; i++) {
			unsigned int ind = 0;
			if (i % 2 == 0) {
				ind = i * 2;
			}
			else {
				ind = i * 2 - 1;
			}
			if (ind >= MAX_BUFFERSIZE) {
				continue;
			}
			this->_str[ind] = str[i];
		}

		setlocale(LC_ALL, "");
		int _len = wcstombs(__str, (wchar_t*)_str, MAX_BUFFERSIZE);
		if (_len > 0) {
			__str[_len] = 0;
			for (int i = 0; __str[i]; i++) {
				_str[i] = __str[i];
			}
			_str[_len] = 0;
		}
		else {
			__str[0] = 0;
			_str[0] = 0;
			printf("Trans failed: %d\n", _len);
		}
		return __str;
#endif
	}
	const char * getStr(unsigned int * len = NULL) {
		unsigned int _len;
		if (len == NULL) {
			len = &_len;
		}
		if (this->pos > this->len) {
			return NULL;
		}
		*len = getInt();
		if (*len > MAX_BUFFERSIZE) {
			*len = MAX_BUFFERSIZE;
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
			for (len = 0; str[len] != 0 && len < MAX_BUFFERSIZE; len++);
			unsigned int _len = *(unsigned int*)(str);
			if (_len <= MAX_BUFFERSIZE && _len > len) {
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

	char _str[MAX_BUFFERSIZE];
	char __str[MAX_BUFFERSIZE];
	char str[MAX_BUFFERSIZE];

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
