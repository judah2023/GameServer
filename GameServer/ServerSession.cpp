#include "pch.h"
#include "ServerSession.h"

#include "SendBuffer.h"
#include "SendBufferManager.h"
#include "SessionManager.h"

void ServerSession::OnConnected()
{
	SessionManager::Get().Add(static_pointer_cast<ServerSession>(shared_from_this()));
}

int ServerSession::OnRecvPacket(BYTE* buffer, int len)
{
	PacketHeader header = *(PacketHeader*)buffer;
	printf("Size : %u, ID : %u\n", header.size, header.id);

	return len;
}

void ServerSession::OnSend(int len)
{
}

void ServerSession::OnDisconnected()
{
	SessionManager::Get().Remove(static_pointer_cast<ServerSession>(shared_from_this()));
}