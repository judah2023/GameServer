#pragma once

#include "IOCPObj.h"

class Listener : public IOCPObj
{
private:
	SOCKET socket = INVALID_SOCKET;

public:
	Listener() = default;
	~Listener();

public:
	HANDLE GetHandle() override;
	virtual void Dispatch(IOCPEvent* iocpEvent, int numOfBytes) override;

public:
	bool Accept(class Service* service);
	void CloseSocket();

private:
	void RegisterAccept(class AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

};

