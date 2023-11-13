#include <iostream>

using namespace std;

// Need to link with Ws2_32.lib
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

	/* The Winsock DLL is acceptable. Proceed to use it. */

	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS);
	if (connectSocket == INVALID_SOCKET)
	{

		PrintFailedError(connectSocket, "Socket failed with error");
		return 1;
	}

	// IPv4

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	SOCKADDR_IN service{ 0 };
	service.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);
	service.sin_port = htons(7777);

	//----------------------
	// Connect to server.
	iResult = connect(connectSocket, (SOCKADDR*)&service, sizeof(service));
	if (iResult == SOCKET_ERROR)
	{
		PrintFailedError(connectSocket, "Connect failed with error");
		return 1;
	}

	cout << "Connected to Server.\n";
	while (true)
	{
		cout << "Connecting...\n";
	
		// Receive until the peer closes the connection
		char recvBuffer[1024];
		iResult = recv(connectSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (iResult <= 0)
		{
			PrintFailedError(connectSocket, "Recv failed");
			return 1;
		}

		cout << "Bytes received -> " << iResult << " byte\n";
		cout << "recv Data -> " << recvBuffer << "\n\n";

		char sendBuffer[1024] = "Client : Hello, Client Socket!";

		iResult = send(connectSocket, sendBuffer, sizeof(sendBuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			PrintFailedError(connectSocket, "Send failed with error");
			return 1;
		}

		cout << "Bytes Sent -> " << iResult << " byte\n";
		cout << "Send Data -> " << sendBuffer << "\n\n";
	}

	closesocket(connectSocket);
	WSACleanup();

	cin.get();
	return 0;
}