#include "pch.h"
#include "SendBufferChunck.h"

#include "SendBuffer.h"

void SendBufferChunk::Init()
{
	isOpen = false;
	usedSize = 0;
}

shared_ptr<SendBuffer> SendBufferChunk::Open(int size)
{
	if (size > BUFFER_SIZE)
		return nullptr;

	if (isOpen)
		return nullptr;

	if (size > FreeSize())
		return nullptr;

	isOpen = true;

	return make_shared<SendBuffer>(shared_from_this(), &buffer[usedSize], size);
}

void SendBufferChunk::Close(int size)
{
	if (!isOpen)
		return;

	isOpen = false;
	usedSize += size;
}
