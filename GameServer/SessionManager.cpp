#include "pch.h"
#include "SessionManager.h"

#include "ServerSession.h"

void SessionManager::Add(shared_ptr<ServerSession> session)
{
	unique_lock<shared_mutex> lock(rwLock);
	sessions.insert(session);
}

void SessionManager::Remove(shared_ptr<ServerSession> session)
{
	unique_lock<shared_mutex> lock(rwLock);
	sessions.erase(session);
}

void SessionManager::Broadcast(shared_ptr<SendBuffer> buffer)
{
	unique_lock<shared_mutex> lock(rwLock);
	for (auto& session : sessions)
	{
		session->Send(buffer);
	}
}
