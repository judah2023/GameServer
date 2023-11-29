#include "pch.h"

#include "Service.h"
#include "Listener.h"
#include "IOCPCore.h"

int main()
{
	shared_ptr<Service> service = make_shared<Service>(L"127.0.0.1", 7777);
	service->Listen();

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