#include "pch.h"
#include "IOCPCore.h"

#include "IOCPEvent.h"
#include "IOCPObj.h"
#include "Session.h"

IOCPCore::IOCPCore()
{
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, NULL);
}

IOCPCore::~IOCPCore()
{
	CloseHandle(iocpHandle);
}

bool IOCPCore::Register(shared_ptr<IOCPObj> iocpObj)
{
	return CreateIoCompletionPort(iocpObj->GetHandle(), iocpHandle, NULL, NULL);
}

bool IOCPCore::Dispatch(DWORD time)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	IOCPEvent* iocpEvent = nullptr;

	cout << "\n=====================================================================\n";

	//GS_LOG();

	printf("Waiting...\n");
	//IOCP에서 완료된 작업을 대기하고 결과를 가져옴
	if (GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&iocpEvent, time))
	{
		shared_ptr<IOCPObj> iocpObj = iocpEvent->iocpObj;
		iocpObj->Dispatch(iocpEvent, bytesTransferred);
	}
	else
	{
		switch (WSAGetLastError())
		{
		case WAIT_TIMEOUT:
			printf("GetQueuedCompletionStatus failed with Error %d\n", WSAGetLastError());
			return false;
		default:
			shared_ptr<IOCPObj> iocpObj = iocpEvent->iocpObj;
			iocpObj->Dispatch(iocpEvent, bytesTransferred);
			break;
		}
	}

	// To do

	return true;
}
