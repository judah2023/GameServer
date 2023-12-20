#pragma once
#include "Session.h"

struct PacketHeader
{
	UINT16 size;	// ũ��
	UINT16 id;		// id
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

public:
	shared_ptr<PacketSession> GetPacketSession() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	// sealed : �ڽ��� �� �Լ��� override �� �� ����.
	virtual int OnRecv(BYTE* buffer, int len) sealed;
	virtual int OnRecvPacket(BYTE* buffer, int len) abstract;

};

