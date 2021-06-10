//CharString.h
//
#ifndef _CharString_H_
#define _CharString_H_

#include "../NanoC/NanoType.h"

#define MAX_BUFFERSIZE 32768

/////////////////////////////////////////////////////
//AES
#define BLOCK_SIZE 16

class AES
{
public:
	enum
	{
		ECB = 0, CBC = 1, CFB = 2
	};

private:
	enum
	{
		DEFAULT_BLOCK_SIZE = 16
	};
	enum
	{
		MAX_BLOCK_SIZE = 32, MAX_ROUNDS = 14, MAX_KC = 8, MAX_BC = 8
	};
public:
	AES();
	virtual ~AES();
private:
	//Key Initialization Flag
	bool m_bKeyInit;
	//Encryption (m_Ke) round key
	int m_Ke[MAX_ROUNDS + 1][MAX_BC];
	//Decryption (m_Kd) round key
	int m_Kd[MAX_ROUNDS + 1][MAX_BC];
	//Key Length
	int m_keylength;
	//Block Size
	int m_blockSize;
	//Number of Rounds
	int m_iROUNDS;
	//Chain Block
	char m_chain0[MAX_BLOCK_SIZE];
	char m_chain[MAX_BLOCK_SIZE];
	//Auxiliary private use buffers
	int tk[MAX_KC];
	int a[MAX_BC];
	int t[MAX_BC];
private:
	void Xor(char* buff, char const* chain);
	void DefEncryptBlock(char const* in, char* result);
	void DefDecryptBlock(char const* in, char* result);
	void EncryptBlock(char const* in, char* result);
	void DecryptBlock(char const* in, char* result);
public:
	void MakeKey(char const* key, char const* chain, int keylength =
		DEFAULT_BLOCK_SIZE, int blockSize = DEFAULT_BLOCK_SIZE);
	void Encrypt(char const* in, char* result, size_t n, int iMode = ECB);
	void Decrypt(char const* in, char* result, size_t n, int iMode = ECB);
};
/////////////////////////////////////////////////////


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

	/////////////////////////////////////////////////////
	//WebSocket frame
#define WS_EMPTY_FRAME  0xF0
#define WS_ERROR_FRAME  0xF1
#define WS_TEXT_FRAME  0x01
#define WS_BINARY_FRAME 0x02
#define WS_PING_FRAME 0x09
#define WS_PONG_FRAME  0x0A
#define WS_OPENING_FRAME  0xF3
#define WS_CLOSING_FRAME  0x08
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
		//检查扩展位并忽略
		if ((inFrame[0] & 0x70) != 0x0) {
			return 0;
		}
		//fin位：为1表示已接收完整报文，为0表示继续监听后续报文
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
				payLoadExtraBytes = 2;
				unsigned short * payloadLength16b = 0;
				payloadLength16b = (unsigned short*)(&inFrame[payLoadExtraBytes]);
				payloadLength = ntohs(*payloadLength16b);;
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
	static int encodeFrame(char * out, int frameType, const char * msg) {
		if (NULL == out) {
			return 0;
		}
		out[0] = 0;
		if (NULL == msg) {
			return 0;
		}
		unsigned int messageLength;
		for (messageLength = 0; msg[messageLength]; messageLength++);
		if (messageLength > MAX_BUFFERSIZE) {
			//数据过长暂不支持
			printf("Frame message too long\n");
			return 0;
		}
		unsigned char payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
		//header: 2字节,mask位设置为0（不加密），则后面的masking key无需填写，省略4字节
		unsigned char frameHeaderSize = 2 + payloadFieldExtraBytes;
		unsigned char frameHeader[4];//使用frameHeaderSize的长度
		for (int i = 0; i < 4; i++) {
			frameHeader[i] = 0;
		}
		//fin位为1，扩展位为0，操作位为frameType
		frameHeader[0] = (unsigned char)(0x80 | frameType);
		//填充数据长度
		if (messageLength <= 0x7d) {
			frameHeader[1] = (unsigned char)(messageLength);
		}
		else {
			frameHeader[1] = 0x7e;
			unsigned short *len = (unsigned short*)(&frameHeader[2]);
			*len = htons(messageLength);
		}
		//填充数据 
		unsigned int frameSize = frameHeaderSize + messageLength;
		char * frame = out;
		for (int i = 0; i < frameHeaderSize; i++) {
			frame[i] = frameHeader[i];
		}
		frame[frameHeaderSize] = 0;
		for (int i = 0; i < messageLength; i++) {
			frame[frameHeaderSize + i] = msg[i];
		}
		frame[frameSize] = 0;

		return frameSize;
	}
	/////////////////////////////////////////////////////
	//WebSocket
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


	/////////////////////////////////////////////////////
	//HTTP
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
	/////////////////////////////////////////////////////



	/////////////////////////////////////////////////////
	//SHA1
#define SHA1_SIZE_BYTE 20
	typedef struct SHAstate_st
	{
		unsigned long h[SHA1_SIZE_BYTE / 4]; /* 存放摘要结果(32*5=160 bits)*/
		unsigned long Nl;
		unsigned long Nh;       /*存放信息总位数，Nh：高32位，Nl：低32位*/
		unsigned long data[16]; /*数据从第0个的高8位开始依次放置*/
		int FlagInWord;     /*标识一个data元素中占用的字节数（从高->低），取值0,1,2,3*/
		int msgIndex;       /*当前已填充满的data数组元素数。*/
		int isTooMang;      /*正常为0，当处理的信息超过2^64 bits时为1；*/
	} SHA1_Context;

#define INIT_DATA_h0 0x67452301U
#define INIT_DATA_h1 0xEFCDAB89U
#define INIT_DATA_h2 0x98BADCFEU
#define INIT_DATA_h3 0x10325476U
#define INIT_DATA_h4 0xC3D2E1F0U

#define SHA1CircularShift(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))

	typedef unsigned long(*SHA1_pFun)(unsigned long, unsigned long, unsigned long);

	/*定义四个函数*/
	static unsigned long SHA1_ft0_19(unsigned long b, unsigned long c, unsigned long d)
	{
		return (b&c) | ((~b)&d);
	}

	static unsigned long SHA1_ft20_39(unsigned long b, unsigned long c, unsigned long d)
	{
		return b ^ c ^ d;
	}

	static unsigned long SHA1_ft40_59(unsigned long b, unsigned long c, unsigned long d)
	{
		return (b&c) | (b&d) | (c&d);
	}

	static unsigned long SHA1_ft60_79(unsigned long b, unsigned long c, unsigned long d)
	{
		return b ^ c ^ d;
	}

	static int SHA1_Init(SHA1_Context *c);
	static int SHA1_GetMsgBits(SHA1_Context *c);
	static int SHA1_Clear_data(SHA1_Context *c);
	static int SHA1_One(SHA1_Context *c);
	static int SHA1_PadMessage(SHA1_Context *c);
	static int SHA1_Update(SHA1_Context *c, const unsigned char *chBuff, unsigned int n);
	static int SHA1_Final(SHA1_Context *c, unsigned char * md);
	/**
	* @brief SHA1_String
	*
	* Detailed description.
	* @param[in] inputString 要进行处理的无符号字符串指针
	* @param[in] len 要处理的信息长度，n<= strlen(p);
	* @param[out] pOutSHA1Buf 输出摘要信息，长度为20的 unsigned char ，共160 bits
	* @return int
	*/
	static int SHA1_String(const unsigned char* inputString, unsigned long len, unsigned char* pOutSHA1Buf);

	/////////////////////////////////////////////////////
	static bool is_base64(unsigned char c);
	static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	static std::string base64_decode(std::string const& encoded_string);
	static int base64_decode(const char*  encoded_string, int in_len, char * res);
	static int base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len, char * res);
	///////////////////////////////////////////////////////
	template <typename T>
	static int EncodeProtocolString(char * str, T val) {
		int size = sizeof(val);
		*((T*)str) = val;
		return size;
	}
	template <typename T>
	static int DecodeProtocolString(char *str, T& val) {
		int size = sizeof(val);
		val = *((T*)str);
		return size;
	}

	//可变参数宏的递归扩展
#define FL_EXPAND(x) x

#define FL_INTERNAL_ARG_COUNT_PRIVATE(\
	 _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, \
	_10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
	_20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
	_30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
	_40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
	_50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
	_60, _61, _62, _63, _64, N, ...) N

#define FL_ARG_COUNT(...) FL_EXPAND(FL_INTERNAL_ARG_COUNT_PRIVATE(0, ##__VA_ARGS__,\
	64, 63, 62, 61, 60, \
	59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
	29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
	 9,  8,  7,  6,  5,  4,  3,  2,  1,  0))

#define FL_CONCAT_(l, r) l ## r

#define FL_CONCAT(l, r) FL_CONCAT_(l, r)

#define FL_DOARG0(s,f,a,o)
#define FL_DOARG1(s,f,a,v1,v2,v3,v,...) f(a,v,v1,v2,v3)
#define FL_DOARG2(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG1(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG3(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG2(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG4(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG3(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG5(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG4(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG6(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG5(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG7(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG6(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG8(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG7(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG9(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG8(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG10(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG9(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG11(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG10(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG12(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG11(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG13(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG12(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG14(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG13(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG15(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG14(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG16(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG15(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG17(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG16(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG18(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG17(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG19(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG18(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG20(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG19(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG21(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG20(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG22(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG21(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG23(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG22(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG24(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG23(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG25(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG24(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG26(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG25(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG27(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG26(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG28(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG27(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG29(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG28(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG30(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG29(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG31(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG30(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG32(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG31(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG33(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG32(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG34(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG33(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG35(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG34(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG36(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG35(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG37(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG36(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG38(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG37(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG39(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG38(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG40(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG39(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG41(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG40(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG42(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG41(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG43(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG42(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG44(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG43(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG45(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG44(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG46(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG45(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG47(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG46(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG48(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG47(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG49(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG48(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG50(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG49(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG51(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG50(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG52(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG51(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG53(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG52(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG54(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG53(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG55(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG54(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG56(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG55(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG57(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG56(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG58(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG57(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG59(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG58(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG60(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG59(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG61(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG60(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG62(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG61(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG63(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG62(s,f,a,v1,v2,v3,__VA_ARGS__))
#define FL_DOARG64(s,f,a,v1,v2,v3,v,...) FL_DOARG1(s,f,a,v1,v2,v3,v) s FL_EXPAND(FL_DOARG63(s,f,a,v1,v2,v3,__VA_ARGS__))

#define FL_FOREACH_(sepatator,fun,funarg,funarg1,funarg2,funarg3,...) \
		FL_EXPAND(FL_CONCAT(FL_DOARG,FL_ARG_COUNT(__VA_ARGS__))(sepatator,fun,funarg,funarg1,funarg2,funarg3,__VA_ARGS__))
#define FL_FOREACH(sepatator,fun,funarg,...) \
		FL_EXPAND(FL_FOREACH_(sepatator,fun,funarg,,,,__VA_ARGS__))

#define enum_elem(p,n,...) p##n,

#define FL_DEF_ENUM(clsName,...)\
enum _##clsName{\
	FL_EXPAND(FL_FOREACH(,enum_elem,clsName##_,__VA_ARGS__))\
	clsName##_LAST_ID \
}clsName;

#define call_elem(p, n,...) p(n),

#define encode_protocol(p, n, str, offset, size) ;offset += p::EncodeProtocolString(str + offset, n);if (offset >= size) offset = size;
#define EncodeProtocol(st, str, offset, size, ...) \
	FL_EXPAND(FL_FOREACH_(, encode_protocol, st, str, offset, size, __VA_ARGS__))

#define decode_protocol(p, n, str, offset, size) ;offset += p::DecodeProtocolString(str + offset, n);if (offset >= size) offset = size;
#define DecodeProtocol(st, str, offset, size, ...) \
	FL_EXPAND(FL_FOREACH_(, decode_protocol, st, str, offset, size, __VA_ARGS__))
	/////////////////////////////////////////////////////

	///////////////////////////////////////////////////////
	//AES
	static string DecryptionAES(const string& strSrc);

	/////////////////////////////////////
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
		if (!src[i] && dest[i]) {
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

		unsigned int _len;
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
			for (unsigned int i = 0; i < len && i < _len; i++) {
				if (_str[i] & 0x80) {
					i++;
					continue;
				}
				cc++;
			}
			//设置实际大小
			unsigned int __len = len - cc;
			if (__len > 0 && __len < MAX_BUFFERSIZE) {
				_str[__len] = 0;
			}
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
			unsigned int __len = len - cc + (len - 2 * cc) / 2;
			if (__len > 0 && __len < MAX_BUFFERSIZE) {
				__str[__len] = 0;
			}
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

	void set(const char * str, int size = 0) {
		if (size > 0) {
			len = size;
			if (len < 0) {
				len = 0;
			}else if (len > MAX_BUFFERSIZE) {
				len = MAX_BUFFERSIZE;
			}
			memcpy(this->str, str, len);
			this->str[len] = 0;
			this->pos = 0;
			return;
		}
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
			for (unsigned int i = 0; i < len; i++) {
				this->str[i] = str[i];
			}
			this->str[len] = 0;
		}
		this->Reflush();
	}

	INetSession * session;

	char _str[MAX_BUFFERSIZE + 1];
	char __str[MAX_BUFFERSIZE + 1];
	char str[MAX_BUFFERSIZE + 1];

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
