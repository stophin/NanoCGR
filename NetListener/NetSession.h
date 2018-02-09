// NetSession.h
//

#pragma once

#include "Platform.h"
class INetSession {
public:
	INetSession() : bIfUse(false) {
	}
	INT iSessionID;
	BOOL bIfUse;
	INT connectionType;
};

#ifdef _NANOC_WINDOWS_
/**
* �ṹ�����ƣ�PER_IO_DATA
* �ṹ�幦�ܣ��ص�I/O��Ҫ�õ��Ľṹ�壬��ʱ��¼IO����
**/
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[MAX_BUFFERSIZE];
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

	union {
		PER_HANDLE_DATA handleData;
		SOCKET socket;
	};
	PER_IO_OPERATEION_DATA operationData;
};

#else

class NetSession : public INetSession {
public:
	NetSession();
	~NetSession();

	SOCKET socket;
};

#endif

#define MAX_SESSIONSIZE 10

class NetSessionManager {
public:
	NetSessionManager();
	~NetSessionManager();

	NetSession * GetFreeSession(SOCKET socket = 0);

	INT32 getSize();
private:
	INT32 n32Size;
	NetSession * netSession;
};