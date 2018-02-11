//CharString.cpp
//


#include "CharString.h"

CharStringPool charStringPool;

extern "C" __NANOC_EXPORT__ ICharStringPool  * GetPool() {
	return &charStringPool;
}

const unsigned long SHA1_Kt[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };

CharString::SHA1_pFun ft[] = { CharString::SHA1_ft0_19, CharString::SHA1_ft20_39, CharString::SHA1_ft40_59, CharString::SHA1_ft60_79 };

int CharString::SHA1_Init(SHA1_Context *c)
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

int CharString::SHA1_GetMsgBits(SHA1_Context *c)
{
	int a = 0;

	if ((NULL == c) || (0 != c->isTooMang))
	{
		return -1;
	}

	a = sizeof(unsigned long)* 8 * c->msgIndex + 8 * c->FlagInWord;

	return a;
}

int CharString::SHA1_Clear_data(SHA1_Context *c)
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

int CharString::SHA1_One(SHA1_Context *c)
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

int CharString::SHA1_PadMessage(SHA1_Context *c)
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

int CharString::SHA1_Update(SHA1_Context *c, const unsigned char *chBuff, unsigned int n)
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

int CharString::SHA1_Final(SHA1_Context *c, unsigned char * md)
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

int CharString::SHA1_String(const unsigned char* inputString, unsigned long len, unsigned char* pOutSHA1Buf)
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


const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

bool CharString::is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string CharString::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
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

std::string CharString::base64_decode(std::string const& encoded_string) {
	int in_len = (int)encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

#include "AES.h"
const char g_key[17] = "0123456789ABCDEF";
const char g_iv[17] = "0123456789ABCDEF";
string CharString::DecryptionAES(const string& strSrc) {
	string strData = base64_decode(strSrc);
	size_t length = strData.length();
	if (length <= 0) {
		return "";
	}
	//����
	char *szDataIn = new char[length + 1];
	memcpy(szDataIn, strData.c_str(), length + 1);
	//����
	char *szDataOut = new char[length + 1];
	memcpy(szDataOut, strData.c_str(), length + 1);

	//����AES��CBCģʽ����
	AES aes;
	aes.MakeKey(g_key, g_iv, 16, 16);
	aes.Decrypt(szDataIn, szDataOut, length, AES::CBC);

	//ȥPKCS7Padding���
	if (0x00 < szDataOut[length - 1] <= 0x16)
	{
		int tmp = szDataOut[length - 1];
		for (int i = length - 1; i >= length - tmp; i--)
		{
			if (szDataOut[i] != tmp)
			{
				memset(szDataOut, 0, length);
				printf("Fill block failed\n");
				break;
			}
			else
				szDataOut[i] = 0;
		}
	}
	string strDest(szDataOut);
	delete[] szDataIn;
	delete[] szDataOut;
	return strDest;
}