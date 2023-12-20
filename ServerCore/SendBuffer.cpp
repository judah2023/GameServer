#include "pch.h"
#include "SendBuffer.h"

#include "SendBufferChunck.h"

SendBuffer::SendBuffer(shared_ptr<SendBufferChunk> chunk, BYTE* start, int size)
	: sendBufferChunk(chunk), buffer(start), freeSize(size)
{
}

SendBuffer::~SendBuffer()
{
}

bool SendBuffer::Close(int usedSize)
{
	if (freeSize < usedSize)
		return false;

	writeSize = usedSize;
	sendBufferChunk->Close(usedSize);
	return true;
}
