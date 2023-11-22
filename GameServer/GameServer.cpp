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

	// TCP_NODELAY :  네트워크 통신에서 지연(latency)를 최소하기 위해 설정
	//옵션 사용하면 TCP소켓이 데이터를 즉시 보내도록 설정
	bool isEnabled = true;
	rc = setsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&isEnabled, sizeof(isEnabled));
	if (rc == SOCKET_ERROR)
	{
		cout << "setsockopt for TCP_NODELAY failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "TCP_NODELAY Value : " << isEnabled << "\n";
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

	
#if 0
	// 상대방이 연결을 끊는다면?
	// 주기적으로 TCP 프로토콜 연결 상태 확인 -> 끊어진 연결 감지
	rc = setsockopt(acceptSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&isEnabled, sizeof(isEnabled));
	if (rc == SOCKET_ERROR)
	{
		cout << "setsockopt for SO_KEEPALIVE failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "SO_KEEPALIVE Value : " << isEnabled << "\n";
	}
	
	// SO_LINGER : 지연하다
	// 송신 버퍼에 있는 데이터를 보낼 것인지 없엘 것인지
	//l_onoff == 0 : closesocket()이 바로 리턴, l_onoff == 1 : l_linger 초만큼 대기(default 0)
	LINGER aLinger;
	aLinger.l_onoff = 1;	// on
	aLinger.l_linger = 5;	// 5초 지연

	rc = setsockopt(acceptSocket, SOL_SOCKET, SO_LINGER, (char*)&aLinger, sizeof(aLinger));
	if (rc == SOCKET_ERROR)
	{
		cout << "setsockopt for SO_LINGER failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "SO_LINGER Value : " << aLinger.l_onoff << "\n";
	}

	// SO_REUSEADDR : IP주소 및 port 재사용
	rc = setsockopt(acceptSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&isEnabled, sizeof(isEnabled));
	if (rc == SOCKET_ERROR)
	{
		cout << "setsockopt for SO_REUSEADDR failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "SO_REUSEADDR Value : " << isEnabled << "\n";
	}

	// SO_SNDBUF : 송신 버퍼의 크기 제어
	int Buf_size = 8192;	// 버퍼 크기
	rc = setsockopt(acceptSocket, SOL_SOCKET, SO_SNDBUF, (char*)&Buf_size, sizeof(Buf_size));
	if (rc == SOCKET_ERROR)
	{
		cout << "getsockopt for SO_SNDBUF failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "SO_SNDBUF Value : " << Buf_size << "\n";
	}

	// SO_RCVBUF : 수신 버퍼의 크기 제어
	rc = setsockopt(acceptSocket, SOL_SOCKET, SO_RCVBUF, (char*)&Buf_size, sizeof(Buf_size));
	if (rc == SOCKET_ERROR)
	{
		cout << "setsockopt for SO_RCVBUF failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "SO_RCVBUF Value : " << Buf_size << "\n";
	}

	// SO_UPDATE_CONNECT_CONTEXT : 클라이언트로부터 새로운 연결을 수락(accept)하면, 새 소켓은 몇가지 정보를 가지지 않는다.
	// listenSocket과 속성과 상태를 업데이트 하기 위해 사용
	rc = setsockopt(acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSocket, sizeof(listenSocket));
	if (rc == SOCKET_ERROR)
	{
		cout << "setsockopt for SO_UPDATE_ACCEPT_CONTEXT failed with error : " << WSAGetLastError() << "\n";
	}
	else
	{
		cout << "SO_UPDATE_ACCEPT_CONTEXT Value : " << listenSocket << "\n";
	}
#endif // 0

	int sendBufSize, recvBufSize;
	int optionLen = sizeof(sendBufSize);
	getsockopt(acceptSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufSize, &optionLen);
	if (rc == SOCKET_ERROR)
	{
		cout << "getsockopt for SO_SNDBUF failed with error : " << WSAGetLastError() << "\n";
	}
	
	cout << "SO_SNDBUF Value : " << sendBufSize << "\n";

	optionLen = sizeof(recvBufSize);
	getsockopt(acceptSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufSize, &optionLen);
	if (rc == SOCKET_ERROR)
	{
		cout << "getsockopt for SO_RCVBUF failed with error : " << WSAGetLastError() << "\n";
	}

	cout << "SO_RCVBUF Value : " << recvBufSize << "\n";

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