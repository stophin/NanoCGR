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
	if (0 == un32RetFlag) {
		// 初始化Socket库  
		WSADATA wsaData;
		int iRc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != iRc) {
			printf("Socket initialize error: return code %d\n", iRc);

			un32RetFlag = -1;
		}
	}

	if (0 == un32RetFlag) {
		//创建IOCP内核对象
		this->hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (NULL == hCompletionPort) {
			printf("Error create IOCP\n");
			un32RetFlag = -1;
		}
	}

	if (0 == un32RetFlag) {
		//bind listen socket with io completion port
		//HANDLE hResult = ::CreateIoCompletionPort((HANDLE)hListenSocket, hCompletionPort, (SIZE_INT)-2, 0);
		//start io thread
		__NANOC_THREAD_BEGIN__(m_phIOThread, NetListener::IOCPThread, this);
		if (NULL == m_phIOThread) {
			printf("IOCP thread start error\n");
			un32RetFlag = -7;
		}
	}

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
		//绑定到本机
		sockaddr_in stAddr = { 0 };
		stAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
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

	SOCKET acceptSocket;
	SOCKADDR_IN saRemote;
	INT32 remoteLen;
	PER_HANDLE_DATA * PerHandleData = NULL;
	while (true) {
		un32RetFlag = 0;
		if (0 == un32RetFlag) {
			//接收客户端的连接
			remoteLen = sizeof(saRemote);
			acceptSocket = accept(hListenSocket, (SOCKADDR*)&saRemote, &remoteLen);
			if (SOCKET_ERROR == acceptSocket) {
				printf("Accept error: %d\n", GetLastError());
				un32RetFlag = -1;
			}
		}

		NetSession * session = netSession.GetFreeSession();
		if (NULL == session) {
			printf("Error create session\n");
			un32RetFlag = -1;
		}

		if (0 == un32RetFlag) {
			// 创建用来和套接字关联的单句柄数据信息结构
			//PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// 在堆中为这个PerHandleData申请指定大小的内存
			PerHandleData = (LPPER_HANDLE_DATA)&session->handleData;
			PerHandleData->socket = acceptSocket;
			memcpy(&PerHandleData->ClientAddr, &saRemote, remoteLen);
			//客户端套接字和完成端口关联
			HANDLE hResult = ::CreateIoCompletionPort((HANDLE)acceptSocket, hCompletionPort, (SIZE_INT)PerHandleData, 0);
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
}

__NANOC_THREAD_FUNC_BEGIN__(NetListener::IOCPThread) {
	printf("This is IOCPThread\n");
	UINT32 un32RetFlag;
	INetListener * pThis = (INetListener*)pv;
	if (NULL == pThis) {
		__NANOC_THREAD_FUNC_END__(0);
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
		if (0 == bRet){
			printf("IOCP error: %d\n", GetLastError());
			un32RetFlag = -1;
		}
		if (NULL == PerHandleData) {
			printf("Recv error: %d\n", GetLastError());
			un32RetFlag = -1;
		}

		if (0 == un32RetFlag) {
			PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

			// 检查在套接字上是否有错误发生
			if (0 == BytesTransferred){
				closesocket(PerHandleData->socket);
				GlobalFree(PerHandleData);
				GlobalFree(PerIoData);
				continue;
			}

			// 开始数据处理，接收来自客户端的数据
			WaitForSingleObject(hMutex, INFINITE);
			printf("A Client says: %s\n", PerIoData->databuff.buf);
			ReleaseMutex(hMutex);

			// 为下一个重叠调用建立单I/O操作数据
			ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
			PerIoData->databuff.len = 1024;
			PerIoData->databuff.buf = PerIoData->buffer;
			PerIoData->operationType = 0;	// read
			WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
		}
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
		stAddr.sin_port = htons(6000);

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
			wait_fds = epoll_wait(epoll_fd, evs, cur_fds, -1);
			if (wait_fds == -1) {
				printf("Epoll wait error: %d\n", errno);
				un32RetFlag = -1;
			}
		}

		if (0 == un32RetFlag) {
			for (int i = 0; i < wait_fds; i++) {
				if (evs[i].data.fd == hListenSocket && cur_fds < MAXEPOLL) {
					conn_fds = accept(hListenSocket, (sockaddr*)&cliaddr, &len);
					if (INVALID_SOCKET == conn_fds) {
						printf("Accept error: %d\n", errno);
						un32RetFlag = -1;
					}

					if (0 == un32RetFlag) {
						printf("Server fot from client\n");

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
						close(evs[i].data.fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);
						--cur_fds;
						continue;
					}
					printf("Recvd: %s\n", buf);
					//write(evs[i].data.fd, buf, nRead);
				}
			}
		}
	}
}

__NANOC_THREAD_FUNC_BEGIN__(NetListener::IOCPThread) {
	printf("This is IOCPThread\n");
	UINT32 un32RetFlag;
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