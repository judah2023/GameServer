#pragma once

class IOCPCore;
class Listener;

class Service
{

private:
	SOCKADDR_IN sockAddr = {};
	shared_ptr<IOCPCore> iocpCore;
	shared_ptr<Listener> listener;

public:
	Service() = delete;
	Service(wstring ip, u_short port);
	~Service();

public:
	bool Listen();

public:
	SOCKADDR_IN& GetSockAddr() { return sockAddr; }
	IOCPCore* GetIOCPCore() { return iocpCore.get(); }
};

