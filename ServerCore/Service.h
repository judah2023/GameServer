#pragma once
class Service
{

private:
	SOCKADDR_IN sockaddr = {};

public:
	Service() = delete;
	Service(wstring ip, u_short port);
	~Service();

public:
	SOCKADDR_IN& GetSockAddr() { return sockaddr; }
};

