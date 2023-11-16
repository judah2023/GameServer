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

	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.
	u_long iMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);
	if (iResult == INVALID_SOCKET)
	{
		PrintFailedError(connectSocket, "Ioctlsocket failed with error");
		return 1;
	}

	//----------------------
	// IPv4
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	SOCKADDR_IN service{ 0 };
	service.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);
	service.sin_port = htons(7777);

	while (true)
	{

		//----------------------
		// Connect to server.
		iResult = connect(connectSocket, (SOCKADDR*)&service, sizeof(service));
		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				cout << "[Client]\t Waiting Server...\n";
				continue;
			}

			if (WSAGetLastError() == WSAEALREADY)
			{
				continue;
			}

			if (WSAGetLastError() == WSAEISCONN)
			{
				cout << "[Client]\t Already Connected.\n";
				break;
			}

			PrintFailedError(connectSocket, "Connect failed with error");
			return 1;
		}
	}

	cout << "[Client]\t Connected to Server.\n";
	char sendBuffer[1024] = "[Client]\t Hello, This is Client's Data!";

	while (true)
	{
		Sleep(1000);
		cout << "\n=====================================================================\n";
		cout << "[Client]\t Connecting...\n";


		iResult = send(connectSocket, sendBuffer, sizeof(sendBuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				cout << "[Client]\t Waiting Data...\n";
				continue;
			}

			break; // Unexpected Error
		}
		else if (iResult == 0)
		{
			cout << "[Client]\t Null Data!\n";
			break;
		}

		cout << "[Client]\t Send Data : " << sendBuffer << "\n";
		cout << "[Client]\t Bytes Sent : " << iResult << " byte\n";

		char recvBuffer[1024];
		iResult = recv(connectSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				cout << "[Client]\t Waiting Data...\n";
				continue;
			}

			break; // Unexpected Error
		}
		else if (iResult == 0)
		{
			cout << "[Client]\t Null Data!\n";
			break;
		}

		cout << "[Client]\t recv Data : " << recvBuffer << "\n";
		cout << "[Client]\t Bytes Sent : " << iResult << " byte\n";

		// Press Enter
		if (GetAsyncKeyState(VK_RETURN))
		{
			shutdown(connectSocket, SD_SEND);
		}

	}


	closesocket(connectSocket);
	WSACleanup();

	cin.get();
	return 0;
}