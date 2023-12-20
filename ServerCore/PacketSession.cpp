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
	// ó���� �������� ����
	int processLen = 0;
	while (true)
	{
		int dataSize = len - processLen;

		// ���� ������ ũ�Ⱑ 4byte���� ���ٸ�
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(PacketHeader*)(&buffer[processLen]);

		// �����Ͱ� ����� ���� ����
		if (dataSize < header.size)
			break;

		// ��Ŷ�� ����
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;
	}
	return processLen;
}

