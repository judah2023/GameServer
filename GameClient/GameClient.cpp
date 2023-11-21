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

void SendThread(HANDLE iocpHandle)
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
		cout << "[Client]\t Waiting...\n";
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

			WSASend(session->socket, &wsaBuf, 1, &recvLen, flags, &session->overlapped, nullptr);;
			Sleep(1000);
		}
	}
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int iResult;

	wVersionRequested = MAKEWORD(2, 2);

	iResult = WSAStartup(wVersionRequested, &wsaData);
	if (iResult != ERROR_SUCCESS) // ERROR_SUCCESS = 0L
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

	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS);
	if (connectSocket == INVALID_SOCKET)
	{

		PrintFailedError(connectSocket, "Socket failed with error");
		return 1;
	}

	u_long iMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);
	if (iResult == INVALID_SOCKET)
	{
		PrintFailedError(connectSocket, "Ioctlsocket failed with error");
		return 1;
	}

	SOCKADDR_IN service{ 0 };
	service.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);
	service.sin_port = htons(7777);

	while (true)
	{

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

	char sendBuffer[DATA_BUFSIZE] = "[Client]\t Hello, This is Client's Data!";

	////데이터 전송을 위한 준비
	//WSAEVENT wsaEvent = WSACreateEvent();	// winsock 이벤트 객체를 생성
	//WSAOVERLAPPED overlapped = {};			// 비동기 I/O 작업을 위한 구조체 초기화
	//overlapped.hEvent = wsaEvent;			// overlapped 구조체에 이벤트 객체를 할당

	//while (true)
	//{
	//	Sleep(1000);
	//	cout << "\n=====================================================================\n";
	//	cout << "[Client]\t Connecting...\n";


	//	WSABUF wsaBuf;					
	//	wsaBuf.buf = sendBuffer;		
	//	wsaBuf.len = sizeof(sendBuffer);

	//	DWORD sendLen = 0;				
	//	DWORD flags = 0;				

	//	iResult = WSASend(connectSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr);
	//	if (iResult == SOCKET_ERROR)
	//	{
	//		if (WSAGetLastError() == WSA_IO_PENDING)
	//		{
	//			WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
	//			WSAGetOverlappedResult(connectSocket, &overlapped, &sendLen, FALSE, &flags);
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}


	//	cout << "Send Buffer Length : " << sizeof(sendBuffer) << "\n";
	//}

	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, NULL);
	thread t(SendThread, iocpHandle);
	while (true)
	{
		// Session* session = new Session(connectSocket);
		shared_ptr<Session> session = make_shared<Session>(connectSocket);
		strcpy_s(session->buffer, sendBuffer);

		CreateIoCompletionPort((HANDLE)connectSocket, iocpHandle, (ULONG_PTR)session.get(), 0);

		WSABUF wsaBuf;
		wsaBuf.buf = session->buffer;
		wsaBuf.len = sizeof(session->buffer);

		DWORD recvLen = 0;
		DWORD flags = 0;

		WSASend(session->socket, &wsaBuf, 1, &recvLen, flags, &session->overlapped, nullptr);;
		t.join();
	}

	closesocket(connectSocket);
	WSACleanup();

	cin.get();
	return 0;
}