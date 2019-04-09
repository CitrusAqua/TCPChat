#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>

#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")


#include "DES.h"


using namespace std;

#define DEFAULT_BUFLEN 1024

class Chat {
public:
	Chat(const char* port);
	~Chat();


private:

	uint64_t key = 0x1610842E20190409;
	DES encryptor;

	SOCKET ConnectSocket = INVALID_SOCKET;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET currentUsingSocket = INVALID_SOCKET;

	sockaddr_in addr;
	bool connected = false;
	const char * listenPort;

	void recvLoop();
	void chatListen();

public:

	void start();

	int chatConnect(const char * ip, const char * port);
	void chatDisconnect();

};

