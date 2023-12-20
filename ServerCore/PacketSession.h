#pragma once
#include "Session.h"

struct PacketHeader
{
	UINT16 size;	// 크기
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
	// sealed : 자식이 이 함수를 override 할 수 없다.
	virtual int OnRecv(BYTE* buffer, int len) sealed;
	virtual int OnRecvPacket(BYTE* buffer, int len) abstract;

};

