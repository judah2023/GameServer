#include "pch.h"
#include "ServerService.h"

#include "Listener.h"

ServerService::ServerService(wstring ip, u_short port, SessionFactory factoryFunc) 
	: Service(ServiceType::SERVER, ip, port, factoryFunc)
{
}

bool ServerService::Start()
{
	listener = make_shared<Listener>();
	return listener->Accept(static_pointer_cast<ServerService>(shared_from_this()));
}
