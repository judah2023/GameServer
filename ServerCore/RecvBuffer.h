#pragma once
class RecvBuffer
{
public:
	using pos = BYTE*;
	enum { BUFFER_SIZE = 0x10 };

private:
	int bufferSize = 0;
	int capacity = 0;
	int readPos = 0;
	int writePos = 0;
	vector<BYTE> buffer;

public:
	RecvBuffer(int size);
	~RecvBuffer();

public:
	BYTE* ReadPos() { return &buffer[readPos]; }
	BYTE* WritePos() { return &buffer[writePos]; }
	int DataSize() const { return writePos - readPos; }
	int FreeSize() const { return capacity - DataSize(); }

public:
	void Clear();
	bool OnRead(int numOfByte);
	bool OnWrite(int numOfByte);

};

