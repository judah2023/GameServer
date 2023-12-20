#pragma once
#include <functional>

enum class ServiceType : u_char
{
	SERVER,
	CLIENT
};

class IOCPCore;
class Session;

using SessionFactory = function<shared_ptr<Session>(void)>;

class Service : public enable_shared_from_this<Service>
{

private:
	ServiceType type;
	SOCKADDR_IN sockAddr = {};
	shared_ptr<IOCPCore> iocpCore = nullptr;

protected:
	shared_mutex rwLock;
	set<shared_ptr<Session>> sessions;
	UINT sessionCount = 0;
	SessionFactory sessionFactory;

public:
	Service(ServiceType type, wstring ip, u_short port, SessionFactory factoryFunc);
	~Service();

public:
	ServiceType GetType() { return type; }
	SOCKADDR_IN& GetSockAddr() { return sockAddr; }
	shared_ptr<IOCPCore> GetIOCPCore() { return iocpCore; }
	int GetSessionCount() { return sessionCount; }

	void SetSessionFactory(const SessionFactory factoryFunc) { sessionFactory = factoryFunc; }

public:
	shared_ptr<Session> CreateSession();
	void AddSession(shared_ptr<Session>  session);
	void RemoveSession(shared_ptr<Session>  session);

public:
	virtual bool Start() abstract;

};

