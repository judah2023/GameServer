#pragma once
#include "IOCPObj.h"

class Listener;
class Service;
class SendBuffer;
class RecvBuffer;

class ConnectEvent;
class RecvEvent;
class SendEvent;
class DisconnectEvent;

class Session : public IOCPObj
{
	friend Listener;

	enum { DATA_BUFSIZE = 0x10000 };

private:
	shared_mutex rwLock;
	atomic<bool> connected = false;
	shared_ptr<Service> service = nullptr;
	SOCKET socket = INVALID_SOCKET;
	SOCKADDR_IN sockAddr = {};

	shared_ptr<ConnectEvent> connectEvent = nullptr;
	shared_ptr<RecvEvent> recvEvent = nullptr;
	shared_ptr<SendEvent> sendEvent = nullptr;
	shared_ptr<DisconnectEvent> disconnectEvent = nullptr;

public:
	shared_ptr<RecvBuffer> recvBuffer;
	queue<shared_ptr<SendBuffer>> sendQueue;
	atomic<bool> sendRegistered = false;

public:
	Session();
	virtual  ~Session();

private:
	HANDLE GetHandle() override;
	void Dispatch(IOCPEvent* iocpEvent, int numOfBytes) override;

public:
	SOCKET GetSocket() { return socket; }
	bool IsConnected() { return connected; }
	shared_ptr<Service> GetService() { return service; }

	void SerService(shared_ptr<Service> inService) { service = inService; }
	void SetSockAddr(const SOCKADDR_IN address) { sockAddr = address; }
	shared_ptr<Session> GetSession() { return static_pointer_cast<Session>(shared_from_this()); }

private:
	bool RegisterConnect();
	void RegisterRecv();
	void RegisterSend();
	bool RegisterDisconnect();

private:
	void ProcessConnect();
	void ProcessRecv(int numOfBytes);
	void ProcessSend(int numOfBytes);
	void ProcessDisConnect();

private:
	void HandleError(int errorCode);

protected:
	virtual void OnConnected() {}
	virtual int OnRecv(BYTE* buffer, int len) { return len; }
	virtual void OnSend(int len) {}
	virtual void OnDisconnected() {}

public:
	bool Connect();
	void Send(shared_ptr<SendBuffer> sendBuffer);
	void Disconnect(const WCHAR* cause);

};

