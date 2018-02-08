// NetListener.cpp : 定义 DLL 应用程序的导出函数。
//

#include "NetListener.h"

NetListener::NetListener() {
	bIfInitialized = false;

	this->isRunning = 1;
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
	INT32 n32RetFlag = 0;
	INT32 n32StateFlag = 0;
	n32StateFlag--;
	if (0 == n32RetFlag) {
		// 初始化Socket库  
		WSADATA wsaData;
		int iRc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != iRc) {
			printf("Socket initialize error: return code %d\n", iRc);

			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//创建IOCP内核对象
		this->hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (NULL == hCompletionPort) {
			printf("Error create IOCP\n");
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//创建流式套接字，绑定到本机
		//this->hListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		this->hListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == hListenSocket) {
			printf("Error create socket: %d\n", WSAGetLastError());
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		// Associate SOCKET with IOCP  
		if (NULL == CreateIoCompletionPort((HANDLE)this->hListenSocket, this->hCompletionPort, NULL, 0))
		{
			printf("CreateIoCompletionPort failed with error code: %d\n ", WSAGetLastError());
			if (INVALID_SOCKET != this->hListenSocket)
			{
				closesocket(this->hListenSocket);
				this->hListenSocket = INVALID_SOCKET;
			}
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//绑定到本机
		sockaddr_in stAddr = { 0 };
		stAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		stAddr.sin_family = AF_INET;
		stAddr.sin_port = htons(9005);

		int iRc = bind(hListenSocket, (sockaddr*)&stAddr, sizeof(stAddr));
		if (iRc < 0) {
			printf("Socket bind error: %d\n", errno);
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//将套接字设置为监听模式
		//start listen, 128 backup connection times
		int iRc = listen(hListenSocket, 128);
		if (iRc < 0) {
			printf("Socket listen error: %d\n", errno);
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		for (int i = this->netSession.getSize(); i > 0; i--) {
			NetSession * session = this->netSession.GetFreeSession();
			if (NULL == session) {
				break;
			}
			//AcceptEx
			if (MakeFreeIOCompletionPort(session)) {
				break;
			}
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//bind listen socket with io completion port
		//HANDLE hResult = ::CreateIoCompletionPort((HANDLE)hListenSocket, hCompletionPort, (SIZE_INT)-2, 0);
		//start io thread
		//创建消息池
		static CharStringPool pool;
		this->msgPool = &pool;
		//创建线程锁
		__NANOC_THREAD_MUTEX_INIT__(hMutex, this);
		//创建线程
		__NANOC_THREAD_BEGIN__(m_phIOThread, NetListener::IOCPThread, this);
		if (NULL == m_phIOThread) {
			printf("IOCP thread start error\n");
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 != n32RetFlag){
		printf("Error: %d\n", n32RetFlag);
		this->CleanUp();
	}
	else {
		printf("NetListener initialized successfully!\n");
		this->bIfInitialized = true;
	}
}

INT32 NetListener::MakeFreeIOCompletionPort(INetSession* _session) {
	INT32 n32RetFlag = 0;
	if (NULL == _session) {
		return n32RetFlag;
	}
	NetSession * session = (NetSession*)_session;

	//使用AcceptEx接收客户端连接而不是开辟线程等待连接
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;//AcceptEx函数指针
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == acceptSocket)
	{
		printf("WSASocket failed with error code: %d\n", WSAGetLastError());
		n32RetFlag = -1;
	}

	if (0 == n32RetFlag) {
		// 开始在接受套接字上处理I/O使用重叠I/O机制
		// 在新建的套接字上投递一个或多个异步
		// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务    
		// 单I/O操作数据(I/O重叠)
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		//PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		PerIoData = (LPPER_IO_OPERATION_DATA)&session->operationData;
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = MAX_BUFFERSIZE;
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
			n32RetFlag = -1;
		}

		if (0 == n32RetFlag) {
			if (FALSE == lpfnAcceptEx(this->hListenSocket, PerIoData->client, PerIoData->databuff.buf, PerIoData->databuff.len - ((sizeof(SOCKADDR_IN)+16) * 2),
				sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, &(PerIoData->overlapped)))
			{
				if (WSA_IO_PENDING != WSAGetLastError())
				{
					printf("lpfnAcceptEx failed with error code: %d\n", WSAGetLastError());

					n32RetFlag = -1;
				}
			}
		}
	}

	return n32RetFlag;
}

__NANOC_THREAD_FUNC_BEGIN__(NetListener::IOCPThread) {
	printf("This is IOCPThread\n");

	NetListener * pThis = (NetListener*)pv;
	if (NULL == pThis) {
		__NANOC_THREAD_FUNC_END__(0);
	}


	INT32 n32RetFlag = 0;

	printf("Waiting for client connection...\n");

	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = false;

	while (true) {
		n32RetFlag = 0;
		bRet = GetQueuedCompletionStatus(pThis->hCompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);

		if (0 == n32RetFlag) {
			if (NULL == IpOverlapped) {
				printf("Recv error: %d, overflapped error\n", GetLastError());
				n32RetFlag = -1;
			}
		}

		if (0 == n32RetFlag) {
			PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);
			if (NULL == PerIoData) {
				printf("Recv error: %d on session ID: %d\n", GetLastError(), PerIoData->netSession->iSessionID);
				//n32RetFlag = -1;
			}
		}

		if (0 == n32RetFlag) {
			if (0 == bRet){
				if (WAIT_TIMEOUT == GetLastError())
				{
					continue;
				}
				printf("IOCP error: %d on session ID: %d\n", GetLastError(), PerIoData->netSession->iSessionID);
				//n32RetFlag = -1;
			}
		}

		if (0 == n32RetFlag) {
			// 检查在套接字上是否有错误发生
			if (0 == BytesTransferred){
				printf("Socket error: 0 bytes transfered\n");
				
				//closesocket(PerHandleData->socket);
				//GlobalFree(PerHandleData);
				//GlobalFree(PerIoData);
				//PerIoData->netSession->bIfUse = false;

				//重新为session配置socket连接
				pThis->MakeFreeIOCompletionPort((NetSession*)PerIoData->netSession);
				continue;
			}

			if (PerIoData->operationType == 1) {
				//获取一个net session
				//NetSession * session = netSession.GetFreeSession();
				NetSession * session = (NetSession*)PerIoData->netSession;
				if (NULL == session) {
					printf("Error create session\n");
					n32RetFlag = -1;
				}

				if (0 == n32RetFlag) {
					printf("Created session on id: %d\n", session->iSessionID);
					// 创建用来和套接字关联的单句柄数据信息结构
					//PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// 在堆中为这个PerHandleData申请指定大小的内存
					PerHandleData = (LPPER_HANDLE_DATA)&session->handleData;
					PerHandleData->socket = PerIoData->client;
					//memcpy(&PerHandleData->ClientAddr, &saRemote, remoteLen);
					//客户端套接字和完成端口关联
					HANDLE hResult = ::CreateIoCompletionPort((HANDLE)PerIoData->client, pThis->hCompletionPort, (SIZE_INT)PerHandleData, 0);
					if (NULL == pThis->hCompletionPort) {
						printf("Error connect IOCP\n");
						n32RetFlag = -1;
					}
				}

				if (0 == n32RetFlag) {
					//从session获取重叠IO，在下面设置数据操作
					//PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
					PerIoData = (LPPER_IO_OPERATION_DATA)&session->operationData;
				}
			}
			
			if (0 == n32RetFlag) {
				// 为下一个重叠调用建立单I/O操作数据
				ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
				PerIoData->databuff.len = MAX_BUFFERSIZE;
				PerIoData->databuff.buf = PerIoData->buffer;
				PerIoData->operationType = 0;	// read
				WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
			}

			if (0 == n32RetFlag) {
				printf("Msg recevd\n");
				// 开始数据处理，接收来自客户端的数据
				__NANOC_THREAD_MUTEX_LOCK__(pThis->hMutex);
				NetSession * session = (NetSession*)PerIoData->netSession;
				PerIoData->databuff.buf[BytesTransferred] = 0;
				pThis->addMsgQueue(session, PerIoData->databuff.buf);
				__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hMutex);
			}
		}
	}

	pThis->CleanUp();
	printf("IOCPThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}
#else
void NetListener::CleanUp() {
	if (NULL != m_phIOThread) {
		if (WAIT_TIMEOUT == __NANOC_THREAD_WAIT__(m_phIOThread)) {
			__NANOC_THREAD_END__(m_phIOThread);
		}
		m_phIOThread = NULL;
	}
	if (INVALID_SOCKET != epoll_fd) {
		printf("Epoll closed\n");
		close(epoll_fd);
		epoll_fd = INVALID_SOCKET;
	}
	if (INVALID_SOCKET != hListenSocket) {
		printf("Socket closed\n");
		close(hListenSocket);
		hListenSocket = INVALID_SOCKET;
	}

	bIfInitialized = false;
}

void NetListener::Init() {
	INT32 n32RetFlag = 0;
	INT32 n32StateFlag = 0;
	

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//创建流式套接字，绑定到本机
		//this->hListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		this->hListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == hListenSocket) {
			printf("Error create socket: %d\n", WSAGetLastError());
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		int iRc = fcntl(hListenSocket, F_SETFL, fcntl(hListenSocket, F_GETFD, 0) | O_NONBLOCK);
		if (SOCKET_ERROR == iRc) {
			printf("Error set nonblocking socket: %d\n", WSAGetLastError());
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//绑定到本机
		sockaddr_in stAddr = { 0 };
		stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		stAddr.sin_family = AF_INET;
		stAddr.sin_port = htons(9005);

		//enable address reuse
		//it's helpful when processed ended and restarted
		int nOptval = 1;
		if (setsockopt(hListenSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&nOptval, sizeof(int)) < 0) {
			printf("Sock opt failed\n", errno);
			n32RetFlag = n32StateFlag;
		}
		int iRc = bind(hListenSocket, (sockaddr*)&stAddr, sizeof(stAddr));
		if (iRc < 0) {
			printf("Socket bind error: %d\n", errno);
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//将套接字设置为监听模式
		//start listen, 128 backup connection times
		int iRc = listen(hListenSocket, 128);
		if (iRc < 0) {
			printf("Socket listen error: %d\n", errno);
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		// 设置每个集成允许打开的最大文件数目
		rlimit rlt;
		rlt.rlim_max = rlt.rlim_cur = MAXEPOLL;
		int iRc = setrlimit(RLIMIT_NOFILE, &rlt);
		if (iRc < 0) {
			printf("Set limit error: %d\n", errno);
			n32RetFlag = n32StateFlag;
		}

	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		// 创建epoll
		epoll_event	ev;
		epoll_fd = epoll_create(MAXEPOLL);
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = this->hListenSocket;
		int iRc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, hListenSocket, &ev);
		if (iRc < 0) {
			printf("Epoll control error: %d\n", errno);
			n32RetFlag = n32StateFlag;
		}
	}

	n32StateFlag--;
	if (0 == n32RetFlag) {
		//start io thread
		//创建消息池
		this->msgPool = (CharStringPool*)GetPool();
		//创建线程锁
		__NANOC_THREAD_MUTEX_INIT__(hMutex, this);
		//创建线程
		__NANOC_THREAD_BEGIN__(m_phIOThread, NetListener::IOCPThread, this);
		if (NULL == m_phIOThread) {
			printf("IOCP thread start error\n");
			n32RetFlag = n32StateFlag;
		}
	}

	if (0 != n32RetFlag){
		printf("Error: %d\n", n32RetFlag);
		this->CleanUp();
	}
	else {
		printf("NetListener initialized successfully!\n");
		this->bIfInitialized = true;
	}
}


__NANOC_THREAD_FUNC_BEGIN__(NetListener::IOCPThread) {
	printf("This is IOCPThread\n");

	NetListener * pThis = (NetListener*)pv;
	if (NULL == pThis) {
		__NANOC_THREAD_FUNC_END__(0);
	}

	INT32 n32RetFlag = 0;

	printf("Waiting for client connection...\n");

	INT32 wait_fds;
	INT32 conn_fds;
	epoll_event	evs[MAXEPOLL];
	epoll_event	ev;
	sockaddr_in cliaddr;
	socklen_t len;
	INT32 nRead;
	char 	buf[MAXLINE];

	pThis->cur_fds = 1;
	while (true) {
		if (pThis->isRunning == 0) {
			break;
		}

		n32RetFlag = 0;

		if (0 == n32RetFlag) {
			//返回epoll获取的消息数，并copy数据到event里面
			wait_fds = epoll_wait(pThis->epoll_fd, evs, pThis->cur_fds, -1);
			if (wait_fds == -1) {
				printf("Epoll wait error: %d: %s\n", errno, strerror(errno));
				n32RetFlag = -1;
			}
		}

		if (0 == n32RetFlag) {
			//遍历event
			for (int i = 0; i < wait_fds; i++) {
				//accept连接请求
				if (evs[i].data.fd == pThis->hListenSocket && pThis->cur_fds < MAXEPOLL) {
					conn_fds = accept(pThis->hListenSocket, (sockaddr*)&cliaddr, &len);
					if (INVALID_SOCKET == conn_fds) {
						printf("Accept error: %d\n", errno);
						n32RetFlag = -1;
					}

					if (0 == n32RetFlag) {
						printf("Create session on id: %d\n", conn_fds);

						ev.events = EPOLLIN | EPOLLET;
						ev.data.fd = conn_fds;
						int iRc = epoll_ctl(pThis->epoll_fd, EPOLL_CTL_ADD, conn_fds, &ev);
						if (iRc < 0) {
							printf("Epoll control error: %d\n", errno);
							n32RetFlag = -1;
						}
					}

					if (0 == n32RetFlag) {
						++pThis->cur_fds;

						NetSession * session = pThis->netSession.GetFreeSession();
						if (NULL != session) {
							session->socket = conn_fds;
						}
						else {
							n32RetFlag = -1;
						}
					}
				}
				else {
					if (0 == n32RetFlag) {
						nRead = read(evs[i].data.fd, buf, sizeof(buf));
						if (nRead <= 0) {
							printf("Recv error on session ID: %d, (%d)\n", evs[i].data.fd, nRead);
							close(evs[i].data.fd);
							epoll_ctl(pThis->epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);
							--pThis->cur_fds;
							continue;
						}

						NetSession * session = pThis->netSession.GetFreeSession(evs[i].data.fd);
						printf("Recv from session ID: %d (%d) \n", evs[i].data.fd, nRead);
						if (NULL != session) {
							__NANOC_THREAD_MUTEX_LOCK__(pThis->hMutex);
							buf[nRead] = 0;
							pThis->addMsgQueue(session, buf);
							__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hMutex);
						}

						//write(evs[i].data.fd, buf, nRead);
					}
				}
			}
		}
	}

	pThis->CleanUp();
	printf("IOCPThread exited\n");
	__NANOC_THREAD_FUNC_END__(0);
}


#endif

int NetListener::addMsgQueue(INetSession * session, const char * buf) {

	//printf("SID %d:  %s\n", session->iSessionID, buf);
	//获取网络消息发送保存到队列里面
	if (this->msgPool->used >= POOL_MAX) {
		this->msgPool->gc();
	}
	CharString * charString = this->msgPool->get();
	if (charString != NULL) {

		//解析数据是否是http请求
		int connectionType = 0;
		if (CharString::match(buf, "GET")) {
			connectionType = 1;
		}
		else if (CharString::match(buf, "POST")) {
			connectionType = 2;
		}
		//TODO
		//http请求
		if (connectionType) {
			session->connectionType = connectionType;
			char * buffer = charString->__str;
			for (int i = 0; i < 12; i++) {
				buffer[i] = 0;
			}
			int * tSize = (int*)&buffer[0];
			int * protocol = (int*)&buffer[4];
			int * size = (int*)&buffer[8];
			char * _buffer = buffer + 12;
			int len;
			for (len = 0; buf[len] && len < MAX_BUFFERSIZE - 12; len++) {
				_buffer[len] = buf[len];
			}
			_buffer[len] = 0;
			*tSize = len + 12;
			*size = len;
			*protocol = 1;
			buf = buffer;
		}
		else {
			//WebSocket
			if (session->connectionType) {
				char * buffer = charString->__str;
				for (int i = 0; i < 12; i++) {
					buffer[i] = 0;
				}
				int * tSize = (int*)&buffer[0];
				int * protocol = (int*)&buffer[4];
				int * size = (int*)&buffer[8];
				char * _buffer = buffer + 12;
				int len;
				for (len = 0; buf[len] && len < MAX_BUFFERSIZE - 12; len++) {
					_buffer[len] = buf[len];
				}
				_buffer[len] = 0;
				*tSize = len + 12;
				*size = len;
				*protocol = 2;
				buf = buffer;
			}
			else {
				buf = buf;
			}
		}

		charString->set(buf);
		charString->session = session;
		charString->f = this->msgQueue.linkcount;
		this->msgQueue.insertLink(charString);
	}
	return session->connectionType;
}

int NetListener::sendMessage(INetSession * session, const char * buf) {
	if (session == NULL) {
		return -1;
	}
	NetSession * _session = (NetSession*)session;
	if (_session->socket == INVALID_SOCKET) {
		return -1;
	}

#ifdef _NANOC_WINDOWS_
	WSABUF  wasBuf;
	wasBuf.buf = (CHAR*)buf;
	wasBuf.len = (ULONG)strlen(buf);
	DWORD dwBytes = -1;

	return send(_session->handleData.socket, buf, strlen(buf), 0);
	//return WSASend(_session->handleData.socket, &wasBuf, 1, &dwBytes, 0, &_session->operationData.overlapped, NULL); 
#else
	return write(_session->socket, buf, strlen(buf));
#endif
}

int NetListener::closeConnection(INetSession * _session) {
	printf("Sesssion closed\n");
#ifdef _NANOC_WINDOWS_
	this->MakeFreeIOCompletionPort(_session);
#else
	NetSession * session = (NetSession*)_session;
	epoll_event ev;
	close(session->socket);
	epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, session->socket, &ev);
	--this->cur_fds;
#endif
	return 0;
}


NetListener g_NetListener;

__NANOC_EXPORT__ INetListener * GetNetListener() {
	return &g_NetListener;
}