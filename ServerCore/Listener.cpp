#include "pch.h"
#include "Listener.h"

#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "ServerService.h"
#include "Session.h"
#include "SocketHelper.h"
#include "RecvBuffer.h"

Listener::~Listener()
{
	CloseSocket();
}

HANDLE Listener::GetHandle()
{
	return (HANDLE)socket;
}

void Listener::Dispatch(IOCPEvent* iocpEvent, int numOfBytes)
{
	AcceptEvent* acceptEvent = (AcceptEvent*)iocpEvent;
	ProcessAccept(acceptEvent);
}

bool Listener::Accept(shared_ptr<ServerService> inService)
{
	service = inService;

	socket = SocketHelper::CreateSocket();
	if (socket == INVALID_SOCKET)
		return false;

	ULONG_PTR key = 0;
	if (service->GetIOCPCore()->Register(shared_from_this()) == false)
		return false;

	// setsockopt
	if (!SocketHelper::SetReuseAddress(socket, true))
		return false;

	if (!SocketHelper::SetLinger(socket, 0, 0))
		return false;

	// Bind
	if (!SocketHelper::Bind(socket, inService->GetSockAddr()))
		return false;

	if (!SocketHelper::Listen(socket))
		return false;

	cout << "[Server]\t Waiting for client to connect...\n";

	AcceptEvent* acceptEvent = new AcceptEvent;
	acceptEvent->iocpObj = shared_from_this();
	RegisterAccept(acceptEvent);

    return true;
}

void Listener::CloseSocket()
{
	SocketHelper::CloseSocket(socket);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<Session> session = service->CreateSession();
	acceptEvent->Init();
	acceptEvent->session = session;

	DWORD dwBytes = 0;
	if (!SocketHelper::AcceptEx(
		socket, session->GetSocket(), session->recvBuffer->WritePos(), 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, (LPOVERLAPPED)acceptEvent
	))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<Session> session = acceptEvent->session;
	if (!SocketHelper::SetUpdateAcceptSocket(session->GetSocket(), socket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddr;
	int addressSize = sizeof(sockAddr);
	if (SOCKET_ERROR == getpeername(session->GetSocket(), (SOCKADDR*)&sockAddr, &addressSize))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetSockAddr(sockAddr);
	session->ProcessConnect();

	RegisterAccept(acceptEvent);
}
