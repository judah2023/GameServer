#pragma once
#include "pch.h"

#define DATA_BUFSIZE 4096

class Listener
{

private:
	SOCKET socket = INVALID_SOCKET;
	HANDLE iocpHandle = nullptr;
	LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;

public:
	HANDLE GetHandle() const { return iocpHandle; }

public:
	Listener();
	~Listener();

public:
	bool Accept(class Service& service);
	void CloseSocket();
};

