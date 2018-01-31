// NetSession.h
//

#pragma once

#include "Platform.h"

#ifdef _NANOC_WINDOWS_

class INetSession {
public:
	INetSession() : bIfUse(false) {
	}
	INT iSessionID;
	BOOL bIfUse;
};

/**
* �ṹ�����ƣ�PER_IO_DATA
* �ṹ�幦�ܣ��ص�I/O��Ҫ�õ��Ľṹ�壬��ʱ��¼IO����
**/
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[1025];
	int BufferLen;
	int operationType;
	INetSession * netSession;
	SOCKET client;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;
/**
* �ṹ�����ƣ�PER_HANDLE_DATA
* �ṹ��洢����¼�����׽��ֵ����ݣ��������׽��ֵı������׽��ֵĶ�Ӧ�Ŀͻ��˵ĵ�ַ��
* �ṹ�����ã��������������Ͽͻ���ʱ����Ϣ�洢���ýṹ���У�֪���ͻ��˵ĵ�ַ�Ա��ڻطá�
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

class NetSession : public INetSession {
public:
	NetSession();
	~NetSession();

	PER_HANDLE_DATA handleData;;
	PER_IO_OPERATEION_DATA operationData;
};

class NetSessionManager {
public:
	NetSessionManager();
	~NetSessionManager();

	NetSession * GetFreeSession();

	INT32 getSize();
private:
	INT32 n32Size;
	NetSession * netSession;
};

#else

#define MAXEPOLL 1000
#define MAXLINE 1024

#endif