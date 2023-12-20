#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int size)
{
	bufferSize = size;
	capacity = bufferSize * BUFFER_SIZE;
	buffer.resize(capacity);
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clear()
{
    int dataSize = DataSize();
    if (dataSize == 0)
    {
        readPos = 0;
        writePos = 0;
    }
    else
    {
        if (FreeSize() < bufferSize)
        {
            memcpy_s(&buffer[0], capacity, &buffer[readPos], dataSize);
            readPos = 0;
            writePos = readPos;

        }
    }

}

bool RecvBuffer::OnRead(int numOfByte)
{
    if (numOfByte > DataSize())
        return false;

    readPos += numOfByte;
    return true;
}

bool RecvBuffer::OnWrite(int numOfByte)
{
    if (numOfByte > FreeSize())
        return false;

    writePos += numOfByte;
    return true;
}
