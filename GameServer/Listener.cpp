#include "pch.h"
#include "Listener.h"
#include "Service.h"

Listener::Listener()
{
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, NULL);
}

Listener::~Listener()
{
}

bool Listener::Accept(Service& service)
{
	int rc;

	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
		return false;

	if (SOCKET_ERROR == bind(socket, (SOCKADDR*)&service.GetSockAddr(), sizeof(service.GetSockAddr())))
		return false;

	if (SOCKET_ERROR == listen(socket, SOMAXCONN))
		return false;

	cout << "[Server]\t Waiting for client to connect...\n";

	DWORD dwBytes;
	if (SOCKET_ERROR == WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
								 &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, nullptr, nullptr))
	{
		return false;
	}

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (acceptSocket == INVALID_SOCKET)
	{
		return false;
	}


	ULONG_PTR key = 0;
	CreateIoCompletionPort((HANDLE)socket, iocpHandle, key, 0);

	char lpfnOutputBuf[DATA_BUFSIZE] = {};
	WSAOVERLAPPED overlapped = {};

	if (!lpfnAcceptEx(socket, acceptSocket, lpfnOutputBuf, DATA_BUFSIZE - ((sizeof(SOCKADDR_IN) + 16) * 2),
					  sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			closesocket(acceptSocket);
			return false;
		}
	}

    return true;
}

void Listener::CloseSocket()
{
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
	}

	socket = INVALID_SOCKET;
}
