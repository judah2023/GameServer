#include "pch.h"
#include "Service.h"

#include "IOCPCore.h"
#include "SocketHelper.h"

Service::Service(ServiceType type, wstring ip, u_short port) : type(type)
{
	SocketHelper::StartUp();

	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;

	IN_ADDR address;
	InetPton(AF_INET, ip.c_str(), &address);
	sockAddr.sin_addr = address;
	sockAddr.sin_port = htons(port);

	iocpCore = make_shared<IOCPCore>();
}

Service::~Service()
{
	SocketHelper::CleanUp();
}

bool Service::Start()
{
	return true;
}

shared_ptr<Session> Service::CreateSession(shared_ptr<Session> session)
{
	shared_ptr<Session> session = factory();
	if (!iocpCore->Register(session))
	{
		return nullptr;
	}

	return session;
}

void Service::AddSession(shared_ptr<Session> session)
{
	unique_lock<shared_mutex> lock(rwLock);
	sessionCount++;
	sessions.insert(session);
}

void Service::RemoveSession(shared_ptr<Session> session)
{
	unique_lock<shared_mutex> lock(rwLock);
	sessions.erase(session);
	sessionCount--;
}
