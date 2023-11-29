#include "pch.h"
#include "Service.h"

#include "Listener.h"
#include "IOCPCore.h"
#include "SocketHelper.h"

Service::Service(wstring ip, u_short port) 
{
	iocpCore = make_shared<IOCPCore>();

	if (!SocketHelper::StartUp())
	{
		GS_LOG();
		printf("Service failed with error : %u\n", WSAGetLastError());
		return;
	}

	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;

	IN_ADDR address;
	InetPton(AF_INET, ip.c_str(), &address);
	sockAddr.sin_addr = address;
	sockAddr.sin_port = htons(port);
}

Service::~Service()
{
	SocketHelper::CleanUp();
}

bool Service::Listen()
{
	listener = make_shared<Listener>();
	return listener->Accept(this);
}
