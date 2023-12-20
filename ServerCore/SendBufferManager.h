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
		//c++11 표준 이후로 static 변수의 초기화는 안정성 보장.
		//메모리 누수 걱정 필요 없음.
		static SendBufferManager instance;
		return instance;
	}

public:
	//복사생성자 삭제
	SendBufferManager(const SendBufferManager& rhs) = delete;
	//복사 대입 연산자 삭제
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

