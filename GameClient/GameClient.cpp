#include "pch.h"

#include "ClientService.h"
#include "IOCPCore.h"
#include "PacketSession.h"
#include "SendBuffer.h"
#include "SendBufferManager.h"

#include "Protocol.pb.h"

using namespace std;

#define THREAD_COUNT 2

char sendData[] = "[Client] : Hello Server!!!\n";

class ClientSession : public PacketSession
{
public:
	~ClientSession()
	{
		printf("[Client]\t ClientSesion Destroy\n");
	}

	virtual void OnConnected() override
	{
	}

	virtual int OnRecvPacket(BYTE* buffer, int len) override
	{
		int headerSize = sizeof(PacketHeader);
		Protocol::TEST packet;
		packet.ParseFromArray(buffer + headerSize, len - headerSize);

		printf("ID : %d, HP : %d, MP : %d\n", packet.id(), packet.hp(), packet.mp());


		return len;
	}

	virtual void OnSend(int len) override
	{
	}

	virtual void OnDisconnected() override
	{
		printf("[Client]\t Disconnected from Server\n");
	}
};

int main()
{
	printf("============= Client =============\n");
	
	shared_ptr<Service> service = make_shared<ClientService>(L"127.0.0.1", 7777, []() {return make_shared< ClientSession>(); });

	for (int i = 0; i < 1; i++)
	{
		if (!service->Start())
		{
			printf("Server Start Error\n");
			return 1;
		}
	}

	vector<thread> threads;

	for (int i = 0; i < THREAD_COUNT; i++)
	{
		threads.push_back(thread
		([=]()
			{
				while (true)
				{
					service->GetIOCPCore()->Dispatch();
				}
			}
		));
	}

	for (int i = 0; i < THREAD_COUNT; i++)
	{
		if (threads[i].joinable())
		{
			threads[i].join();
		}
	}
	
	cin.get();
	return 0;
}