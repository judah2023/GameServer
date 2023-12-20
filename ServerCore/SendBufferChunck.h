#pragma once

class SendBuffer;

class SendBufferChunk : public enable_shared_from_this<SendBufferChunk>
{
	enum { BUFFER_SIZE = 0x10000 };

private:
	vector<BYTE> buffer;
	bool isOpen = false;
	int usedSize = 0;

public:
	SendBufferChunk() : buffer(BUFFER_SIZE) {}
	~SendBufferChunk() {}

public:
	void Init();
	shared_ptr<SendBuffer> Open(int size);
	void Close(int size);

public:
	bool IsOpen() const { return isOpen; }
	size_t FreeSize() { return buffer.size() - usedSize; }

};


