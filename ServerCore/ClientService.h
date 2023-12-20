#pragma once
#include "Service.h"
class ClientService : public Service
{
public:
	ClientService(wstring ip, u_short port, SessionFactory factoryFunc);
	virtual ~ClientService() {}

	virtual bool Start() override;
};

