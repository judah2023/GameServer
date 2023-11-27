#include "pch.h"

#include "Service.h"
#include "Listener.h"

// IOCP�� ����Ͽ� ���ŵ� �����͸� ó���ϴ� �Լ�
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
		//IOCP���� �Ϸ�� �۾��� ����ϰ� ����� ������
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