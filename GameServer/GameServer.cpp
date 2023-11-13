#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#define DEFAULT_BUFFER_LEN 1024

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
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_HOPOPTS);
	if (serverSocket == INVALID_SOCKET)
	{
		PrintFailedError(serverSocket, "Socket failed with error");
		return 1;
	}

	// IPv4

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	SOCKADDR_IN service{ 0 };
	char ipAddress[16];

	service.sin_family = AF_INET;						// AF_INET : IPv4
	
	service.sin_addr.s_addr = htonl(INADDR_ANY);
	// Alternative
	// inet_pton(AF_INET, "127.0.0.1", &service.sin_addr); // IP 127.0.0.1 : MyCom IP
	
	// htons : host to network short
	service.sin_port = htons(7777);						// port : 7777

	iResult = bind(serverSocket, (SOCKADDR*)&service, sizeof(service));
	if (iResult == SOCKET_ERROR)
	{
		PrintFailedError(serverSocket, "Bind failed with error");
		return 1;
	}

	while (true)
	{
		cout << "\n\n=============================================================================\n";
		cout << "Listening...\n";

		char recvBuffer[1024];
		SOCKADDR_IN clientService{ 0 };

		int addLen = sizeof(clientService);

		iResult = recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&clientService, &addLen);

		if (iResult == SOCKET_ERROR)
		{
			PrintFailedError(serverSocket, "Recvfrom failed with error");
			return 1;
		}

		inet_ntop(AF_INET, &clientService.sin_addr, ipAddress, sizeof(ipAddress));

		cout << "CLIENT_IP : " << ipAddress << "\n";
		cout << "recv Data : " << recvBuffer << "\n";
		cout << "recv Bytes : " << iResult << " byte\n";

		char sendBuffer[DEFAULT_BUFFER_LEN] = "[Server] \"Hello, Client!\"";

		iResult = sendto(serverSocket, sendBuffer, sizeof(sendBuffer), 0, (SOCKADDR*)&clientService, addLen);
		if (iResult == SOCKET_ERROR)
		{
			PrintFailedError(serverSocket, "Sendto failed with error");
			return 1;
		}

		cout << "Send Data : " << sendBuffer << "\n";
		cout << "Bytes Sent : " << iResult << " byte\n";

	}

	/* then call WSACleanup when done using the Winsock dll */

	// Close the socket to release the resources associated
	// Normally an application calls shutdown() before closesocket 
	//   to  disables sends or receives on a socket first
	// This isn't needed in this simple sample
	
	// No longer need server socket
	closesocket(serverSocket);
	WSACleanup();
	
	cin.get();
	return 0;
}