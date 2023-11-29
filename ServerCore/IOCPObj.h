#pragma once

class IOCPEvent;

class IOCPObj
{

public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(IOCPEvent* iocpEvent, int numofBytes = 0) abstract;

};

