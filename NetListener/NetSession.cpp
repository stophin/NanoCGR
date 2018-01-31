// NetSession.cpp
//

#include "NetListener.h"


#ifdef _NANOC_WINDOWS_
NetSession::NetSession(){
}

NetSession::~NetSession() {
}

NetSessionManager::NetSessionManager() {
	this->n32Size = 10;
	this->netSession = new NetSession[this->n32Size];
	memset(this->netSession, 0, this->n32Size * sizeof(NetSession));
	// 创建递增的sessionID
	for (int i = 0; i < this->n32Size; i++) {
		this->netSession[i].iSessionID = i + 100;
	}
}

NetSessionManager::~NetSessionManager() {
	if (NULL != this->netSession) {
		delete[] this->netSession;
		this->netSession = NULL;
	}
}

NetSession * NetSessionManager::GetFreeSession() {
	for (int i = 0; i < this->n32Size; i++) {
		if (this->netSession[i].bIfUse) {
			continue;
		}
		this->netSession[i].bIfUse = true;
		this->netSession[i].operationData.netSession = &this->netSession[i];
		return &this->netSession[i];
	}
	return NULL;
}
#else

#define MAXEPOLL 1000
#define MAXLINE 1024

#endif