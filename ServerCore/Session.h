#pragma once
#include "IOCPObj.h"

class Session : public IOCPObj
{
private:
	SOCKET socket = INVALID_SOCKET;
	SOCKADDR_IN sockAddr = {};

public:
	char buffer[DATA_BUFSIZE] = {};

public:
	Session();
	virtual  ~Session();

private:
	HANDLE GetHandle() override;
	void Dispatch(IOCPEvent* iocpEvent, int numofBytes) override;

public:
	void ProcessConnect();

public:
	SOCKET GetSocket() { return socket; }

public:
	void SetSockAddr(const SOCKADDR_IN address) { sockAddr = address; }

};

