// NanoCImp.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Platform.h"

#include "../NetListener/NetListener.h"

NanoCImp::NanoCImp() {
	printf("NanoCImp is registered\n");
	this->isRunning = 1;
	GetNanoC()->SetModelInstance(this);
}
NanoCImp::~NanoCImp() {

}

void NanoCImp::Init() {
	printf("This is NanoCImp Init\n");
}

void NanoCImp::Sleep(INT32 n32MilliSecond) {
#ifdef _WINDOWS
	::Sleep(n32MilliSecond);
#else
	usleep(n32MilliSecond * 1000);
#endif
}

void constServer(CharString * charString, NetSession * session);

void NanoCImp::MainLoop() {
	printf("This is NanoCImp MainLoop\n");
	NanoCImp * pThis = (NanoCImp*)this;
	if (NULL == pThis) {
		return;
	}
	printf("NanoC msg pool: %p\n", GetNanoC()->msgPool);

	int count = 0;
	MultiLinkList<CharString> * msgQueue = &GetNanoC()->msgQueue;
	while (true) {
		//结束主线程，其余子线程也结束
		if (pThis->isRunning == 0) {
			break;
		}
		__NANOC_THREAD_MUTEX_LOCK__(pThis->hMutex);
		if (msgQueue->linkcount > 0) {
			CharString * charString = msgQueue->getPos(0);
			if (NULL != charString) {
				msgQueue->removeLink(charString);

				UINT32 n32Protocol = charString->getInt();

				switch (n32Protocol) {
				case 0://Unicode Stream
				{
							charString->transFromUnicode();

							printf("Unicode Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
							printf("%s\n", charString->getLastAsANSI());

							//回复
							if (GetNanoC()->sendMessage(charString->session, charString->getLastAsUTF8()) > 0) {
								printf("Unicode Send\n");
							}
							break;
				}
				case 1://Http
				{
						   printf("Http:\n");
						   const char * str = charString->getStr();
						  printf("%s\n", str);

						   //解析地址
						   const char * url = str;
						   for (int i = 0; url[i]; i++) {
							   if (CharString::match(&url[i], "GET ")) {
								   url = url + i + strlen("GET ");
								   break;
							   }
						   }

							if (CharString::match(url, "/socket.io/1/websocket/96e1cc6b-b2c7-4372-967f-172b3f9a2a99")) {

								char _key[128];
								const char * key = str;
								for (int i = 0; key[i]; i++) {
									if (CharString::match(&key[i], "Sec-WebSocket-Key: ")) {
										key = key + i + strlen("Sec-WebSocket-Key: ");
										break;
									}
								}
								int len;
								for (len = 0; key[len]; len++) {
									if (key[len] == '\r') {
										break;
									}
									_key[len] = key[len];
								}
								_key[len] = 0;

								//返回http信息
								CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "1::");
								if (CharString::makeWS(charString->_str, charString->__str, 101, _key, charString->str)) {
									//回复
									int sendCompleted = 0;
									if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
										printf("WS Header Send\n");
										//printf("%s", charString->_str);
										sendCompleted = 1;
									}
									if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
										printf("WS Content Send\n");
										//printf("%s", charString->__str);
										sendCompleted = 2;
									}

									if (sendCompleted == 2) {
									}
									else {
										printf("Http Msg send error\n");
									}
								}
							}
						   else if (CharString::match(url, "/socket.io/1")) {
							   //返回http信息
							   if (CharString::makeHTTP(charString->_str, charString->__str, 200, "96e1cc6b-b2c7-4372-967f-172b3f9a2a99:200:60:websocket,flashsocket")) {
								   //回复
								   int sendCompleted = 0;
								   if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
									   printf("Http Header Send\n");
									   sendCompleted = 1;
								   }
								   if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
									   printf("Http Content Send\n");
									   sendCompleted = 2;
								   }

								   if (sendCompleted == 2) {
								   }
								   else {
									   printf("Http Msg send error\n");
									   msgQueue->insertLink(charString);
								   }
							   }
						   }
						   else {
							   //返回http信息
							   if (CharString::makeHTTP(charString->_str, charString->__str, 200, "NO DATA CONTENT")) {
								   //回复
								   int sendCompleted = 0;
								   if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
									   printf("Http Header Send\n");
									   sendCompleted = 1;
								   }
								   if (GetNanoC()->sendMessage(charString->session, charString->__str) > 0) {
									   printf("Http Content Send\n");
									   sendCompleted = 2;
								   }

								   if (sendCompleted == 2) {
								   }
								   else {
									   printf("Http Msg send error\n");
									   msgQueue->insertLink(charString);
								   }
							   }
						   }
						  break;
				}
				case 2: //WebSocket
				{
							const char * str = charString->getStr();

							CharString::decodeFrame(charString->_str, str);

							printf("WebSocket Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
							printf("%s\n", charString->_str);

							//x:::
							const char * aes = charString->_str;
							int ind = 0;
							int len;
							for (len = 0; aes[len]; len++) {
								if (aes[len + 1] && aes[len] == ':') {
									ind = len;
								}
							}
							if (ind > 0) {
								aes = aes + ind + 1;
								string _aes(aes);
								string _aes_d = CharString::DecryptionAES(_aes);
								printf("Encoded msg: %s\n", _aes_d.c_str());

								int pos = CharString::match(_aes_d.c_str(), "{\"cmd\":\"");
								if (pos > 0) {
									const char * cmd = _aes_d.c_str() + pos;
									if (CharString::match(cmd, "demo")) {
										printf("WS: Login\n");
										//回复
										//loginInfo
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"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\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}

										//allTableStatus
										CharString::encodeFrame(charString->_str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"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\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
											printf("WebSocket Send\n");
										}
									}
									else if (CharString::match(cmd, "logout")) {
										printf("WS: Logou\n");
									}
									else if (CharString::match(cmd, "leaveRoom")) {
										printf("WS: Leave\n");
										//回复
										//leaveRoom
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"eJwFwcuWQzAAANC9X7FoKINlG6WS9Igy9dhlGIaoNp6pr597CxaUPTHbODmeH4xcJh486aWE8sqntfecP//qp2bNFjuJXLt4N8vW1TDwFpmS2P+M480aE2i0ExOD8sXYvNwf0EWg0l1sWR+cZ6ssCKeJ9hvOQ8w1VPWmettljSut/6gnVqNv0W6HbBNCxeYzgEm5NwgqhOXdy1Ahw/PJjGxs/DRbPCDnvMtQHuuu9GgzzJBGaNC5RRGbstLuwShDGkW7vtyTgHHi0s0wmAJSBIDQw0Obv6vXHHhIxL7TVmvsg390PFaX\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}
									}
									else if (CharString::match(cmd, "join") ||
										CharString::match(cmd, "quickChangeRoom")) {
										printf("WS: Join\n");
										//回复
										//join
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"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\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}

										//allTableStatus
										CharString::encodeFrame(charString->_str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"eJwN0jeiqkAAAMCeq1BIDsUvRMmI5LAdyBKWHAVP/98JphmQ6Z/OYhs/oKUos+S11XvHeVt5g/evNuM9U/4kVyO6dfo+EVD8QwPd6s8we67loVCFd5AP2477QmWwG1OnSnBepzjbondLnvDa3uBgy1wJLX68TK3LZ5qbnbvvqjjsdEpFiPE9wn0/5HKhpPkFgIn3u17PWJBycgR+NNV8zXya6VHx25iXdJ8gcItNvkRhsqL4o7yh1UWmWSM2SsxqrwPkFC5qdy4I8gNtaeq/ecxi1k74tLeoCvA8px88cwifTuN+nD4OOl9k7P380OJhR9W1A4rmUoTsDledenHT4aE3e5wuVC8T7kZiR6XqULypetYyjHQ2+sbAIYOp4zc7TolvL/TGHG+TxktoWTZWFQ3nvN4DRmxCKSdHSAO3BgQno2zAzqpHmRaCIbSq4uNfqjsbDmmncVoIsFvETUsm/De5N+sIq9dzfbRshZupamkwRYCbyLAwCCUHA6PEWMZEhJi+QC8OpBSir4IIIlLjSxZe9kQ7NjQjVWv4b7VNN6j8GrM4qmQ3G1KygYOEKzZZiv28WqV0f9iVmpU0lpskM8/CDOl0Fvc4UHrTGu1W8pqLHJ4c18xUCf2JnJjJlBme9wxq9TRgPRPBeuzIy9mtEN9Ygr7O5Aef8g5TG66jl9WX+9t7eBR3Tpdbwz++bV/nhCDT+ICOugvvJuE9f9oF3y9ETwXxBrrKk5N4YrcjdlJGfczJD1hGn3rnvKstt7ar1L6W5/fY+fIVtaVvPaxWFeM9+FbSCaltc5NBQ7u8zMW64DrrjBDTkpPMjBFyLJKWW3eWbtNMBikVKrrwfilHnsLJdpGFkLkorzOOVEYhWpnPL+xHaulmM7npxvlQ0bZhdTEXxo3QwLBa5t/VNzSfgLuiLMq1FdC2xd5HRSEu366M86qMXTgIqNCqzkdrtai8C76RJ31nakY4BhD9CpmIzh9yfCdzqTzEPzFcBHT/9x+uZDuf\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
											printf("WebSocket Send\n");
										}

										//SelectRooom时可能没有权限
										//CharString::encodeFrame(charString->_str, WS_TEXT_FRAME, "5:::{\"name\":\"error\",\"args\":[\"eJwFwckSQzAAANB7f8UhsU4cHJTYglGtKcfGVkFMxoSpr+97tINj13e00hDK27M27HSFE9bMkhECfLDdJUWk58vZWtEVXFq4fGmWDzOuDzluuDB+vKlm8YpmdAM1fsd6ngHLeLYqTPy7xY4QBZd0bUVnJXMnQHS+p57chRqbydoIGHIYo+FsAlH4yoa9TJDPw3H+1bIw2w==\"]}");
										//if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
										//	printf("WebSocket Send\n");
										//}
									}
									else if (CharString::match(cmd, "rouletteJoin")) {
										printf("WS: Roulette Join\n");
										
										//join
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"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\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}

										//rouletteTableStatus
										CharString::encodeFrame(charString->_str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"eJwN0Umia0AAAMC9q1gg5iWJNos2xGMnpu7Qkpjl9P/XFaoo7WrwRBwnvP4oPWPubRKaWSr028xm+8jVdxAU/PvnGFqEqvzs2uzX9e4ucnoalaGK5pjtjvWw9Ts1HWvCSZz1WnKirKSbxN99qyR9a3d2l9xy7D/+42gB8/AuQ3Tr65kIlRwqB07RHvtGLSDO5gx/P7qQ8rJVCnMs5OalaBK5gfJ31Gisj6zX8K+gvEjn6eHCXKBWtz0IfQaGzGhX0FdL42s6ul81Ui5Z3supqcl81HGm2sTeHCV5wrqeD8H+hWr2i5IdAYWtQBNwIFeVqNf8vxTn+m3Trskq6T6O3JlIG/Lbt0rOlmo/Q9eP2Gpf5V461jnFGq1GxgI9kLTiVaxXGWWRAVJ3iSZtsWkm2IHpWCqwA6gSNzBLHFZzRT+XmjLmGHijv7b8Fq0fMqDCIWxC5gXxcf7BpLPp+dAMOdtyBek5RKxQ8f3fBtNUQjChhxdOv28ywLJoqRqfUyqVqWh6GpQa7qoeViHnIvN1zdTQmuY+/Q99Lpo4CFfl0zGCcLPAW/8HwjKr+Q==\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
											printf("WebSocket Send\n");
										}
									}
									else if (CharString::match(cmd, "rouletteLeaveTable")) {
										printf("WS: Roulette Leave Room\n");

										//rouletteOtherLeaveTable
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"eJwVzD16gjAAANDdqzAUBBSGDmATCg0IiYGYjaagkfJnS0M8vV/fAR6vY/GNXElOdljWCPx0cZ9neOR6InSkhke5EAiY1w+3PPmyRNWFlG1RJMEIduwgJc2UFZBeFIWzeYu7HMMVfoXdkeqqblSSB369nf+gedb9RWsWldVghzHq7nRRDzXkh3QkQ+ZsbRRPNbNSi87taP9nxmzT46As8u414PpbzUEiM9E64vO2wODBKDxb7gKx692QAZgbcSn0Cvf7exFiRaJds07RlKwJTjcvwIc5N1+fINZONQ==\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}

										//rouletteLeaveTable
										CharString::encodeFrame(charString->_str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"eJwFwcuWQzAAANC9X7GYNtTIYhYRQtRRUxVlR44znm2cxKO+fu4tK8rH6NKlD8NhVeTJgU4JuSlgvOJYzmEGwKvkRptHqQmRK6KMdoXz8ay/JThzLIFKpaCGjWobaCzfcsu7mubMToFQuaP7ik76KtgEFizn7F3U3zrchgnFnPewwTZ6O/RMSHdvWkrokQiL3G3A0kYTy/acZt35hPupD938q1+7bQc9hqvL4eiLsmzHRT52hLbDLeqCWary6dPzjx6kc3IcdRHBfSTegDUUBzGsf3/+AXvaTqA=\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->_str) > 0) {
											printf("WebSocket Send\n");
										}
									}
									else if (CharString::match(cmd, "selectRoom")) {
										printf("WS: Select Room\n");

										//selectRoom
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"eJwNzMfCc0oAANC9V8kiRF/chVGCT40+OzV6GCHG0//3PMCBuVGOFtsFIQ3i3FK3wZg8BZLYhhZq+0seFFe4kcYueY9sPUFnkUeNzWH1dOEB6/ai2WClNtBDWSeSIi3ES3n6oQcHTYzUqYCVK5Q+VczK8pWXIKH5sZ1d8DeYVsfZDo50LpF1Btrv44CKer7x2Xpp4FJEtKTPtKizzPBWxbpvm3A7Rr5bQIQOzQm5mrvVqWNhRU1XStkfip9yzS5dzut5rJEMkONM20jTLlsBYvNmKk/sWM6V+kA6nQtm0lNkMfCJSWYSFthMLgYQokIO3HHM3XL/5buSes53uGzERyP/Qlb4qTI9IIb+cal52zBun8sbroDB8bqzO33ufWODdBN7F9XlpCRvRzv+rc/VnwoNx26E5ttxsKYQcAy9Hl8tuxO9SQoaCgM/ELsxEOUGMaNWtmr6qPkriWyWqvwvn561iG70793nTzi86s8vvBsBU3AJKwc6KPJqn8SUcJjbc461mmNPyDb72mxlJXfK/oIwMpUfcnygr8pyvwRS1yL5EwYzl6wuaWnfolciE/jBa5l6AE8bECAEhngbNh5djC+JSwyHzxBYnca8fbW/hNX+Jc76oOjj9svpRDxP8uDC/nX/HFIliz9naYr1eI3rzl0ERTUxdpq6KhdSGtj/S7LKZtm7mxiGH5PWDVLl/9QQgUaPAotlv9oR4y7NfQOmiNP4QCqrPrtvSugRUfknL4s+8FhfetK4T3JfRnHsVnYAs/3525vMQu6U05kOJXN0N+FRbnMf+1wpaTLme5fb+shdS6zMRATtD/ws6it19sml6b1x+bbxS8bgc4zqKUIfA+/dyJXguQlibgvDeLHh1baftsYx24vHjxlaqJgkSwgoRSSPWBHPzp3hzxMCLazcl77PrR8fAxDKSbg9RbGogrJ6vCkm0CUJuCHfdMY2kZ7O5CiLNidOJGIOoKodURfsLP1l+Zp8eH+Fziv+E4cskBq7PIpND6uZU7sfdTeHQp0NrhFnJRyvIgf6aXTY8zT7pADRSyyV5+/pwJ4mtu///gHLklEQ\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}
									}
									else if (CharString::match(cmd, "selectTable")) {
										printf("WS: Select Table\n");

										//selectTable
										CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"ok\",\"args\":[\"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\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
											printf("WebSocket Send\n");
										}
									}
									else {
										printf("WS: Unknown\n");

										//error
										int size = CharString::encodeFrame(charString->str, WS_TEXT_FRAME, "5:::{\"name\":\"error\",\"args\":[\"eJwFwckSQzAAANC7X3GwxBSHHqhlatcwJbdYolpUS4J+fd8jAA3mAXAxrQ3NYMw0TZn1pOr5KvQnpeqXbLq1Lg7Rbg318aU2XD/NcofGeC2NKhoTIHh1VyjPsuSEw09blr03KgSofph6T4gtix4KTLDbo6HRLRBPHfoRVSgA7coL3B01gsCNMZGBKbVUdAZR2Sw2c8xHCKdYSi3txdewzXPj/AcnBTli\"]}");
										if (GetNanoC()->sendMessage(charString->session, charString->str, size) > 0) {
											printf("WebSocket Send\n");
										}
									}
								}
							}
							else {
								printf("WS: common\n");


								//回复
								//loginInfo
								CharString::encodeFrame(charString->str, WS_TEXT_FRAME, charString->_str);
								if (GetNanoC()->sendMessage(charString->session, charString->str) > 0) {
									printf("WebSocket Send\n");
								}
							}


							break;
				}
				case 3://https
				{
						   printf("HTTPS GOT\n");
						   const char * str = charString->getStr();

						   printf("Message Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
						   printf("%s\n", str);

						   //回复
						   if (GetNanoC()->sendMessage(charString->session, str) > 0) {
							   printf("Message Send\n");
						   }
						   break;
				}
				default://Normal Stream
				{
							charString->Reflush();
						   const char * str = charString->getStr();

						   printf("Message Get(%d/%d):", msgQueue->linkcount, GetNanoC()->msgPool->used);
						   printf("%s\n", str);

						   constServer(charString, (NetSession*)charString->session);

						   //回复
						   /*
						   if (GetNanoC()->sendMessage(charString->session, str) > 0) {
							   printf("Message Send\n");
						   }
						   */

						   break;
				}
				}
			}
		}
		__NANOC_THREAD_MUTEX_UNLOCK__(pThis->hMutex);
		pThis->Sleep(100);
	}

	printf("This is NanoCImp MainLoop End\n");
}

NanoCImp g_NanoCImp;
extern "C" __NANOC_EXPORT__ INanoCImp * GetNanoCImp() {
	return &g_NanoCImp;
}

typedef enum NanoCGR_Protocol_tag {
	Nano_Login,
	Nano_Logout,
	Nano_Position
} NanoCGR_Protocol;

int sendMessage(INetSession * session, const char * buf, int size=0) {
	char temp[300];
	int len = size;
	if (size == 0) {
		len = strlen(buf);
	}
	int *rlen = (int*)temp;
	char * str = temp + sizeof(int);
	memcpy(str, buf, len);
	len += sizeof(int);
	*rlen = len;

	int r = GetNanoC()->sendMessage(session, temp, len);
	return r;
}
int emitMessage(INetSession * session, const char * buf, int size=0) {
	char temp[300];
	int len = size;
	if (size == 0) {
		len = strlen(buf);
	}
	int *rlen = (int*)temp;
	char * str = temp + sizeof(int);
	memcpy(str, buf, len);
	len += sizeof(int);
	*rlen = len;

	int r = GetNanoC()->emitMessage(session, temp, len);
	return r;
}

void constServer(CharString * charString, NetSession * session) {
	float x, y;
	const char *str = charString->str;
	int len = strlen(str);
	char * temp = charString->_str;
	char * _temp = charString->__str;
	CharString::base64_decode(str, len, temp);
	NanoCGR_Protocol p;
	int offset = 0;
	int size = 100;
	NetSession * sessions[100];
	DecodeProtocol(CharString, temp, offset, size, p);
	switch (p) {
	case Nano_Login:
		printf("Login %d\n", session->iSessionID);

		offset = 0;
		EncodeProtocol(CharString, temp, offset, size, Nano_Login, -session->iSessionID);
		temp[offset] = 0;
		CharString::base64_encode((const unsigned char*)temp, offset, _temp);
		sendMessage(session, _temp);

		len = GetNanoC()->aliveSession((INetSession**)sessions, size);
		for (int i = 0; i < len; i++) {
			if (sessions[i] == session) {
				continue;
			}
			offset = 0;
			EncodeProtocol(CharString, temp, offset, size, Nano_Login, sessions[i]->iSessionID);
			temp[offset] = 0;
			CharString::base64_encode((const unsigned char*)temp, offset, _temp);
			sendMessage(session, _temp);
		}
		printf("Send: %d\n", len);

		offset = 0;
		EncodeProtocol(CharString, temp, offset, size, Nano_Login, session->iSessionID);
		temp[offset] = 0;
		CharString::base64_encode((const unsigned char*)temp, offset, _temp);
		len = emitMessage(session, _temp);
		printf("Emit: %d\n", len);
		break;
	case Nano_Logout:
		printf("Logout %d\n", session->iSessionID);

		offset = 0;
		EncodeProtocol(CharString, temp, offset, size, Nano_Logout, session->iSessionID);
		temp[offset] = 0;
		CharString::base64_encode((const unsigned char*)temp, offset, _temp);
		emitMessage(session, _temp);
		break;
	case Nano_Position:
		DecodeProtocol(CharString, temp, offset, size, x, y);
		printf("Position %d %.2f, %.2f\n", session->iSessionID, x, y);

		offset = 0;
		EncodeProtocol(CharString, temp, offset, size, Nano_Position, session->iSessionID, x, y);
		temp[offset] = 0;
		CharString::base64_encode((const unsigned char*)temp, offset, _temp);
		emitMessage(session, _temp);
		break;
	default:
		printf("Unknown %d %s\n", session->iSessionID, str);
		if (CharString::match(str, "ENV_SOCK_CLOSE")) {
			offset = 0;
			EncodeProtocol(CharString, temp, offset, size, Nano_Logout, session->iSessionID);
			temp[offset] = 0;
			CharString::base64_encode((const unsigned char*)temp, offset, _temp);
			emitMessage(session, _temp);
		}
		break;
	}
}