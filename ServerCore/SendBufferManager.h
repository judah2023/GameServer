#pragma once

class SendBuffer;
class SendBufferChunk;


class SendBufferManager
{
private:
	SendBufferManager() {}
	~SendBufferManager() {}

public:
	static SendBufferManager& Get()
	{
		//c++11 ǥ�� ���ķ� static ������ �ʱ�ȭ�� ������ ����.
		//�޸� ���� ���� �ʿ� ����.
		static SendBufferManager instance;
		return instance;
	}

public:
	//��������� ����
	SendBufferManager(const SendBufferManager& rhs) = delete;
	//���� ���� ������ ����
	SendBufferManager& operator=(const SendBufferManager& rhs) = delete;

private:
	shared_mutex rwLock;
	vector<shared_ptr<SendBufferChunk>> sendBufferChunks;

public:
	static thread_local shared_ptr<SendBufferChunk> localSendBufferChunk;

public:
	shared_ptr<SendBuffer> Open(int size);

public:
	shared_ptr<SendBufferChunk> Pop();
	void Push(shared_ptr<SendBufferChunk> bufferChunk);
	static void PushGlobal(SendBufferChunk* bufferChunk);

};

