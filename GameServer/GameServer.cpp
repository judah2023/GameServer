#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <vector>

#define DEFAULT_BUF_LEN 1024

struct Session
{
	SOCKET sock = INVALID_SOCKET;
	char recvBuffer[DEFAULT_BUF_LEN]{};
	char sendBuffer[DEFAULT_BUF_LEN]{};
	UINT64 recvLen = 0;
	UINT64 sendLen = 0;

	Session(SOCKET inSocket) : sock(inSocket) {}
};

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
	vector<WSAEVENT> wsaEvents;
	
	sockets.push_back(listenSocket);

	//-------------------------
	// Create new event
	WSAEVENT listenEvent = WSACreateEvent();
	wsaEvents.push_back(listenEvent);

	//-------------------------
	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and NewEvent
	iResult = WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE);
	if (iResult == SOCKET_ERROR)
	{
		PrintFailedError(listenSocket, "WSAEventSelect failed with errer : ");
		return 1;
	}

	while (true)
	{
		cout << "\n============================================================================================\n";
		//WSAWaitForMultipleEvents : 여러 이벤트를 감시
		//cEvents : 이벤트 갯수
		//lphEvents : 이벤트 배열의 시작점 주소	//->[][][][][]
		//fWaitAll : (true)모든 이벤트들을 기다릴것인지, (false) 준비 되는 값을 반환
		//dwTimeout : 이벤트를 기다리는 시간
		//fAlertable : 스레드 경고 가능한 대기 상태에 배치되는지 여부를 지정하는 값

		//wsaEvents.size() : 이벤트 갯수
		//&wsaEvents[0] : vector wsaEvents의 배열 첫번째 주소
		//FALSE : 준비되는 족족 반환
		//WSA_INFINITE : 무한 계속~
		//FALSE : 값 설정 안함
		DWORD index = WSAWaitForMultipleEvents(wsaEvents.size(), wsaEvents.data(), false, WSA_INFINITE, false);
		if (index == WSA_WAIT_FAILED)
		{
			// Dormammu!
			continue;
		}

		index -= WSA_WAIT_EVENT_0;


		WSANETWORKEVENTS networkEvent;
		// WSAEnumNetworkEvents : 소켓과 이벤트가 어떤 상태인지 확인하기 위해 사용
		// [in] s : socket
		// [in] hEventObject : Event
		// [out] lpNetworkEvents : Handle of Object for identifying
		iResult = WSAEnumNetworkEvents(sockets[index], wsaEvents[index], &networkEvent);
		if (iResult == SOCKET_ERROR)
		{
			// Dormammu!
			continue;
		}
		
		// check listenSocket : ready accept
		if (networkEvent.lNetworkEvents & FD_ACCEPT)
		{
			// chech Error
			if (networkEvent.iErrorCode[FD_ACCEPT_BIT] != ERROR_SUCCESS)
			{
				// Dormammu!
				continue;
			}

			// All Grean!
			SOCKET acceptSocket = accept(listenSocket, nullptr, nullptr);
			if (acceptSocket == INVALID_SOCKET)
			{
				PrintFailedError(acceptSocket, "Accept failed with error : ");
				return 1;
			}

			cout << "[Server]\t Complete to connect Client!\n";

			WSAEVENT acceptEvent = WSACreateEvent();
			
			iResult = WSAEventSelect(acceptSocket, acceptEvent, FD_READ | FD_WRITE | FD_CLOSE);
			if (iResult == SOCKET_ERROR)
			{
				closesocket(acceptSocket);
				PrintFailedError(listenSocket, "WSAEventSelect failed with errer : ");
				return 1;
			}

			sockets.push_back(acceptSocket);
			wsaEvents.push_back(acceptEvent);
		}

		SOCKET& sock = sockets[index];

		// check listenSocket : ready read
		if (networkEvent.lNetworkEvents & FD_READ)
		{
			// chech Error
			if (networkEvent.iErrorCode[FD_ACCEPT_BIT] != ERROR_SUCCESS)
			{
				// Dormammu!
				continue;
			}


			char recvBuffer[DEFAULT_BUF_LEN];
			iResult = recv(sock, recvBuffer, sizeof(recvBuffer), 0);
			if (iResult == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					continue;
				}
			}

			cout << "[Server]\t recv Data : " << recvBuffer << "\n";

		}

		// check listenSocket : ready write
		if (networkEvent.lNetworkEvents & FD_WRITE)
		{
			// chech Error
			if (networkEvent.iErrorCode[FD_ACCEPT_BIT] != ERROR_SUCCESS)
			{
				// Dormammu!
				continue;
			}

			char sendBuffer[DEFAULT_BUF_LEN];
			iResult = send(sock, sendBuffer, sizeof(sendBuffer), 0);
			if (iResult == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					continue;
				}
			}

			cout << "[Server]\t send Data : " << sendBuffer << "\n";
		}

		if (networkEvent.lNetworkEvents & FD_CLOSE)
		{
			//연결 끊겼다면 해당 vector에서 제거
			sockets.erase(sockets.begin() + index);
			wsaEvents.erase(wsaEvents.begin() + index);
		}

		Sleep(1000);
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