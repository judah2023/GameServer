#include "pch.h"
#include "PacketSession.h"

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

int PacketSession::OnRecv(BYTE* buffer, int len)
{
	// 처리한 데이터의 길이
	int processLen = 0;
	while (true)
	{
		int dataSize = len - processLen;

		// 만약 데이터 크기가 4byte보다 적다면
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(PacketHeader*)(&buffer[processLen]);

		// 데이터가 제대로 오지 않음
		if (dataSize < header.size)
			break;

		// 패킷만 전달
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;
	}
	return processLen;
}

