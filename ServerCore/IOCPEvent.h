#pragma once

enum class EventType : u_char
{
	CONNECT,
	DISCONNECT,
	ACCEPT,
	RECV,
	SEND
};

class IOCPObj;
class Session;
class SendBuffer;

class IOCPEvent :public OVERLAPPED
{

public:
	EventType eventType;
	shared_ptr<IOCPObj> iocpObj;

public:
	IOCPEvent(EventType type);

public:
	void Init();

};

class ConnectEvent : public IOCPEvent
{
public:
	ConnectEvent() : IOCPEvent(EventType::CONNECT) {}

};

class AcceptEvent : public IOCPEvent
{

public:
	AcceptEvent() : IOCPEvent(EventType::ACCEPT) {}

public:
	shared_ptr<Session> session = nullptr;

};

class RecvEvent : public IOCPEvent
{
public:
	RecvEvent() : IOCPEvent(EventType::RECV) {}
};

class SendEvent : public IOCPEvent
{
public:
	SendEvent() : IOCPEvent(EventType::SEND) {}
public:
	vector<shared_ptr<SendBuffer>> buffers;
};

class DisconnectEvent : public IOCPEvent
{
public:
	DisconnectEvent() : IOCPEvent(EventType::DISCONNECT) {}
};