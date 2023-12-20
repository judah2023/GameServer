#pragma once

#include "IOCPObj.h"

class ServerService;
class AcceptEvent;

class Listener : public IOCPObj
{
private:
	shared_ptr<ServerService> service = nullptr;
	SOCKET socket = INVALID_SOCKET;

public:
	Listener() = default;
	virtual ~Listener();

public:
	HANDLE GetHandle() override;
	virtual void Dispatch(IOCPEvent* iocpEvent, int numOfBytes) override;

public:
	bool Accept(shared_ptr<ServerService> serverService);
	void CloseSocket();

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

};

