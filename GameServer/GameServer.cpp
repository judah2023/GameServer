#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

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

	// IPv4

	//----------------------
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

	while (true)
	{
		//----------------------
		// Create a SOCKET for accepting incoming requests.
		cout << "\nWaiting for client to connect...\n";

		SOCKADDR_IN clientService{ 0 }; 	// Client Socket Info
		int addLen = sizeof(clientService);

		//----------------------
		// Accept the connection.
		SOCKET acceptSocket = accept(listenSocket, (SOCKADDR*)&clientService, &addLen);
		if (acceptSocket == INVALID_SOCKET)
		{
			PrintFailedError(listenSocket, "Accept failed with error");
			return 1;
		}

		char ipAddress[16];
		inet_ntop(AF_INET, &clientService.sin_addr, ipAddress, sizeof(ipAddress)); // binary to char
		cout << "Client connected IP : " << ipAddress << "\n";

		//----------------------
		// Send an initial buffer
		char sendBuffer[1024] = "Server : Hello, Server Socket!";
		iResult = send(acceptSocket, sendBuffer, sizeof(sendBuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			PrintFailedError(acceptSocket, "Send failed with error");
			return 1;
		}

		cout << "Bytes Sent -> " << iResult << " byte\n";
		cout << "Send Data -> " << sendBuffer << "\n\n";

		// Receive until the peer closes the connection
		char recvBuffer[1024];
		iResult = recv(acceptSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (iResult > 0)
		{
			cout << "Bytes received : " << iResult << " byte\n";
			cout << "recv Data -> " << recvBuffer << "\n\n";
		}
		else if (iResult < 0)
		{
			cout << "recv failed : " << WSAGetLastError() << "\n";
		}
		else
		{
			cout << "Connection closed\n";
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