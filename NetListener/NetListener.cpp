// NetListener.cpp : 定义 DLL 应用程序的导出函数。
//

#include "NetListener.h"

NetListener::NetListener() {
	bIfInitialized = false;
}

NetListener::~NetListener() {
	this->CleanUp();
}

#ifdef _NANOC_WINDOWS_
void NetListener::CleanUp() {

	if (NULL != m_phIOThread) {
		//exit iocp thread
		BOOL bPosted = ::PostQueuedCompletionStatus(hCompletionPort, 0, (SIZE_INT)0, NULL);
		if (WAIT_TIMEOUT == __NANOC_THREAD_WAIT__(m_phIOThread)) {
			__NANOC_THREAD_END__(m_phIOThread);
		}
		m_phIOThread = NULL;
	}

	if (NULL != hCompletionPort) {
		CloseHandle(hCompletionPort);
		hCompletionPort = NULL;
	}

	if (INVALID_SOCKET != hListenSocket) {
		closesocket(hListenSocket);
		hListenSocket = INVALID_SOCKET;
	}

	bIfInitialized = false;
}

void NetListener::Init() {
	UINT32 un32RetFlag = 0;
	UINT32 un32StateFlag = 0;
	un32StateFlag--;
	if (0 == un32RetFlag) {
		// 初始化Socket库  
		WSADATA wsaData;
		int iRc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != iRc) {
			printf("Socket initialize error: return code %d\n", iRc);

			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		//创建IOCP内核对象
		this->hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (NULL == hCompletionPort) {
			printf("Error create IOCP\n");
			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		//bind listen socket with io completion port
		//HANDLE hResult = ::CreateIoCompletionPort((HANDLE)hListenSocket, hCompletionPort, (SIZE_INT)-2, 0);
		//start io thread
		__NANOC_THREAD_BEGIN__(m_phIOThread, NetListener::IOCPThread, this);
		if (NULL == m_phIOThread) {
			printf("IOCP thread start error\n");
			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		//创建流式套接字，绑定到本机
		//this->hListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		this->hListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == hListenSocket) {
			printf("Error create socket: %d\n", WSAGetLastError());
			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		// Associate SOCKET with IOCP  
		if (NULL == CreateIoCompletionPort((HANDLE)this->hListenSocket, this->hCompletionPort, NULL, 0))
		{
			printf("CreateIoCompletionPort failed with error code: %d\n ", WSAGetLastError());
			if (INVALID_SOCKET != this->hListenSocket)
			{
				closesocket(this->hListenSocket);
				this->hListenSocket = INVALID_SOCKET;
			}
			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		//绑定到本机
		sockaddr_in stAddr = { 0 };
		stAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		stAddr.sin_family = AF_INET;
		stAddr.sin_port = htons(9005);

		int iRc = bind(hListenSocket, (sockaddr*)&stAddr, sizeof(stAddr));
		if (iRc < 0) {
			printf("Socket bind error: %d\n", errno);
			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		//将套接字设置为监听模式
		//start listen, 128 backup connection times
		int iRc = listen(hListenSocket, 128);
		if (iRc < 0) {
			printf("Socket listen error: %d\n", errno);
			un32RetFlag = un32StateFlag;
		}
	}

	un32StateFlag--;
	if (0 == un32RetFlag) {
		for (int i = this->netSession.getSize(); i > 0; i--) {
			NetSession * session = this->netSession.GetFreeSession();
			if (NULL == session) {
				break;
			}
			if (MakeFreeIOCompletionPort(session)) {
				break;
			}
		}
	}

	un32StateFlag--;
	if (0 != un32RetFlag){
		printf("Error: %d\n", un32RetFlag);
		this->CleanUp();
	}
	else {
		printf("NetListener initialized successfully!\n");
		this->bIfInitialized = true;
	}
}

UINT32 NetListener::MakeFreeIOCompletionPort(NetSession* session) {
	UINT32 un32RetFlag = 0;
	if (NULL == session) {
		return un32RetFlag;
	}

	//使用AcceptEx接收客户端连接而不是开辟线程等待连接
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;//AcceptEx函数指针
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == acceptSocket)
	{
		printf("WSASocket failed with error code: %d\n", WSAGetLastError());
		un32RetFlag = -1;
	}

	if (0 == un32RetFlag) {
		// 开始在接受套接字上处理I/O使用重叠I/O机制
		// 在新建的套接字上投递一个或多个异步
		// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务    
		// 单I/O操作数据(I/O重叠)
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		//PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		PerIoData = (LPPER_IO_OPERATION_DATA)&session->operationData;
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 1;  // read
		PerIoData->client = acceptSocket;

		if (SOCKET_ERROR == WSAIoctl(this->hListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx,
			sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL))
		{
			printf("WSAIoctl failed with error code: %d\n", WSAGetLastError());
			if (INVALID_SOCKET != this->hListenSocket)
			{
				closesocket(this->hListenSocket);
				this->hListenSocket = INVALID_SOCKET;
			}
			//goto EXIT_CODE;
			un32RetFlag = -1;
		}

		if (0 == un32RetFlag) {
			if (FALSE == lpfnAcceptEx(this->hListenSocket, PerIoData->client, PerIoData->databuff.buf, PerIoData->databuff.len - ((sizeof(SOCKADDR_IN)+16) * 2),
				sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, &(PerIoData->overlapped)))
			{
				if (WSA_IO_PENDING != WSAGetLastError())
				{
					printf("lpfnAcceptEx failed with error code: %d\n", WSAGetLastError());

					un32RetFlag = -1;
				}
			}
		}
	}

	return un32RetFlag;
}

void NetListener::MainLoop() {
	UINT32 un32RetFlag = 0;
	if (!bIfInitialized) {
		printf("NetListener not initialized!\n");
		return;
	}

	printf("Waiting for client connection...\n");


	NetListener * pThis = (NetListener*)this;
	if (NULL == pThis) {
		return;
	}

	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = false;

	static HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
	while (true) {
		un32RetFlag = 0;
		bRet = GetQueuedCompletionStatus(pThis->hCompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);

		if (0 == un32RetFlag) {
			if (NULL == IpOverlapped) {
				printf("Recv error: %d, overflapped error\n", GetLastError());
				un32RetFlag = -1;
			}
		}

		if (0 == un32RetFlag) {
			PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);
			if (NULL == PerIoData) {
				printf("Recv error: %d on session ID: %d\n", GetLastError(), PerIoData->netSession->iSessionID);
				//un32RetFlag = -1;
			}
		}

		if (0 == un32RetFlag) {
			if (0 == bRet){
				if (WAIT_TIMEOUT == GetLastError())
				{
					continue;
				}
				printf("IOCP error: %d on session ID: %d\n", GetLastError(), PerIoData->netSession->iSessionID);
				//un32RetFlag = -1;
			}
		}

		if (0 == un32RetFlag) {
			// 检查在套接字上是否有错误发生
			if (0 == BytesTransferred){
				//closesocket(PerHandleData->socket);
				//GlobalFree(PerHandleData);
				//GlobalFree(PerIoData);
				//PerIoData->netSession->bIfUse = false;

				//重新为session配置socket连接
				MakeFreeIOCompletionPort((NetSession*)PerIoData->netSession);
				continue;
			}

			if (PerIoData->operationType == 1) {//accept 
				//获取一个net session
				//NetSession * session = netSession.GetFreeSession();
				NetSession * session = (NetSession*)PerIoData->netSession;
				if (NULL == session) {
					printf("Error create session\n");
					un32RetFlag = -1;
				}

				if (0 == un32RetFlag) {
					printf("Created session on id: %d\n", session->iSessionID);
					// 创建用来和套接字关联的单句柄数据信息结构
					//PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// 在堆中为这个PerHandleData申请指定大小的内存
					PerHandleData = (LPPER_HANDLE_DATA)&session->handleData;
					PerHandleData->socket = PerIoData->client;
					//memcpy(&PerHandleData->ClientAddr, &saRemote, remoteLen);
					//客户端套接字和完成端口关联
					HANDLE hResult = ::CreateIoCompletionPort((HANDLE)PerIoData->client, hCompletionPort, (SIZE_INT)PerHandleData, 0);
					if (NULL == hCompletionPort) {
						printf("Error connect IOCP\n");
						un32RetFlag = -1;
					}
				}

				if (0 == un32RetFlag) {
					// 开始在接受套接字上处理I/O使用重叠I/O机制
					// 在新建的套接字上投递一个或多个异步
					// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务	
					// 单I/O操作数据(I/O重叠)
					LPPER_IO_OPERATION_DATA PerIoData = NULL;
					//PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
					PerIoData = (LPPER_IO_OPERATION_DATA)&session->operationData;
					ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
					PerIoData->databuff.len = 1024;
					PerIoData->databuff.buf = PerIoData->buffer;
					PerIoData->operationType = 0;	// read

					DWORD RecvBytes;
					DWORD Flags = 0;
					WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
				}
			}
			else {//data
				// 开始数据处理，接收来自客户端的数据
				WaitForSingleObject(hMutex, INFINITE);
				printf("SID %d:  %s\n", PerIoData->netSession->iSessionID, PerIoData->databuff.buf);
				ReleaseMutex(hMutex);

				// 为下一个重叠调用建立单I/O操作数据
				ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
				PerIoData->databuff.len = 1024;
				PerIoData->databuff.buf = PerIoData->buffer;
				PerIoData->operationType = 0;	// read
				WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
			}
		}
	}
}

__NANOC_THREAD_FUNC_BEGIN__(NetListener::IOCPThread) {
	printf("This is IOCPThread\n");

	NetListener * pThis = (NetListener*)pv;
	if (NULL == pThis) {
		__NANOC_THREAD_FUNC_END__(0);
	}

	printf("IOCPThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}
#else
void NetListener::CleanUp() {
	if (INVALID_SOCKET != epoll_fd) {
		close(epoll_fd);
		epoll_fd = INVALID_SOCKET;
	}
	if (INVALID_SOCKET != hListenSocket) {
		close(hListenSocket);
		hListenSocket = INVALID_SOCKET;
	}

	bIfInitialized = false;
}

void NetListener::Init() {
	UINT32 un32RetFlag = 0;

	//if (0 == un32RetFlag) {
	//	//bind listen socket with io completion port
	//	//HANDLE hResult = ::CreateIoCompletionPort((HANDLE)hListenSocket, hCompletionPort, (SIZE_INT)-2, 0);
	//	//start io thread
	//	__NANOC_THREAD_BEGIN__(m_phIOThread, NetListener::IOCPThread, this);
	//	if (NULL == m_phIOThread) {
	//		printf("IOCP thread start error\n");
	//		un32RetFlag = -7;
	//	}
	//}

	if (0 == un32RetFlag) {
		//创建流式套接字，绑定到本机
		//this->hListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		this->hListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == hListenSocket) {
			printf("Error create socket: %d\n", WSAGetLastError());
			un32RetFlag = -2;
		}
	}

	if (0 == un32RetFlag) {
		int iRc = fcntl(hListenSocket, F_SETFL, fcntl(hListenSocket, F_GETFD, 0) | O_NONBLOCK);
		if (SOCKET_ERROR == iRc) {
			printf("Error set nonblocking socket: %d\n", WSAGetLastError());
			un32RetFlag = -2;
		}
	}

	if (0 == un32RetFlag) {
		//绑定到本机
		sockaddr_in stAddr = { 0 };
		stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		stAddr.sin_family = AF_INET;
		stAddr.sin_port = htons(9005);

		int iRc = bind(hListenSocket, (sockaddr*)&stAddr, sizeof(stAddr));
		if (iRc < 0) {
			printf("Socket bind error: %d\n", errno);
			un32RetFlag = -5;
		}
	}

	if (0 == un32RetFlag) {
		//将套接字设置为监听模式
		//start listen, 128 backup connection times
		int iRc = listen(hListenSocket, 128);
		if (iRc < 0) {
			printf("Socket listen error: %d\n", errno);
			un32RetFlag = -6;
		}
	}
	if (0 == un32RetFlag) {
		// 设置每个集成允许打开的最大文件数目
		rlimit rlt;
		rlt.rlim_max = rlt.rlim_cur = MAXEPOLL;
		int iRc = setrlimit(RLIMIT_NOFILE, &rlt);
		if (iRc < 0) {
			printf("Set limit error: %d\n", errno);
			un32RetFlag = -6;
		}


	}
	if (0 == un32RetFlag) {
		// 创建epoll
		epoll_event	ev;
		epoll_fd = epoll_create(MAXEPOLL);
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = this->hListenSocket;
		int iRc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, hListenSocket, &ev);
		if (iRc < 0) {
			printf("Epoll control error: %d\n", errno);
			un32RetFlag = -1;
		}
	}

	if (0 != un32RetFlag){
		printf("Error: %d\n", un32RetFlag);
		this->CleanUp();
	}
	else {
		printf("NetListener initialized successfully!\n");
		this->bIfInitialized = true;
	}
}

void NetListener::MainLoop() {
	UINT32 un32RetFlag = 0;
	if (!bIfInitialized) {
		printf("NetListener not initialized!\n");
		return;
	}

	printf("Waiting for client connection...\n");

	INT32 cur_fds = 1;
	INT32 wait_fds;
	INT32 conn_fds;
	epoll_event	evs[MAXEPOLL];
	epoll_event	ev;
	sockaddr_in cliaddr;
	socklen_t len;
	INT32 nRead;
	char 	buf[MAXLINE];
	while (true) {
		un32RetFlag = 0;

		if (0 == un32RetFlag) {
			//返回epoll获取的消息数，并copy数据到event里面
			wait_fds = epoll_wait(epoll_fd, evs, cur_fds, -1);
			if (wait_fds == -1) {
				printf("Epoll wait error: %d\n", errno);
				un32RetFlag = -1;
			}
		}

		if (0 == un32RetFlag) {
			//遍历event
			for (int i = 0; i < wait_fds; i++) {
				//accept连接请求
				if (evs[i].data.fd == hListenSocket && cur_fds < MAXEPOLL) {
					conn_fds = accept(hListenSocket, (sockaddr*)&cliaddr, &len);
					if (INVALID_SOCKET == conn_fds) {
						printf("Accept error: %d\n", errno);
						un32RetFlag = -1;
					}

					if (0 == un32RetFlag) {
						printf("Create session on id: %d\n", conn_fds);

						ev.events = EPOLLIN | EPOLLET;
						ev.data.fd = conn_fds;
						int iRc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fds, &ev);
						if (iRc < 0) {
							printf("Epoll control error: %d\n", errno);
							un32RetFlag = -1;
						}
					}

					if (0 == un32RetFlag) {
						++cur_fds;
						continue;
					}
				}

				if (0 == un32RetFlag) {
					nRead = read(evs[i].data.fd, buf, sizeof(buf));
					if (nRead <= 0) {
						printf("Recv error on session ID: %d\n", evs[i].data.fd);
						close(evs[i].data.fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);
						--cur_fds;
						continue;
					}
					printf("SID %d: %s\n", evs[i].data.fd, buf);
					//write(evs[i].data.fd, buf, nRead);
				}
			}
		}
	}
}

__NANOC_THREAD_FUNC_BEGIN__(NetListener::IOCPThread) {
	printf("This is IOCPThread\n");

	INetListener * pThis = (INetListener*)pv;
	if (NULL == pThis) {
		__NANOC_THREAD_FUNC_END__(0);
	}


	printf("IOCPThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}


#endif

NetListener g_NetListener;

__NANOC_EXPORT__ INetListener * GetNetListener() {
	return &g_NetListener;
}