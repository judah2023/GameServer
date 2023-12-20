#include "pch.h"
#include "Session.h"

#include "Service.h"
#include "SocketHelper.h"
#include "IOCPEvent.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"

Session::Session()
{
	socket = SocketHelper::CreateSocket();

	connectEvent = make_shared<ConnectEvent>();
	recvEvent = make_shared<RecvEvent>();
	sendEvent = make_shared<SendEvent>();
	disconnectEvent = make_shared<DisconnectEvent>();

	recvBuffer = make_shared<RecvBuffer>(DATA_BUFSIZE);
}

Session::~Session()
{
	SocketHelper::CloseSocket(socket);
}

HANDLE Session::GetHandle()
{
	return (HANDLE)socket;
}

void Session::Dispatch(IOCPEvent* iocpEvent, int numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::CONNECT:
		ProcessConnect();
		break;

	case EventType::RECV:
		ProcessRecv(numOfBytes);
		break;

	case EventType::SEND:
		ProcessSend(numOfBytes);
		break;

	case EventType::DISCONNECT:
		ProcessDisConnect();
		break;

	default:
		break;
	}

	//delete iocpEvent;
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;
	if (GetService()->GetType() != ServiceType::CLIENT)
		return false;
	if (!SocketHelper::SetReuseAddress(socket, true))
		return false;
	if (!SocketHelper::BindAnyAddress(socket, 0))
		return false;

	connectEvent->Init();
	connectEvent->iocpObj = shared_from_this();

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetSockAddr();
	if (SocketHelper::ConnectEx(socket, (SOCKADDR*)&sockAddr, sizeof(sockAddr), nullptr, 0, &numOfBytes, connectEvent.get()))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			HandleError(errorCode);
			connectEvent->iocpObj = nullptr;
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	if (!IsConnected())
		return;

	recvEvent->Init();
	recvEvent->iocpObj = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)recvBuffer->WritePos();;
	wsaBuf.len = recvBuffer->FreeSize();

	DWORD recvLen = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(socket, &wsaBuf, 1, &recvLen, &flags, recvEvent.get(), nullptr))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			recvEvent->iocpObj = nullptr;
		}
	}
	
}

void Session::RegisterSend()
{
	if (!IsConnected())
		return;

	sendEvent->Init();
	sendEvent->iocpObj = shared_from_this();

	{
		unique_lock<shared_mutex> lock(rwLock);

		int writeSize = 0;
		while (!sendQueue.empty())
		{
			shared_ptr<SendBuffer> sendBuffer = sendQueue.front(); sendQueue.pop();
			writeSize += sendBuffer->WriteSize();
			sendEvent->buffers.push_back(sendBuffer);
		}
	}

	vector<WSABUF> wsaBufs;
	wsaBufs.reserve(sendEvent->buffers.size());
	for (shared_ptr<SendBuffer> sendBuffer : sendEvent->buffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = (char*)sendBuffer->GetBuffer();
		wsaBuf.len = (ULONG)sendBuffer->WriteSize();
		wsaBufs.push_back(wsaBuf);
	}


	DWORD numOfBytes = 0;
	if (WSASend(socket, wsaBufs.data(), 1, &numOfBytes, 0, sendEvent.get(), nullptr) == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			sendEvent->iocpObj = nullptr;
			sendEvent->buffers.clear();
			sendRegistered.store(false);
		}
	}

}

bool Session::RegisterDisconnect()
{
	disconnectEvent->Init();
	disconnectEvent->iocpObj = shared_from_this();

	if (SocketHelper::DisconnectEx(socket, disconnectEvent.get(), TF_REUSE_SOCKET, 0))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			disconnectEvent->iocpObj = nullptr;
			return false;
		}
	}

	return true;
}

void Session::ProcessConnect()
{
	connectEvent->iocpObj = nullptr;

	connected.store(true);

	GetService()->AddSession(GetSession());

	OnConnected();

	RegisterRecv();

}

void Session::ProcessRecv(int numOfBytes)
{
	recvEvent->iocpObj = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0 bytes.");
		return;
	}

	if (!recvBuffer->OnWrite(numOfBytes))
	{
		Disconnect(L"OnWrite overflow.");
		return;
	}

	int dataSize = recvBuffer->DataSize();
	int processLen = OnRecv(recvBuffer->ReadPos(), numOfBytes);

	bool isVaildProcessLen = (0 <= processLen) && (processLen <= dataSize);
	if (!isVaildProcessLen || !recvBuffer->OnRead(processLen))
	{
		Disconnect(L"OnRead overflow");
		return;
	}

	recvBuffer->Clear();

	RegisterRecv();
}

void Session::ProcessSend( int numOfBytes)
{
	sendEvent->iocpObj = nullptr;
	sendEvent->buffers.clear();

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0 bytes");
	}

	OnSend(numOfBytes);

	unique_lock<shared_mutex> lock(rwLock);
	if (sendQueue.empty())
		sendRegistered.store(false);
	else
		RegisterSend();
}

void Session::ProcessDisConnect()
{
	disconnectEvent->iocpObj = nullptr;

	//ÀÌµ¿
	OnDisconnected();
	GetService()->RemoveSession(GetSession());
}

void Session::HandleError(int errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		printf("Handle Error\n");
		break;

	default:
		printf("ErroCode : %d\n", errorCode);
		break;
	}
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Send(shared_ptr<SendBuffer> sendBuffer)
{
	if (!IsConnected())
		return;

	bool registerSend = false;
	{
		unique_lock<shared_mutex> lock(rwLock);

		sendQueue.push(sendBuffer);
		if (sendRegistered.exchange(true) == false)
		{
			registerSend = true;
		}
	}

	if (registerSend)
	{
		RegisterSend();
	}
}

void Session::Disconnect(const WCHAR* cause)
{
	if (connected.exchange(false) == false)
		return;

	wprintf(L"Disconnect reason : %ls\n", cause);

	/*OnDisconnected();
	GetService()->RemoveSession(GetSession());*/
	RegisterDisconnect();
}
