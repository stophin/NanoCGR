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
#include <string>
using namespace std;

#ifdef __cplusplus
extern"C"
{
#endif /**< __cplusplus */

#define SHA1_SIZE_BYTE 20

	/**
	* @brief SHA1_String
	*
	* Detailed description.
	* @param[in] inputString 要进行处理的无符号字符串指针
	* @param[in] len 要处理的信息长度，n<= strlen(p);
	* @param[out] pOutSHA1Buf 输出摘要信息，长度为20的 unsigned char ，共160 bits
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
	* @param[in] filePath 要计算MD5的文件的路径
	* @param[in] buff 输出的MD5值，该缓冲区大小应为MD5_SIZE_BYTE
	* @return int
	*/
	int SHA1_File(const char* filePath, unsigned char* buff);

	/**
	* @brief SHA1_File_Compare
	*
	* Detailed description.
	* @param[in] filePathA 要对比MD5的第一个文件的路径
	* @param[in] filePathB 要对比MD5的第二个文件的路径
	* @return int
	*/
	int SHA1_File_Compare(const char* filePathA, const char* filePathB);

#ifdef __cplusplus

	bool is_base64(unsigned char c);

	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
}

#endif /**< __cplusplus */

#endif /**< _SHA1_H */
