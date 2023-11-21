#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <vector>
#include <thread>

#define DATA_BUFSIZE 4096

struct Session
{
	WSAOVERLAPPED overlapped = {};
	SOCKET socket = INVALID_SOCKET;
	char buffer[DATA_BUFSIZE] = {};

	Session() = default;
	Session(SOCKET inSocket) : socket(inSocket) {}
};

void PrintFailedError(SOCKET sock, const char* sentence)
{
	cout << sentence << " = " << WSAGetLastError() << "\n";
	closesocket(sock);
	WSACleanup();
}

// IOCP를 사용하여 수신된 데이터를 처리하는 함수
void RecvThread(HANDLE iocpHandle)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	Session* session = nullptr;
	WSAOVERLAPPED* overlapped = {};

	int err = 0;

	// Todo
	while (true)
	{
		cout << "\n=====================================================================\n";
		cout << "[Server]\t Waiting...\n";
		// IOCP에서 작업이 완료될 때까지 대기
		err = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&overlapped, INFINITE);
		if (err)
		{
			session = (Session*)overlapped;
			cout << "\t\tData : " << session->buffer << "\n";
			cout << "\t\tData Length : " << bytesTransferred << "\n";
			cout << "\t\tData key : " << key << "\n";

			WSABUF wsaBuf;
			wsaBuf.buf = session->buffer;
			wsaBuf.len = sizeof(session->buffer);

			DWORD recvLen = 0;
			DWORD flags = 0;

			WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &session->overlapped, nullptr);
			Sleep(1000);
		}
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

	// IOCP 핸들 생성
	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, NULL);
	thread t(RecvThread, iocpHandle);

	while (true)
	{
		SOCKET acceptSocket = accept(listenSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET)
		{
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		cout << "[Server]\t Client Connected!!!\n";

		// Session* session = new Session(acceptSocket);
		shared_ptr<Session> session = make_shared<Session>(acceptSocket);

		CreateIoCompletionPort((HANDLE)acceptSocket, iocpHandle, (ULONG_PTR)session.get(), 0);


		// 수신 버퍼 설정 및 WSARecv 호출
		WSABUF wsaBuf;
		wsaBuf.buf = session->buffer;
		wsaBuf.len = sizeof(session->buffer);

		DWORD recvLen = 0;
		DWORD flags = 0;

		WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &session->overlapped, NULL);
		t.join();
	}

	closesocket(listenSocket);
	WSACleanup();
	
	cin.get();
	return 0;
}