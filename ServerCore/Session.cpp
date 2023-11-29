#include "pch.h"
#include "Session.h"
#include "SocketHelper.h"

Session::Session()
{
	socket = SocketHelper::CreateSocket();

}

Session::~Session()
{
	SocketHelper::CloseSocket(socket);
}

HANDLE Session::GetHandle()
{
	return (HANDLE)socket;
}

void Session::Dispatch(IOCPEvent* iocpEvent, int numofBytes)
{
	// To do
}

void Session::ProcessConnect()
{
	printf("[Server]\t Client Connected\n");
}
