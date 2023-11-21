#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

#include <thread>

#define DATA_BUFSIZE 4096
#define GS_LOG() {cout << "Running..." << __FUNCTION__ << "(" << __LINE__ << ")\n";}

void PrintFailedError(SOCKET sock, const char* sentence)
{
	cout << sentence << " = " << WSAGetLastError() << "\n";
	closesocket(sock);
	WSACleanup();
}

// IOCP를 사용하여 수신된 데이터를 처리하는 함수
void AcceptThread(HANDLE iocpHandle)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED overlapped = {};

	while (true)
	{
		cout << "\n=====================================================================\n";

		GS_LOG();

		printf("[Server]\t Waiting...\n");
		//IOCP에서 완료된 작업을 대기하고 결과를 가져옴
		if (GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&overlapped, INFINITE))
		{
			printf("[Server]\t Client Connected!\n");
		}

		this_thread::sleep_for(1s);
	}
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int rc;
	int err = 0;

	wVersionRequested = MAKEWORD(2, 2);

	rc = WSAStartup(wVersionRequested, &wsaData);
	if (rc != ERROR_SUCCESS) // ERROR_SUCCESS = 0L
	{
		cout << "WSAStartup failed with error.\n";
		return 1;
	}

	if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
		HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))	
	{
		std::cout << "Could not find a usable version of Winsock.dll\n";
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "The Winsock 2.2 dll was found okay\n\n";
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS);
	if (listenSocket == INVALID_SOCKET)
	{
		PrintFailedError(listenSocket, "Socket failed with error");
		return 1;
	}

	SOCKADDR_IN service{ 0 };
	service.sin_family = AF_INET;						
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr); 
	service.sin_port = htons(7777);						

	rc = bind(listenSocket, (SOCKADDR*)&service, sizeof(service));
	if (rc == SOCKET_ERROR)
	{
		PrintFailedError(listenSocket, "Bind failed with error");
		return 1;
	}

	rc = listen(listenSocket, SOMAXCONN);
	if (rc == SOCKET_ERROR)
	{
		PrintFailedError(listenSocket, "Listen failed with error");
		return 1;
	}

	cout << "[Server]\t Waiting for client to connect...\n";

	// AcceptEx 함수포인터 로드

	// Load the AcceptEx function into memory using WSAIoctl.
	// The WSAIoctl function is an extension of the ioctlsocket()
	// function that can use overlapped I/O. The function's 3rd
	// through 6th parameters are input and output buffers where
	// we pass the pointer to our AcceptEx function. This is used
	// so that we can call the AcceptEx function directly, rather
	// than refer to the Mswsock.lib library.
	DWORD dwBytes;
	LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	rc = WSAIoctl(
		listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, nullptr, nullptr
	);
	if (rc == SOCKET_ERROR)
	{
		PrintFailedError(listenSocket, "WSAIoctl failed with error : ");
		return 1;
	}

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (acceptSocket == INVALID_SOCKET)
	{
		PrintFailedError(listenSocket, "Create accept socket failed with error: ");
		return 1;
	}

	// IOCP 핸들 생성
	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, NULL);
	thread t(AcceptThread, iocpHandle);

	ULONG_PTR key = 0;
	CreateIoCompletionPort((HANDLE)listenSocket, iocpHandle, key, 0);

	char lpfnOutputBuf[DATA_BUFSIZE] = {};
	WSAOVERLAPPED overlapped = {};

	err = lpfnAcceptEx(listenSocket, acceptSocket, lpfnOutputBuf,
		DATA_BUFSIZE - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &overlapped);
	if (err == false)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			closesocket(acceptSocket);
			PrintFailedError(listenSocket, "AcceptEx failed with error : ");
			return 1;
		}
	}

	t.join();

	closesocket(listenSocket);
	WSACleanup();
	
	cin.get();
	return 0;
}