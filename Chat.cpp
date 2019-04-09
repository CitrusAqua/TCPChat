#include "Chat.h"

#include <iostream>
#include <thread>
#include <string>
using namespace std;


void Chat::recvLoop() {

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	WSADATA wsaData;

	int iResult;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	// Receive data until the server closes the connection
	do {
		iResult = recv(currentUsingSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			char * decrypted = new char[iResult];
			encryptor.Decry(recvbuf, iResult, decrypted, iResult, (char *)&key, 8);
			cout << decrypted << endl;
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);
}


void Chat::chatListen() {

	WSADATA wsaData;

	int iResult;

	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}
	

	addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, listenPort, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}


	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
	iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}


	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	currentUsingSocket = ClientSocket;
	connected = true;
	thread recvThread_(bind(&Chat::recvLoop, this));
	recvThread_.detach();
	//WSACleanup();
}


Chat::Chat(const char* port) : listenPort(port) {
}


Chat::~Chat() {
}

void Chat::start() {

	thread listenThread_(bind(&Chat::chatListen, this));

	string content;

	cout << "Hello!" << endl;
	cout << "Avaliable commands:" << endl;
	cout << "connect    : Connect to another chat app." << endl;
	cout << "disconnect : Disconnect." << endl;
	cout << "send       : Send message." << endl;
	cout << "quit       : Quit this app." << endl;

	while (true) {

		cout << " > " << flush;
		cin >> content;
		if (content == "quit") {
			return;
		}
		else if (content == "connect") {

			if (connected) {
				cout << "Alread connected! Disconnect before a new connection." << endl;
				continue;
			}

			string dst_ip;
			string dst_port;
			cout << "Please input the target ip." << endl;
			cin >> dst_ip;
			cout << "Please input the target port." << endl;
			cin >> dst_port;

			int errcode = chatConnect(dst_ip.c_str(), dst_port.c_str());

			if (errcode != 0) {
				cout << "Unable to connect to server." << endl;
				continue;
			}

			cout << "Connection established." << endl;

			currentUsingSocket = ConnectSocket;
			connected = true;
			closesocket(ListenSocket);
			thread recvThread_(bind(&Chat::recvLoop, this));
			recvThread_.detach();
		}
		else if (content == "disconnect") {

			if (!connected) {
				cout << "No connection to close." << endl;
				continue;
			}



		}
		else if (content == "send") {

			if (!connected) {
				cout << "You must connect before send any message!" << endl;
				continue;
			}

			cout << "Please enter the message to send:" << endl;
			string sendbuf;
			cin >> sendbuf;

			int datalen = (sendbuf.length() + 1) * 8;
			cout << datalen << endl;

			char * plain = new char[datalen];
			char * encrypted = new char[datalen];

			strcpy_s(plain, sendbuf.length() + 1, sendbuf.c_str());

			encryptor.Encry(plain, sendbuf.length() + 1, encrypted, datalen, (char *)&key, 8);
			cout << encrypted << endl;

			int iResult = send(currentUsingSocket, encrypted, datalen, 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
			}

		}
		else {
			cout << "Unrecognized command." << endl;
		}
	}
}

int Chat::chatConnect(const char * ip, const char * port) {
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	// Resolve the server address and port
	iResult = getaddrinfo(ip, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}


	iResult = getaddrinfo(ip, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		printf("Unable to connect to server!\n");
		return 1;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		return 1;
	}

	return 0;
}

void Chat::chatDisconnect() {
}

