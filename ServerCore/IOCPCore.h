#pragma once

class IOCPObj;

class IOCPCore
{
private:
	HANDLE iocpHandle;

public:
	IOCPCore();
	~IOCPCore();

public:
	HANDLE GetHandle() { return iocpHandle; }

	bool Register(IOCPObj* iocpObj);
	bool Dispatch(DWORD time = INFINITE);
};

