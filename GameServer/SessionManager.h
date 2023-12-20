#pragma once

class ServerSession;
class SendBuffer;

class SessionManager
{

private:
	SessionManager() {};
	~SessionManager() {};

public:
	static SessionManager& Get()
	{
		static SessionManager instance;
		return instance;
	}

public:
	SessionManager(const SessionManager& rhs) = delete;
	SessionManager& operator=(const SessionManager& rhs) = delete;

private:
	shared_mutex rwLock;
	set<shared_ptr<ServerSession>> sessions;

public:
	void Add(shared_ptr<ServerSession> session);
	void Remove(shared_ptr<ServerSession> session);
	void Broadcast(shared_ptr<SendBuffer> buffer);
};

