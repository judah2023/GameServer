#pragma once

class IOCPObj;
class Session;

class IOCPCore
{
private:
	HANDLE iocpHandle = nullptr;

public:
	IOCPCore();
	~IOCPCore();

public:
	HANDLE GetHandle() { return iocpHandle; }

	bool Register(shared_ptr<IOCPObj> iocpObj);
	bool Dispatch(DWORD time = INFINITE);
};

