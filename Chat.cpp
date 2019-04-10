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
		printf("\nWSAStartup failed: %d\n > ", iResult);
		return;
	}

	// Receive data until the server closes the connection
	do {
		iResult = recv(currentUsingSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("\nMessage received from %s\n", peerAddress);
			char * decrypted = new char[iResult];
			encryptor.Decry(recvbuf, iResult, decrypted, iResult, (char *)&key, 8);
			printf("\n%s\n > ", decrypted);
		}
		else if (iResult == 0)
			printf("\nConnection closed\n > ");
	} while (iResult > 0);
}


void Chat::chatListen() {

	WSADATA wsaData;

	int iResult;

	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("\nWSAStartup failed: %d\n > ", iResult);
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
		printf("\nNot a legal port number.\n > ");
		WSACleanup();
		return;
	}


	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("\nError at socket(): %ld\n > ", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
	iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("\nPort is already allocated.\n > ");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("\nListen failed with error: %ld\n > ", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	listening = true;

	// Accept a client socket
	sockaddr_in client_info = { 0 };
	int addrsize = sizeof(client_info);
	ClientSocket = accept(ListenSocket, (sockaddr*)&client_info, &addrsize);
	if (ClientSocket == INVALID_SOCKET) {
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	inet_ntop(AF_INET, (void*)&client_info.sin_addr, peerAddress, 30);
	printf("\nConnected to %s\n > ", peerAddress);
	currentUsingSocket = ClientSocket;
	connected = true;
	thread recvThread_(bind(&Chat::recvLoop, this));
	recvThread_.detach();
	//WSACleanup();
}


Chat::Chat() {
}


Chat::~Chat() {
}

void Chat::start() {

	while (!listening) {
		getline(cin, listenPort);
		thread listenThread_(bind(&Chat::chatListen, this));
		while (!listening) {
			listenThread_.join();
		}
	}

	string content;

	cout << "Hello!" << endl;
	cout << "Avaliable commands:" << endl;
	cout << "connect    : Connect to another chat app." << endl;
	cout << "disconnect : Disconnect." << endl;
	cout << "send       : Send message." << endl;
	cout << "quit       : Quit this app." << endl;

	while (true) {

		cout << " > " << flush;
		getline(cin, content);
		if (content.length() == 0) {
			cout << '\r';
			continue;
		}
		if (content == "quit") {
			closesocket(ListenSocket);
			return;
		}
		else if (content == "connect") {

			if (connected) {
				cout << "Alread connected! Disconnect before establishing a new connection." << endl;
				continue;
			}

			string dst_ip;
			string dst_port;
			cout << "Please input the target ip." << endl;
			getline(cin, dst_ip);
			cout << "Please input the target port." << endl;
			getline(cin, dst_port);

			int errcode = chatConnect(dst_ip.c_str(), dst_port.c_str());

			if (errcode != 0) {
				continue;
			}

			cout << "Connection established." << endl;
			strcpy_s(peerAddress, 30, dst_ip.c_str());
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

			int iResult = shutdown(currentUsingSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed: %d\n", WSAGetLastError());
			}
			connected = false;
			closesocket(currentUsingSocket);
			thread listenThread_(bind(&Chat::chatListen, this));

		}
		else if (content == "send") {

			if (!connected) {
				cout << "You must connect before send any message!" << endl;
				continue;
			}

			cout << "Please enter the message to send:" << endl;
			string sendbuf;
			getline(cin, sendbuf);

			int datalen = (sendbuf.length() + 1) * 8;
			char * plain = new char[datalen];
			char * encrypted = new char[datalen];
			strcpy_s(plain, sendbuf.length() + 1, sendbuf.c_str());
			encryptor.Encry(plain, sendbuf.length() + 1, encrypted, datalen, (char *)&key, 8);
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
		printf("Please input a vaild address.\n");
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


	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		printf("Unable to connect to server.\n");
		return 1;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server.\n");
		return 1;
	}

	return 0;
}

void Chat::chatDisconnect() {
}

