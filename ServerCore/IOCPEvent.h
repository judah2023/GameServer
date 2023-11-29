#pragma once

class IOCPObj;
class Session;

enum class EventType : u_char
{
	CONNECT,
	DISCONNECT,
	ACCEPT,
	RECV,
	SEND
};

class IOCPEvent :public OVERLAPPED
{

public:
	EventType eventType;
	IOCPObj* iocpObj;

public:
	IOCPEvent(EventType type);

public:
	void Init();

};

class AcceptEvent : public IOCPEvent
{

public:
	Session* session = nullptr;

public:
	AcceptEvent() : IOCPEvent(EventType::ACCEPT) {}

};

