#include "pch.h"
#include "ClientService.h"

#include "Session.h"

ClientService::ClientService(wstring ip, u_short port, SessionFactory factoryFunc) : 
    Service(ServiceType::CLIENT, ip, port, factoryFunc)
{
}

bool ClientService::Start()
{
    shared_ptr<Session> session = CreateSession();
    return session->Connect();
}