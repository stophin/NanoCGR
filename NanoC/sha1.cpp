/**
* @file SHA1.c
* @brief SHA-1 coden take from gnupg 1.3.92.
* @author Don
* @date 2011-7-28 22:49:55
* @version
* <pre><b>copyright: </b></pre>
* <pre><b>email: </b>hao.limin@gmail.com</pre>
* <pre><b>company:
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>*/




#include "SHA1.h"


typedef struct SHAstate_st
{
	unsigned long h[SHA1_SIZE_BYTE / 4]; /* ���ժҪ���(32*5=160 bits)*/
	unsigned long Nl;
	unsigned long Nh;       /*�����Ϣ��λ����Nh����32λ��Nl����32λ*/
	unsigned long data[16]; /*���ݴӵ�0���ĸ�8λ��ʼ���η���*/
	int FlagInWord;     /*��ʶһ��dataԪ����ռ�õ��ֽ������Ӹ�->�ͣ���ȡֵ0,1,2,3*/
	int msgIndex;       /*��ǰ���������data����Ԫ������*/
	int isTooMang;      /*����Ϊ0�����������Ϣ����2^64 bitsʱΪ1��*/
} SHA1_Context;

#define INIT_DATA_h0 0x67452301U
#define INIT_DATA_h1 0xEFCDAB89U
#define INIT_DATA_h2 0x98BADCFEU
#define INIT_DATA_h3 0x10325476U
#define INIT_DATA_h4 0xC3D2E1F0U

#define SHA1CircularShift(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))

const unsigned long SHA1_Kt[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
typedef unsigned long(*SHA1_pFun)(unsigned long, unsigned long, unsigned long);

/*�����ĸ�����*/
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

SHA1_pFun ft[] = { SHA1_ft0_19, SHA1_ft20_39, SHA1_ft40_59, SHA1_ft60_79 };

static int SHA1_Init(SHA1_Context *c)
{
	if (NULL == c)
	{
		return -1;
	}

	c->h[0] = INIT_DATA_h0;

	c->h[1] = INIT_DATA_h1;
	c->h[2] = INIT_DATA_h2;
	c->h[3] = INIT_DATA_h3;
	c->h[4] = INIT_DATA_h4;
	c->Nl = 0;
	c->Nh = 0;
	c->FlagInWord = 0;
	c->msgIndex = 0;
	c->isTooMang = 0;
	memset(c->data, 0, 64);

	return 1;
}

int SHA1_GetMsgBits(SHA1_Context *c)
{
	int a = 0;

	if ((NULL == c) || (0 != c->isTooMang))
	{
		return -1;
	}

	a = sizeof(unsigned long)* 8 * c->msgIndex + 8 * c->FlagInWord;

	return a;
}

int SHA1_Clear_data(SHA1_Context *c)
{
	if (NULL == c)
	{
		return -1;
	}

	memset(c->data, 0, 64);

	c->msgIndex = 0;

	c->FlagInWord = 0;

	return 1;
}

int SHA1_One(SHA1_Context *c)
{
	unsigned long AE[5];
	unsigned long w[80];
	unsigned long temp = 0;
	int t = 0;

	if ((NULL == c) || (0 != c->isTooMang))
	{
		return -1;
	}

	for (t = 0; t < 16; ++t)
	{
		w[t] = c->data[t];
	}

	for (t = 16; t < 80; ++t)
	{
		w[t] = SHA1CircularShift(1, w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]);
	}

	for (t = 0; t < 5; ++t)
	{
		AE[t] = c->h[t];
	}

	for (t = 0; t <= 79; ++t)
	{
		temp = SHA1CircularShift(5, AE[0]) + (*ft[t / 20])(AE[1], AE[2], AE[3]) + AE[4] + w[t] + SHA1_Kt[t / 20];
		AE[4] = AE[3];
		AE[3] = AE[2];
		AE[2] = SHA1CircularShift(30, AE[1]);
		AE[1] = AE[0];
		AE[0] = temp;
	}

	for (t = 0; t < 5; ++t)
	{
		c->h[t] += AE[t];
	}

	SHA1_Clear_data(c);

	return 1;
}

int SHA1_PadMessage(SHA1_Context *c)
{
	int msgBits = -1;

	if ((NULL == c) || (0 != c->isTooMang))
	{
		return -1;
	}

	msgBits = SHA1_GetMsgBits(c);

	if (440 < msgBits)
	{
		c->data[c->msgIndex++] |= (1 << (8 * (4 - c->FlagInWord) - 1));
		c->FlagInWord = 0;

		while (c->msgIndex < 16)
		{
			c->data[c->msgIndex++] = 0;
		}

		SHA1_One(c);

		while (c->msgIndex < 14)
		{
			c->data[c->msgIndex++] = 0;
		}

	}
	else
	{
		c->data[c->msgIndex++] |= (1 << (8 * (4 - c->FlagInWord) - 1));
		c->FlagInWord = 0;

		while (c->msgIndex < 14)
		{
			c->data[c->msgIndex++] = 0;
		}
	}

	while (c->msgIndex < 14)
	{
		c->data[c->msgIndex++] = 0;
	}

	c->data[c->msgIndex++] = c->Nh;

	c->data[c->msgIndex++] = c->Nl;

	return 1;
}

static int SHA1_Update(SHA1_Context *c, const unsigned char *chBuff, unsigned int n)
{
	unsigned int lastBytes = 0;
	unsigned int temp = 0;
	unsigned int i = 0;
	unsigned int tempBits = 0;

	if (!c || !chBuff || !n || c->isTooMang)
	{
		return -1;
	}

	if (n > strlen((char *)chBuff))
	{
		n = strlen((char *)chBuff);
	}

	if (c->FlagInWord > 0)
	{
		temp = (unsigned int)(4 - c->FlagInWord) < n ? (unsigned int)(4 - c->FlagInWord) : n;

		for (i = temp; i > 0; --i)
		{
			c->data[c->msgIndex] |= ((unsigned long)chBuff[temp - i]) << (3 - c->FlagInWord++) * 8;
		}

		tempBits = c->Nl;

		c->Nl += 8 * temp;

		if (tempBits > c->Nl)
		{
			++(c->Nh);

			if (c->Nh == 0)
			{
				c->isTooMang = 1;
			}
		}

		if ((c->FlagInWord) / 4 > 0)
		{
			++(c->msgIndex);
		}

		c->FlagInWord = c->FlagInWord % 4;

		if (c->msgIndex == 16)
		{
			SHA1_One(c);
		}
	}

	chBuff += temp;

	n -= temp;

	if (n >= 4)
	{
		for (i = 0; i <= n - 4; i += 4)
		{
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i]) << 24;
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i + 1]) << 16;
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i + 2]) << 8;
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i + 3]);
			++(c->msgIndex);

			tempBits = c->Nl;
			c->Nl += 32;

			if (tempBits > c->Nl)
			{
				c->Nh++;

				if (c->Nh == 0)
				{
					c->isTooMang = 1;
				}
			}

			if (c->msgIndex == 16)
			{
				SHA1_One(c);
			}
		}
	}

	if (n > 0 && n % 4 != 0)
	{
		lastBytes = n - i;

		switch (lastBytes)
		{

		case 3:
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i + 2]) << 8;

		case 2:
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i + 1]) << 16;

		case 1:
			c->data[c->msgIndex] |= ((unsigned long)chBuff[i]) << 24;
		}

		c->FlagInWord = lastBytes;

		tempBits = c->Nl;
		c->Nl += 8 * lastBytes;

		if (tempBits > c->Nl)
		{
			++(c->Nh);

			if (0 == c->Nh)
			{
				c->isTooMang = 1;
			}
		}

		if (16 == c->msgIndex)
		{
			SHA1_One(c);
		}
	}

	return 1;
}

static int SHA1_Final(SHA1_Context *c, unsigned char * md)
{
	int i = 0;

	if ((NULL == md) || (NULL == c) || (c->isTooMang))
	{
		return -1;
	}

	SHA1_PadMessage(c);

	SHA1_One(c);

	for (i = 0; i < 5; ++i)
	{
		md[4 * i] = (unsigned char)((c->h[i] & 0xff000000) >> 24);
		md[4 * i + 1] = (unsigned char)((c->h[i] & 0x00ff0000) >> 16);
		md[4 * i + 2] = (unsigned char)((c->h[i] & 0x0000ff00) >> 8);
		md[4 * i + 3] = (unsigned char)(c->h[i] & 0x000000ff);
	}

	return 1;
}

int SHA1_String(const unsigned char* inputString, unsigned long len, unsigned char* pOutSHA1Buf)
{
	int rt = -1;
	SHA1_Context c;

	if ((NULL == inputString) || (NULL == pOutSHA1Buf))
	{
		return -1;
	}

	rt = SHA1_Init(&c);

	if (-1 == rt)
	{
		return -1;
	}

	SHA1_Update(&c, inputString, len);

	SHA1_Final(&c, pOutSHA1Buf);
	SHA1_Clear_data(&c);
	return 1;
}


int SHA1_String_Compare(const unsigned char* inputString, unsigned long len, const unsigned char* pOutSHA1Buf)
{
	unsigned char buff[SHA1_SIZE_BYTE] = { 0 };
	int rt = -1;
	int i = 0;

	rt = SHA1_String(inputString, len, buff);

	if (1 == rt)
	{
		for (i = 0; i < SHA1_SIZE_BYTE; ++i)
		{
			if (buff[i] != pOutSHA1Buf[i])
			{
				return -1;
			}
		}

		return 1;
	}
	else
	{
		return -1;
	}
}

int SHA1_File(const char* filePath, unsigned char *buff)
{
	int rt = -1;
	FILE *file = NULL;
	SHA1_Context context;
	int len = 0;
	unsigned char buffer[0x0400] = { 0 };

	file = fopen(filePath, "rb");

	if (NULL == file)
	{
		return -1;
	}
	else
	{
		rt = SHA1_Init(&context);

		if (-1 == rt)
		{
			fclose(file);

			return -1;
		}

		while (len = fread(buffer, 1, 1024, file))
		{
			rt = SHA1_Update(&context, buffer, len);

			if (-1 == rt)
			{
				fclose(file);

				return -1;
			}
		}

		rt = SHA1_Final(&context, buff);

		fclose(file);

		return rt;
	}
}

int SHA1_File_Compare(const char* filePathA, const char *filePathB)
{
	unsigned char hashValueA[SHA1_SIZE_BYTE] = { 0 };
	unsigned char hashValueB[SHA1_SIZE_BYTE] = { 1 };
	int rt = -1;
	int i = 0;

	if ((NULL == filePathA) || (NULL == filePathB))
	{
		return -1;
	}

	rt = SHA1_File(filePathA, hashValueA);

	if (1 == rt)
	{
		rt = SHA1_File(filePathB, hashValueB);

		if (1 == rt)
		{
			for (i = 0; i < SHA1_SIZE_BYTE; ++i)
			{
				if (hashValueA[i] != hashValueB[i])
				{
					return -1;
				}
			}

			return 1;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}



const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}
	return ret;
}

/*
void main()
{
char p[20];
SHA1_String("123", 3, p);

return;
}
*/