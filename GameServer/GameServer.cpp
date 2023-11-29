#include "pch.h"

#include "ServerService.h"
#include "Listener.h"
#include "IOCPCore.h"

using namespace std;

int main()
{
	shared_ptr<ServerService> service = make_shared<ServerService>(L"127.0.0.1", 7777);
	if (!service->Start())
	{
		printf("[Server]\t Service failed to start with errer : %u", WSAGetLastError());
		return 1;
	}

	thread t([=]()
		{
			while (true)
			{
				service->GetIOCPCore()->Dispatch();
				this_thread::sleep_for(1s);
			}
		});

	t.join();
	
	cin.get();
	return 0;
}