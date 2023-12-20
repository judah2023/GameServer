#include "pch.h"
#include "SendBufferManager.h"

#include "SendBuffer.h"
#include "SendBufferChunck.h"

// static 변수 초기화
thread_local shared_ptr<SendBufferChunk> SendBufferManager::localSendBufferChunk = nullptr;


shared_ptr<SendBuffer> SendBufferManager::Open(int size)
{
	// 최초 호출 시점
	if (localSendBufferChunk == nullptr)
	{
		localSendBufferChunk = Pop();	// 새로운 청크
		localSendBufferChunk->Init();	// 초기화
	}

	// 이미 열려있는 경우
	if (localSendBufferChunk->IsOpen())
		return nullptr;

	// 청크에 여유공간이 부족할 경우
	if (localSendBufferChunk->FreeSize() < size)
	{
		localSendBufferChunk = Pop();	// 새로운 청크
		localSendBufferChunk->Init();	// 초기화
	}

	printf("Chunk Free Data : %llu\n", localSendBufferChunk->FreeSize());

	//localSendbufferChunk에 size 만큼 사용할수 있는 sendBuffer를 반환
	return localSendBufferChunk->Open(size);
}

shared_ptr<SendBufferChunk> SendBufferManager::Pop()
{
	{
		unique_lock<shared_mutex> lock(rwLock);
		if (!sendBufferChunks.empty())
		{
			shared_ptr<SendBufferChunk> sendBufferChunk = sendBufferChunks.back();
			sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	return shared_ptr<SendBufferChunk>(new SendBufferChunk, PushGlobal);
}

void SendBufferManager::Push(shared_ptr<SendBufferChunk> bufferChunk)
{
	unique_lock<shared_mutex> lock(rwLock);
	sendBufferChunks.push_back(bufferChunk);
}

void SendBufferManager::PushGlobal(SendBufferChunk* bufferChunk)
{
	printf("Chunks Size : %llu\n", Get().sendBufferChunks.size());
	SendBufferManager::Get().Push(shared_ptr<SendBufferChunk>(bufferChunk, PushGlobal));
}
