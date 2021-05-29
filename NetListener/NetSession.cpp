// NetSession.cpp
//

#include "NetListener.h"

NetSession::NetSession(){
}

NetSession::~NetSession() {
}

NetSessionManager::NetSessionManager() {
	this->n32Size = MAX_SESSIONSIZE;
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
INT32 NetSessionManager::getSize() {
	return this->n32Size;
}

#ifdef _NANOC_WINDOWS_

NetSession * NetSessionManager::GetFreeSession(SOCKET socket) {
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

NetSession * NetSessionManager::GetFreeSession(SOCKET socket) {
	if (0 != socket) {
		for (int i = 0; i < this->n32Size; i++) {
			if (this->netSession[i].bIfUse) {
				if (socket == this->netSession[i].socket) {
					return &this->netSession[i];
				}
			}
		}
		return NULL;
	}
	else {
		for (int i = 0; i < this->n32Size; i++) {
			if (this->netSession[i].bIfUse) {
				continue;
			}
			this->netSession[i].bIfUse = true;
			this->netSession[i].socket = socket;
			return &this->netSession[i];
		}
		return NULL;
	}
}

#endif


int NetSessionManager::aliveSession(INetSession** sessions, int size) {
	int index = 0;
	for (int i = 0; i < this->n32Size; i++) {
		if (!this->netSession[i].bIfUse) {
			continue;
		}
		if (!this->netSession[i].bIsAlive) {
			continue;
		}
		sessions[index++] = &this->netSession[i];
		if (index >= size) {
			break;
		}
	}
	return index;
}