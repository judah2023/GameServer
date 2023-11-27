#include "pch.h"
#include "Service.h"

Service::Service(wstring ip, u_short port) 
{
	WSADATA wsaData;
	int rc;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc != ERROR_SUCCESS) // ERROR_SUCCESS = 0L
	{
		cout << "WSAStartup failed with error.\n";
		return;
	}

	if (wsaData.wVersion != MAKEWORD(2, 2))
	{
		std::cout << "Could not find a usable version of Winsock.dll\n";
		return;
	}
	else
	{
		std::cout << "The Winsock 2.2 dll was found okay\n\n";
	}

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;

	IN_ADDR address;
	InetPton(AF_INET, ip.c_str(), &address);
	sockaddr.sin_addr = address;
	sockaddr.sin_port = htons(port);
}

Service::~Service()
{
	WSACleanup();
}
