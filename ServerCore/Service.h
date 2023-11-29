#pragma once
#include <functional>

class IOCPCore;
class Session;

enum class ServiceType : u_char
{
	SERVER,
	CLIENT
};

using SessionFactory = function<shared_ptr<Session>(void)>;

class Service
{

private:
	ServiceType type;
	SOCKADDR_IN sockAddr = {};
	shared_ptr<IOCPCore> iocpCore;

protected:
	shared_mutex rwLock;
	UINT sessionCount = 0;
	set<shared_ptr<Session>> sessions;
	SessionFactory factory;

public:
	Service() = delete;
	Service(ServiceType type, wstring ip, u_short port);
	~Service();

public:
	virtual bool Start();

public:
	ServiceType GetType() { return type; }
	SOCKADDR_IN& GetSockAddr() { return sockAddr; }
	IOCPCore* GetIOCPCore() { return iocpCore.get(); }
	int GetSessionCount() { return sessionCount; }

	void SetFactory(const SessionFactory func) { factory = func; }

public:
	shared_ptr<Session> CreateSession(shared_ptr<Session> session);
	void AddSession(shared_ptr<Session> session);
	void RemoveSession(shared_ptr<Session> session);

};

