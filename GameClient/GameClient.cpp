#include "pch.h"

#include "Service.h"
#include "Listener.h"

enum IOCP_TYPE
{
	IOCP_NONE,
	IOCP_CONNECT,
	IOCP_DISCONNECT
};

struct Session
{
	WSAOVERLAPPED overlapped = {};
	IOCP_TYPE TYPE = IOCP_NONE;
};

void PrintFailedError(SOCKET sock, const char* sentence)
{
	cout << sentence << " = " << WSAGetLastError() << "\n";
	closesocket(sock);
	WSACleanup();
}

// 비동기 연결 완료를 대기하고 결과를 처리하는 스레드 함수
void ConnectThread(HANDLE iocpHandle)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED overlapped = {};
	Session* session = nullptr;
	int err = 0;

	while (true)
	{
		cout << "[Client]\t Waiting...\n";
		err = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&session, INFINITE);
		if (err == true)
		{

			switch (session->TYPE)
			{
			case IOCP_CONNECT:
				cout << "[Client]\t Connect Successed!\n";

				break;
			case IOCP_DISCONNECT:
				cout << "[Client]\t Disconnect Successed!\n";
				return;
			default:
				break;
			}
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

	SOCKET connectSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (connectSocket == INVALID_SOCKET)
	{
		cout << "Socket failed with error : " << WSAGetLastError() << "\n";
		WSACleanup();
		return 1;
	}

	DWORD dwBytes;
	LPFN_CONNECTEX lpfnConnectEX = nullptr;
	GUID GuidConnectEX = WSAID_CONNECTEX;
	rc = WSAIoctl(
		connectSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEX, sizeof(GuidConnectEX),
		&lpfnConnectEX, sizeof(lpfnConnectEX),
		&dwBytes, nullptr, nullptr
	);
	if (rc == SOCKET_ERROR)
	{
		PrintFailedError(connectSocket, "WSAIoctl failed with error : ");
		return 1;
	}

	LPFN_DISCONNECTEX lpfnDisconnectEX = nullptr;
	GUID GuidDisconnect = WSAID_DISCONNECTEX;

	rc = WSAIoctl(
		connectSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidDisconnect, sizeof(GuidDisconnect),
		&lpfnDisconnectEX, sizeof(lpfnDisconnectEX),
		&dwBytes, nullptr, nullptr
	);
	if (rc == SOCKET_ERROR)
	{
		PrintFailedError(connectSocket, "WSAIoctl failed with error : ");
		return 1;
	}

	// 서버의 주소
	SOCKADDR_IN service{ 0 };
	service.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);
	service.sin_port = htons(7777);

	// 로컬 서버의 주소
	SOCKADDR_IN localService{ 0 };
	localService.sin_family = AF_INET;
	localService.sin_addr.s_addr = htonl(INADDR_ANY);
	localService.sin_port = htons(0);

	// 로컬 주소와 connectSocket와의 바인딩
	rc = bind(connectSocket, (SOCKADDR*)&localService, sizeof(localService));
	if (rc == SOCKET_ERROR)
	{
		PrintFailedError(connectSocket, "Bind failed with error : ");
		return 1;
	}
	cout << "[Client]\t Connected to Server.\n";

	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, NULL);
	ULONG_PTR key = 0;
	CreateIoCompletionPort((HANDLE)connectSocket, iocpHandle, key, 0);

	char sendBuffer[DATA_BUFSIZE] = "[Client]\t Hello, This is Client's Data!";

	thread t(ConnectThread, iocpHandle);

	DWORD numOfBytes = 0;
	Session* connectSession = new Session;
	connectSession->TYPE = IOCP_CONNECT;

	// 비동기 연결 시작
	rc = lpfnConnectEX(connectSocket, (SOCKADDR*)&service, sizeof(service), nullptr, 0, &numOfBytes, &connectSession->overlapped);
	if (rc == false)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			PrintFailedError(connectSocket, "ConnectEx failed with error : ");
			return 1;
		}
	}

	Session* disconnectSession = new Session;
	disconnectSession->TYPE = IOCP_DISCONNECT;

	rc = lpfnDisconnectEX(connectSocket, &disconnectSession->overlapped, NULL, NULL);
	if (rc == false)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			PrintFailedError(connectSocket, "DisconnectEx failed with error : ");
			return 1;
		}
	}


	t.join();

	closesocket(connectSocket);
	WSACleanup();

	cin.get();
	return 0;
}