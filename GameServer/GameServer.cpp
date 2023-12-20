#include "pch.h"

#include "IOCPCore.h"
#include "ServerService.h"
#include "SendBuffer.h"
#include "SendBufferManager.h"

#include "ServerSession.h"
#include "SessionManager.h"

#include "Protocol.pb.h"

using namespace std;

#define THREAD_COUNT 5

char sendData[1000] = "[Server] : Hello Client!!!\n";

int main()
{
	printf("============= Server =============\n");
	
	shared_ptr<Service> service = make_shared<ServerService>(L"127.0.0.1", 7777, []() { return make_shared<ServerSession>(); });
	if (!service->Start())
	{
		printf("[Server]\t Service failed to start with errer : %u", WSAGetLastError());
		return 1;
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

	while (true)
	{
		Protocol::TEST packet;
		packet.set_id(1);
		packet.set_hp(100);
		packet.set_mp(200);

		UINT16 dataSize = (UINT16)packet.ByteSizeLong();
		UINT16 packetSize = dataSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBuffer = SendBufferManager::Get().Open(packetSize);
		BYTE* buffer = sendBuffer->GetBuffer();
		((PacketHeader*)buffer)->size = packetSize;
		((PacketHeader*)buffer)->id = 0;
		packet.SerializeToArray(&buffer[4], dataSize);
		if (sendBuffer->Close(packetSize))
		{
			SessionManager::Get().Broadcast(sendBuffer);
		}

		this_thread::sleep_for(250ms);
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