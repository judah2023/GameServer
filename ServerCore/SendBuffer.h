#pragma once

class SendBufferChunk;

class SendBuffer : public enable_shared_from_this<SendBuffer>
{
	shared_ptr<SendBufferChunk> sendBufferChunk;
	BYTE* buffer;
	UINT freeSize = 0;
	UINT writeSize = 0;

public:
	SendBuffer(shared_ptr<SendBufferChunk> chunk, BYTE* start, int size);
	~SendBuffer();

public:
	BYTE* GetBuffer() { return buffer; }
	UINT WriteSize() const { return writeSize; }
	UINT FreeSize() const { return freeSize; }

public:
	bool Close(int usedSize);

};

