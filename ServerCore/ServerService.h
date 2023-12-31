#pragma once
#include "Service.h"

class Listener;

class ServerService : public Service
{
private:
	shared_ptr<Listener> listener = nullptr;

public:
	ServerService(wstring ip, u_short port, SessionFactory factoryFunc);
	virtual ~ServerService() {}

public:
	virtual bool Start() override;
};
