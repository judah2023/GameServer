#include "pch.h"
#include "SocketHelper.h"

LPFN_ACCEPTEX SocketHelper::lpfnAcceptEx = nullptr;


bool SocketHelper::StartUp()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != ERROR_SUCCESS)
	{
		return false;
	}

	SOCKET tempSocket = CreateSocket();
	SetIOControl(tempSocket, WSAID_ACCEPTEX, (LPVOID*)&lpfnAcceptEx);
	CloseSocket(tempSocket);

	return true;
}

void SocketHelper::CleanUp()
{
	WSACleanup();
}

bool SocketHelper::SetIOControl(SOCKET socket, GUID guid, LPVOID* func)
{
	DWORD dwBytes = 0;
	return WSAIoctl(
		socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid), func, sizeof(*func),
		&dwBytes, nullptr, nullptr
	) != SOCKET_ERROR;
}

SOCKET SocketHelper::CreateSocket()
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS, nullptr, NULL, WSA_FLAG_OVERLAPPED);
}

bool SocketHelper::SetReuseAddress(SOCKET socket, bool enable)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, enable);
}

bool SocketHelper::SetLinger(SOCKET socket, u_short onoff, u_short time)
{
	LINGER sockLinger;
	sockLinger.l_onoff = onoff;
	sockLinger.l_linger = time;
	return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, sockLinger);
}

bool SocketHelper::SetUpdateAcceptSocket(SOCKET acceptSocket, SOCKET listenSocket)
{
	return SetSocketOpt(acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketHelper::Bind(SOCKET socket, SOCKADDR_IN sockAddr)
{
	return bind(socket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR;
}

bool SocketHelper::Listen(SOCKET socket, int backlog)
{
	return listen(socket, backlog) != SOCKET_ERROR;
}

void SocketHelper::CloseSocket(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
	}
	socket = INVALID_SOCKET;
}
