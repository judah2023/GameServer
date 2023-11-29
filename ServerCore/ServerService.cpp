#include "pch.h"
#include "ServerService.h"

#include "Listener.h"

ServerService::ServerService(wstring ip, u_short port) 
	: Service(ServiceType::SERVER, ip, port)
{
}

bool ServerService::Start()
{
	listener = make_shared<Listener>();
	return listener->Accept(this);
}
