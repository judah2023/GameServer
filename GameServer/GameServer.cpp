#include "pch.h"

#include "Service.h"
#include "Listener.h"

// IOCP를 사용하여 수신된 데이터를 처리하는 함수
void AcceptThread(HANDLE iocpHandle)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED overlapped = {};

	while (true)
	{
		cout << "\n=====================================================================\n";

		GS_LOG();

		printf("[Server]\t Waiting...\n");
		//IOCP에서 완료된 작업을 대기하고 결과를 가져옴
		if (GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&overlapped, INFINITE))
		{
			printf("[Server]\t Client Connected!\n");
		}

		this_thread::sleep_for(1s);
	}
}

int main()
{
	Service service(L"127.0.0.1", 7777);

	Listener listener;

	thread t(AcceptThread, listener.GetHandle());

	listener.Accept(service);

	t.join();
	
	cin.get();
	return 0;
}