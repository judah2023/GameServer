#include "pch.h"

#include "Listener.h"
#include "Service.h"
#include "SocketHelper.h"

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

	socket = SocketHelper::CreateSocket();
	if (socket == INVALID_SOCKET)
		return false;

	if (!SocketHelper::SetReuseAddress(socket, true))
		return false;

	if (!SocketHelper::SetLinger(socket, 0, 0))
		return false;

	if (!SocketHelper::Bind(socket, service.GetSockAddr()))
		return false;

	if (!SocketHelper::Listen(socket))
		return false;

	cout << "[Server]\t Waiting for client to connect...\n";

	/*if (WSAIoctl(
		socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, nullptr, nullptr
	) != SOCKET_ERROR)
		return false;*/

	if (!SocketHelper::SetIOControl(socket, WSAID_ACCEPTEX, (LPVOID*)&lpfnAcceptEx))
		return false;

	SOCKET acceptSocket = SocketHelper::CreateSocket();
	if (acceptSocket == INVALID_SOCKET)
		return false;


	ULONG_PTR key = 0;
	CreateIoCompletionPort((HANDLE)socket, iocpHandle, key, 0);

	char lpfnOutputBuf[DATA_BUFSIZE] = {};
	WSAOVERLAPPED overlapped = {};

	DWORD dwBytes = 0;
	if (!lpfnAcceptEx(
		socket, acceptSocket, lpfnOutputBuf, DATA_BUFSIZE - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &overlapped
	))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			return false;
		}
	}

    return true;
}

void Listener::CloseSocket()
{
	SocketHelper::CloseSocket(socket);
}
