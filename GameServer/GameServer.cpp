#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <vector>

#define DEFAULT_BUF_LEN 1024

void PrintFailedError(SOCKET sock, const char* sentence)
{
	cout << sentence << " = " << WSAGetLastError() << "\n";
	closesocket(sock);
	WSACleanup();
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int iResult;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	//----------------------
	// Initialize Winsock
	iResult = WSAStartup(wVersionRequested, &wsaData);
	if (iResult != ERROR_SUCCESS) // ERROR_SUCCESS = 0L
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		cout << "WSAStartup failed with error.\n";
		return 1;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
		HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))	// Checking Version
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		std::cout << "Could not find a usable version of Winsock.dll\n";
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "The Winsock 2.2 dll was found okay\n\n";
	}

	//----------------------
	// Create a SOCKET for listening for 
	// incoming connection requests
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS);
	if (listenSocket == INVALID_SOCKET)
	{
		PrintFailedError(listenSocket, "Socket failed with error");
		return 1;
	}

	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.
	u_long iMode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &iMode);
	if (iResult == INVALID_SOCKET)
	{
		PrintFailedError(listenSocket, "Ioctlsocket failed with error");
		return 1;
	}

	//----------------------
	// IPv4
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	SOCKADDR_IN service{ 0 };
	service.sin_family = AF_INET;						// AF_INET : IPv4
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr); // IP 127.0.0.1 : MyCom IP
	// htons : host to network short
	service.sin_port = htons(7777);						// port : 7777

	//----------------------
	// Bind the socket. (SOCKET, SOCKADDR)
	iResult = bind(listenSocket, (SOCKADDR*)&service, sizeof(service));
	if (iResult == SOCKET_ERROR)
	{
		PrintFailedError(listenSocket, "Bind failed with error");
		return 1;
	}

	//----------------------
	// Listen for incoming connection requests 
	// on the created socket
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		PrintFailedError(listenSocket, "Listen failed with error");
		return 1;
	}

	//----------------------
	// Create a SOCKET for accepting incoming requests.
	cout << "[Server]\t Waiting for client to connect...\n";

	vector<SOCKET> sockets;

	fd_set reads;
	fd_set writes;

	while (true)
	{
		FD_ZERO(&reads);

		for (auto& sock : sockets)
		{
			FD_SET(sock, &reads);
		}

		FD_SET(listenSocket, &reads);

		cout << "Hello, ";

		iResult = select(NULL, &reads, nullptr, nullptr, nullptr);

		cout << "World\n";

		if (iResult == SOCKET_ERROR)
		{
			break;
		}

		bool isSet = FD_ISSET(listenSocket, &reads);
		if (isSet)
		{
			SOCKET acceptSocket = accept(listenSocket, nullptr, nullptr);

			sockets.push_back(acceptSocket);

			cout << "[Server]\t Client Connected!\n";
		}
	}
	
	/* then call WSACleanup when done using the Winsock dll */

	// Close the socket to release the resources associated
	// Normally an application calls shutdown() before closesocket 
	//   to  disables sends or receives on a socket first
	// This isn't needed in this simple sample
	
	// No longer need server socket
	closesocket(listenSocket);
	WSACleanup();
	
	cin.get();
	return 0;
}