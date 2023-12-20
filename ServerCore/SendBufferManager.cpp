#include "pch.h"
#include "SendBufferManager.h"

#include "SendBuffer.h"
#include "SendBufferChunck.h"

// static ���� �ʱ�ȭ
thread_local shared_ptr<SendBufferChunk> SendBufferManager::localSendBufferChunk = nullptr;


shared_ptr<SendBuffer> SendBufferManager::Open(int size)
{
	// ���� ȣ�� ����
	if (localSendBufferChunk == nullptr)
	{
		localSendBufferChunk = Pop();	// ���ο� ûũ
		localSendBufferChunk->Init();	// �ʱ�ȭ
	}

	// �̹� �����ִ� ���
	if (localSendBufferChunk->IsOpen())
		return nullptr;

	// ûũ�� ���������� ������ ���
	if (localSendBufferChunk->FreeSize() < size)
	{
		localSendBufferChunk = Pop();	// ���ο� ûũ
		localSendBufferChunk->Init();	// �ʱ�ȭ
	}

	printf("Chunk Free Data : %llu\n", localSendBufferChunk->FreeSize());

	//localSendbufferChunk�� size ��ŭ ����Ҽ� �ִ� sendBuffer�� ��ȯ
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
