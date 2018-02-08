/**
* @file SHA1.h
* @brief SHA-1 coden take from gnupg 1.3.92.
* @author Don
* @date 2011-7-28 22:49:55
* @version
* <pre><b>copyright: </b></pre>
* <pre><b>email: </b>hao.limin@gmail.com</pre>
* <pre><b>company:
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef _SHA1_H  
#define _SHA1_H  

#include <stdio.h>  
#include <memory.h>  

#ifdef __cplusplus  
extern"C"
{
#endif /**< __cplusplus */  

#define SHA1_SIZE_BYTE 20  

	/**
	* @brief SHA1_String
	*
	* Detailed description.
	* @param[in] inputString Ҫ���д�����޷����ַ���ָ��
	* @param[in] len Ҫ�������Ϣ���ȣ�n<= strlen(p);
	* @param[out] pOutSHA1Buf ���ժҪ��Ϣ������Ϊ20�� unsigned char ����160 bits
	* @return int
	*/
	int SHA1_String(const unsigned char* inputString, unsigned long len, unsigned char* pOutSHA1Buf);

	/**
	* @brief SHA1_String_Compare
	*
	* Detailed description.
	* @param[in] inputString
	* @param[in] len
	* @param[in] pOutSHA1Buf
	* @return int
	*/
	int SHA1_String_Compare(const unsigned char* inputString, unsigned long len, const unsigned char* pOutSHA1Buf);

	/**
	* @brief SHA1_File
	*
	* Detailed description.
	* @param[in] filePath Ҫ����MD5���ļ���·��
	* @param[in] buff �����MD5ֵ���û�������СӦΪMD5_SIZE_BYTE
	* @return int
	*/
	int SHA1_File(const char* filePath, unsigned char* buff);

	/**
	* @brief SHA1_File_Compare
	*
	* Detailed description.
	* @param[in] filePathA Ҫ�Ա�MD5�ĵ�һ���ļ���·��
	* @param[in] filePathB Ҫ�Ա�MD5�ĵڶ����ļ���·��
	* @return int
	*/
	int SHA1_File_Compare(const char* filePathA, const char* filePathB);

#ifdef __cplusplus  
}

#endif /**< __cplusplus */  


#include <string>
using namespace std;

static bool is_base64(unsigned char c);
std::string 	base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

#endif /**< _SHA1_H */  

