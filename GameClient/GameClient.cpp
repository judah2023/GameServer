#include <iostream>

using namespace std;

// Need to link with Ws2_32.lib
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

	/* The Winsock DLL is acceptable. Proceed to use it. */

	SOCKET connectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_HOPOPTS);
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

	while (true)
	{
		cout << "\n\n=============================================================================\n";

		char sendBuffer[DEFAULT_BUFFER_LEN] = "[Client] \"Hello, Server!\"";

		iResult = sendto(connectSocket, sendBuffer, sizeof(sendBuffer), 0, (SOCKADDR*)&service, sizeof(service));
		if (iResult == SOCKET_ERROR)
		{
			PrintFailedError(connectSocket, "Sendto failed with error");
			return 1;
		}

		cout << "Send Data : " << sendBuffer << "\n";
		cout << "Bytes Sent : " << iResult << " byte\n";

		SOCKADDR_IN recvService{ 0 };
		int addLen = sizeof(recvService);

		char recvBuffer[DEFAULT_BUFFER_LEN];
		iResult = recvfrom(connectSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&recvService, &addLen);
		if (iResult == SOCKET_ERROR)
		{
			PrintFailedError(connectSocket, "Recvfrom failed with error");
			return 1;
		}

		cout << "Recv Data : " << recvBuffer << "\n";
		cout << "Recv Bytes : " << iResult << " byte\n";

		Sleep(1000);
	}

	closesocket(connectSocket);
	WSACleanup();

	cin.get();
	return 0;
}